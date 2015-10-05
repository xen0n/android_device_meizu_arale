/*
 * - 08/21/2007 ATAG support added by Uli Luckas <u.luckas@road.de>
 *
 */
#define _GNU_SOURCE
#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <unistd.h>
#include <arch/options.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libfdt.h>
#include "../../kexec.h"
#include "../../kexec-syscall.h"
#include "crashdump-arm.h"
#include "mach.h"

off_t initrd_base = 0, initrd_size = 0;

struct tag_header {
	uint32_t size;
	uint32_t tag;
};

/* The list must start with an ATAG_CORE node */
#define ATAG_CORE       0x54410001

struct tag_core {
	uint32_t flags;	    /* bit 0 = read-only */
	uint32_t pagesize;
	uint32_t rootdev;
};

/* it is allowed to have multiple ATAG_MEM nodes */
#define ATAG_MEM	0x54410002

struct tag_mem32 {
	uint32_t   size;
	uint32_t   start;  /* physical start address */
};

/* describes where the compressed ramdisk image lives (virtual address) */
/*
 * this one accidentally used virtual addresses - as such,
 * it's deprecated.
 */
#define ATAG_INITRD     0x54410005

/* describes where the compressed ramdisk image lives (physical address) */
#define ATAG_INITRD2    0x54420005

struct tag_initrd {
        uint32_t start;    /* physical start address */
        uint32_t size;     /* size of compressed ramdisk image in bytes */
};

/* command line: \0 terminated string */
#define ATAG_CMDLINE    0x54410009

struct tag_cmdline {
	char    cmdline[1];     /* this is the minimum size */
};

/* The list ends with an ATAG_NONE node. */
#define ATAG_NONE       0x00000000

struct tag {
	struct tag_header hdr;
	union {
		struct tag_core	 core;
		struct tag_mem32	mem;
		struct tag_initrd       initrd;
		struct tag_cmdline      cmdline;
	} u;
};

#define tag_next(t)     ((struct tag *)((uint32_t *)(t) + (t)->hdr.size))
#define byte_size(t)    ((t)->hdr.size << 2)
#define tag_size(type)  ((sizeof(struct tag_header) + sizeof(struct type) + 3) >> 2)

int zImage_arm_probe(const char *UNUSED(buf), off_t UNUSED(len))
{
	/*
	 * Only zImage loading is supported. Do not check if
	 * the buffer is valid kernel image
	 */
	return 0;
}

void zImage_arm_usage(void)
{
	printf(	"     --command-line=STRING Set the kernel command line to STRING.\n"
		"     --append=STRING       Set the kernel command line to STRING.\n"
		"     --initrd=FILE         Use FILE as the kernel's initial ramdisk.\n"
		"     --ramdisk=FILE        Use FILE as the kernel's initial ramdisk.\n"
		"     --dtb(=dtb.img)       Load dtb from zImage, dtb.img or /proc/device-tree instead of using atags.\n"
		"                           DTB appended to zImage and dtb.img currently only works on MSM devices.\n"
		"     --boardname=NAME      Required if using DTB. Options: m8 hammerhead bacon d851 shamu\n"
		"     --rd-addr=<addr>      Address to load initrd to.\n"
		"     --atags-addr=<addr>   Address to load atags/dtb to.\n"
		);
}

static
struct tag * atag_read_tags(void)
{
	unsigned long buf[1024];
	unsigned long *tags = NULL;
	ssize_t size = 0, read_b;
	const char fn[]= "/proc/atags";
	int fd = open(fn, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Cannot open %s: %s\n",
			fn, strerror(errno));
		return NULL;
	}

	do {
		read_b = read(fd, buf, sizeof(buf));
		if(read_b == -1) {
			fprintf(stderr, "Cannot read %s: %s\n", fn, strerror(errno));
			goto fail;
		}

		tags = realloc(tags, (size+read_b));
		memcpy(((char*)tags) + size, buf, read_b);
		size += read_b;
	} while(read_b != 0);

	if (size == 0) {
		fprintf(stderr, "Read 0 atags bytes: %s\n", fn);
		goto fail;
	}

	goto exit;
fail:
	free(tags);
	tags = NULL;
exit:
	close(fd);
	return (struct tag *) tags;
}

static
void tag_buf_add(struct tag *t, char **buf, size_t *size)
{
	*buf = xrealloc(*buf, (*size) + byte_size(t));
	memcpy((*buf) + (*size), t, byte_size(t));
	*size += byte_size(t);
}

static
uint32_t *tag_buf_find_initrd_start(struct tag *buf)
{
	for(; byte_size(buf); buf = tag_next(buf))
		if(buf->hdr.tag == ATAG_INITRD2)
			return &buf->u.initrd.start;
	return NULL;
}

