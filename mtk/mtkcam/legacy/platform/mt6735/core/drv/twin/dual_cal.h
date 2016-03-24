#ifndef __DUAL_CAM_H__
#define __DUAL_CAM_H__

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

//#define DUAL_PLATFORM_DRIVER /* to change define with platform unique */

#if defined(_MSC_VER)
#include <conio.h>
#define _kbhit() _kbhit()
#define mtk_stricmp                                             _stricmp
#define FOLDER_SYMBOL_STR                                       "\\"
#define FOLDER_SYMBOL_CHAR                                      '\\'
#define mtk_sprintf(dst_ptr, size_dst, format, ...)             sprintf_s(dst_ptr, size_dst, format, __VA_ARGS__)
#define mtk_fopen(file_ptr, filename_ptr, mode)                 fopen_s(&file_ptr, filename_ptr, mode)
#define mtk_fscanf                                              fscanf_s
#define MOVE_CMD                                                "move"
#define COPY_CMD                                                "copy"
#define CMP_CMD                                                 "fc"
#define DEL_CMD                                                 "del"
#define mtk_sscanf_1(x, y, a, b)                               sscanf_s(x, y, a, b)
#define mtk_sscanf_2(x, y, a, b, c, d)                            sscanf_s(x, y, a, b, c, d)
#define mtk_sscanf_8(...)                            sscanf_s(__VA_ARGS__)
#define DUAL_DEFAULT_PATH ".\\dual_conf"
#elif defined(linux) || defined(__linux) || defined(__arm__)
#define _kbhit() true
#define mtk_stricmp                                             strcasecmp
#define FOLDER_SYMBOL_STR                                       "/"
#define FOLDER_SYMBOL_CHAR                                      '/'
#define mtk_sprintf(dst_ptr, size_dst, ...)                     sprintf(dst_ptr, __VA_ARGS__)
#define mtk_fopen(file_ptr, filename_ptr, mode)                 file_ptr = fopen(filename_ptr, mode)
#define mtk_fscanf                                              fscanf
#define MOVE_CMD                                                "mv"
#define COPY_CMD                                                "cp"
#define CMP_CMD                                                 "cmp"
#define DEL_CMD                                                 "rm"
#define mtk_sscanf_1(x, y, a, b)                               sscanf(x, y, a)
#define mtk_sscanf_2(x, y, a, b, c, d)        sscanf(x, y, a, b)
#define mtk_sscanf_8(x, y, a, b, c, d, e, f, g, h,...)        sscanf(x, y, a, b, c, d, e, f, g, h)
#define O_BINARY (0)
#define DUAL_DEFAULT_PATH "./dual_conf"
#else
#error("Non-supported c compiler environment\n")
#endif

#ifdef DUAL_PLATFORM_DRIVER
#if defined(_MSC_VER)           // VC
#define dual_driver_printf printf_prefix
#define dual_driver_printf_no_prefix printf
#define uart_printf printf
#elif defined(linux) || defined(__linux) //Linux
#if defined(__arm__) //arm
#define dual_driver_printf(...)
#define dual_driver_printf_no_prefix(...)
#define NEW_LINE_CHAR   "\n"
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
#define dual_driver_printf printf_prefix
#define dual_driver_printf_no_prefix printf
#define uart_printf printf
#endif //END platform
#endif //END VC
#else
#define dual_driver_printf printf_prefix
#define dual_driver_printf_no_prefix printf
#endif

/* common define */
#ifndef __cplusplus
#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif

#define MAX_COMMAND_LENGTH (512)
#define MAX_FILENAME_LENGTH (128)

#define DUAL_KNOBS_CMP_FILENAME "dual_knobs_cmp.txt"
#define DUAL_ISP_REG_CMP_FILENAME "dual_isp_reg_cmp.txt"
#define DUAL_IN_CONFIG_FILENAME "dual_in_config.txt"
#define DUAL_REG_DUMP_FILENAME "dual_reg_dump.txt"

/* macro for debug print to file, #include <stdio.h> */
#define printf_prefix(...) {\
    const char *ptr_char = get_current_file_name(__FILE__);\
    printf("[%s][%s][%d] ", ptr_char,  __FUNCTION__, __LINE__);\
    printf(__VA_ARGS__);\
}
#define STR_SIZE (256)
#define  MAX_DUMP_COLUMN_LENGTH (512)
#define SPACE_EQUAL_SYMBOL_STR " = "

#define ERROR_MESSAGE_DATA(n, CMD) \
    CMD(n, ISP_MESSAGE_OK)\
    CMD(n, ISP_MESSAGE_FILE_OPEN_ERROR)\
    CMD(n, ISP_MESSAGE_NULL_PTR_ERROR)\
    CMD(n, ISP_MESSAGE_MISSING_READ_REG_ERROR)\
    CMD(n, ISP_MESSAGE_MISSING_WRITE_REG_ERROR)\
    CMD(n, ISP_MESSAGE_DUPLICATED_NAME_ERROR)\
    CMD(n, ISP_MESSAGE_DUAL_REG_CMP_ERROR)\
    CMD(n, ISP_MESSAGE_INVALID_CONFIG_ERROR)\
    CMD(n, ISP_MESSAGE_RRZ_SCALING_UP_ERROR)\
    CMD(n, ISP_MESSAGE_OUTPUT_FILE_COPY_ERROR)\
    CMD(n, ISP_MESSAGE_OUTPUT_FILE_CMP_ERROR)\
    CMD(n, ISP_MESSAGE_OUTPUT_FILE_DEL_ERROR)\
    CMD(n, ISP_MESSAGE_TWO_DIFF_PASS1_ERROR)\
    CMD(n, ISP_MESSAGE_DIFF_LSC_D_FILE_ERROR)\
    CMD(n, ISP_MESSAGE_READ_LSC_FILE_ERROR)\
    CMD(n, ISP_MESSAGE_RRZ_XE_CAL_ERROR)\
    CMD(n, ISP_MESSAGE_RRZ_D_XS_CAL_ERROR)\
    CMD(n, ISP_MESSAGE_RRZ_DISABLE_ERROR)\
    CMD(n, ISP_MESSAGE_HW_REG_DIFF_ERROR)\
    CMD(n, ISP_MESSAGE_HW_REG_ADDR_ERROR)\
    CMD(n, ISP_MESSAGE_RRZ_LEFT_SHIFT_ERROR)\
    CMD(n, ISP_MESSAGE_RRZ_RIGHT_SHIFT_ERROR)\
    CMD(n, ISP_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR)\

#define ISP_ENUM_STRING(n, a) if ((a) == (n)) return #a;
#define ISP_ENUM_DECLARE(n, a) a,

/* to prevent from directly calling macro */
#define GET_ERROR_NAME(n) \
    if (0 == (n)) return "ISP_MESSAGE_UNKNOWN";\
    ERROR_MESSAGE_DATA(n, ISP_ENUM_STRING)\
    return "";

/* error enum */
typedef enum ISP_MESSAGE_ENUM
{
    ISP_MESSAGE_UNKNOWN=0,
    ERROR_MESSAGE_DATA(,ISP_ENUM_DECLARE)
}ISP_MESSAGE_ENUM;

#define DUAL_RRZ_PREC (32768)
#define PREC_RRZ_LEFT_TILE_LOSS (4)
#define PREC_RRZ_RIGHT_TILE_LOSS (5)
/* tile loss */
#define TILE_LOSS_BNR (2)
#define TILE_LOSS_RRZ (8) /* cover DMX x4 but RRZ out not x4 */

/* PLATFORM parameters */
/* a, b, c, d, e reserved */
/* data type */
/* internal variable name */
/* knobs prefix 0x1-CFG:, 0x2-VAR:, 0x4-RAL: */
/* knobs fullname */
/* R/W type: R, RW, W */
/* driver variable name */
#define DUAL_DEBUG_LUT(CMD, a, b, c, d, e, f) \
    /* LSC */\
    CMD(a, b, c, d, e, f, int, DUAL_LOG_EN,, DEBUG.DUAL_LOG_EN, R, DEBUG, DUAL_LOG_EN)\
    CMD(a, b, c, d, e, f, int, DUAL_LOG_ID,, DEBUG.DUAL_LOG_ID, R, DEBUG, DUAL_LOG_ID)\
    CMD(a, b, c, d, e, f, int, TWIN_RRZ_IN_CROP_HT,, SW.TWIN_RRZ_IN_CROP_HT, R, SW, TWIN_RRZ_IN_CROP_HT)\
    CMD(a, b, c, d, e, f, int, TWIN_LSC_CROP_OFFX,, SW.TWIN_LSC_CROP_OFFX, R, SW, TWIN_LSC_CROP_OFFX)\
    CMD(a, b, c, d, e, f, int, TWIN_LSC_CROP_OFFY,, SW.TWIN_LSC_CROP_OFFY, R, SW, TWIN_LSC_CROP_OFFY)\

/* SW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* internal variable name */
/* knobs prefix 0x1-CFG:, 0x2-VAR:, 0x4-RAL: */
/* knobs fullname */
/* R/W type: R, RW, W */
/* driver variable name */
#define DUAL_SW_LUT(CMD, a, b, c, d, e, f) \
    /* LSC */\
    CMD(a, b, c, d, e, f, int, TWIN_SDBLK_XNUM_ALL, 0x2, TWIN_MODE_SDBLK_XNUM_ALL, R, SW, TWIN_MODE_SDBLK_XNUM_ALL)\
    CMD(a, b, c, d, e, f, int, TWIN_SDBLK_lWIDTH_ALL, 0x2, TWIN_MODE_SDBLK_lWIDTH_ALL, R, SW, TWIN_MODE_SDBLK_lWIDTH_ALL)\
    /* RRZ DZ */\
    CMD(a, b, c, d, e, f, int, TWIN_RRZ_HORI_INT_OFST, 0x2, TWIN_RRZ_HORI_INT_OFST, R, SW, TWIN_RRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, f, int, TWIN_RRZ_HORI_INT_OFST_LAST, 0x2, TWIN_RRZ_HORI_INT_OFST_LAST, R, SW, TWIN_RRZ_HORI_INT_OFST_LAST)\
    CMD(a, b, c, d, e, f, int, TWIN_RRZ_HORI_SUB_OFST, 0x2, TWIN_RRZ_HORI_SUB_OFST, R, SW, TWIN_RRZ_HORI_SUB_OFST)\
    CMD(a, b, c, d, e, f, int, TWIN_RRZ_OUT_WD, 0x2, TWIN_RRZ_OUT_WD, R, SW, TWIN_RRZ_OUT_WD)\

