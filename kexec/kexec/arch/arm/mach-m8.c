#include <stdint.h>
#include <stdio.h>
#include <libfdt.h>

#include "../../kexec.h"
#include "mach.h"

#define INVALID_SOC_REV_ID 0xFFFFFFFF

struct htc_project_id
{
    uint32_t pid;
    uint32_t pcbid;
    uint32_t socver;
};

static uint32_t m8_dtb_compatible(void *dtb, struct htc_project_id *htcid_dev, struct htc_project_id *htcid_dtb, int ignore_pid)
{
    int root_offset;
    const void *prop;
    int len;

    root_offset = fdt_path_offset(dtb, "/");
    if (root_offset < 0)
    {
        fprintf(stderr, "DTB: Couldn't find root path in dtb!\n");
        return 0;
    }

    prop = fdt_getprop(dtb, root_offset, "htc,project-id", &len);
    if (!prop || len <= 0) {
        printf("DTB: htc,project-id entry not found\n");
        return 0;
    } else if (len < (int)sizeof(struct htc_project_id)) {
        printf("DTB: htc,project-id entry size mismatch (%d != %d)\n",
            len, sizeof(struct htc_project_id));
        return 0;
    }

    htcid_dtb->pid = fdt32_to_cpu(((const struct htc_project_id *)prop)->pid);
    htcid_dtb->pcbid = fdt32_to_cpu(((const struct htc_project_id *)prop)->pcbid);
    htcid_dtb->socver = fdt32_to_cpu(((const struct htc_project_id *)prop)->socver);

    printf("DTB: found dtb htc pid %u pcbid 0x%x soc version 0x%x\n",
            htcid_dtb->pid, htcid_dtb->pcbid, htcid_dtb->socver);

    if(!ignore_pid && htcid_dev->pid != htcid_dtb->pid)
        return 0;
    return 1;
}