static
int atag_arm_load(struct kexec_info *info, unsigned long base,
	const char *command_line, off_t command_line_len,
	const char *initrd, off_t initrd_len, off_t initrd_off)
{
	struct tag *saved_tags = atag_read_tags();
	char *buf = NULL;
	size_t buf_size = 0;
	struct tag *params, *tag;
	uint32_t *initrd_start = NULL;

	params = xmalloc(getpagesize());
	if (!params) {
		fprintf(stderr, "Compiling ATAGs: out of memory\n");
		free(saved_tags);
		return -1;
	}
	memset(params, 0xff, getpagesize());

	if (saved_tags) {
		// Copy tags
		tag = saved_tags;
		while(byte_size(tag)) {
			switch (tag->hdr.tag) {
			case ATAG_INITRD:
			case ATAG_INITRD2:
			case ATAG_CMDLINE:
			case ATAG_NONE:
				// skip these tags
				break;
			default:
				// copy all other tags
				tag_buf_add(tag, &buf, &buf_size);
				break;
			}
			tag = tag_next(tag);
		}
		free(saved_tags);
	} else {
		params->hdr.size = 2;
		params->hdr.tag = ATAG_CORE;
		tag_buf_add(params, &buf, &buf_size);
		memset(params, 0xff, byte_size(params));
	}

	if (initrd) {
		params->hdr.size = tag_size(tag_initrd);
		params->hdr.tag = ATAG_INITRD2;
		params->u.initrd.size = initrd_len;

		tag_buf_add(params, &buf, &buf_size);
		memset(params, 0xff, byte_size(params));
	}

	if (command_line) {
		params->hdr.size = (sizeof(struct tag_header) + command_line_len + 3) >> 2;
		params->hdr.tag = ATAG_CMDLINE;
		memcpy(params->u.cmdline.cmdline, command_line,
			command_line_len);
		params->u.cmdline.cmdline[command_line_len - 1] = '\0';

		tag_buf_add(params, &buf, &buf_size);
		memset(params, 0xff, byte_size(params));
	}

	params->hdr.size = 0;
	params->hdr.tag = ATAG_NONE;
	tag_buf_add(params, &buf, &buf_size);

	free(params);

	add_segment(info, buf, buf_size, base, buf_size);

	if (initrd) {
		initrd_start = tag_buf_find_initrd_start((struct tag *)buf);
		if(!initrd_start)
		{
			fprintf(stderr, "Failed to find initrd start!\n");
			return -1;
		}

		*initrd_start = locate_hole(info, initrd_len, getpagesize(),
				initrd_off, ULONG_MAX, INT_MAX);
		if (*initrd_start == ULONG_MAX)
			return -1;
		add_segment(info, initrd, initrd_len, *initrd_start, initrd_len);
	}

	return 0;
}

#define DTB_MAGIC               0xedfe0dd0
#define DTB_OFFSET              0x2C
#define DTB_PAD_SIZE            65536

static int get_appended_dtb(const char *kernel, off_t kernel_len, char **dtb_img, off_t *dtb_img_len)
{
	uint32_t app_dtb_offset = 0;
	char *kernel_end = (char*)kernel + kernel_len;
	char *dtb;

	memcpy((void*) &app_dtb_offset, (void*) (kernel + DTB_OFFSET), sizeof(uint32_t));

	dtb = (char*)kernel + app_dtb_offset;
	if(dtb >= kernel_end)
	{
		fprintf(stderr, "DTB: Failed to load dtb appended to zImage, invalid offset!");
		return 0;
	}

	*dtb_img = dtb;
	*dtb_img_len = kernel_end - dtb;
	return 1;
}

static int load_dtb_image(const char *path, char **dtb_img, off_t *dtb_img_len)
{
	int fd;
	struct stat info;
	char buff[4];
	char *img = NULL;
	off_t size;

	if(stat(path, &info) < 0)
	{
		fprintf(stderr, "DTB: Failed to stat() dtb image %s\n", path);
		return 0;
	}

	if(info.st_size <= 2048)
	{
		fprintf(stderr, "DTB: invalid dtb image (too small) %s\n", path);
		return 0;
	}

	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "DTB: Failed to open dtb image %s\n", path);
		return 0;
	}

	if(read(fd, buff, 4) != 4 || strncmp(buff, "QCDT", 4) != 0)
	{
		fprintf(stderr, "DTB: Invalid dtb image header in %s\n", path);
		close(fd);
		return 0;
	}

	// skip header
	size = info.st_size - 2048;
	lseek(fd, 2048, SEEK_SET);

	img = xmalloc(size);
	if(read(fd, img, size) != size)
	{
		fprintf(stderr, "Failed to read %lld bytes from dtb image %s\n", size, path);
		free(img);
		close(fd);
		return 0;
	}

	*dtb_img = img;
	*dtb_img_len = size;

	close(fd);
	return 1;
}

