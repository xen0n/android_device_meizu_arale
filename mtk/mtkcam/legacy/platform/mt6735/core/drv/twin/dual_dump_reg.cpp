#include "MyUtils.h"

#if defined(_MSC_VER)           // VC
#define uart_printf printf
#define FOLDER_SYMBOL_STR "\\"
#define FOLDER_SYMBOL_CHAR '\\'
#define mtk_sprintf(dst_ptr, size_dst, format, ...)             sprintf_s(dst_ptr, size_dst, format, __VA_ARGS__)
#define mtk_fopen(file_ptr, filename_ptr, mode)                 fopen_s(&file_ptr, filename_ptr, mode)
#elif defined(linux) || defined(__linux) //Linux
#define FOLDER_SYMBOL_STR "/"
#define FOLDER_SYMBOL_CHAR '/'
#define mtk_sprintf(dst_ptr, size_dst, ...)                     sprintf(dst_ptr, __VA_ARGS__)
#define mtk_fopen(file_ptr, filename_ptr, mode)                 file_ptr = fopen(filename_ptr, mode)
#if defined(__arm__) //arm
#define NEW_LINE_CHAR   "\n"
#define LOG_TAG     "tpipe_reg_dump"
#if defined(USING_MTK_LDVT)     // LDVT
#pragma message("LDVT environment")
#include "uvvf.h"
#define uart_printf(fmt, arg...) VV_MSG(LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)
#else //not LDVT
#pragma message("FPGA/EVB/Phone environment")
#include <cutils/log.h>        // Android
#define uart_printf(fmt, arg...) ALOGD(LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)
#endif //END LDVT
#else //not arm
#define uart_printf printf
#endif //END platform
#endif //END VC
#define MTK_LOG_ENABLE 1
#define DUAL_ISP_REG_ADDR_START (0x15000000)
#define DUAL_REG_DUMP_HEADER "[ISP][DUAL_REG]"
#define DUAL_DUMP_VERSION "V.0.0.0.1"
#define DUAL_DUMP_START "start MT6593"
#define DUAL_DUMP_END "end MT6593"
#define MAX_COMMAND_LENGTH (512)