/* HW parameters */
/* a, b, c, d, e reserved */
/* data type */
/* internal variable name */
/* array bracket [xx] */
#define DUAL_HW_LUT(CMD, a, b, c, d, e, f) \
    /* Dual mode */\
    CMD(a, b, c, d, e, f, int, DUAL_MODE, 0x4, CAM_CTL_SEL_GLOBAL.DUAL_MODE, R, CAM_CTL_SEL_GLOBAL, DUAL_MODE)\
    CMD(a, b, c, d, e, f, int, PXL_S, 0x4, CAM_TG_SEN_GRAB_PXL.PXL_S, R, CAM_TG_SEN_GRAB_PXL, PXL_S)\
    CMD(a, b, c, d, e, f, int, PXL_E, 0x4, CAM_TG_SEN_GRAB_PXL.PXL_E, R, CAM_TG_SEN_GRAB_PXL, PXL_E)\
    CMD(a, b, c, d, e, f, int, LIN_S, 0x4, CAM_TG_SEN_GRAB_LIN.LIN_S, R, CAM_TG_SEN_GRAB_LIN, LIN_S)\
    CMD(a, b, c, d, e, f, int, LIN_E, 0x4, CAM_TG_SEN_GRAB_LIN.LIN_E, R, CAM_TG_SEN_GRAB_LIN, LIN_E)\
    /* DMX */\
    CMD(a, b, c, d, e, f, int, DMX_EN, 0x4, CAM_CTL_EN_P1.DMX_EN, R, CAM_CTL_EN_P1, DMX_EN)\
    CMD(a, b, c, d, e, f, int, DMX_EDGE, 0x4, CAM_DMX_CTL.DMX_EDGE, W, CAM_DMX_CTL, DMX_EDGE)\
    CMD(a, b, c, d, e, f, int, DMX_STR_X, 0x4, CAM_DMX_CROP.DMX_STR_X, W, CAM_DMX_CROP, DMX_STR_X)\
    CMD(a, b, c, d, e, f, int, DMX_END_X, 0x4, CAM_DMX_CROP.DMX_END_X, W, CAM_DMX_CROP, DMX_END_X)\
    CMD(a, b, c, d, e, f, int, DMX_HT, 0x4, CAM_DMX_VSIZE.DMX_HT, R, CAM_DMX_VSIZE, DMX_HT)\
    /* DMX_D */\
    CMD(a, b, c, d, e, f, int, DMX_D_EN, 0x4, CAM_CTL_EN_P1_D.DMX_D_EN, W, CAM_CTL_EN_P1_D, DMX_D_EN)\
    CMD(a, b, c, d, e, f, int, DMX_D_EDGE, 0x4, CAM_DMX_D_CTL.DMX_EDGE, W, CAM_DMX_D_CTL, DMX_EDGE)\
    CMD(a, b, c, d, e, f, int, DMX_D_STR_X, 0x4, CAM_DMX_D_CROP.DMX_STR_X, W, CAM_DMX_D_CROP, DMX_STR_X)\
    CMD(a, b, c, d, e, f, int, DMX_D_END_X, 0x4, CAM_DMX_D_CROP.DMX_END_X, W, CAM_DMX_D_CROP, DMX_END_X)\
    CMD(a, b, c, d, e, f, int, DMX_D_HT, 0x4, CAM_DMX_D_VSIZE.DMX_HT, W, CAM_DMX_D_VSIZE, DMX_HT)\
    /* OBC */\
    CMD(a, b, c, d, e, f, int, OB_EN, 0x4, CAM_CTL_EN_P1.OB_EN, R, CAM_CTL_EN_P1, OB_EN)\
    CMD(a, b, c, d, e, f, int, OBC_OFST_B, 0x4, CAM_OBC_OFFST0.OBC_OFST_B, R, CAM_OBC_OFFST0, OBC_OFST_B)\
    CMD(a, b, c, d, e, f, int, OBC_OFST_GR, 0x4, CAM_OBC_OFFST1.OBC_OFST_GR, R, CAM_OBC_OFFST1, OBC_OFST_GR)\
    CMD(a, b, c, d, e, f, int, OBC_OFST_GB, 0x4, CAM_OBC_OFFST2.OBC_OFST_GB, R, CAM_OBC_OFFST2, OBC_OFST_GB)\
    CMD(a, b, c, d, e, f, int, OBC_OFST_R, 0x4, CAM_OBC_OFFST3.OBC_OFST_R, R, CAM_OBC_OFFST3, OBC_OFST_R)\
    CMD(a, b, c, d, e, f, int, OBC_GAIN_B, 0x4, CAM_OBC_GAIN0.OBC_GAIN_B, R, CAM_OBC_GAIN0, OBC_GAIN_B)\
    CMD(a, b, c, d, e, f, int, OBC_GAIN_GR, 0x4, CAM_OBC_GAIN1.OBC_GAIN_GR, R, CAM_OBC_GAIN1, OBC_GAIN_GR)\
    CMD(a, b, c, d, e, f, int, OBC_GAIN_GB, 0x4, CAM_OBC_GAIN2.OBC_GAIN_GB, R, CAM_OBC_GAIN2, OBC_GAIN_GB)\
    CMD(a, b, c, d, e, f, int, OBC_GAIN_R, 0x4, CAM_OBC_GAIN3.OBC_GAIN_R, R, CAM_OBC_GAIN3, OBC_GAIN_R)\
    /* OBC_D */\
    CMD(a, b, c, d, e, f, int, OB_D_EN, 0x4, CAM_CTL_EN_P1_D.OB_D_EN, W, CAM_CTL_EN_P1_D, OB_D_EN)\
    CMD(a, b, c, d, e, f, int, OBC_D_OFST_B, 0x4, CAM_OBC_D_OFFST0.OBC_D_OFST_B, W, CAM_OBC_D_OFFST0, OBC_D_OFST_B)\
    CMD(a, b, c, d, e, f, int, OBC_D_OFST_GR, 0x4, CAM_OBC_D_OFFST1.OBC_D_OFST_GR, W, CAM_OBC_D_OFFST1, OBC_D_OFST_GR)\
    CMD(a, b, c, d, e, f, int, OBC_D_OFST_GB, 0x4, CAM_OBC_D_OFFST2.OBC_D_OFST_GB, W, CAM_OBC_D_OFFST2, OBC_D_OFST_GB)\
    CMD(a, b, c, d, e, f, int, OBC_D_OFST_R, 0x4, CAM_OBC_D_OFFST3.OBC_D_OFST_R, W, CAM_OBC_D_OFFST3, OBC_D_OFST_R)\
    CMD(a, b, c, d, e, f, int, OBC_D_GAIN_B, 0x4, CAM_OBC_D_GAIN0.OBC_D_GAIN_B, W, CAM_OBC_D_GAIN0, OBC_D_GAIN_B)\
    CMD(a, b, c, d, e, f, int, OBC_D_GAIN_GR, 0x4, CAM_OBC_D_GAIN1.OBC_D_GAIN_GR, W, CAM_OBC_D_GAIN1, OBC_D_GAIN_GR)\
    CMD(a, b, c, d, e, f, int, OBC_D_GAIN_GB, 0x4, CAM_OBC_D_GAIN2.OBC_D_GAIN_GB, W, CAM_OBC_D_GAIN2, OBC_D_GAIN_GB)\
    CMD(a, b, c, d, e, f, int, OBC_D_GAIN_R, 0x4, CAM_OBC_D_GAIN3.OBC_D_GAIN_R, W, CAM_OBC_D_GAIN3, OBC_D_GAIN_R)\
    /* BNR */\
    CMD(a, b, c, d, e, f, int, BNR_EN, 0x4, CAM_CTL_EN_P1.BNR_EN, R, CAM_CTL_EN_P1, BNR_EN)\
    CMD(a, b, c, d, e, f, int, BPC_EN, 0x4, CAM_BPC_CON.BPC_EN, R, CAM_BPC_CON, BPC_EN)\
    CMD(a, b, c, d, e, f, int, BPC_LUT_EN, 0x4, CAM_BPC_CON.BPC_LUT_EN, R, CAM_BPC_CON, BPC_LUT_EN)\
    CMD(a, b, c, d, e, f, int, BPC_TABLE_END_MODE, 0x4, CAM_BPC_CON.BPC_TABLE_END_MODE, R, CAM_BPC_CON, BPC_TABLE_END_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_DTC_MODE, 0x4, CAM_BPC_CON.BPC_DTC_MODE, R, CAM_BPC_CON, BPC_DTC_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_CS_MODE, 0x4, CAM_BPC_CON.BPC_CS_MODE, R, CAM_BPC_CON, BPC_CS_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_CRC_MODE, 0x4, CAM_BPC_CON.BPC_CRC_MODE, R, CAM_BPC_CON, BPC_CRC_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_TH_LWB, 0x4, CAM_BPC_TH1.BPC_TH_LWB, R, CAM_BPC_TH1, BPC_TH_LWB)\
    CMD(a, b, c, d, e, f, int, BPC_TH_Y, 0x4, CAM_BPC_TH1.BPC_TH_Y, R, CAM_BPC_TH1, BPC_TH_Y)\
    CMD(a, b, c, d, e, f, int, BPC_TH_UPB, 0x4, CAM_BPC_TH2.BPC_TH_UPB, R, CAM_BPC_TH2, BPC_TH_UPB)\
    CMD(a, b, c, d, e, f, int, BPC_TH_XA, 0x4, CAM_BPC_TH3.BPC_TH_XA, R, CAM_BPC_TH3, BPC_TH_XA)\
    CMD(a, b, c, d, e, f, int, BPC_TH_XB, 0x4, CAM_BPC_TH3.BPC_TH_XB, R, CAM_BPC_TH3, BPC_TH_XB)\
    CMD(a, b, c, d, e, f, int, BPC_TH_SLA, 0x4, CAM_BPC_TH3.BPC_TH_SLA, R, CAM_BPC_TH3, BPC_TH_SLA)\
    CMD(a, b, c, d, e, f, int, BPC_TH_SLB, 0x4, CAM_BPC_TH3.BPC_TH_SLB, R, CAM_BPC_TH3, BPC_TH_SLB)\
    CMD(a, b, c, d, e, f, int, BPC_DK_TH_XA, 0x4, CAM_BPC_TH4.BPC_DK_TH_XA, R, CAM_BPC_TH4, BPC_DK_TH_XA)\
    CMD(a, b, c, d, e, f, int, BPC_DK_TH_XB, 0x4, CAM_BPC_TH4.BPC_DK_TH_XB, R, CAM_BPC_TH4, BPC_DK_TH_XB)\
    CMD(a, b, c, d, e, f, int, BPC_DK_TH_SLA, 0x4, CAM_BPC_TH4.BPC_DK_TH_SLA, R, CAM_BPC_TH4, BPC_DK_TH_SLA)\
    CMD(a, b, c, d, e, f, int, BPC_DK_TH_SLB, 0x4, CAM_BPC_TH4.BPC_DK_TH_SLB, R, CAM_BPC_TH4, BPC_DK_TH_SLB)\
    CMD(a, b, c, d, e, f, int, BPC_RNG, 0x4, CAM_BPC_DTC.BPC_RNG, R, CAM_BPC_DTC, BPC_RNG)\
    CMD(a, b, c, d, e, f, int, BPC_CS_RNG, 0x4, CAM_BPC_DTC.BPC_CS_RNG, R, CAM_BPC_DTC, BPC_CS_RNG)\
    CMD(a, b, c, d, e, f, int, BPC_CT_LV, 0x4, CAM_BPC_DTC.BPC_CT_LV, R, CAM_BPC_DTC, BPC_CT_LV)\
    CMD(a, b, c, d, e, f, int, BPC_TH_MUL, 0x4, CAM_BPC_DTC.BPC_TH_MUL, R, CAM_BPC_DTC, BPC_TH_MUL)\
    CMD(a, b, c, d, e, f, int, BPC_NO_LV, 0x4, CAM_BPC_DTC.BPC_NO_LV, R, CAM_BPC_DTC, BPC_NO_LV)\
    CMD(a, b, c, d, e, f, int, BPC_DIR_MAX, 0x4, CAM_BPC_COR.BPC_DIR_MAX, R, CAM_BPC_COR, BPC_DIR_MAX)\
    CMD(a, b, c, d, e, f, int, BPC_DIR_TH, 0x4, CAM_BPC_COR.BPC_DIR_TH, R, CAM_BPC_COR, BPC_DIR_TH)\
    CMD(a, b, c, d, e, f, int, BPC_RANK_IDX, 0x4, CAM_BPC_COR.BPC_RANK_IDX, R, CAM_BPC_COR, BPC_RANK_IDX)\
    CMD(a, b, c, d, e, f, int, BPC_DIR_TH2, 0x4, CAM_BPC_COR.BPC_DIR_TH2, R, CAM_BPC_COR, BPC_DIR_TH2)\
    CMD(a, b, c, d, e, f, int, NR1_CT_EN, 0x4, CAM_NR1_CON.NR1_CT_EN, R, CAM_NR1_CON, NR1_CT_EN)\
    CMD(a, b, c, d, e, f, int, NR1_CT_MD, 0x4, CAM_NR1_CT_CON.NR1_CT_MD, R, CAM_NR1_CT_CON, NR1_CT_MD)\
    CMD(a, b, c, d, e, f, int, NR1_CT_MD2, 0x4, CAM_NR1_CT_CON.NR1_CT_MD2, R, CAM_NR1_CT_CON, NR1_CT_MD2)\
    CMD(a, b, c, d, e, f, int, NR1_CT_THRD, 0x4, CAM_NR1_CT_CON.NR1_CT_THRD, R, CAM_NR1_CT_CON, NR1_CT_THRD)\
    CMD(a, b, c, d, e, f, int, NR1_MBND, 0x4, CAM_NR1_CT_CON.NR1_MBND, R, CAM_NR1_CT_CON, NR1_MBND)\
    CMD(a, b, c, d, e, f, int, NR1_CT_SLOPE, 0x4, CAM_NR1_CT_CON.NR1_CT_SLOPE, R, CAM_NR1_CT_CON, NR1_CT_SLOPE)\
    CMD(a, b, c, d, e, f, int, NR1_CT_DIV, 0x4, CAM_NR1_CT_CON.NR1_CT_DIV, R, CAM_NR1_CT_CON, NR1_CT_DIV)\
    CMD(a, b, c, d, e, f, int, BPC_XOFFSET, 0x4, CAM_BPC_TBLI1.BPC_XOFFSET, W, CAM_BPC_TBLI1, BPC_XOFFSET)\
    CMD(a, b, c, d, e, f, int, BPC_YOFFSET, 0x4, CAM_BPC_TBLI1.BPC_YOFFSET, W, CAM_BPC_TBLI1, BPC_YOFFSET)\
    CMD(a, b, c, d, e, f, int, BPC_XSIZE, 0x4, CAM_BPC_TBLI2.BPC_XSIZE, W, CAM_BPC_TBLI2, BPC_XSIZE)\
    CMD(a, b, c, d, e, f, int, BPC_YSIZE, 0x4, CAM_BPC_TBLI2.BPC_YSIZE, W, CAM_BPC_TBLI2, BPC_YSIZE)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_OSC_TH, 0x4, CAM_BPC_RMM1.BPC_RMM_OSC_TH, R, CAM_BPC_RMM1, BPC_RMM_OSC_TH)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_SEDIR_SL, 0x4, CAM_BPC_RMM1.BPC_RMM_SEDIR_SL, R, CAM_BPC_RMM1, BPC_RMM_SEDIR_SL)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_SEBLD_WD, 0x4, CAM_BPC_RMM1.BPC_RMM_SEBLD_WD, R, CAM_BPC_RMM1, BPC_RMM_SEBLD_WD)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_LEBLD_WD, 0x4, CAM_BPC_RMM1.BPC_RMM_LEBLD_WD, R, CAM_BPC_RMM1, BPC_RMM_LEBLD_WD)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_MO_MODE, 0x4, CAM_BPC_RMM2.BPC_RMM_MO_MODE, R, CAM_BPC_RMM2, BPC_RMM_MO_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_MOBLD_FT, 0x4, CAM_BPC_RMM2.BPC_RMM_MOBLD_FT, R, CAM_BPC_RMM2, BPC_RMM_MOBLD_FT)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_MOTH_RATIO, 0x4, CAM_BPC_RMM2.BPC_RMM_MOTH_RATIO, R, CAM_BPC_RMM2, BPC_RMM_MOTH_RATIO)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_HORI_ADDWT, 0x4, CAM_BPC_RMM2.BPC_RMM_HORI_ADDWT, R, CAM_BPC_RMM2, BPC_RMM_HORI_ADDWT)\
    CMD(a, b, c, d, e, f, int, BPC_C_TH_LWB, 0x4, CAM_BPC_TH1_C.BPC_C_TH_LWB, R, CAM_BPC_TH1_C, BPC_C_TH_LWB)\
    CMD(a, b, c, d, e, f, int, BPC_C_TH_Y, 0x4, CAM_BPC_TH1_C.BPC_C_TH_Y, R, CAM_BPC_TH1_C, BPC_C_TH_Y)\
    CMD(a, b, c, d, e, f, int, BPC_C_TH_UPB, 0x4, CAM_BPC_TH2_C.BPC_C_TH_UPB, R, CAM_BPC_TH2_C, BPC_C_TH_UPB)\
    CMD(a, b, c, d, e, f, int, BPC_C_TH_XA, 0x4, CAM_BPC_TH3_C.BPC_C_TH_XA, R, CAM_BPC_TH3_C, BPC_C_TH_XA)\
    CMD(a, b, c, d, e, f, int, BPC_C_TH_XB, 0x4, CAM_BPC_TH3_C.BPC_C_TH_XB, R, CAM_BPC_TH3_C, BPC_C_TH_XB)\
    CMD(a, b, c, d, e, f, int, BPC_C_TH_SLA, 0x4, CAM_BPC_TH3_C.BPC_C_TH_SLA, R, CAM_BPC_TH3_C, BPC_C_TH_SLA)\
    CMD(a, b, c, d, e, f, int, BPC_C_TH_SLB, 0x4, CAM_BPC_TH3_C.BPC_C_TH_SLB, R, CAM_BPC_TH3_C, BPC_C_TH_SLB)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_REVG_R, 0x4, CAM_BPC_RMM_REVG_1.BPC_RMM_REVG_R, R, CAM_BPC_RMM_REVG_1, BPC_RMM_REVG_R)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_REVG_GR, 0x4, CAM_BPC_RMM_REVG_1.BPC_RMM_REVG_GR, R, CAM_BPC_RMM_REVG_1, BPC_RMM_REVG_GR)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_REVG_B, 0x4, CAM_BPC_RMM_REVG_2.BPC_RMM_REVG_B, R, CAM_BPC_RMM_REVG_2, BPC_RMM_REVG_B)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_REVG_GB, 0x4, CAM_BPC_RMM_REVG_2.BPC_RMM_REVG_GB, R, CAM_BPC_RMM_REVG_2, BPC_RMM_REVG_GB)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_LEOS_GRAY, 0x4, CAM_BPC_RMM_LEOS.BPC_RMM_LEOS_GRAY, R, CAM_BPC_RMM_LEOS, BPC_RMM_LEOS_GRAY)\
    CMD(a, b, c, d, e, f, int, BPC_RMM_GRAY_CNT, 0x4, CAM_BPC_RMM_GCNT.BPC_RMM_GRAY_CNT, R, CAM_BPC_RMM_GCNT, BPC_RMM_GRAY_CNT)\
    /* BNR_D */\
    CMD(a, b, c, d, e, f, int, BNR_D_EN, 0x4, CAM_CTL_EN_P1_D.BNR_D_EN, W, CAM_CTL_EN_P1_D, BNR_D_EN)\
    CMD(a, b, c, d, e, f, int, BPC_D_EN, 0x4, CAM_BPC_D_CON.BPC_EN, W, CAM_BPC_D_CON, BPC_EN)\
    CMD(a, b, c, d, e, f, int, BPC_D_LUT_EN, 0x4, CAM_BPC_D_CON.BPC_LUT_EN, W, CAM_BPC_D_CON, BPC_LUT_EN)\
    CMD(a, b, c, d, e, f, int, BPC_D_TABLE_END_MODE, 0x4, CAM_BPC_D_CON.BPC_TABLE_END_MODE, W, CAM_BPC_D_CON, BPC_TABLE_END_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_D_DTC_MODE, 0x4, CAM_BPC_D_CON.BPC_DTC_MODE, W, CAM_BPC_D_CON, BPC_DTC_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_D_CS_MODE, 0x4, CAM_BPC_D_CON.BPC_CS_MODE, W, CAM_BPC_D_CON, BPC_CS_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_D_CRC_MODE, 0x4, CAM_BPC_D_CON.BPC_CRC_MODE, W, CAM_BPC_D_CON, BPC_CRC_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_LWB, 0x4, CAM_BPC_D_TH1.BPC_TH_LWB, W, CAM_BPC_D_TH1, BPC_TH_LWB)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_Y, 0x4, CAM_BPC_D_TH1.BPC_TH_Y, W, CAM_BPC_D_TH1, BPC_TH_Y)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_UPB, 0x4, CAM_BPC_D_TH2.BPC_TH_UPB, W, CAM_BPC_D_TH2, BPC_TH_UPB)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_XA, 0x4, CAM_BPC_D_TH3.BPC_TH_XA, W, CAM_BPC_D_TH3, BPC_TH_XA)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_XB, 0x4, CAM_BPC_D_TH3.BPC_TH_XB, W, CAM_BPC_D_TH3, BPC_TH_XB)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_SLA, 0x4, CAM_BPC_D_TH3.BPC_TH_SLA, W, CAM_BPC_D_TH3, BPC_TH_SLA)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_SLB, 0x4, CAM_BPC_D_TH3.BPC_TH_SLB, W, CAM_BPC_D_TH3, BPC_TH_SLB)\
    CMD(a, b, c, d, e, f, int, BPC_D_DK_TH_XA, 0x4, CAM_BPC_D_TH4.BPC_DK_TH_XA, W, CAM_BPC_D_TH4, BPC_DK_TH_XA)\
    CMD(a, b, c, d, e, f, int, BPC_D_DK_TH_XB, 0x4, CAM_BPC_D_TH4.BPC_DK_TH_XB, W, CAM_BPC_D_TH4, BPC_DK_TH_XB)\
    CMD(a, b, c, d, e, f, int, BPC_D_DK_TH_SLA, 0x4, CAM_BPC_D_TH4.BPC_DK_TH_SLA, W, CAM_BPC_D_TH4, BPC_DK_TH_SLA)\
    CMD(a, b, c, d, e, f, int, BPC_D_DK_TH_SLB, 0x4, CAM_BPC_D_TH4.BPC_DK_TH_SLB, W, CAM_BPC_D_TH4, BPC_DK_TH_SLB)\
    CMD(a, b, c, d, e, f, int, BPC_D_RNG, 0x4, CAM_BPC_D_DTC.BPC_RNG, W, CAM_BPC_D_DTC, BPC_RNG)\
    CMD(a, b, c, d, e, f, int, BPC_D_CS_RNG, 0x4, CAM_BPC_D_DTC.BPC_CS_RNG, W, CAM_BPC_D_DTC, BPC_CS_RNG)\
    CMD(a, b, c, d, e, f, int, BPC_D_CT_LV, 0x4, CAM_BPC_D_DTC.BPC_CT_LV, W, CAM_BPC_D_DTC, BPC_CT_LV)\
    CMD(a, b, c, d, e, f, int, BPC_D_TH_MUL, 0x4, CAM_BPC_D_DTC.BPC_TH_MUL, W, CAM_BPC_D_DTC, BPC_TH_MUL)\
    CMD(a, b, c, d, e, f, int, BPC_D_NO_LV, 0x4, CAM_BPC_D_DTC.BPC_NO_LV, W, CAM_BPC_D_DTC, BPC_NO_LV)\
    CMD(a, b, c, d, e, f, int, BPC_D_DIR_MAX, 0x4, CAM_BPC_D_COR.BPC_DIR_MAX, W, CAM_BPC_D_COR, BPC_DIR_MAX)\
    CMD(a, b, c, d, e, f, int, BPC_D_DIR_TH, 0x4, CAM_BPC_D_COR.BPC_DIR_TH, W, CAM_BPC_D_COR, BPC_DIR_TH)\
    CMD(a, b, c, d, e, f, int, BPC_D_RANK_IDX, 0x4, CAM_BPC_D_COR.BPC_RANK_IDX, W, CAM_BPC_D_COR, BPC_RANK_IDX)\
    CMD(a, b, c, d, e, f, int, BPC_D_DIR_TH2, 0x4, CAM_BPC_D_COR.BPC_DIR_TH2, W, CAM_BPC_D_COR, BPC_DIR_TH2)\
    CMD(a, b, c, d, e, f, int, NR1_D_CT_EN, 0x4, CAM_NR1_D_CON.NR1_CT_EN, W, CAM_NR1_D_CON, NR1_CT_EN)\
    CMD(a, b, c, d, e, f, int, NR1_D_CT_MD, 0x4, CAM_NR1_D_CT_CON.NR1_CT_MD, W, CAM_NR1_D_CT_CON, NR1_CT_MD)\
    CMD(a, b, c, d, e, f, int, NR1_D_CT_MD2, 0x4, CAM_NR1_D_CT_CON.NR1_CT_MD2, W, CAM_NR1_D_CT_CON, NR1_CT_MD2)\
    CMD(a, b, c, d, e, f, int, NR1_D_CT_THRD, 0x4, CAM_NR1_D_CT_CON.NR1_CT_THRD, W, CAM_NR1_D_CT_CON, NR1_CT_THRD)\
    CMD(a, b, c, d, e, f, int, NR1_D_MBND, 0x4, CAM_NR1_D_CT_CON.NR1_MBND, W, CAM_NR1_D_CT_CON, NR1_MBND)\
    CMD(a, b, c, d, e, f, int, NR1_D_CT_SLOPE, 0x4, CAM_NR1_D_CT_CON.NR1_CT_SLOPE, W, CAM_NR1_D_CT_CON, NR1_CT_SLOPE)\
    CMD(a, b, c, d, e, f, int, NR1_D_CT_DIV, 0x4, CAM_NR1_D_CT_CON.NR1_CT_DIV, W, CAM_NR1_D_CT_CON, NR1_CT_DIV)\
    CMD(a, b, c, d, e, f, int, BPC_D_XOFFSET, 0x4, CAM_BPC_D_TBLI1.BPC_XOFFSET, W, CAM_BPC_D_TBLI1, BPC_XOFFSET)\
    CMD(a, b, c, d, e, f, int, BPC_D_YOFFSET, 0x4, CAM_BPC_D_TBLI1.BPC_YOFFSET, W, CAM_BPC_D_TBLI1, BPC_YOFFSET)\
    CMD(a, b, c, d, e, f, int, BPC_D_XSIZE, 0x4, CAM_BPC_D_TBLI2.BPC_XSIZE, W, CAM_BPC_D_TBLI2, BPC_XSIZE)\
    CMD(a, b, c, d, e, f, int, BPC_D_YSIZE, 0x4, CAM_BPC_D_TBLI2.BPC_YSIZE, W, CAM_BPC_D_TBLI2, BPC_YSIZE)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_OSC_TH, 0x4, CAM_BPC_D_RMM1.BPC_RMM_OSC_TH, W, CAM_BPC_D_RMM1, BPC_RMM_OSC_TH)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_SEDIR_SL, 0x4, CAM_BPC_D_RMM1.BPC_RMM_SEDIR_SL, W, CAM_BPC_D_RMM1, BPC_RMM_SEDIR_SL)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_SEBLD_WD, 0x4, CAM_BPC_D_RMM1.BPC_RMM_SEBLD_WD, W, CAM_BPC_D_RMM1, BPC_RMM_SEBLD_WD)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_LEBLD_WD, 0x4, CAM_BPC_D_RMM1.BPC_RMM_LEBLD_WD, W, CAM_BPC_D_RMM1, BPC_RMM_LEBLD_WD)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_MO_MODE, 0x4, CAM_BPC_D_RMM2.BPC_RMM_MO_MODE, W, CAM_BPC_D_RMM2, BPC_RMM_MO_MODE)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_MOBLD_FT, 0x4, CAM_BPC_D_RMM2.BPC_RMM_MOBLD_FT, W, CAM_BPC_D_RMM2, BPC_RMM_MOBLD_FT)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_MOTH_RATIO, 0x4, CAM_BPC_D_RMM2.BPC_RMM_MOTH_RATIO, W, CAM_BPC_D_RMM2, BPC_RMM_MOTH_RATIO)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_HORI_ADDWT, 0x4, CAM_BPC_D_RMM2.BPC_RMM_HORI_ADDWT, W, CAM_BPC_D_RMM2, BPC_RMM_HORI_ADDWT)\
    CMD(a, b, c, d, e, f, int, BPC_D_C_TH_LWB, 0x4, CAM_BPC_D_TH1_C.BPC_C_TH_LWB, W, CAM_BPC_D_TH1_C, BPC_C_TH_LWB)\
    CMD(a, b, c, d, e, f, int, BPC_D_C_TH_Y, 0x4, CAM_BPC_D_TH1_C.BPC_C_TH_Y, W, CAM_BPC_D_TH1_C, BPC_C_TH_Y)\
    CMD(a, b, c, d, e, f, int, BPC_D_C_TH_UPB, 0x4, CAM_BPC_D_TH2_C.BPC_C_TH_UPB, W, CAM_BPC_D_TH2_C, BPC_C_TH_UPB)\
    CMD(a, b, c, d, e, f, int, BPC_D_C_TH_XA, 0x4, CAM_BPC_D_TH3_C.BPC_C_TH_XA, W, CAM_BPC_D_TH3_C, BPC_C_TH_XA)\
    CMD(a, b, c, d, e, f, int, BPC_D_C_TH_XB, 0x4, CAM_BPC_D_TH3_C.BPC_C_TH_XB, W, CAM_BPC_D_TH3_C, BPC_C_TH_XB)\
    CMD(a, b, c, d, e, f, int, BPC_D_C_TH_SLA, 0x4, CAM_BPC_D_TH3_C.BPC_C_TH_SLA, W, CAM_BPC_D_TH3_C, BPC_C_TH_SLA)\
    CMD(a, b, c, d, e, f, int, BPC_D_C_TH_SLB, 0x4, CAM_BPC_D_TH3_C.BPC_C_TH_SLB, W, CAM_BPC_D_TH3_C, BPC_C_TH_SLB)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_REVG_R, 0x4, CAM_BPC_D_RMM_REVG_1.BPC_RMM_REVG_R, W, CAM_BPC_D_RMM_REVG_1, BPC_RMM_REVG_R)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_REVG_GR, 0x4, CAM_BPC_D_RMM_REVG_1.BPC_RMM_REVG_GR, W, CAM_BPC_D_RMM_REVG_1, BPC_RMM_REVG_GR)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_REVG_B, 0x4, CAM_BPC_D_RMM_REVG_2.BPC_RMM_REVG_B, W, CAM_BPC_D_RMM_REVG_2, BPC_RMM_REVG_B)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_REVG_GB, 0x4, CAM_BPC_D_RMM_REVG_2.BPC_RMM_REVG_GB, W, CAM_BPC_D_RMM_REVG_2, BPC_RMM_REVG_GB)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_LEOS_GRAY, 0x4, CAM_BPC_D_RMM_LEOS.BPC_RMM_LEOS_GRAY, W, CAM_BPC_D_RMM_LEOS, BPC_RMM_LEOS_GRAY)\
    CMD(a, b, c, d, e, f, int, BPC_D_RMM_GRAY_CNT, 0x4, CAM_BPC_D_RMM_GCNT.BPC_RMM_GRAY_CNT, W, CAM_BPC_D_RMM_GCNT, BPC_RMM_GRAY_CNT)\
    /* BPCI */\
    CMD(a, b, c, d, e, f, int, BPCI_OFFSET_ADDR, 0x4, CAM_BPCI_OFST_ADDR.OFFSET_ADDR, R, CAM_BPCI_OFST_ADDR, OFFSET_ADDR)\
    CMD(a, b, c, d, e, f, int, BPCI_XSIZE, 0x4, CAM_BPCI_XSIZE.XSIZE, R, CAM_BPCI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_YSIZE, 0x4, CAM_BPCI_YSIZE.YSIZE, R, CAM_BPCI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_STRIDE, 0x4, CAM_BPCI_STRIDE.STRIDE, R, CAM_BPCI_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, f, int, BPCI_BUS_SIZE, 0x4, CAM_BPCI_STRIDE.BUS_SIZE, R, CAM_BPCI_STRIDE, BUS_SIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_FORMAT, 0x4, CAM_BPCI_STRIDE.FORMAT, R, CAM_BPCI_STRIDE, FORMAT)\
    CMD(a, b, c, d, e, f, int, BPCI_FORMAT_EN, 0x4, CAM_BPCI_STRIDE.FORMAT_EN, R, CAM_BPCI_STRIDE, FORMAT_EN)\
    CMD(a, b, c, d, e, f, int, BPCI_BUS_SIZE_EN, 0x4, CAM_BPCI_STRIDE.BUS_SIZE_EN, R, CAM_BPCI_STRIDE, BUS_SIZE_EN)\
    CMD(a, b, c, d, e, f, int, BPCI_SWAP, 0x4, CAM_BPCI_STRIDE.SWAP, R, CAM_BPCI_STRIDE, SWAP)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_SIZE, 0x4, CAM_BPCI_CON.FIFO_SIZE, R, CAM_BPCI_CON, FIFO_SIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRI_THRL, 0x4, CAM_BPCI_CON.FIFO_PRI_THRL, R, CAM_BPCI_CON, FIFO_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRI_THRH, 0x4, CAM_BPCI_CON.FIFO_PRI_THRH, R, CAM_BPCI_CON, FIFO_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_SIZE_MSB, 0x4, CAM_BPCI_CON.FIFO_SIZE_MSB, R, CAM_BPCI_CON, FIFO_SIZE_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRI_THRL_MSB, 0x4, CAM_BPCI_CON.FIFO_PRI_THRL_MSB, R, CAM_BPCI_CON, FIFO_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRI_THRH_MSB, 0x4, CAM_BPCI_CON.FIFO_PRI_THRH_MSB, R, CAM_BPCI_CON, FIFO_PRI_THRH_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_MAX_BURST_LEN, 0x4, CAM_BPCI_CON.MAX_BURST_LEN, R, CAM_BPCI_CON, MAX_BURST_LEN)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRE_PRI_THRL, 0x4, CAM_BPCI_CON2.FIFO_PRE_PRI_THRL, R, CAM_BPCI_CON2, FIFO_PRE_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRE_PRI_THRH, 0x4, CAM_BPCI_CON2.FIFO_PRE_PRI_THRH, R, CAM_BPCI_CON2, FIFO_PRE_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRE_PRI_THRL_MSB, 0x4, CAM_BPCI_CON2.FIFO_PRE_PRI_THRL_MSB, R, CAM_BPCI_CON2, FIFO_PRE_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_FIFO_PRE_PRI_THRH_MSB, 0x4, CAM_BPCI_CON2.FIFO_PRE_PRI_THRH_MSB, R, CAM_BPCI_CON2, FIFO_PRE_PRI_THRH_MSB)\
    /* BPCI_D */\
    CMD(a, b, c, d, e, f, int, BPCI_D_OFFSET_ADDR, 0x4, CAM_BPCI_D_OFST_ADDR.OFFSET_ADDR, W, CAM_BPCI_D_OFST_ADDR, OFFSET_ADDR)\
    CMD(a, b, c, d, e, f, int, BPCI_D_XSIZE, 0x4, CAM_BPCI_D_XSIZE.XSIZE, W, CAM_BPCI_D_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_D_YSIZE, 0x4, CAM_BPCI_D_YSIZE.YSIZE, W, CAM_BPCI_D_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_D_STRIDE, 0x4, CAM_BPCI_D_STRIDE.STRIDE, W, CAM_BPCI_D_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, f, int, BPCI_D_BUS_SIZE, 0x4, CAM_BPCI_D_STRIDE.BUS_SIZE, W, CAM_BPCI_D_STRIDE, BUS_SIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FORMAT, 0x4, CAM_BPCI_D_STRIDE.FORMAT, W, CAM_BPCI_D_STRIDE, FORMAT)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FORMAT_EN, 0x4, CAM_BPCI_D_STRIDE.FORMAT_EN, W, CAM_BPCI_D_STRIDE, FORMAT_EN)\
    CMD(a, b, c, d, e, f, int, BPCI_D_BUS_SIZE_EN, 0x4, CAM_BPCI_D_STRIDE.BUS_SIZE_EN, W, CAM_BPCI_D_STRIDE, BUS_SIZE_EN)\
    CMD(a, b, c, d, e, f, int, BPCI_D_SWAP, 0x4, CAM_BPCI_D_STRIDE.SWAP, W, CAM_BPCI_D_STRIDE, SWAP)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_SIZE, 0x4, CAM_BPCI_D_CON.FIFO_SIZE, W, CAM_BPCI_D_CON, FIFO_SIZE)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRI_THRL, 0x4, CAM_BPCI_D_CON.FIFO_PRI_THRL, W, CAM_BPCI_D_CON, FIFO_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRI_THRH, 0x4, CAM_BPCI_D_CON.FIFO_PRI_THRH, W, CAM_BPCI_D_CON, FIFO_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_SIZE_MSB, 0x4, CAM_BPCI_D_CON.FIFO_SIZE_MSB, W, CAM_BPCI_D_CON, FIFO_SIZE_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRI_THRL_MSB, 0x4, CAM_BPCI_D_CON.FIFO_PRI_THRL_MSB, W, CAM_BPCI_D_CON, FIFO_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRI_THRH_MSB, 0x4, CAM_BPCI_D_CON.FIFO_PRI_THRH_MSB, W, CAM_BPCI_D_CON, FIFO_PRI_THRH_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_D_MAX_BURST_LEN, 0x4, CAM_BPCI_D_CON.MAX_BURST_LEN, W, CAM_BPCI_D_CON, MAX_BURST_LEN)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRE_PRI_THRL, 0x4, CAM_BPCI_D_CON2.FIFO_PRE_PRI_THRL, W, CAM_BPCI_D_CON2, FIFO_PRE_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRE_PRI_THRH, 0x4, CAM_BPCI_D_CON2.FIFO_PRE_PRI_THRH, W, CAM_BPCI_D_CON2, FIFO_PRE_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRE_PRI_THRL_MSB, 0x4, CAM_BPCI_D_CON2.FIFO_PRE_PRI_THRL_MSB, W, CAM_BPCI_D_CON2, FIFO_PRE_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, BPCI_D_FIFO_PRE_PRI_THRH_MSB, 0x4, CAM_BPCI_D_CON2.FIFO_PRE_PRI_THRH_MSB, W, CAM_BPCI_D_CON2, FIFO_PRE_PRI_THRH_MSB)\
    /* LSC */\
    CMD(a, b, c, d, e, f, int, LSC_EN, 0x4, CAM_CTL_EN_P1.LSC_EN, R, CAM_CTL_EN_P1, LSC_EN)\
    CMD(a, b, c, d, e, f, int, LSC_SDBLK_XNUM, 0x4, CAM_LSC_CTL2.LSC_SDBLK_XNUM, W, CAM_LSC_CTL2, LSC_SDBLK_XNUM)\
    CMD(a, b, c, d, e, f, int, LSC_SDBLK_WIDTH, 0x4, CAM_LSC_CTL2.LSC_SDBLK_WIDTH, R, CAM_LSC_CTL2, LSC_SDBLK_WIDTH)\
    CMD(a, b, c, d, e, f, int, LSC_SDBLK_lWIDTH, 0x4, CAM_LSC_LBLOCK.LSC_SDBLK_lWIDTH, W, CAM_LSC_LBLOCK, LSC_SDBLK_lWIDTH)\
    CMD(a, b, c, d, e, f, int, LSC_TPIPE_OFST_X, 0x4, CAM_LSC_TPIPE_OFST.LSC_TPIPE_OFST_X, W, CAM_LSC_TPIPE_OFST, LSC_TPIPE_OFST_X)\
    CMD(a, b, c, d, e, f, int, LSC_TPIPE_SIZE_X, 0x4, CAM_LSC_TPIPE_SIZE.LSC_TPIPE_SIZE_X, W, CAM_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_X)\
    CMD(a, b, c, d, e, f, int, SDBLK_XOFST, 0x4, CAM_LSC_CTL1.SDBLK_XOFST, R, CAM_LSC_CTL1, SDBLK_XOFST)\
    CMD(a, b, c, d, e, f, int, SDBLK_YOFST, 0x4, CAM_LSC_CTL1.SDBLK_YOFST, R, CAM_LSC_CTL1, SDBLK_YOFST)\
    CMD(a, b, c, d, e, f, int, SD_COEFRD_MODE, 0x4, CAM_LSC_CTL1.SD_COEFRD_MODE, R, CAM_LSC_CTL1, SD_COEFRD_MODE)\
    CMD(a, b, c, d, e, f, int, SD_ULTRA_MODE, 0x4, CAM_LSC_CTL1.SD_ULTRA_MODE, R, CAM_LSC_CTL1, SD_ULTRA_MODE)\
    CMD(a, b, c, d, e, f, int, LSC_OFLN, 0x4, CAM_LSC_CTL2.LSC_OFLN, R, CAM_LSC_CTL2, LSC_OFLN)\
    CMD(a, b, c, d, e, f, int, LSC_SDBLK_HEIGHT, 0x4, CAM_LSC_CTL3.LSC_SDBLK_HEIGHT, R, CAM_LSC_CTL3, LSC_SDBLK_HEIGHT)\
    CMD(a, b, c, d, e, f, int, LSC_SDBLK_YNUM, 0x4, CAM_LSC_CTL3.LSC_SDBLK_YNUM, R, CAM_LSC_CTL3, LSC_SDBLK_YNUM)\
    CMD(a, b, c, d, e, f, int, LSC_SDBLK_lHEIGHT, 0x4, CAM_LSC_LBLOCK.LSC_SDBLK_lHEIGHT, R, CAM_LSC_LBLOCK, LSC_SDBLK_lHEIGHT)\
    CMD(a, b, c, d, e, f, int, LSC_RA3, 0x4, CAM_LSC_RATIO.LSC_RA3, R, CAM_LSC_RATIO, LSC_RA3)\
    CMD(a, b, c, d, e, f, int, LSC_RA2, 0x4, CAM_LSC_RATIO.LSC_RA2, R, CAM_LSC_RATIO, LSC_RA2)\
    CMD(a, b, c, d, e, f, int, LSC_RA1, 0x4, CAM_LSC_RATIO.LSC_RA1, R, CAM_LSC_RATIO, LSC_RA1)\
    CMD(a, b, c, d, e, f, int, LSC_RA0, 0x4, CAM_LSC_RATIO.LSC_RA0, R, CAM_LSC_RATIO, LSC_RA0)\
    CMD(a, b, c, d, e, f, int, LSC_TPIPE_OFST_Y, 0x4, CAM_LSC_TPIPE_OFST.LSC_TPIPE_OFST_Y, R, CAM_LSC_TPIPE_OFST, LSC_TPIPE_OFST_Y)\
    CMD(a, b, c, d, e, f, int, LSC_TPIPE_SIZE_Y, 0x4, CAM_LSC_TPIPE_SIZE.LSC_TPIPE_SIZE_Y, R, CAM_LSC_TPIPE_SIZE, LSC_TPIPE_SIZE_Y)\
    CMD(a, b, c, d, e, f, int, LSC_GAIN_TH2, 0x4, CAM_LSC_GAIN_TH.LSC_GAIN_TH2, R, CAM_LSC_GAIN_TH, LSC_GAIN_TH2)\
    CMD(a, b, c, d, e, f, int, LSC_GAIN_TH1, 0x4, CAM_LSC_GAIN_TH.LSC_GAIN_TH1, R, CAM_LSC_GAIN_TH, LSC_GAIN_TH1)\
    CMD(a, b, c, d, e, f, int, LSC_GAIN_TH0, 0x4, CAM_LSC_GAIN_TH.LSC_GAIN_TH0, R, CAM_LSC_GAIN_TH, LSC_GAIN_TH0)\
    /* LSC_D */\
    CMD(a, b, c, d, e, f, int, LSC_D_EN, 0x4, CAM_CTL_EN_P1_D.LSC_D_EN, W, CAM_CTL_EN_P1_D, LSC_D_EN)\
    CMD(a, b, c, d, e, f, int, LSC_D_SDBLK_XNUM, 0x4, CAM_LSC_D_CTL2.LSC_D_SDBLK_XNUM, W, CAM_LSC_D_CTL2, LSC_D_SDBLK_XNUM)\
    CMD(a, b, c, d, e, f, int, LSC_D_SDBLK_WIDTH, 0x4, CAM_LSC_D_CTL2.LSC_D_SDBLK_WIDTH, W, CAM_LSC_D_CTL2, LSC_D_SDBLK_WIDTH)\
    CMD(a, b, c, d, e, f, int, LSC_D_SDBLK_lWIDTH, 0x4, CAM_LSC_D_LBLOCK.LSC_D_SDBLK_lWIDTH, W, CAM_LSC_D_LBLOCK, LSC_D_SDBLK_lWIDTH)\
    CMD(a, b, c, d, e, f, int, LSC_D_TPIPE_OFST_X, 0x4, CAM_LSC_D_TPIPE_OFST.LSC_D_TPIPE_OFST_X, W, CAM_LSC_D_TPIPE_OFST, LSC_D_TPIPE_OFST_X)\
    CMD(a, b, c, d, e, f, int, LSC_D_TPIPE_SIZE_X, 0x4, CAM_LSC_D_TPIPE_SIZE.LSC_D_TPIPE_SIZE_X, W, CAM_LSC_D_TPIPE_SIZE, LSC_D_TPIPE_SIZE_X)\
    CMD(a, b, c, d, e, f, int, SDBLK_D_XOFST, 0x4, CAM_LSC_D_CTL1.SDBLK_XOFST, W, CAM_LSC_D_CTL1, SDBLK_XOFST)\
    CMD(a, b, c, d, e, f, int, SDBLK_D_YOFST, 0x4, CAM_LSC_D_CTL1.SDBLK_YOFST, W, CAM_LSC_D_CTL1, SDBLK_YOFST)\
    CMD(a, b, c, d, e, f, int, SD_D_COEFRD_MODE, 0x4, CAM_LSC_D_CTL1.SD_COEFRD_MODE, W, CAM_LSC_D_CTL1, SD_COEFRD_MODE)\
    CMD(a, b, c, d, e, f, int, SD_D_ULTRA_MODE, 0x4, CAM_LSC_D_CTL1.SD_ULTRA_MODE, W, CAM_LSC_D_CTL1, SD_ULTRA_MODE)\
    CMD(a, b, c, d, e, f, int, LSC_D_OFLN, 0x4, CAM_LSC_D_CTL2.LSC_D_OFLN, W, CAM_LSC_D_CTL2, LSC_D_OFLN)\
    CMD(a, b, c, d, e, f, int, LSC_D_SDBLK_HEIGHT, 0x4, CAM_LSC_D_CTL3.LSC_D_SDBLK_HEIGHT, W, CAM_LSC_D_CTL3, LSC_D_SDBLK_HEIGHT)\
    CMD(a, b, c, d, e, f, int, LSC_D_SDBLK_YNUM, 0x4, CAM_LSC_D_CTL3.LSC_D_SDBLK_YNUM, W, CAM_LSC_D_CTL3, LSC_D_SDBLK_YNUM)\
    CMD(a, b, c, d, e, f, int, LSC_D_SDBLK_lHEIGHT, 0x4, CAM_LSC_D_LBLOCK.LSC_D_SDBLK_lHEIGHT, W, CAM_LSC_D_LBLOCK, LSC_D_SDBLK_lHEIGHT)\
    CMD(a, b, c, d, e, f, int, LSC_D_RA3, 0x4, CAM_LSC_D_RATIO.LSC_D_RA3, W, CAM_LSC_D_RATIO, LSC_D_RA3)\
    CMD(a, b, c, d, e, f, int, LSC_D_RA2, 0x4, CAM_LSC_D_RATIO.LSC_D_RA2, W, CAM_LSC_D_RATIO, LSC_D_RA2)\
    CMD(a, b, c, d, e, f, int, LSC_D_RA1, 0x4, CAM_LSC_D_RATIO.LSC_D_RA1, W, CAM_LSC_D_RATIO, LSC_D_RA1)\
    CMD(a, b, c, d, e, f, int, LSC_D_RA0, 0x4, CAM_LSC_D_RATIO.LSC_D_RA0, W, CAM_LSC_D_RATIO, LSC_D_RA0)\
    CMD(a, b, c, d, e, f, int, LSC_D_TPIPE_OFST_Y, 0x4, CAM_LSC_D_TPIPE_OFST.LSC_D_TPIPE_OFST_Y, W, CAM_LSC_D_TPIPE_OFST, LSC_D_TPIPE_OFST_Y)\
    CMD(a, b, c, d, e, f, int, LSC_D_TPIPE_SIZE_Y, 0x4, CAM_LSC_D_TPIPE_SIZE.LSC_D_TPIPE_SIZE_Y, W, CAM_LSC_D_TPIPE_SIZE, LSC_D_TPIPE_SIZE_Y)\
    CMD(a, b, c, d, e, f, int, LSC_D_GAIN_TH2, 0x4, CAM_LSC_D_GAIN_TH.LSC_D_GAIN_TH2, W, CAM_LSC_D_GAIN_TH, LSC_D_GAIN_TH2)\
    CMD(a, b, c, d, e, f, int, LSC_D_GAIN_TH1, 0x4, CAM_LSC_D_GAIN_TH.LSC_D_GAIN_TH1, W, CAM_LSC_D_GAIN_TH, LSC_D_GAIN_TH1)\
    CMD(a, b, c, d, e, f, int, LSC_D_GAIN_TH0, 0x4, CAM_LSC_D_GAIN_TH.LSC_D_GAIN_TH0, W, CAM_LSC_D_GAIN_TH, LSC_D_GAIN_TH0)\
    /* LSCI */\
    CMD(a, b, c, d, e, f, int, LSCI_OFFSET_ADDR, 0x4, CAM_LSCI_OFST_ADDR.OFFSET_ADDR, W, CAM_LSCI_OFST_ADDR, OFFSET_ADDR)\
    CMD(a, b, c, d, e, f, int, LSCI_XSIZE, 0x4, CAM_LSCI_XSIZE.XSIZE, RW, CAM_LSCI_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_YSIZE, 0x4, CAM_LSCI_YSIZE.YSIZE, R, CAM_LSCI_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_STRIDE, 0x4, CAM_LSCI_STRIDE.STRIDE, R, CAM_LSCI_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, f, int, LSCI_BUS_SIZE, 0x4, CAM_LSCI_STRIDE.BUS_SIZE, R, CAM_LSCI_STRIDE, BUS_SIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_FORMAT, 0x4, CAM_LSCI_STRIDE.FORMAT, R, CAM_LSCI_STRIDE, FORMAT)\
    CMD(a, b, c, d, e, f, int, LSCI_FORMAT_EN, 0x4, CAM_LSCI_STRIDE.FORMAT_EN, R, CAM_LSCI_STRIDE, FORMAT_EN)\
    CMD(a, b, c, d, e, f, int, LSCI_BUS_SIZE_EN, 0x4, CAM_LSCI_STRIDE.BUS_SIZE_EN, R, CAM_LSCI_STRIDE, BUS_SIZE_EN)\
    CMD(a, b, c, d, e, f, int, LSCI_SWAP, 0x4, CAM_LSCI_STRIDE.SWAP, R, CAM_LSCI_STRIDE, SWAP)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_SIZE, 0x4, CAM_LSCI_CON.FIFO_SIZE, R, CAM_LSCI_CON, FIFO_SIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRI_THRL, 0x4, CAM_LSCI_CON.FIFO_PRI_THRL, R, CAM_LSCI_CON, FIFO_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRI_THRH, 0x4, CAM_LSCI_CON.FIFO_PRI_THRH, R, CAM_LSCI_CON, FIFO_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_SIZE_MSB, 0x4, CAM_LSCI_CON.FIFO_SIZE_MSB, R, CAM_LSCI_CON, FIFO_SIZE_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRI_THRL_MSB, 0x4, CAM_LSCI_CON.FIFO_PRI_THRL_MSB, R, CAM_LSCI_CON, FIFO_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRI_THRH_MSB, 0x4, CAM_LSCI_CON.FIFO_PRI_THRH_MSB, R, CAM_LSCI_CON, FIFO_PRI_THRH_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_MAX_BURST_LEN, 0x4, CAM_LSCI_CON.MAX_BURST_LEN, R, CAM_LSCI_CON, MAX_BURST_LEN)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRE_PRI_THRL, 0x4, CAM_LSCI_CON2.FIFO_PRE_PRI_THRL, R, CAM_LSCI_CON2, FIFO_PRE_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRE_PRI_THRH, 0x4, CAM_LSCI_CON2.FIFO_PRE_PRI_THRH, R, CAM_LSCI_CON2, FIFO_PRE_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRE_PRI_THRL_MSB, 0x4, CAM_LSCI_CON2.FIFO_PRE_PRI_THRL_MSB, R, CAM_LSCI_CON2, FIFO_PRE_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_FIFO_PRE_PRI_THRH_MSB, 0x4, CAM_LSCI_CON2.FIFO_PRE_PRI_THRH_MSB, R, CAM_LSCI_CON2, FIFO_PRE_PRI_THRH_MSB)\
    /* LSCI_D */\
    CMD(a, b, c, d, e, f, int, LSCI_D_OFFSET_ADDR, 0x4, CAM_LSCI_D_OFST_ADDR.OFFSET_ADDR, W, CAM_LSCI_D_OFST_ADDR, OFFSET_ADDR)\
    CMD(a, b, c, d, e, f, int, LSCI_D_XSIZE, 0x4, CAM_LSCI_D_XSIZE.XSIZE, W, CAM_LSCI_D_XSIZE, XSIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_D_YSIZE, 0x4, CAM_LSCI_D_YSIZE.YSIZE, W, CAM_LSCI_D_YSIZE, YSIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_D_STRIDE, 0x4, CAM_LSCI_D_STRIDE.STRIDE, W, CAM_LSCI_D_STRIDE, STRIDE)\
    CMD(a, b, c, d, e, f, int, LSCI_D_BUS_SIZE, 0x4, CAM_LSCI_D_STRIDE.BUS_SIZE, W, CAM_LSCI_D_STRIDE, BUS_SIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FORMAT, 0x4, CAM_LSCI_D_STRIDE.FORMAT, W, CAM_LSCI_D_STRIDE, FORMAT)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FORMAT_EN, 0x4, CAM_LSCI_D_STRIDE.FORMAT_EN, W, CAM_LSCI_D_STRIDE, FORMAT_EN)\
    CMD(a, b, c, d, e, f, int, LSCI_D_BUS_SIZE_EN, 0x4, CAM_LSCI_D_STRIDE.BUS_SIZE_EN, W, CAM_LSCI_D_STRIDE, BUS_SIZE_EN)\
    CMD(a, b, c, d, e, f, int, LSCI_D_SWAP, 0x4, CAM_LSCI_D_STRIDE.SWAP, W, CAM_LSCI_D_STRIDE, SWAP)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_SIZE, 0x4, CAM_LSCI_D_CON.FIFO_SIZE, W, CAM_LSCI_D_CON, FIFO_SIZE)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRI_THRL, 0x4, CAM_LSCI_D_CON.FIFO_PRI_THRL, W, CAM_LSCI_D_CON, FIFO_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRI_THRH, 0x4, CAM_LSCI_D_CON.FIFO_PRI_THRH, W, CAM_LSCI_D_CON, FIFO_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_SIZE_MSB, 0x4, CAM_LSCI_D_CON.FIFO_SIZE_MSB, W, CAM_LSCI_D_CON, FIFO_SIZE_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRI_THRL_MSB, 0x4, CAM_LSCI_D_CON.FIFO_PRI_THRL_MSB, W, CAM_LSCI_D_CON, FIFO_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRI_THRH_MSB, 0x4, CAM_LSCI_D_CON.FIFO_PRI_THRH_MSB, W, CAM_LSCI_D_CON, FIFO_PRI_THRH_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_D_MAX_BURST_LEN, 0x4, CAM_LSCI_D_CON.MAX_BURST_LEN, W, CAM_LSCI_D_CON, MAX_BURST_LEN)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRE_PRI_THRL, 0x4, CAM_LSCI_D_CON2.FIFO_PRE_PRI_THRL, W, CAM_LSCI_D_CON2, FIFO_PRE_PRI_THRL)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRE_PRI_THRH, 0x4, CAM_LSCI_D_CON2.FIFO_PRE_PRI_THRH, W, CAM_LSCI_D_CON2, FIFO_PRE_PRI_THRH)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRE_PRI_THRL_MSB, 0x4, CAM_LSCI_D_CON2.FIFO_PRE_PRI_THRL_MSB, W, CAM_LSCI_D_CON2, FIFO_PRE_PRI_THRL_MSB)\
    CMD(a, b, c, d, e, f, int, LSCI_D_FIFO_PRE_PRI_THRH_MSB, 0x4, CAM_LSCI_D_CON2.FIFO_PRE_PRI_THRH_MSB, W, CAM_LSCI_D_CON2, FIFO_PRE_PRI_THRH_MSB)\
    /* BMX */\
    CMD(a, b, c, d, e, f, int, BMX_EN, 0x4, CAM_CTL_EN_P1.BMX_EN, R, CAM_CTL_EN_P1, BMX_EN)\
    CMD(a, b, c, d, e, f, int, BMX_EDGE, 0x4, CAM_BMX_CTL.BMX_EDGE, W, CAM_BMX_CTL, BMX_EDGE)\
    CMD(a, b, c, d, e, f, int, BMX_STR_X, 0x4, CAM_BMX_CROP.BMX_STR_X, W, CAM_BMX_CROP, BMX_STR_X)\
    CMD(a, b, c, d, e, f, int, BMX_END_X, 0x4, CAM_BMX_CROP.BMX_END_X, W, CAM_BMX_CROP, BMX_END_X)\
    CMD(a, b, c, d, e, f, int, BMX_HT, 0x4, CAM_BMX_VSIZE.BMX_HT, W, CAM_BMX_VSIZE, BMX_HT)\
    /* BMX_D */\
    CMD(a, b, c, d, e, f, int, BMX_D_EN, 0x4, CAM_CTL_EN_P1_D.BMX_D_EN, W, CAM_CTL_EN_P1_D, BMX_D_EN)\
    CMD(a, b, c, d, e, f, int, BMX_D_EDGE, 0x4, CAM_BMX_D_CTL.BMX_EDGE, W, CAM_BMX_D_CTL, BMX_EDGE)\
    CMD(a, b, c, d, e, f, int, BMX_D_STR_X, 0x4, CAM_BMX_D_CROP.BMX_STR_X, W, CAM_BMX_D_CROP, BMX_STR_X)\
    CMD(a, b, c, d, e, f, int, BMX_D_END_X, 0x4, CAM_BMX_D_CROP.BMX_END_X, W, CAM_BMX_D_CROP, BMX_END_X)\
    CMD(a, b, c, d, e, f, int, BMX_D_HT, 0x4, CAM_BMX_D_VSIZE.BMX_HT, W, CAM_BMX_D_VSIZE, BMX_HT)\
    /* RPG */\
    CMD(a, b, c, d, e, f, int, RPG_EN, 0x4, CAM_CTL_EN_P1.RPG_EN, R, CAM_CTL_EN_P1, RPG_EN)\
    CMD(a, b, c, d, e, f, int, RPG_SATU_B, 0x4, CAM_RPG_SATU_1.RPG_SATU_B, R, CAM_RPG_SATU_1, RPG_SATU_B)\
    CMD(a, b, c, d, e, f, int, RPG_SATU_GB, 0x4, CAM_RPG_SATU_1.RPG_SATU_GB, R, CAM_RPG_SATU_1, RPG_SATU_GB)\
    CMD(a, b, c, d, e, f, int, RPG_SATU_GR, 0x4, CAM_RPG_SATU_2.RPG_SATU_GR, R, CAM_RPG_SATU_2, RPG_SATU_GR)\
    CMD(a, b, c, d, e, f, int, RPG_SATU_R, 0x4, CAM_RPG_SATU_2.RPG_SATU_R, R, CAM_RPG_SATU_2, RPG_SATU_R)\
    CMD(a, b, c, d, e, f, int, RPG_GAIN_B, 0x4, CAM_RPG_GAIN_1.RPG_GAIN_B, R, CAM_RPG_GAIN_1, RPG_GAIN_B)\
    CMD(a, b, c, d, e, f, int, RPG_GAIN_GB, 0x4, CAM_RPG_GAIN_1.RPG_GAIN_GB, R, CAM_RPG_GAIN_1, RPG_GAIN_GB)\
    CMD(a, b, c, d, e, f, int, RPG_GAIN_GR, 0x4, CAM_RPG_GAIN_2.RPG_GAIN_GR, R, CAM_RPG_GAIN_2, RPG_GAIN_GR)\
    CMD(a, b, c, d, e, f, int, RPG_GAIN_R, 0x4, CAM_RPG_GAIN_2.RPG_GAIN_R, R, CAM_RPG_GAIN_2, RPG_GAIN_R)\
    CMD(a, b, c, d, e, f, int, RPG_OFST_B, 0x4, CAM_RPG_OFST_1.RPG_OFST_B, R, CAM_RPG_OFST_1, RPG_OFST_B)\
    CMD(a, b, c, d, e, f, int, RPG_OFST_GB, 0x4, CAM_RPG_OFST_1.RPG_OFST_GB, R, CAM_RPG_OFST_1, RPG_OFST_GB)\
    CMD(a, b, c, d, e, f, int, RPG_OFST_GR, 0x4, CAM_RPG_OFST_2.RPG_OFST_GR, R, CAM_RPG_OFST_2, RPG_OFST_GR)\
    CMD(a, b, c, d, e, f, int, RPG_OFST_R, 0x4, CAM_RPG_OFST_2.RPG_OFST_R, R, CAM_RPG_OFST_2, RPG_OFST_R)\
    /* RPG_D */\
    CMD(a, b, c, d, e, f, int, RPG_D_EN, 0x4, CAM_CTL_EN_P1_D.RPG_D_EN, W, CAM_CTL_EN_P1_D, RPG_D_EN)\
    CMD(a, b, c, d, e, f, int, RPG_D_SATU_B, 0x4, CAM_RPG_D_SATU_1.RPG_SATU_B, W, CAM_RPG_D_SATU_1, RPG_SATU_B)\
    CMD(a, b, c, d, e, f, int, RPG_D_SATU_GB, 0x4, CAM_RPG_D_SATU_1.RPG_SATU_GB, W, CAM_RPG_D_SATU_1, RPG_SATU_GB)\
    CMD(a, b, c, d, e, f, int, RPG_D_SATU_GR, 0x4, CAM_RPG_D_SATU_2.RPG_SATU_GR, W, CAM_RPG_D_SATU_2, RPG_SATU_GR)\
    CMD(a, b, c, d, e, f, int, RPG_D_SATU_R, 0x4, CAM_RPG_D_SATU_2.RPG_SATU_R, W, CAM_RPG_D_SATU_2, RPG_SATU_R)\
    CMD(a, b, c, d, e, f, int, RPG_D_GAIN_B, 0x4, CAM_RPG_D_GAIN_1.RPG_GAIN_B, W, CAM_RPG_D_GAIN_1, RPG_GAIN_B)\
    CMD(a, b, c, d, e, f, int, RPG_D_GAIN_GB, 0x4, CAM_RPG_D_GAIN_1.RPG_GAIN_GB, W, CAM_RPG_D_GAIN_1, RPG_GAIN_GB)\
    CMD(a, b, c, d, e, f, int, RPG_D_GAIN_GR, 0x4, CAM_RPG_D_GAIN_2.RPG_GAIN_GR, W, CAM_RPG_D_GAIN_2, RPG_GAIN_GR)\
    CMD(a, b, c, d, e, f, int, RPG_D_GAIN_R, 0x4, CAM_RPG_D_GAIN_2.RPG_GAIN_R, W, CAM_RPG_D_GAIN_2, RPG_GAIN_R)\
    CMD(a, b, c, d, e, f, int, RPG_D_OFST_B, 0x4, CAM_RPG_D_OFST_1.RPG_OFST_B, W, CAM_RPG_D_OFST_1, RPG_OFST_B)\
    CMD(a, b, c, d, e, f, int, RPG_D_OFST_GB, 0x4, CAM_RPG_D_OFST_1.RPG_OFST_GB, W, CAM_RPG_D_OFST_1, RPG_OFST_GB)\
    CMD(a, b, c, d, e, f, int, RPG_D_OFST_GR, 0x4, CAM_RPG_D_OFST_2.RPG_OFST_GR, W, CAM_RPG_D_OFST_2, RPG_OFST_GR)\
    CMD(a, b, c, d, e, f, int, RPG_D_OFST_R, 0x4, CAM_RPG_D_OFST_2.RPG_OFST_R, W, CAM_RPG_D_OFST_2, RPG_OFST_R)\
    /* RRZ */\
    CMD(a, b, c, d, e, f, int, RRZ_EN, 0x4, CAM_CTL_EN_P1.RRZ_EN, R, CAM_CTL_EN_P1, RRZ_EN)\
    CMD(a, b, c, d, e, f, int, RRZ_HORI_EN, 0x4, CAM_RRZ_CTL.RRZ_HORI_EN, R, CAM_RRZ_CTL, RRZ_HORI_EN)\
    CMD(a, b, c, d, e, f, int, RRZ_VERT_EN, 0x4, CAM_RRZ_CTL.RRZ_VERT_EN, R, CAM_RRZ_CTL, RRZ_VERT_EN)\
    CMD(a, b, c, d, e, f, int, RRZ_HORI_TBL_SEL, 0x4, CAM_RRZ_CTL.RRZ_HORI_TBL_SEL, R, CAM_RRZ_CTL, RRZ_HORI_TBL_SEL)\
    CMD(a, b, c, d, e, f, int, RRZ_VERT_TBL_SEL, 0x4, CAM_RRZ_CTL.RRZ_VERT_TBL_SEL, R, CAM_RRZ_CTL, RRZ_VERT_TBL_SEL)\
    CMD(a, b, c, d, e, f, int, RRZ_VERT_STEP, 0x4, CAM_RRZ_VERT_STEP.RRZ_VERT_STEP, RW, CAM_RRZ_VERT_STEP, RRZ_VERT_STEP)\
    CMD(a, b, c, d, e, f, int, RRZ_VERT_INT_OFST, 0x4, CAM_RRZ_VERT_INT_OFST.RRZ_VERT_INT_OFST, R, CAM_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, f, int, RRZ_VERT_SUB_OFST, 0x4, CAM_RRZ_VERT_SUB_OFST.RRZ_VERT_SUB_OFST, R, CAM_RRZ_VERT_SUB_OFST, RRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, f, int, RRZ_TH_MD, 0x4, CAM_RRZ_MODE_TH.RRZ_TH_MD, R, CAM_RRZ_MODE_TH, RRZ_TH_MD)\
    CMD(a, b, c, d, e, f, int, RRZ_TH_HI, 0x4, CAM_RRZ_MODE_TH.RRZ_TH_HI, R, CAM_RRZ_MODE_TH, RRZ_TH_HI)\
    CMD(a, b, c, d, e, f, int, RRZ_TH_LO, 0x4, CAM_RRZ_MODE_TH.RRZ_TH_LO, R, CAM_RRZ_MODE_TH, RRZ_TH_LO)\
    CMD(a, b, c, d, e, f, int, RRZ_TH_MD2, 0x4, CAM_RRZ_MODE_TH.RRZ_TH_MD2, R, CAM_RRZ_MODE_TH, RRZ_TH_MD2)\
    CMD(a, b, c, d, e, f, int, RRZ_PRF_BLD, 0x4, CAM_RRZ_MODE_CTL.RRZ_PRF_BLD, R, CAM_RRZ_MODE_CTL, RRZ_PRF_BLD)\
    CMD(a, b, c, d, e, f, int, RRZ_PRF, 0x4, CAM_RRZ_MODE_CTL.RRZ_PRF, R, CAM_RRZ_MODE_CTL, RRZ_PRF)\
    CMD(a, b, c, d, e, f, int, RRZ_BLD_SL, 0x4, CAM_RRZ_MODE_CTL.RRZ_BLD_SL, R, CAM_RRZ_MODE_CTL, RRZ_BLD_SL)\
    CMD(a, b, c, d, e, f, int, RRZ_CR_MODE, 0x4, CAM_RRZ_MODE_CTL.RRZ_CR_MODE, R, CAM_RRZ_MODE_CTL, RRZ_CR_MODE)\
    CMD(a, b, c, d, e, f, int, RRZ_IN_HT, 0x4, CAM_RRZ_IN_IMG.RRZ_IN_HT, W, CAM_RRZ_IN_IMG, RRZ_IN_HT)\
    CMD(a, b, c, d, e, f, int, RRZ_IN_WD, 0x4, CAM_RRZ_IN_IMG.RRZ_IN_WD, W, CAM_RRZ_IN_IMG, RRZ_IN_WD)\
    CMD(a, b, c, d, e, f, int, RRZ_OUT_HT, 0x4, CAM_RRZ_OUT_IMG.RRZ_OUT_HT, RW, CAM_RRZ_OUT_IMG, RRZ_OUT_HT)\
    CMD(a, b, c, d, e, f, int, RRZ_OUT_WD, 0x4, CAM_RRZ_OUT_IMG.RRZ_OUT_WD, W, CAM_RRZ_OUT_IMG, RRZ_OUT_WD)\
    CMD(a, b, c, d, e, f, int, RRZ_HORI_STEP, 0x4, CAM_RRZ_HORI_STEP.RRZ_HORI_STEP, W, CAM_RRZ_HORI_STEP, RRZ_HORI_STEP)\
    CMD(a, b, c, d, e, f, int, RRZ_HORI_INT_OFST, 0x4, CAM_RRZ_HORI_INT_OFST.RRZ_HORI_INT_OFST, W, CAM_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, f, int, RRZ_HORI_SUB_OFST, 0x4, CAM_RRZ_HORI_SUB_OFST.RRZ_HORI_SUB_OFST, W, CAM_RRZ_HORI_SUB_OFST, RRZ_HORI_SUB_OFST)\
    /* RRZ_D */\
    CMD(a, b, c, d, e, f, int, RRZ_D_EN, 0x4, CAM_CTL_EN_P1_D.RRZ_D_EN, W, CAM_CTL_EN_P1_D, RRZ_D_EN)\
    CMD(a, b, c, d, e, f, int, RRZ_HORI_EN, 0x4, CAM_RRZ_D_CTL.RRZ_HORI_EN, W, CAM_RRZ_D_CTL, RRZ_HORI_EN)\
    CMD(a, b, c, d, e, f, int, RRZ_VERT_EN, 0x4, CAM_RRZ_D_CTL.RRZ_VERT_EN, W, CAM_RRZ_D_CTL, RRZ_VERT_EN)\
    CMD(a, b, c, d, e, f, int, RRZ_D_HORI_TBL_SEL, 0x4, CAM_RRZ_D_CTL.RRZ_HORI_TBL_SEL, W, CAM_RRZ_D_CTL, RRZ_HORI_TBL_SEL)\
    CMD(a, b, c, d, e, f, int, RRZ_D_VERT_TBL_SEL, 0x4, CAM_RRZ_D_CTL.RRZ_VERT_TBL_SEL, W, CAM_RRZ_D_CTL, RRZ_VERT_TBL_SEL)\
    CMD(a, b, c, d, e, f, int, RRZ_D_VERT_STEP, 0x4, CAM_RRZ_D_VERT_STEP.RRZ_VERT_STEP, W, CAM_RRZ_D_VERT_STEP, RRZ_VERT_STEP)\
    CMD(a, b, c, d, e, f, int, RRZ_D_VERT_INT_OFST, 0x4, CAM_RRZ_D_VERT_INT_OFST.RRZ_VERT_INT_OFST, W, CAM_RRZ_D_VERT_INT_OFST, RRZ_VERT_INT_OFST)\
    CMD(a, b, c, d, e, f, int, RRZ_D_VERT_SUB_OFST, 0x4, CAM_RRZ_D_VERT_SUB_OFST.RRZ_VERT_SUB_OFST, W, CAM_RRZ_D_VERT_SUB_OFST, RRZ_VERT_SUB_OFST)\
    CMD(a, b, c, d, e, f, int, RRZ_D_TH_MD, 0x4, CAM_RRZ_D_MODE_TH.RRZ_TH_MD, W, CAM_RRZ_D_MODE_TH, RRZ_TH_MD)\
    CMD(a, b, c, d, e, f, int, RRZ_D_TH_HI, 0x4, CAM_RRZ_D_MODE_TH.RRZ_TH_HI, W, CAM_RRZ_D_MODE_TH, RRZ_TH_HI)\
    CMD(a, b, c, d, e, f, int, RRZ_D_TH_LO, 0x4, CAM_RRZ_D_MODE_TH.RRZ_TH_LO, W, CAM_RRZ_D_MODE_TH, RRZ_TH_LO)\
    CMD(a, b, c, d, e, f, int, RRZ_D_TH_MD2, 0x4, CAM_RRZ_D_MODE_TH.RRZ_TH_MD2, W, CAM_RRZ_D_MODE_TH, RRZ_TH_MD2)\
    CMD(a, b, c, d, e, f, int, RRZ_D_PRF_BLD, 0x4, CAM_RRZ_D_MODE_CTL.RRZ_PRF_BLD, W, CAM_RRZ_D_MODE_CTL, RRZ_PRF_BLD)\
    CMD(a, b, c, d, e, f, int, RRZ_D_PRF, 0x4, CAM_RRZ_D_MODE_CTL.RRZ_PRF, W, CAM_RRZ_D_MODE_CTL, RRZ_PRF)\
    CMD(a, b, c, d, e, f, int, RRZ_D_BLD_SL, 0x4, CAM_RRZ_D_MODE_CTL.RRZ_BLD_SL, W, CAM_RRZ_D_MODE_CTL, RRZ_BLD_SL)\
    CMD(a, b, c, d, e, f, int, RRZ_D_CR_MODE, 0x4, CAM_RRZ_D_MODE_CTL.RRZ_CR_MODE, W, CAM_RRZ_D_MODE_CTL, RRZ_CR_MODE)\
    CMD(a, b, c, d, e, f, int, RRZ_D_IN_HT, 0x4, CAM_RRZ_D_IN_IMG.RRZ_IN_HT, W, CAM_RRZ_D_IN_IMG, RRZ_IN_HT)\
    CMD(a, b, c, d, e, f, int, RRZ_D_IN_WD, 0x4, CAM_RRZ_D_IN_IMG.RRZ_IN_WD, W, CAM_RRZ_D_IN_IMG, RRZ_IN_WD)\
    CMD(a, b, c, d, e, f, int, RRZ_D_OUT_HT, 0x4, CAM_RRZ_D_OUT_IMG.RRZ_OUT_HT, W, CAM_RRZ_D_OUT_IMG, RRZ_OUT_HT)\
    CMD(a, b, c, d, e, f, int, RRZ_D_OUT_WD, 0x4, CAM_RRZ_D_OUT_IMG.RRZ_OUT_WD, W, CAM_RRZ_D_OUT_IMG, RRZ_OUT_WD)\
    CMD(a, b, c, d, e, f, int, RRZ_D_HORI_STEP, 0x4, CAM_RRZ_D_HORI_STEP.RRZ_HORI_STEP, W, CAM_RRZ_D_HORI_STEP, RRZ_HORI_STEP)\
    CMD(a, b, c, d, e, f, int, RRZ_D_HORI_INT_OFST, 0x4, CAM_RRZ_D_HORI_INT_OFST.RRZ_HORI_INT_OFST, W, CAM_RRZ_D_HORI_INT_OFST, RRZ_HORI_INT_OFST)\
    CMD(a, b, c, d, e, f, int, RRZ_D_HORI_SUB_OFST, 0x4, CAM_RRZ_D_HORI_SUB_OFST.RRZ_HORI_SUB_OFST, W, CAM_RRZ_D_HORI_SUB_OFST, RRZ_HORI_SUB_OFST)\
    /* RMX */\
    CMD(a, b, c, d, e, f, int, RMX_EN, 0x4, CAM_CTL_EN_P1.RMX_EN, R, CAM_CTL_EN_P1, RMX_EN)\
    CMD(a, b, c, d, e, f, int, RMX_EDGE, 0x4, CAM_RMX_CTL.RMX_EDGE, W, CAM_RMX_CTL, RMX_EDGE)\
    CMD(a, b, c, d, e, f, int, RMX_STR_X, 0x4, CAM_RMX_CROP.RMX_STR_X, W, CAM_RMX_CROP, RMX_STR_X)\
    CMD(a, b, c, d, e, f, int, RMX_END_X, 0x4, CAM_RMX_CROP.RMX_END_X, W, CAM_RMX_CROP, RMX_END_X)\
    CMD(a, b, c, d, e, f, int, RMX_HT, 0x4, CAM_RMX_VSIZE.RMX_HT, W, CAM_RMX_VSIZE, RMX_HT)\
    CMD(a, b, c, d, e, f, int, RMX_SINGLE_MODE_1, 0x4, CAM_RMX_CTL.RMX_SINGLE_MODE_1, W, CAM_RMX_CTL, RMX_SINGLE_MODE_1)\
    /* RMX_D */\
    CMD(a, b, c, d, e, f, int, RMX_D_EN, 0x4, CAM_CTL_EN_P1_D.RMX_D_EN, W, CAM_CTL_EN_P1_D, RMX_D_EN)\
    CMD(a, b, c, d, e, f, int, RMX_D_EDGE, 0x4, CAM_RMX_D_CTL.RMX_EDGE, W, CAM_RMX_D_CTL, RMX_EDGE)\
    CMD(a, b, c, d, e, f, int, RMX_D_STR_X, 0x4, CAM_RMX_D_CROP.RMX_STR_X, W, CAM_RMX_D_CROP, RMX_STR_X)\
    CMD(a, b, c, d, e, f, int, RMX_D_END_X, 0x4, CAM_RMX_D_CROP.RMX_END_X, W, CAM_RMX_D_CROP, RMX_END_X)\
    CMD(a, b, c, d, e, f, int, RMX_D_HT, 0x4, CAM_RMX_D_VSIZE.RMX_HT, W, CAM_RMX_D_VSIZE, RMX_HT)\
    CMD(a, b, c, d, e, f, int, RMX_SINGLE_MODE_2, 0x4, CAM_RMX_CTL.RMX_SINGLE_MODE_2, W, CAM_RMX_CTL, RMX_SINGLE_MODE_2)\
    /* RMM */\
    CMD(a, b, c, d, e, f, int, IHDR_GAIN, 0x4, CAM_CTL_IHDR.IHDR_GAIN, R, CAM_CTL_IHDR, IHDR_GAIN)\
    CMD(a, b, c, d, e, f, int, IHDR_MODE, 0x4, CAM_CTL_IHDR.IHDR_MODE, R, CAM_CTL_IHDR, IHDR_MODE)\
    CMD(a, b, c, d, e, f, int, IHDR_LE_FIRST, 0x4, CAM_CTL_IHDR.IHDR_LE_FIRST, R, CAM_CTL_IHDR, IHDR_LE_FIRST)\
    /* RMM_D */\
    CMD(a, b, c, d, e, f, int, IHDR_D_GAIN, 0x4, CAM_CTL_IHDR_D.IHDR_D_GAIN, W, CAM_CTL_IHDR_D, IHDR_D_GAIN)\
    CMD(a, b, c, d, e, f, int, IHDR_D_MODE, 0x4, CAM_CTL_IHDR_D.IHDR_D_MODE, W, CAM_CTL_IHDR_D, IHDR_D_MODE)\
    CMD(a, b, c, d, e, f, int, IHDR_D_LE_FIRST, 0x4, CAM_CTL_IHDR_D.IHDR_D_LE_FIRST, W, CAM_CTL_IHDR_D, IHDR_D_LE_FIRST)\