int zImage_arm_load(int argc, char **argv, const char *buf, off_t len,
	struct kexec_info *info)
{
	unsigned long base;
	unsigned int atag_offset = 0x1000; /* 4k offset from memory start */
	unsigned int offset = 0x8000;      /* 32k offset from memory start */
	unsigned int opt_ramdisk_addr;
	unsigned int opt_atags_addr;
	const char *command_line;
	char *modified_cmdline = NULL;
	off_t command_line_len;
	const char *ramdisk;
	char *ramdisk_buf;
	int opt;
	char *endptr;
	int use_dtb;
	const char *dtb_file;
	char *dtb_buf;
	off_t dtb_length;
	off_t dtb_offset;
	struct arm_mach *mach;

	/* See options.h -- add any more there, too. */
	static const struct option options[] = {
		KEXEC_ARCH_OPTIONS
		{ "command-line",	1, 0, OPT_APPEND },
		{ "append",		1, 0, OPT_APPEND },
		{ "initrd",		1, 0, OPT_RAMDISK },
		{ "ramdisk",		1, 0, OPT_RAMDISK },
		{ "dtb",		2, 0, OPT_DTB },
		{ "rd-addr",		1, 0, OPT_RD_ADDR },
		{ "atags-addr",		1, 0, OPT_ATAGS_ADDR },
		{ "boardname",  1, 0, OPT_BOARDNAME },
		{ 0, 			0, 0, 0 },
	};
	static const char short_options[] = KEXEC_ARCH_OPT_STR "a:r:d::i:g:b:";

	/*
	 * Parse the command line arguments
	 */
	command_line = 0;
	command_line_len = 0;
	ramdisk = 0;
	ramdisk_buf = 0;
	use_dtb = 0;
	dtb_file = NULL;
	opt_ramdisk_addr = 0;
	opt_atags_addr = 0;
	mach = NULL;
	while((opt = getopt_long(argc, argv, short_options, options, 0)) != -1) {
		switch(opt) {
		default:
			/* Ignore core options */
			if (opt < OPT_ARCH_MAX) {
				break;
			}
		case '?':
			usage();
			return -1;
		case OPT_APPEND:
			command_line = optarg;
			break;
		case OPT_RAMDISK:
			ramdisk = optarg;
			break;
		case OPT_DTB:
			use_dtb = 1;
			if(optarg)
				dtb_file = optarg;
			break;
		case OPT_RD_ADDR:
			opt_ramdisk_addr = strtoul(optarg, &endptr, 0);
			if (*endptr) {
				fprintf(stderr,
					"Bad option value in --rd-addr=%s\n",
					optarg);
				usage();
				return -1;
			}
			break;
		case OPT_ATAGS_ADDR:
			opt_atags_addr = strtoul(optarg, &endptr, 0);
			if (*endptr) {
				fprintf(stderr,
					"Bad option value in --atag-addr=%s\n",
					optarg);
				usage();
				return -1;
			}
			break;
		case OPT_BOARDNAME:
			mach = arm_mach_choose(optarg);
			if(!mach)
			{
				fprintf(stderr, "Unknown boardname '%s'!\n", optarg);
				return -1;
			}
			break;
		}
	}
	if (command_line) {
		command_line_len = strlen(command_line) + 1;
		if (command_line_len > COMMAND_LINE_SIZE)
			command_line_len = COMMAND_LINE_SIZE;
	}
	if (ramdisk) {
		ramdisk_buf = slurp_file(ramdisk, &initrd_size);
	}

	/*
	 * If we are loading a dump capture kernel, we need to update kernel
	 * command line and also add some additional segments.
	 */
	if (info->kexec_flags & KEXEC_ON_CRASH) {
		uint64_t start, end;

		modified_cmdline = xmalloc(COMMAND_LINE_SIZE);
		if (!modified_cmdline)
			return -1;

		if (command_line) {
			(void) strncpy(modified_cmdline, command_line,
				       COMMAND_LINE_SIZE);
			modified_cmdline[COMMAND_LINE_SIZE - 1] = '\0';
		}

		if (load_crashdump_segments(info, modified_cmdline) < 0) {
			free(modified_cmdline);
			return -1;
		}

		command_line = modified_cmdline;
		command_line_len = strlen(command_line) + 1;

		/*
		 * We put the dump capture kernel at the start of crashkernel
		 * reserved memory.
		 */
		if (parse_iomem_single("Crash kernel\n", &start, &end)) {
			/*
			 * No crash kernel memory reserved. We cannot do more
			 * but just bail out.
			 */
			return -1;
		}
		base = start;
	} else {
		base = locate_hole(info,len+offset,0,0,ULONG_MAX,INT_MAX);
	}