/* a: fprintf, b: start addr, c, hw start addr */
#define DUAL_CPP_CODE_PRINTF_REG(a, b, c, d, e, f) \
    a("%s %s = 0x%08X\r\n", DUAL_REG_DUMP_HEADER, #d, *(b + ((d - DUAL_ISP_REG_ADDR_START)>>2)));\

/* a: fprintf, b: start addr, c, hw start addr */
#define DUAL_CPP_CODE_FPRINTF_REG(a, b, c, d, e, f) \
    fprintf(a, "%s %s = 0x%08X\r\n", DUAL_REG_DUMP_HEADER, #d, *(b + ((d - DUAL_ISP_REG_ADDR_START)>>2)));\

/* a: target buffer ptr, b: src buffer ptr */
#define DUAL_CPP_IN_COPY(a, b, c, d, e, f) DUAL_CPP_IN_COPY_##f(a, b, c, d, e, f)
#define DUAL_CPP_IN_COPY_RW(a, b, c, d, e, f) DUAL_CPP_IN_COPY_R(a, b, c, d, e, f)
#define DUAL_CPP_IN_COPY_W(a, b, c, d, e, f)
#define DUAL_CPP_IN_COPY_R(a, b, c, d, e, f) (a)->e.Raw = (b)->e.Raw;

/* a: target buffer ptr, b: src buffer ptr */
#define DUAL_CPP_OUT_COPY(a, b, c, d, e, f) DUAL_CPP_OUT_COPY_##f(a, b, c, d, e, f)
#define DUAL_CPP_OUT_COPY_RW(a, b, c, d, e, f)  DUAL_CPP_OUT_COPY_W(a, b, c, d, e, f)
#define DUAL_CPP_OUT_COPY_R(a, b, c, d, e, f)
#define DUAL_CPP_OUT_COPY_W(a, b, c, d, e, f) (a)->e.Raw = (b)->e.Raw;

/* register addr, register name(optinal to fill) */
#define DUMP_ISP_REG_MAP(CMD, a, b, c) \
    /* Dump ISP registers - start, replaced with auto-code gen */\
    CMD(a, b, c, 0x15004004, CAM_CTL_EN_P1, R)\
    CMD(a, b, c, 0x15004010, CAM_CTL_EN_P1_D, RW)\
    CMD(a, b, c, 0x15004040, CAM_CTL_SEL_GLOBAL, R)\
    CMD(a, b, c, 0x15004418, CAM_TG_SEN_GRAB_PXL, R)\
    CMD(a, b, c, 0x1500441C, CAM_TG_SEN_GRAB_LIN, R)\
    CMD(a, b, c, 0x15004500, CAM_OBC_OFFST0, R)\
    CMD(a, b, c, 0x15004504, CAM_OBC_OFFST1, R)\
    CMD(a, b, c, 0x15004508, CAM_OBC_OFFST2, R)\
    CMD(a, b, c, 0x1500450C, CAM_OBC_OFFST3, R)\
    CMD(a, b, c, 0x15004510, CAM_OBC_GAIN0, R)\
    CMD(a, b, c, 0x15004514, CAM_OBC_GAIN1, R)\
    CMD(a, b, c, 0x15004518, CAM_OBC_GAIN2, R)\
    CMD(a, b, c, 0x1500451C, CAM_OBC_GAIN3, R)\
    CMD(a, b, c, 0x15004530, CAM_LSC_CTL1, R)\
    CMD(a, b, c, 0x15004534, CAM_LSC_CTL2, RW)\
    CMD(a, b, c, 0x15004538, CAM_LSC_CTL3, R)\
    CMD(a, b, c, 0x1500453C, CAM_LSC_LBLOCK, RW)\
    CMD(a, b, c, 0x15004540, CAM_LSC_RATIO, R)\
    CMD(a, b, c, 0x15004544, CAM_LSC_TPIPE_OFST, RW)\
    CMD(a, b, c, 0x15004548, CAM_LSC_TPIPE_SIZE, RW)\
    CMD(a, b, c, 0x1500454C, CAM_LSC_GAIN_TH, R)\
    CMD(a, b, c, 0x15004550, CAM_RPG_SATU_1, R)\
    CMD(a, b, c, 0x15004554, CAM_RPG_SATU_2, R)\
    CMD(a, b, c, 0x15004558, CAM_RPG_GAIN_1, R)\
    CMD(a, b, c, 0x1500455C, CAM_RPG_GAIN_2, R)\
    CMD(a, b, c, 0x15004560, CAM_RPG_OFST_1, R)\
    CMD(a, b, c, 0x15004564, CAM_RPG_OFST_2, R)\
    CMD(a, b, c, 0x150047A0, CAM_RRZ_CTL, R)\
    CMD(a, b, c, 0x150047A4, CAM_RRZ_IN_IMG, W)\
    CMD(a, b, c, 0x150047A8, CAM_RRZ_OUT_IMG, RW)\
    CMD(a, b, c, 0x150047AC, CAM_RRZ_HORI_STEP, RW)\
    CMD(a, b, c, 0x150047B0, CAM_RRZ_VERT_STEP, RW)\
    CMD(a, b, c, 0x150047B4, CAM_RRZ_HORI_INT_OFST, W)\
    CMD(a, b, c, 0x150047B8, CAM_RRZ_HORI_SUB_OFST, W)\
    CMD(a, b, c, 0x150047BC, CAM_RRZ_VERT_INT_OFST, R)\
    CMD(a, b, c, 0x150047C0, CAM_RRZ_VERT_SUB_OFST, R)\
    CMD(a, b, c, 0x150047C4, CAM_RRZ_MODE_TH, R)\
    CMD(a, b, c, 0x150047C8, CAM_RRZ_MODE_CTL, R)\
    CMD(a, b, c, 0x15004800, CAM_BPC_CON, R)\
    CMD(a, b, c, 0x15004804, CAM_BPC_TH1, R)\
    CMD(a, b, c, 0x15004808, CAM_BPC_TH2, R)\
    CMD(a, b, c, 0x1500480C, CAM_BPC_TH3, R)\
    CMD(a, b, c, 0x15004810, CAM_BPC_TH4, R)\
    CMD(a, b, c, 0x15004814, CAM_BPC_DTC, R)\
    CMD(a, b, c, 0x15004818, CAM_BPC_COR, R)\
    CMD(a, b, c, 0x1500481C, CAM_BPC_TBLI1, W)\
    CMD(a, b, c, 0x15004820, CAM_BPC_TBLI2, W)\
    CMD(a, b, c, 0x15004824, CAM_BPC_TH1_C, R)\
    CMD(a, b, c, 0x15004828, CAM_BPC_TH2_C, R)\
    CMD(a, b, c, 0x1500482C, CAM_BPC_TH3_C, R)\
    CMD(a, b, c, 0x15004830, CAM_BPC_RMM1, R)\
    CMD(a, b, c, 0x15004834, CAM_BPC_RMM2, R)\
    CMD(a, b, c, 0x15004838, CAM_BPC_RMM_REVG_1, R)\
    CMD(a, b, c, 0x1500483C, CAM_BPC_RMM_REVG_2, R)\
    CMD(a, b, c, 0x15004840, CAM_BPC_RMM_LEOS, R)\
    CMD(a, b, c, 0x15004844, CAM_BPC_RMM_GCNT, R)\
    CMD(a, b, c, 0x15004850, CAM_NR1_CON, R)\
    CMD(a, b, c, 0x15004854, CAM_NR1_CT_CON, R)\
    CMD(a, b, c, 0x15004E00, CAM_DMX_CTL, W)\
    CMD(a, b, c, 0x15004E04, CAM_DMX_CROP, W)\
    CMD(a, b, c, 0x15004E08, CAM_DMX_VSIZE, R)\
    CMD(a, b, c, 0x15004E10, CAM_BMX_CTL, W)\
    CMD(a, b, c, 0x15004E14, CAM_BMX_CROP, W)\
    CMD(a, b, c, 0x15004E18, CAM_BMX_VSIZE, W)\
    CMD(a, b, c, 0x15004E20, CAM_RMX_CTL, W)\
    CMD(a, b, c, 0x15004E24, CAM_RMX_CROP, W)\
    CMD(a, b, c, 0x15004E28, CAM_RMX_VSIZE, W)\
    CMD(a, b, c, 0x15006500, CAM_OBC_D_OFFST0, W)\
    CMD(a, b, c, 0x15006504, CAM_OBC_D_OFFST1, W)\
    CMD(a, b, c, 0x15006508, CAM_OBC_D_OFFST2, W)\
    CMD(a, b, c, 0x1500650C, CAM_OBC_D_OFFST3, W)\
    CMD(a, b, c, 0x15006510, CAM_OBC_D_GAIN0, W)\
    CMD(a, b, c, 0x15006514, CAM_OBC_D_GAIN1, W)\
    CMD(a, b, c, 0x15006518, CAM_OBC_D_GAIN2, W)\
    CMD(a, b, c, 0x1500651C, CAM_OBC_D_GAIN3, W)\
    CMD(a, b, c, 0x15006530, CAM_LSC_D_CTL1, RW)\
    CMD(a, b, c, 0x15006534, CAM_LSC_D_CTL2, W)\
    CMD(a, b, c, 0x15006538, CAM_LSC_D_CTL3, RW)\
    CMD(a, b, c, 0x1500653C, CAM_LSC_D_LBLOCK, W)\
    CMD(a, b, c, 0x15006540, CAM_LSC_D_RATIO, W)\
    CMD(a, b, c, 0x15006544, CAM_LSC_D_TPIPE_OFST, W)\
    CMD(a, b, c, 0x15006548, CAM_LSC_D_TPIPE_SIZE, W)\
    CMD(a, b, c, 0x1500654C, CAM_LSC_D_GAIN_TH, W)\
    CMD(a, b, c, 0x15006550, CAM_RPG_D_SATU_1, W)\
    CMD(a, b, c, 0x15006554, CAM_RPG_D_SATU_2, W)\
    CMD(a, b, c, 0x15006558, CAM_RPG_D_GAIN_1, W)\
    CMD(a, b, c, 0x1500655C, CAM_RPG_D_GAIN_2, W)\
    CMD(a, b, c, 0x15006560, CAM_RPG_D_OFST_1, W)\
    CMD(a, b, c, 0x15006564, CAM_RPG_D_OFST_2, W)\
    CMD(a, b, c, 0x150067A0, CAM_RRZ_D_CTL, W)\
    CMD(a, b, c, 0x150067A4, CAM_RRZ_D_IN_IMG, W)\
    CMD(a, b, c, 0x150067A8, CAM_RRZ_D_OUT_IMG, W)\
    CMD(a, b, c, 0x150067AC, CAM_RRZ_D_HORI_STEP, W)\
    CMD(a, b, c, 0x150067B0, CAM_RRZ_D_VERT_STEP, W)\
    CMD(a, b, c, 0x150067B4, CAM_RRZ_D_HORI_INT_OFST, W)\
    CMD(a, b, c, 0x150067B8, CAM_RRZ_D_HORI_SUB_OFST, W)\
    CMD(a, b, c, 0x150067BC, CAM_RRZ_D_VERT_INT_OFST, W)\
    CMD(a, b, c, 0x150067C0, CAM_RRZ_D_VERT_SUB_OFST, W)\
    CMD(a, b, c, 0x150067C4, CAM_RRZ_D_MODE_TH, W)\
    CMD(a, b, c, 0x150067C8, CAM_RRZ_D_MODE_CTL, W)\
    CMD(a, b, c, 0x15006800, CAM_BPC_D_CON, W)\
    CMD(a, b, c, 0x15006804, CAM_BPC_D_TH1, W)\
    CMD(a, b, c, 0x15006808, CAM_BPC_D_TH2, W)\
    CMD(a, b, c, 0x1500680C, CAM_BPC_D_TH3, W)\
    CMD(a, b, c, 0x15006810, CAM_BPC_D_TH4, W)\
    CMD(a, b, c, 0x15006814, CAM_BPC_D_DTC, W)\
    CMD(a, b, c, 0x15006818, CAM_BPC_D_COR, W)\
    CMD(a, b, c, 0x15007254, CAM_BPCI_OFST_ADDR, R)\
    CMD(a, b, c, 0x15007258, CAM_BPCI_XSIZE, R)\
    CMD(a, b, c, 0x1500725C, CAM_BPCI_YSIZE, R)\
    CMD(a, b, c, 0x15007260, CAM_BPCI_STRIDE, R)\
    CMD(a, b, c, 0x15007264, CAM_BPCI_CON, R)\
    CMD(a, b, c, 0x15007268, CAM_BPCI_CON2, R)\
    CMD(a, b, c, 0x150072A0, CAM_BPCI_D_OFST_ADDR, W)\
    CMD(a, b, c, 0x150072A4, CAM_BPCI_D_XSIZE, W)\
    CMD(a, b, c, 0x150072A8, CAM_BPCI_D_YSIZE, W)\
    CMD(a, b, c, 0x150072AC, CAM_BPCI_D_STRIDE, W)\
    CMD(a, b, c, 0x150072B0, CAM_BPCI_D_CON, W)\
    CMD(a, b, c, 0x150072B4, CAM_BPCI_D_CON2, W)\
    CMD(a, b, c, 0x1500681C, CAM_BPC_D_TBLI1, W)\
    CMD(a, b, c, 0x15006820, CAM_BPC_D_TBLI2, W)\
    CMD(a, b, c, 0x15006824, CAM_BPC_D_TH1_C, W)\
    CMD(a, b, c, 0x15006828, CAM_BPC_D_TH2_C, W)\
    CMD(a, b, c, 0x1500682C, CAM_BPC_D_TH3_C, W)\
    CMD(a, b, c, 0x15006830, CAM_BPC_D_RMM1, W)\
    CMD(a, b, c, 0x15006834, CAM_BPC_D_RMM2, W)\
    CMD(a, b, c, 0x15006838, CAM_BPC_D_RMM_REVG_1, W)\
    CMD(a, b, c, 0x1500683C, CAM_BPC_D_RMM_REVG_2, W)\
    CMD(a, b, c, 0x15006840, CAM_BPC_D_RMM_LEOS, W)\
    CMD(a, b, c, 0x15006844, CAM_BPC_D_RMM_GCNT, W)\
    CMD(a, b, c, 0x15006850, CAM_NR1_D_CON, W)\
    CMD(a, b, c, 0x15006854, CAM_NR1_D_CT_CON, W)\
    CMD(a, b, c, 0x15006E00, CAM_DMX_D_CTL, W)\
    CMD(a, b, c, 0x15006E04, CAM_DMX_D_CROP, W)\
    CMD(a, b, c, 0x15006E08, CAM_DMX_D_VSIZE, W)\
    CMD(a, b, c, 0x15006E10, CAM_BMX_D_CTL, W)\
    CMD(a, b, c, 0x15006E14, CAM_BMX_D_CROP, W)\
    CMD(a, b, c, 0x15006E18, CAM_BMX_D_VSIZE, W)\
    CMD(a, b, c, 0x15006E20, CAM_RMX_D_CTL, W)\
    CMD(a, b, c, 0x15006E24, CAM_RMX_D_CROP, W)\
    CMD(a, b, c, 0x15006E28, CAM_RMX_D_VSIZE, W)\
    CMD(a, b, c, 0x15007270, CAM_LSCI_OFST_ADDR, W)\
    CMD(a, b, c, 0x15007274, CAM_LSCI_XSIZE, RW)\
    CMD(a, b, c, 0x15007278, CAM_LSCI_YSIZE, R)\
    CMD(a, b, c, 0x1500727C, CAM_LSCI_STRIDE, R)\
    CMD(a, b, c, 0x15007280, CAM_LSCI_CON, R)\
    CMD(a, b, c, 0x15007284, CAM_LSCI_CON2, R)\
    CMD(a, b, c, 0x150074BC, CAM_LSCI_D_OFST_ADDR, W)\
    CMD(a, b, c, 0x150074C0, CAM_LSCI_D_XSIZE, W)\
    CMD(a, b, c, 0x150074C4, CAM_LSCI_D_YSIZE, W)\
    CMD(a, b, c, 0x150074C8, CAM_LSCI_D_STRIDE, W)\
    CMD(a, b, c, 0x150074CC, CAM_LSCI_D_CON, W)\
    CMD(a, b, c, 0x150074D0, CAM_LSCI_D_CON2, W)\
    CMD(a, b, c, 0x15004104, CAM_CTL_IHDR, R)\
    CMD(a, b, c, 0x15004108, CAM_CTL_IHDR_D, W)\
    /* Dump ISP registers - end */\
    /* Manually insert more ISP registers below */\

void dual_print_reg_dump(unsigned int dual_id, unsigned int *ptr_isp_reg)
{
    if (ptr_isp_reg)
    {
        uart_printf("%s %s %s\r\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
        uart_printf("%s DEBUG.DUAL_LOG_ID = 0x%08X\r\n", DUAL_REG_DUMP_HEADER, dual_id);
        DUMP_ISP_REG_MAP(DUAL_CPP_CODE_PRINTF_REG, uart_printf, ptr_isp_reg,);
        uart_printf("%s %s\r\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_END);
    }
}

void dual_copy_reg_to_input(ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, const isp_reg_t *ptr_isp_reg)
{
    if (ptr_in_param && ptr_isp_reg)
    {
        DUMP_ISP_REG_MAP(DUAL_CPP_IN_COPY, ptr_in_param, ptr_isp_reg,);
    }
}

void dual_copy_output_to_reg(isp_reg_t *ptr_isp_reg, const ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    if (ptr_out_param && ptr_isp_reg)
    {
        DUMP_ISP_REG_MAP(DUAL_CPP_OUT_COPY, ptr_isp_reg, ptr_out_param,);
    }
}

/* Following codes w/o copy */
void dual_fprint_reg_dump(unsigned int dual_id, unsigned int *ptr_isp_reg, const char *dir_path, const char *filename)
{
    if (ptr_isp_reg && dir_path && filename)
    {
        FILE *fpt_out=NULL;
        char full_name[MAX_COMMAND_LENGTH];
        mtk_sprintf(full_name, sizeof(full_name), "%s%s%s", dir_path, FOLDER_SYMBOL_STR, filename);
        mtk_fopen(fpt_out, full_name, "wb");
        if (fpt_out)
        {
            fprintf(fpt_out, "%s %s %s\r\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
            fprintf(fpt_out, "%s DEBUG.DUAL_LOG_ID = 0x%08X\r\n", DUAL_REG_DUMP_HEADER, dual_id);
            DUMP_ISP_REG_MAP(DUAL_CPP_CODE_FPRINTF_REG, fpt_out, ptr_isp_reg,);
            fprintf(fpt_out, "%s %s\r\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_END);
            fclose(fpt_out);
        }
        printf("\nOutput file: %s\n", full_name);
    }
}

void dual_copy_input_to_reg(isp_reg_t *ptr_isp_reg, const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param)
{
    if (ptr_in_param && ptr_isp_reg)
    {
        DUMP_ISP_REG_MAP(DUAL_CPP_IN_COPY, ptr_isp_reg, ptr_in_param,);
    }
}