static int m8_choose_dtb(const char *dtb_img, off_t dtb_len, char **dtb_buf, off_t *dtb_length)
{
    char *dtb = (char*)dtb_img;
    char *dtb_end = dtb + dtb_len;
    FILE *f;
    struct htc_project_id htcid_dev, htcid_dtb, htcid_best;
    char *bestmatch_tag = NULL;
    uint32_t bestmatch_tag_size = 0;
    int ignore_pid = 0;

    f = fopen("/proc/device-tree/htc,project-id", "r");
    if(!f)
    {
        fprintf(stderr, "DTB: Couldn't open /proc/device-tree/htc,project-id!\n");
        return 0;
    }

    fread(&htcid_dev, sizeof(struct htc_project_id), 1, f);
    fclose(f);

    htcid_dev.pid = fdt32_to_cpu(htcid_dev.pid);
    htcid_dev.pcbid = fdt32_to_cpu(htcid_dev.pcbid);
    htcid_dev.socver = fdt32_to_cpu(htcid_dev.socver);

    printf("DTB: htc pid %u pcbid 0x%x soc version 0x%x\n", htcid_dev.pid, htcid_dev.pcbid, htcid_dev.socver);

scan_dtb:
    dtb = (char*)dtb_img;
    bestmatch_tag = NULL;

    memset(&htcid_dtb, 0xFF, sizeof(struct htc_project_id));
    memset(&htcid_best, 0xFF, sizeof(struct htc_project_id));

    while(dtb + sizeof(struct fdt_header) < dtb_end)
    {
        uint32_t dtb_soc_rev_id;
        struct fdt_header dtb_hdr;
        uint32_t dtb_size;

        /* the DTB could be unaligned, so extract the header,
         * and operate on it separately */
        memcpy(&dtb_hdr, dtb, sizeof(struct fdt_header));
        if (fdt_check_header((const void *)&dtb_hdr) != 0 ||
            (dtb + fdt_totalsize((const void *)&dtb_hdr) > dtb_end))
        {
            fprintf(stderr, "DTB: Invalid dtb header!\n");
            break;
        }
        dtb_size = fdt_totalsize(&dtb_hdr);

        if(m8_dtb_compatible(dtb, &htcid_dev, &htcid_dtb, ignore_pid))
        {
            if (htcid_dev.pcbid == htcid_dtb.pcbid &&
                htcid_dev.socver == htcid_dtb.socver)
            {
                *dtb_buf = xmalloc(dtb_size);
                memcpy(*dtb_buf, dtb, dtb_size);
                *dtb_length = dtb_size;
                printf("DTB: using direct match %u 0x%x 0x%x, my %u 0x%x 0x%x, len %u\n",
                        htcid_dtb.pid, htcid_dtb.pcbid, htcid_dtb.socver,
                        htcid_dev.pid, htcid_dev.pcbid, htcid_dev.socver, dtb_size);
                return 1;
            }
            else if (htcid_dtb.pcbid <= htcid_dev.pcbid &&
                    htcid_dtb.socver < htcid_dev.socver)
            {
                if((htcid_best.pcbid == INVALID_SOC_REV_ID) ||
                    (htcid_best.pcbid < htcid_dtb.pcbid) ||
                    (htcid_best.pcbid == htcid_dtb.pcbid &&
                    htcid_best.socver < htcid_dtb.socver))
                {
                    bestmatch_tag = dtb;
                    bestmatch_tag_size = dtb_size;
                    memcpy(&htcid_best, &htcid_dtb, sizeof(struct htc_project_id));
                }
            }
        }

        /* goto the next device tree if any */
        dtb += dtb_size;

        // try to skip padding in standalone dtb.img files
        while(dtb < dtb_end && *dtb == 0)
            ++dtb;
    }

    if(bestmatch_tag) {
        printf("DTB: using bestmatch %u 0x%x 0x%x, my %u 0x%x 0x%x\n",
                htcid_best.pid, htcid_best.pcbid, htcid_best.socver,
                htcid_dev.pid, htcid_dev.pcbid, htcid_dev.socver);
        *dtb_buf = xmalloc(bestmatch_tag_size);
        memcpy(*dtb_buf, bestmatch_tag, bestmatch_tag_size);
        *dtb_length = bestmatch_tag_size;
        return 1;
    }

    // failed to find DTB for this device. Might be that current dtb has both
    // carrier and global models, in that case, remove their tag and try again.
    if(!ignore_pid)
    {
        printf("DTB: failed to match, try again, and ignore pid this time.\n");
        ignore_pid = 1;
        goto scan_dtb;
    }

    return 0;
}

static const char *chosenConfigProps[][6] = { "bootloaderflag", "kernelflag",
            "radioflag", "radioflag_ex2", "debugflag", "radioflag_ex1"};
static const char *calibrationProps[][13] = { "als_flash", "bs_flash", "bt_flash",
            "c-sensor", "cam_awb", "g-sensor", "gs_flash", "gyro_flash",
            "p-sensor", "ps_adi_flash", "ps_flash", "wifi_eeprom",
            "ws_flash"};
static const char *htc_workaround_reserve_leading_pagesProps[2] = { "compatible",
            "qcom,memblock-reserve"};
static int dtb_add_htc_projectid(void *dtb_buf, int off)
{
    FILE *f;
    uint32_t reg;
    int res;

    f = fopen("/proc/device-tree/htc,project-id", "r");
    if(!f)
    {
        fprintf(stderr, "DTB: Failed to open /proc/device-tree/htc,project-id!\n");
        return 0;
    }

    fdt_delprop(dtb_buf, off, "htc,project-id");

    while(fread(&reg, sizeof(reg), 1, f) == 1)
        fdt_appendprop(dtb_buf, off, "htc,project-id", &reg, sizeof(reg));

    fclose(f);
    return 1;
}

static int dtb_add_property(void *dtb_buf, int off, char *path, char *property)
{
    FILE *f;
    uint32_t reg;
    int res;

    char proppath[250];
    sprintf(proppath, "/proc/device-tree/%s/%s", path, property);

    f = fopen(proppath, "r");
    if(!f)
    {
        fprintf(stderr, "DTB: Failed to open %s!\n", proppath);
        return 0;
    }

    while(fread(&reg, sizeof(reg), 1, f) == 1)
        fdt_appendprop(dtb_buf, off, property, &reg, sizeof(reg));

    fclose(f);
    return 1;
}