	if (base == ULONG_MAX)
		return -1;

	/* assume the maximum kernel compression ratio is 4,
	 * and just to be safe, place ramdisk after that
	 */
	if(opt_ramdisk_addr == 0)
		initrd_base = _ALIGN(base + len * 4, getpagesize());
	else
		initrd_base = opt_ramdisk_addr;

	if(!use_dtb)
	{
		if (atag_arm_load(info, base + atag_offset,
				command_line, command_line_len,
				ramdisk_buf, initrd_size, initrd_base) == -1)
			return -1;
	}
	else
	{
		char *dtb_img = NULL;
		off_t dtb_img_len = 0;
		int free_dtb_img = 0;
		int choose_res = 0;
		int ret, off;

		if(!mach)
		{
			fprintf(stderr, "DTB: --boardname was not specified.\n");
			return -1;
		}

		if(dtb_file)
		{
			if(!load_dtb_image(dtb_file, &dtb_img, &dtb_img_len))
				return -1;

			printf("DTB: Using DTB from file %s\n", dtb_file);
			free_dtb_img = 1;
		}
		else
		{
			if(!get_appended_dtb(buf, len, &dtb_img, &dtb_img_len))
				return -1;

			printf("DTB: Using DTB appended to zImage\n");
		}

		choose_res = (mach->choose_dtb)(dtb_img, dtb_img_len, &dtb_buf, &dtb_length);

		if(free_dtb_img)
			free(dtb_img);

		if(!choose_res)
		{
			fprintf(stderr, "Failed to load DTB!\n");
			return -1;
		}

		dtb_length = fdt_totalsize(dtb_buf) + DTB_PAD_SIZE;
		dtb_buf = xrealloc(dtb_buf, dtb_length);
		ret = fdt_open_into(dtb_buf, dtb_buf, dtb_length);
		if(ret)
			die("DTB: fdt_open_into failed");

		ret = (mach->add_extra_regs)(dtb_buf);
		if (ret < 0)
		{
			fprintf(stderr, "DTB: error while adding mach-specific extra regs\n");
			return -1;
		}

		if (command_line) {
			const char *node_name = "/chosen";
			const char *prop_name = "bootargs";

			/* check if a /choosen subnode already exists */
			off = fdt_path_offset(dtb_buf, node_name);

			if (off == -FDT_ERR_NOTFOUND)
				off = fdt_add_subnode(dtb_buf, off, node_name);

			if (off < 0) {
				fprintf(stderr, "DTB: Error adding %s node.\n", node_name);
				return -1;
			}

			if (fdt_setprop(dtb_buf, off, prop_name,
					command_line, strlen(command_line) + 1) != 0) {
				fprintf(stderr, "DTB: Error setting %s/%s property.\n",
					node_name, prop_name);
				return -1;
			}
		}

		if(ramdisk)
		{
			const char *node_name = "/chosen";
			uint32_t initrd_start, initrd_end;

			/* check if a /choosen subnode already exists */
			off = fdt_path_offset(dtb_buf, node_name);

			if (off == -FDT_ERR_NOTFOUND)
				off = fdt_add_subnode(dtb_buf, off, node_name);

			if (off < 0) {
				fprintf(stderr, "DTB: Error adding %s node.\n", node_name);
				return -1;
			}

			initrd_start = cpu_to_fdt32(initrd_base);
			initrd_end = cpu_to_fdt32(initrd_base + initrd_size);

			ret = fdt_setprop(dtb_buf, off, "linux,initrd-start", &initrd_start, sizeof(initrd_start));
			if (ret)
				die("DTB: Error setting %s/linux,initrd-start property.\n", node_name);

			ret = fdt_setprop(dtb_buf, off, "linux,initrd-end", &initrd_end, sizeof(initrd_end));
			if (ret)
				die("DTB: Error setting %s/linux,initrd-end property.\n", node_name);
		}

		fdt_pack(dtb_buf);

		if(ramdisk)
		{
			add_segment(info, ramdisk_buf, initrd_size, initrd_base,
				initrd_size);
		}

		if(opt_atags_addr != 0)
			dtb_offset = opt_atags_addr;
		else
		{
			dtb_offset = initrd_base + initrd_size + getpagesize();
			dtb_offset = _ALIGN_DOWN(dtb_offset, getpagesize());
		}

		printf("DTB: add dtb segment 0x%x\n", (unsigned int)dtb_offset);
		add_segment(info, dtb_buf, dtb_length,
		            dtb_offset, dtb_length);
	}

	add_segment(info, buf, len, base + offset, len);

	info->entry = (void*)base + offset;

	return 0;
}