#define DUAL_READ_DECLARE(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_READ_DECLARE_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_DECLARE_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_READ_DECLARE_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_DECLARE_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) g h;
#define DUAL_READ_DECLARE_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)

#define DUAL_WRITE_DECLARE(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_WRITE_DECLARE_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_DECLARE_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_WRITE_DECLARE_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_DECLARE_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) g h;
#define DUAL_WRITE_DECLARE_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)

#define DUAL_READ_FLAG_DECLARE(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_READ_FLAG_DECLARE_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_FLAG_DECLARE_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_READ_FLAG_DECLARE_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_FLAG_DECLARE_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) int h;
#define DUAL_READ_FLAG_DECLARE_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)

#define DUAL_WRITE_FLAG_DECLARE(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_WRITE_FLAG_DECLARE_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_FLAG_DECLARE_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_WRITE_FLAG_DECLARE_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_FLAG_DECLARE_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) int h;
#define DUAL_WRITE_FLAG_DECLARE_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_KNOBS_VAL(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_KNOBS_VAL_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_KNOBS_VAL_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_KNOBS_VAL_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_KNOBS_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_KNOBS_VAL_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (i == (f))\
        {\
            if (0 == memcmp(b, #j, sizeof(#j)))\
            {\
                (a)->h = (c);\
                (d) = true;\
                (e)->h = 1;\
            }\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_KONBS_FLAG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_KONBS_FLAG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_KONBS_FLAG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_KONBS_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_KONBS_FLAG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_KONBS_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->h)\
    {\
        (b) = true;\
        printf("Not found knobs: %s, reg: %s\n", #j, #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_KNOBS_VAL(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_KNOBS_VAL_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_KNOBS_VAL_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_KNOBS_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_KNOBS_VAL_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_KNOBS_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (i == (f))\
        {\
            if (0 == memcmp(b, #j, sizeof(#j)))\
            {\
                (a)->h = (c);\
                (d) = true;\
                (e)->h = 1;\
            }\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_CHECK_KNOBS_FLAG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_KNOBS_FLAG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_KNOBS_FLAG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_KNOBS_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_KNOBS_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_KNOBS_FLAG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->h)\
    {\
        (b) = true;\
        printf("Not found knobs: %s, reg: %s\n", #j, #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_LOG_FILL_VAL_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_LOG_FILL_VAL_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_LOG_FILL_VAL_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_LOG_FILL_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_LOG_FILL_VAL_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (0 == memcmp(b, #l, sizeof(#l) - 1))\
        {\
            if (0 == memcmp(b + sizeof(#l), #m, sizeof(#m)))\
            {\
                (a)->l.m = (c);\
                (d) = true;\
                (e)->l.m = 1;\
            }\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_LOG_FILL_VAL_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_LOG_FILL_VAL_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_LOG_FILL_VAL_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_LOG_FILL_VAL_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_LOG_FILL_VAL_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_LOG_FILL_VAL_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (0 == memcmp(b, #l, sizeof(#l) - 1))\
        {\
            if (0 == memcmp(b + sizeof(#l), #m, sizeof(#m)))\
            {\
                (a)->l.Bits.m = (c);\
                (d) = true;\
                (e)->l.Bits.m = 1;\
            }\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_LOG_FLAG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_LOG_FLAG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_LOG_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_LOG_FLAG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_LOG_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->l.m)\
    {\
        (b) = true;\
        printf("Not found input config: %s.%s\n", #l, #m);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_LOG_FLAG_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_LOG_FLAG_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_LOG_FLAG_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_LOG_FLAG_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_LOG_FLAG_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_LOG_FLAG_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->l.Bits.m)\
    {\
        (b) = true;\
        printf("Not found input config: %s.%s\n", #l, #m);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_LOG_FILL_VAL(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_LOG_FILL_VAL_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_LOG_FILL_VAL_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_LOG_FILL_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_LOG_FILL_VAL_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_LOG_FILL_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (0 == memcmp(b, #l, sizeof(#l) - 1))\
        {\
            if (0 == memcmp(b + sizeof(#l), #m, sizeof(#m)))\
            {\
                (a)->l.m = (c);\
                (d) = true;\
                (e)->l.m = 1;\
            }\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_LOG_FILL_VAL_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_LOG_FILL_VAL_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_LOG_FILL_VAL_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_WRITE_LOG_FILL_VAL_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_LOG_FILL_VAL_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_LOG_FILL_VAL_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (0 == memcmp(b, #l, sizeof(#l) - 1))\
        {\
            if (0 == memcmp(b + sizeof(#l), #m, sizeof(#m)))\
            {\
                (a)->l.Bits.m = (c);\
                (d) = true;\
                (e)->l.Bits.m = 1;\
            }\
        }\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_CHECK_LOG_FLAG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_LOG_FLAG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_LOG_FLAG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_LOG_FLAG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_LOG_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_LOG_FLAG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->l.m)\
    {\
        (b) = true;\
        printf("Not found input config: %s.%s\n", #l, #m);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_CHECK_LOG_FLAG_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_LOG_FLAG_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_LOG_FLAG_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_LOG_FLAG_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_LOG_FLAG_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_LOG_FLAG_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->l.Bits.m)\
    {\
        (b) = true;\
        printf("Not found input config: %s.%s\n", #l, #m);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_CHECK_REG_FLAG_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_REG_FLAG_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_REG_FLAG_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_CHECK_REG_FLAG_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_REG_FLAG_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_CHECK_REG_FLAG_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->l.Bits.m)\
    {\
        (b) = true;\
        printf("Not found input config: %s.%s\n", #l, #m);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_CHECK_REG_FLAG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_REG_FLAG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_REG_FLAG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_CHECK_REG_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_REG_FLAG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_CHECK_REG_FLAG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (0 == (a)->h)\
    {\
        (b) = true;\
        printf("Not found knobs: %s, reg: %s\n", #j, #h);\
    }\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_WRITE_REG_FILL_VAL(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_REG_FILL_VAL_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_REG_FILL_VAL_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_REG_FILL_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_REG_FILL_VAL_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_REG_FILL_VAL_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (0 == memcmp(b, #l, sizeof(#l) - 1))\
        {\
            if (0 == memcmp(b + sizeof(#l), #m, sizeof(#m)))\
            {\
                (a)->h = (c);\
                (d) = true;\
                (e)->h = 1;\
            }\
        }\
    }\

#define DUAL_WRITE_FILE_CHECK(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_FILE_CHECK_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_FILE_CHECK_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_WRITE_FILE_CHECK_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_FILE_CHECK_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_WRITE_FILE_CHECK_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if ((b)->h != (d)->h)\
    {\
        if (0x5a5a5a5a == (b)->h)\
        {\
            fprintf(a, "%s%s0x%X(%d) <non-fill>0x%X(%d) //%s\n", #j, SPACE_EQUAL_SYMBOL_STR,\
                (d)->h, (d)->h, (b)->h, (b)->h, #h);\
            printf("[not fill] %s, knobs: %d(0x%x), cal: %d(0x%x)\n", #h, (d)->h, (d)->h, (b)->h, (b)->h);\
            (f)++;\
        }\
        else\
        {\
            fprintf(a, "%s%s0x%X(%d) <cal diff>0x%X(%d) //%s\n", #j, SPACE_EQUAL_SYMBOL_STR,\
                (d)->h, (d)->h, (b)->h, (b)->h, #h);\
            printf("[diff] %s, knobs: %d(0x%x), cal: %d(0x%x)\n", #h, (d)->h, (d)->h, (b)->h, (b)->h);\
            if (c)\
            {\
                fprintf(c, "RAL: %s %s 0x%X\r\n", #j, SPACE_EQUAL_SYMBOL_STR, (b)->h);\
            }\
            (e)++;\
        }\
    }\
    else\
    {\
        fprintf(a, "%s%s0x%X //%s\n", #j, SPACE_EQUAL_SYMBOL_STR, (b)->h, #h);\
    }\

/* copy read to write */
#define DUAL_BYPASS_W(a, b, c) (b)->c = (a)->c;
#define DUAL_COPY_L2R(a, b, c, d) (b)->d = (a)->c;
/* copy read to write */
#define DUAL_CONVERT_REG_TO_INPUT(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_INPUT_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_INPUT_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (memcmp(#j, #l, sizeof(#l) - 1))\
    {\
        printf("[Warning] diff reg names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#l) - 1);\
    }\
    else if (memcmp(#j + sizeof(#l), #m, sizeof(#m)))\
    {\
        printf("[Warning] diff field names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#m));\
    }\
    if (0 == (c)->l.m)\
    {\
        (c)->l.m = 1;\
        (b)->l.m = (a)->h;\
    }\
    else\
    {\
        printf("[Error] duplicated names, knobs: %s, driver: %s.%s\n", #j, #l, #m);\
        (d) = ISP_MESSAGE_DUPLICATED_NAME_ERROR;\
    }\

/* copy read to write */
#define DUAL_CONVERT_REG_TO_INPUT_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_INPUT_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_INPUT_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (memcmp(#j, #l, sizeof(#l) - 1))\
    {\
        printf("[Warning] diff reg names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#l) - 1);\
    }\
    else if (memcmp(#j + sizeof(#l), #m, sizeof(#m)))\
    {\
        printf("[Warning] diff field names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#m));\
    }\
    if (0 == (c)->l.Bits.m)\
    {\
        (c)->l.Bits.m = 1;\
        (b)->l.Bits.m = (a)->h;\
    }\
    else\
    {\
        printf("[Error] duplicated names, knobs: %s, driver: %s.%s\n", #j, #l, #m);\
        (d) = ISP_MESSAGE_DUPLICATED_NAME_ERROR;\
    }\

/* copy read to write */
#define DUAL_CONVERT_REG_TO_INPUT_DEBUG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_INPUT_DEBUG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_DEBUG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_INPUT_DEBUG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_DEBUG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_INPUT_DEBUG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (memcmp(#j, #l, sizeof(#l) - 1))\
    {\
        printf("[Warning] diff reg names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#l) - 1);\
    }\
    else if (memcmp(#j + sizeof(#l), #m, sizeof(#m)))\
    {\
        printf("[Warning] diff field names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#m));\
    }\
    if (0 == (c)->l.m)\
    {\
        (c)->l.m = 1;\
        (b)->l.m = (a)->h;\
    }\
    else\
    {\
        printf("[Error] duplicated names, knobs: %s, driver: %s.%s\n", #j, #l, #m);\
        (d) = ISP_MESSAGE_DUPLICATED_NAME_ERROR;\
    }\

/* copy read to write */
#define DUAL_CONVERT_REG_TO_OUTPUT(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_OUTPUT_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_OUTPUT_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_OUTPUT_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_OUTPUT_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_OUTPUT_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (memcmp(#j, #l, sizeof(#l) - 1))\
    {\
        printf("[Warning] diff reg names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#l) - 1);\
    }\
    else if (memcmp(#j + sizeof(#l), #m, sizeof(#m)))\
    {\
        printf("[Warning] diff field names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#m));\
    }\
    if (0 == (c)->l.m)\
    {\
        (c)->l.m = 1;\
        (b)->l.m = (a)->h;\
    }\
    else\
    {\
        printf("[Error] duplicated names, knobs: %s, driver: %s.%s\n", #j, #l, #m);\
        (d) = ISP_MESSAGE_DUPLICATED_NAME_ERROR;\
    }\

/* copy read to write */
#define DUAL_CONVERT_REG_TO_OUTPUT_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_OUTPUT_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_OUTPUT_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_REG_TO_OUTPUT_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_OUTPUT_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_REG_TO_OUTPUT_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (memcmp(#j, #l, sizeof(#l) - 1))\
    {\
        printf("[Warning] diff reg names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#l) - 1);\
    }\
    else if (memcmp(#j + sizeof(#l), #m, sizeof(#m)))\
    {\
        printf("[Warning] diff field names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#m));\
    }\
    if (0 == (c)->l.Bits.m)\
    {\
        (c)->l.Bits.m = 1;\
        (b)->l.Bits.m = (a)->h;\
    }\
    else\
    {\
        printf("[Error] duplicated names, knobs: %s, driver: %s.%s\n", #j, #l, #m);\
        (d) = ISP_MESSAGE_DUPLICATED_NAME_ERROR;\
    }\

#define DUAL_CONVERT_OUTPUT_TO_REG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_OUTPUT_TO_REG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_OUTPUT_TO_REG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_OUTPUT_TO_REG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_OUTPUT_TO_REG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_OUTPUT_TO_REG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (memcmp(#j, #l, sizeof(#l) - 1))\
    {\
        printf("[Warning] diff reg names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#l) - 1);\
    }\
    else if (memcmp(#j + sizeof(#l), #m, sizeof(#m))\
    {\
        printf("[Warning] diff field names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#m));\
    }\
    if (0 == (c)->l.m)\
    {\
        (c)->l.m = 1;\
        (a)->h = (b)->l.m;\
    }\
    else\
    {\
        printf("[Error] duplicated names, knobs: %s, driver: %s.%s\n", #j, #l, #m);\
        (d) = ISP_MESSAGE_DUPLICATED_NAME_ERROR;\
    }\

#define DUAL_CONVERT_OUTPUT_TO_REG_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_OUTPUT_TO_REG_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_OUTPUT_TO_REG_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CONVERT_OUTPUT_TO_REG_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_OUTPUT_TO_REG_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CONVERT_OUTPUT_TO_REG_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (memcmp(#j, #l, sizeof(#l) - 1))\
    {\
        printf("[Warning] diff reg names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#l) - 1);\
    }\
    else if (memcmp(#j + sizeof(#l), #m, sizeof(#m)))\
    {\
        printf("[Warning] diff field names, knobs: %s, driver: %s.%s, size comp: %d\n", #j, #l, #m, (int)sizeof(#m));\
    }\
    if (0 == (c)->l.Bits.m)\
    {\
        (c)->l.Bits.m = 1;\
        (a)->h = (b)->l.Bits.m;\
    }\
    else\
    {\
        printf("[Error] duplicated names, knobs: %s, driver: %s.%s\n", #j, #l, #m);\
        (d) = ISP_MESSAGE_DUPLICATED_NAME_ERROR;\
    }\

#define DUAL_CMP_PRINT_TXT(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CMP_PRINT_TXT_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CMP_PRINT_TXT_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if ((c)->l.Bits.m != (d)->l.Bits.m)\
    {\
        b(a, "%s %s%s%d (0x%x)%s%d (0x%x) //0x%08X, R\n", DUAL_CMP_DUMP_HEADER, #j, DUAL_SPACE_EQUAL_SYMBOL_STR,\
            (d)->l.Bits.m, (d)->l.Bits.m, DUAL_HW_DIFF_STR, (c)->l.Bits.m, (c)->l.Bits.m,\
            DUAL_ISP_REG_ADDR_START + 4*(unsigned int)((unsigned int *)&(c)->l - (unsigned int *)(c)));\
        (f) = ISP_MESSAGE_HW_REG_DIFF_ERROR;\
    }\

#define DUAL_CMP_PRINT_TXT_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if ((c)->l.Bits.m != (e)->l.Bits.m)\
    {\
        b(a, "%s %s%s%d (0x%x)%s%d (0x%x) //0x%08X, W\n", DUAL_CMP_DUMP_HEADER, #j, DUAL_SPACE_EQUAL_SYMBOL_STR,\
            (e)->l.Bits.m, (e)->l.Bits.m, DUAL_HW_DIFF_STR, (c)->l.Bits.m, (c)->l.Bits.m,\
            DUAL_ISP_REG_ADDR_START + 4*(unsigned int)((unsigned int *)&(c)->l - (unsigned int *)(c)));\
        (f) = ISP_MESSAGE_HW_REG_DIFF_ERROR;\
    }\

#define DUAL_CMP_PRINT_TXT_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if ((c)->l.Bits.m != (e)->l.Bits.m)\
    {\
        b(a, "%s %s%s%d (0x%x)%s%d (0x%x) //0x%08X, RW\n", DUAL_CMP_DUMP_HEADER, #j, DUAL_SPACE_EQUAL_SYMBOL_STR,\
            (e)->l.Bits.m, (e)->l.Bits.m, DUAL_HW_DIFF_STR, (c)->l.Bits.m, (c)->l.Bits.m,\
            DUAL_ISP_REG_ADDR_START + 4*(unsigned int)((unsigned int *)&(c)->l - (unsigned int *)(c)));\
        (f) = ISP_MESSAGE_HW_REG_DIFF_ERROR;\
    }\

#define DUAL_CMP_PRINT_TXT_UART(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CMP_PRINT_TXT_UART_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CMP_PRINT_TXT_UART_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_CMP_PRINT_TXT_UART_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_CMP_PRINT_TXT_UART_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if ((c)->l.Bits.m != (b)->l.Bits.m)\
    {\
        a("%s %s%s%d (0x%x)%s%d (0x%x) //0x%08X\n", DUAL_CMP_DUMP_HEADER, #j, DUAL_SPACE_EQUAL_SYMBOL_STR,\
            (c)->l.Bits.m, (c)->l.Bits.m, DUAL_HW_DIFF_STR, (b)->l.Bits.m, (b)->l.Bits.m,\
            DUAL_ISP_REG_ADDR_START + 4*(unsigned int)((unsigned int *)&(b)->l - (unsigned int *)(b)));\
        (e) = ISP_MESSAGE_HW_REG_DIFF_ERROR;\
    }\

#define DUAL_CMP_PRINT_TXT_UART_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if ((c)->l.Bits.m != (d)->l.Bits.m)\
    {\
        a("%s %s%s%d (0x%x)%s%d (0x%x) //0x%08X\n", DUAL_CMP_DUMP_HEADER, #j, DUAL_SPACE_EQUAL_SYMBOL_STR,\
            (d)->l.Bits.m, (d)->l.Bits.m, DUAL_HW_DIFF_STR, (b)->l.Bits.m, (b)->l.Bits.m,\
            DUAL_ISP_REG_ADDR_START + 4*(unsigned int)((unsigned int *)&(b)->l - (unsigned int *)(b)));\
        (e) = ISP_MESSAGE_HW_REG_DIFF_ERROR;\
    }\

#define DUAL_LOG_PRINT_TXT(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    b(a, "%s %s.%s%s%d (0x%x)\n", DUAL_IN_DUMP_HEADER, #l, #m, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->l.m, (c)->l.m);\

#define DUAL_LOG_PRINT_TXT_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    b(a, "%s %s.%s%s%d (0x%x)\n", DUAL_IN_DUMP_HEADER, #l, #m, DUAL_SPACE_EQUAL_SYMBOL_STR, (c)->l.Bits.m, (c)->l.Bits.m);\

#define DUAL_LOG_PRINT_TXT_UART(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_UART_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_UART_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_UART_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_UART_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_UART_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    a("%s %s.%s%s%d (0x%x)\n", DUAL_IN_DUMP_HEADER, #l, #m, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->l.m, (b)->l.m);\

#define DUAL_LOG_PRINT_TXT_UART_HW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_UART_HW_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_UART_HW_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_LOG_PRINT_TXT_UART_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_UART_HW_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_LOG_PRINT_TXT_UART_HW_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    a("%s %s.%s%s%d (0x%x)\n", DUAL_IN_DUMP_HEADER, #l, #m, DUAL_SPACE_EQUAL_SYMBOL_STR, (b)->l.Bits.m, (b)->l.Bits.m);\

/* data search & update reg mapper */
/* a: ptr_reg */
/* b: cmp str */
/* c: value to fill */
/* d: found flag */
#define DUAL_READ_REG_CMP_DEBUG(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_REG_CMP_DEBUG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_REG_CMP_DEBUG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)  DUAL_READ_REG_CMP_DEBUG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_REG_CMP_DEBUG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_READ_REG_CMP_DEBUG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) \
    if (false == (d))\
    {\
        if (0 == memcmp(b, #l, sizeof(#l) - 1))\
        {\
            if (0 == memcmp(b + sizeof(#l), #m, sizeof(#m)))\
            {\
                if ((a)->l.m != (c))\
                {\
                    printf("[Warning] diff %s.%s, config: %d, reg: %d\n", #l, #m, (a)->l.m, (c));\
                }\
            }\
        }\
    }\

#define DUAL_COPY_R_REG(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_COPY_R_REG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_R_REG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_R_REG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_R_REG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...) (a)->l.Bits.m = (b)->l.Bits.m;

#define DUAL_COPY_RW_REG(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_COPY_RW_REG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_RW_REG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...) (a)->l.Bits.m = (b)->l.Bits.m;
#define DUAL_COPY_RW_REG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_RW_REG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)

#define DUAL_IN_OUT_COPY(a, b, c, d, e, f, g, h, i,...) DUAL_IN_OUT_COPY_##i(a, b, c, d, e, f, g, h, i,...)
#define DUAL_IN_OUT_COPY_RW(a, b, c, d, e, f, g, h, i,...) (a)->h.Raw = (b)->h.Raw;
#define DUAL_IN_OUT_COPY_R(a, b, c, d, e, f, g, h, i,...)
#define DUAL_IN_OUT_COPY_W(a, b, c, d, e, f, g, h, i,...)


#define DUAL_COPY_W_REG(a, b, c, d, e, f, g, h, i, j, k, l, m,...) DUAL_COPY_W_REG_##k(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_W_REG_RW(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_W_REG_R(a, b, c, d, e, f, g, h, i, j, k, l, m,...)
#define DUAL_COPY_W_REG_W(a, b, c, d, e, f, g, h, i, j, k, l, m,...) (a)->l.Bits.m = (b)->l.Bits.m;

/* struct type */
typedef struct DUAL_IN_STRUCT
{
    /* COMMON */
    DUAL_DEBUG_LUT(DUAL_READ_DECLARE,,,,,,)
    DUAL_SW_LUT(DUAL_READ_DECLARE,,,,,,)
    DUAL_HW_LUT(DUAL_READ_DECLARE,,,,,,)
}DUAL_IN_STRUCT;

/* struct type */
typedef struct DUAL_OUT_STRUCT
{
    /* COMMON */
    DUAL_DEBUG_LUT(DUAL_WRITE_DECLARE,,,,,,)
    DUAL_SW_LUT(DUAL_WRITE_DECLARE,,,,,,)
    DUAL_HW_LUT(DUAL_WRITE_DECLARE,,,,,,)
}DUAL_OUT_STRUCT;

/* flag type to check read or not */
typedef struct DUAL_IN_FLAG_STRUCT
{
    DUAL_DEBUG_LUT(DUAL_READ_FLAG_DECLARE,,,,,,)
    DUAL_SW_LUT(DUAL_READ_FLAG_DECLARE,,,,,,)
    DUAL_HW_LUT(DUAL_READ_FLAG_DECLARE,,,,,,)
}DUAL_IN_FLAG_STRUCT;

/* flag type to check read or not */
typedef struct DUAL_OUT_FLAG_STRUCT
{
    DUAL_DEBUG_LUT(DUAL_WRITE_FLAG_DECLARE,,,,,,)
    DUAL_SW_LUT(DUAL_WRITE_FLAG_DECLARE,,,,,,)
    DUAL_HW_LUT(DUAL_WRITE_FLAG_DECLARE,,,,,,)
}DUAL_OUT_FLAG_STRUCT;

#endif