static int dtb_add_properties_recursive(void *dtb_buf, int off, char *path, char **properties, int nrprops)
{
    int i, ret;

    for (i = 0; i < nrprops; i++) {
        ret = dtb_add_property(dtb_buf, off, path, properties[i]);
        if (!ret) {
            return ret;
        }
    }
    return 1;
}

static int dtb_add_htc_m8_specific(void *dtb_buf)
{
    int ret, off;
    char **configProperties = chosenConfigProps;

    printf("DTB: adding HTC M8 specific\n");

    // calibration_data
    printf("DTB: HTC M8: adding calibration data\n");
    ret = fdt_path_offset(dtb_buf, "/calibration_data");
    if (ret == -FDT_ERR_NOTFOUND) {
        ret = fdt_add_subnode(dtb_buf, 0, "/calibration_data");
    }
    if (ret < 0) {
        fprintf(stderr, "DTB: Error adding /calibration_data node.\n");
        return -1;
    }
    dtb_add_properties_recursive(dtb_buf, ret, "calibration_data", calibrationProps, 13);

    //chosen/config
    printf("DTB: HTC M8: adding chosen/config\n");
    ret = off = fdt_path_offset(dtb_buf, "/chosen");
    if (ret == -FDT_ERR_NOTFOUND) {
        ret = fdt_add_subnode(dtb_buf, ret, "/chosen");
    }

    if (ret < 0) {
        fprintf(stderr, "DTB: Error adding /chosen node.\n");
        return -1;
    }

    ret = fdt_path_offset(dtb_buf, "/chosen/config");
    if (ret == -FDT_ERR_NOTFOUND) {
        ret = fdt_add_subnode(dtb_buf, off, "config");
    }

    if (ret < 0) {
        fprintf(stderr, "DTB: Error adding /chosen/config node. %d\n", ret);
        return -1;
    }
    dtb_add_properties_recursive(dtb_buf, ret, "chosen/config", configProperties, 6);

    //htc projid
    printf("DTB: HTC M8: adding htc,project-id\n");
    ret = fdt_path_offset(dtb_buf, "/");
    dtb_add_htc_projectid(dtb_buf, ret);

    //htc_workaround_reserve_leading_pages
    printf("DTB: HTC M8: adding htc_workaround_reserve_leading_pages\n");
    ret = fdt_path_offset(dtb_buf, "/htc_workaround_reserve_leading_pages");
    if (ret == -FDT_ERR_NOTFOUND) {
        ret = fdt_add_subnode(dtb_buf, 0, "/htc_workaround_reserve_leading_pages");
    }

    if (ret < 0) {
        fprintf(stderr, "DTB: Error adding /htc_workaround_reserve_leading_pages node.\n");
        return -1;
    }
    dtb_add_properties_recursive(dtb_buf, ret, "htc_workaround_reserve_leading_pages",
        htc_workaround_reserve_leading_pagesProps, 2);

    return 0;
}

static int m8_add_extra_regs(void *dtb_buf)
{
    FILE *f;
    uint32_t reg;
    int res;
    int off;

    off = fdt_path_offset(dtb_buf, "/memory");
    if (off < 0)
    {
        fprintf(stderr, "DTB: Could not find memory node.\n");
        return -1;
    }

    f = fopen("/proc/device-tree/memory/reg", "r");
    if(!f)
    {
        fprintf(stderr, "DTB: Failed to open /proc/device-tree/memory/reg!\n");
        return -1;
    }

    fdt_delprop(dtb_buf, off, "reg");

    while(fread(&reg, sizeof(reg), 1, f) == 1)
        fdt_appendprop(dtb_buf, off, "reg", &reg, sizeof(reg));

    fclose(f);

    if(dtb_add_htc_m8_specific(dtb_buf) < 0)
    {
        fprintf(stderr, "DTB: Failed to add m8 specifics!\n");
        return -1;
    }
    return 0;
}

const struct arm_mach arm_mach_m8 = {
    .boardnames = { "m8", NULL },
    .choose_dtb = m8_choose_dtb,
    .add_extra_regs = m8_add_extra_regs,
};

