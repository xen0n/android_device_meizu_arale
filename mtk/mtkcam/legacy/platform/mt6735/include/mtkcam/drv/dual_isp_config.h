#ifndef __DUAL_ISP_CONFIG_H__
#define __DUAL_ISP_CONFIG_H__

/* following define can be changed with rebuilding dual_isp_driver.cpp */
#define DUAL_IN_DUMP_HEADER "[ISP][DUAL_IN]"
#define DUAL_DUMP_VERSION "V.0.0.0.1"
#define DUAL_DUMP_START "start MT6593"
#define DUAL_DUMP_END "end MT6593"
#define DUAL_HW_DIFF_STR " <hw diff> " /* can be changed */
#define DUAL_SPACE_EQUAL_SYMBOL_STR " = "

#define DUAL_ISP_REG_ADDR_START (0x15000000)
#define DUAL_REG_DUMP_HEADER "[ISP][DUAL_REG]"
#define DUAL_CMP_DUMP_HEADER "[ISP][DUAL_CMP]"

#define DUAL_IN_OUT_STRUCT_LUT_SW(CMD, a, b, c, d, e, f) \
    /* Driver */\
    CMD(a, b, c, d, e, f, DUAL_DEBUG_STRUCT, DEBUG, R)\
    /* SW */\
    CMD(a, b, c, d, e, f, DUAL_SW_STRUCT, SW, R)\

#define DUAL_IN_OUT_STRUCT_LUT_HW(CMD, a, b, c, d, e, f) \
    /* TG */\
    CMD(a, b, c, d, e, f, CAM_REG_CTL_SEL_GLOBAL, CAM_CTL_SEL_GLOBAL, R)\
    CMD(a, b, c, d, e, f, CAM_REG_TG_SEN_GRAB_PXL, CAM_TG_SEN_GRAB_PXL, R)\
    CMD(a, b, c, d, e, f, CAM_REG_TG_SEN_GRAB_LIN, CAM_TG_SEN_GRAB_LIN, R)\
    /* ENABLE */\
    CMD(a, b, c, d, e, f, CAM_REG_CTL_EN_P1, CAM_CTL_EN_P1, R)\
    /* ENABLE_D */\
    CMD(a, b, c, d, e, f, CAM_REG_CTL_EN_P1_D, CAM_CTL_EN_P1_D, RW)/* partial W */\
    /* DMX */\
    CMD(a, b, c, d, e, f, CAM_REG_DMX_CTL, CAM_DMX_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_DMX_CROP, CAM_DMX_CROP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_DMX_VSIZE, CAM_DMX_VSIZE, R)\
    /* DMX_D */\
    CMD(a, b, c, d, e, f, CAM_REG_DMX_D_CTL, CAM_DMX_D_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_DMX_D_CROP, CAM_DMX_D_CROP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_DMX_D_VSIZE, CAM_DMX_D_VSIZE, W)\
    /* OB */\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_OFFST0, CAM_OBC_OFFST0, R)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_OFFST1, CAM_OBC_OFFST1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_OFFST2, CAM_OBC_OFFST2, R)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_OFFST3, CAM_OBC_OFFST3, R)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_GAIN0, CAM_OBC_GAIN0, R)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_GAIN1, CAM_OBC_GAIN1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_GAIN2, CAM_OBC_GAIN2, R)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_GAIN3, CAM_OBC_GAIN3, R)\
    /* OB_D */\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_OFFST0, CAM_OBC_D_OFFST0, W)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_OFFST1, CAM_OBC_D_OFFST1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_OFFST2, CAM_OBC_D_OFFST2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_OFFST3, CAM_OBC_D_OFFST3, W)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_GAIN0, CAM_OBC_D_GAIN0, W)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_GAIN1, CAM_OBC_D_GAIN1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_GAIN2, CAM_OBC_D_GAIN2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_OBC_D_GAIN3, CAM_OBC_D_GAIN3, W)\
    /* BPC */\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_CON, CAM_BPC_CON, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TH1, CAM_BPC_TH1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TH2, CAM_BPC_TH2, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TH3, CAM_BPC_TH3, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TH4, CAM_BPC_TH4, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_DTC, CAM_BPC_DTC, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_COR, CAM_BPC_COR, R)\
    CMD(a, b, c, d, e, f, CAM_REG_NR1_CON, CAM_NR1_CON, R)\
    CMD(a, b, c, d, e, f, CAM_REG_NR1_CT_CON, CAM_NR1_CT_CON, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TBLI1, CAM_BPC_TBLI1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TBLI2, CAM_BPC_TBLI2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_RMM1, CAM_BPC_RMM1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_RMM2, CAM_BPC_RMM2, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TH1_C, CAM_BPC_TH1_C, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TH2_C, CAM_BPC_TH2_C, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_TH3_C, CAM_BPC_TH3_C, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_RMM_REVG_1, CAM_BPC_RMM_REVG_1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_RMM_REVG_2, CAM_BPC_RMM_REVG_2, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_RMM_LEOS, CAM_BPC_RMM_LEOS, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_RMM_GCNT, CAM_BPC_RMM_GCNT, R)\
    /* BPC_D */\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_CON, CAM_BPC_D_CON, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TH1, CAM_BPC_D_TH1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TH2, CAM_BPC_D_TH2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TH3, CAM_BPC_D_TH3, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TH4, CAM_BPC_D_TH4, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_DTC, CAM_BPC_D_DTC, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_COR, CAM_BPC_D_COR, W)\
    CMD(a, b, c, d, e, f, CAM_REG_NR1_D_CON, CAM_NR1_D_CON, W)\
    CMD(a, b, c, d, e, f, CAM_REG_NR1_D_CT_CON, CAM_NR1_D_CT_CON, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TBLI1, CAM_BPC_D_TBLI1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TBLI2, CAM_BPC_D_TBLI2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_RMM1, CAM_BPC_D_RMM1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_RMM2, CAM_BPC_D_RMM2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TH1_C, CAM_BPC_D_TH1_C, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TH2_C, CAM_BPC_D_TH2_C, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_TH3_C, CAM_BPC_D_TH3_C, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_RMM_REVG_1, CAM_BPC_D_RMM_REVG_1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_RMM_REVG_2, CAM_BPC_D_RMM_REVG_2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_RMM_LEOS, CAM_BPC_D_RMM_LEOS, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPC_D_RMM_GCNT, CAM_BPC_D_RMM_GCNT, W)\
    /* BPCI */\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_OFST_ADDR, CAM_BPCI_OFST_ADDR, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_XSIZE, CAM_BPCI_XSIZE, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_YSIZE, CAM_BPCI_YSIZE, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_STRIDE, CAM_BPCI_STRIDE, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_CON, CAM_BPCI_CON, R)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_CON2, CAM_BPCI_CON2, R)\
    /* BPCI_D */\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_D_OFST_ADDR, CAM_BPCI_D_OFST_ADDR, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_D_XSIZE, CAM_BPCI_D_XSIZE, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_D_YSIZE, CAM_BPCI_D_YSIZE, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_D_STRIDE, CAM_BPCI_D_STRIDE, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_D_CON, CAM_BPCI_D_CON, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BPCI_D_CON2, CAM_BPCI_D_CON2, W)\
    /* LSC */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_CTL2, CAM_LSC_CTL2, RW)/* partial W */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_LBLOCK, CAM_LSC_LBLOCK, RW)/* partial W */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_TPIPE_OFST, CAM_LSC_TPIPE_OFST, RW)/* partial W */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_TPIPE_SIZE, CAM_LSC_TPIPE_SIZE, RW)/* partial W */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_CTL1, CAM_LSC_CTL1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_CTL3, CAM_LSC_CTL3, R)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_RATIO, CAM_LSC_RATIO, R)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_GAIN_TH, CAM_LSC_GAIN_TH, R)\
    /* LSC_D */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_CTL2, CAM_LSC_D_CTL2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_LBLOCK, CAM_LSC_D_LBLOCK, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_TPIPE_OFST, CAM_LSC_D_TPIPE_OFST, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_TPIPE_SIZE, CAM_LSC_D_TPIPE_SIZE, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_CTL1, CAM_LSC_D_CTL1, RW)/* partial W */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_CTL3, CAM_LSC_D_CTL3, RW)/* partial W */\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_RATIO, CAM_LSC_D_RATIO, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSC_D_GAIN_TH, CAM_LSC_D_GAIN_TH, W)\
    /* LSCI */\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_OFST_ADDR, CAM_LSCI_OFST_ADDR, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_XSIZE, CAM_LSCI_XSIZE, RW)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_YSIZE, CAM_LSCI_YSIZE, R)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_STRIDE, CAM_LSCI_STRIDE, R)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_CON, CAM_LSCI_CON, R)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_CON2, CAM_LSCI_CON2, R)\
    /* LSCI_D */\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_D_OFST_ADDR, CAM_LSCI_D_OFST_ADDR, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_D_XSIZE, CAM_LSCI_D_XSIZE, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_D_YSIZE, CAM_LSCI_D_YSIZE, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_D_STRIDE, CAM_LSCI_D_STRIDE, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_D_CON, CAM_LSCI_D_CON, W)\
    CMD(a, b, c, d, e, f, CAM_REG_LSCI_D_CON2, CAM_LSCI_D_CON2, W)\
    /* BMX */\
    CMD(a, b, c, d, e, f, CAM_REG_BMX_CTL, CAM_BMX_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BMX_CROP, CAM_BMX_CROP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BMX_VSIZE, CAM_BMX_VSIZE, W)\
    /* BMX_D */\
    CMD(a, b, c, d, e, f, CAM_REG_BMX_D_CTL, CAM_BMX_D_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BMX_D_CROP, CAM_BMX_D_CROP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_BMX_D_VSIZE, CAM_BMX_D_VSIZE, W)\
    /* RPG */\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_SATU_1, CAM_RPG_SATU_1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_SATU_2, CAM_RPG_SATU_2, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_GAIN_1, CAM_RPG_GAIN_1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_GAIN_2, CAM_RPG_GAIN_2, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_OFST_1, CAM_RPG_OFST_1, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_OFST_2, CAM_RPG_OFST_2, R)\
    /* RPG_D */\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_D_SATU_1, CAM_RPG_D_SATU_1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_D_SATU_2, CAM_RPG_D_SATU_2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_D_GAIN_1, CAM_RPG_D_GAIN_1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_D_GAIN_2, CAM_RPG_D_GAIN_2, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_D_OFST_1, CAM_RPG_D_OFST_1, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RPG_D_OFST_2, CAM_RPG_D_OFST_2, W)\
    /* RRZ */\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_CTL, CAM_RRZ_CTL, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_OUT_IMG, CAM_RRZ_OUT_IMG, RW)/* partial W */\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_VERT_STEP, CAM_RRZ_VERT_STEP, RW)/* 0 will be re-cal */\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_VERT_INT_OFST, CAM_RRZ_VERT_INT_OFST, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_VERT_SUB_OFST, CAM_RRZ_VERT_SUB_OFST, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_MODE_TH, CAM_RRZ_MODE_TH, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_MODE_CTL, CAM_RRZ_MODE_CTL, R)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_IN_IMG, CAM_RRZ_IN_IMG, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_HORI_STEP, CAM_RRZ_HORI_STEP, RW)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_HORI_INT_OFST, CAM_RRZ_HORI_INT_OFST, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_HORI_SUB_OFST, CAM_RRZ_HORI_SUB_OFST, W)\
    /* RRZ_D */\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_CTL, CAM_RRZ_D_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_OUT_IMG, CAM_RRZ_D_OUT_IMG, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_VERT_STEP, CAM_RRZ_D_VERT_STEP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_VERT_INT_OFST, CAM_RRZ_D_VERT_INT_OFST, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_VERT_SUB_OFST, CAM_RRZ_D_VERT_SUB_OFST, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_MODE_TH, CAM_RRZ_D_MODE_TH, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_MODE_CTL, CAM_RRZ_D_MODE_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_IN_IMG, CAM_RRZ_D_IN_IMG, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_HORI_STEP, CAM_RRZ_D_HORI_STEP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_HORI_INT_OFST, CAM_RRZ_D_HORI_INT_OFST, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RRZ_D_HORI_SUB_OFST, CAM_RRZ_D_HORI_SUB_OFST, W)\
    /* RMX */\
    CMD(a, b, c, d, e, f, CAM_REG_RMX_CTL, CAM_RMX_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RMX_CROP, CAM_RMX_CROP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RMX_VSIZE, CAM_RMX_VSIZE, W)\
    /* RMX_D */\
    CMD(a, b, c, d, e, f, CAM_REG_RMX_D_CTL, CAM_RMX_D_CTL, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RMX_D_CROP, CAM_RMX_D_CROP, W)\
    CMD(a, b, c, d, e, f, CAM_REG_RMX_D_VSIZE, CAM_RMX_D_VSIZE, W)\
    /* RMM */\
    CMD(a, b, c, d, e, f, CAM_REG_CTL_IHDR, CAM_CTL_IHDR, R)\
    /* RMM_D */\
    CMD(a, b, c, d, e, f, CAM_REG_CTL_IHDR_D, CAM_CTL_IHDR_D, W)\

#define DUAL_IN_STRUCT_DECLARE(a, b, c, d, e, f, g, h, i,...) DUAL_IN_STRUCT_DECLARE_##i(a, b, c, d, e, f, g, h, i,...)
#define DUAL_IN_STRUCT_DECLARE_RW(a, b, c, d, e, f, g, h, i,...) DUAL_IN_STRUCT_DECLARE_R(a, b, c, d, e, f, g, h, i,...)
#define DUAL_IN_STRUCT_DECLARE_R(a, b, c, d, e, f, g, h, i,...) g h;
#define DUAL_IN_STRUCT_DECLARE_W(a, b, c, d, e, f, g, h, i,...)

#define DUAL_OUT_STRUCT_DECLARE(a, b, c, d, e, f, g, h, i,...) DUAL_OUT_STRUCT_DECLARE_##i(a, b, c, d, e, f, g, h, i,...)
#define DUAL_OUT_STRUCT_DECLARE_RW(a, b, c, d, e, f, g, h, i,...) DUAL_OUT_STRUCT_DECLARE_W(a, b, c, d, e, f, g, h, i,...)
#define DUAL_OUT_STRUCT_DECLARE_W(a, b, c, d, e, f, g, h, i,...) g h;
#define DUAL_OUT_STRUCT_DECLARE_R(a, b, c, d, e, f, g, h, i,...)

/* a: isp_reg_ptr */
#define DUAL_CPP_CODE_SET_REG(a, b, c, d, e, f, g, h, i,...)  DUAL_CPP_CODE_SET_REG_##i(a, b, c, d, e, f, g, h, i,...)
#define DUAL_CPP_CODE_SET_REG_RW(a, b, c, d, e, f, g, h, i,...)  DUAL_CPP_CODE_SET_REG_R(a, b, c, d, e, f, g, h, i,...)
#define DUAL_CPP_CODE_SET_REG_W(a, b, c, d, e, f, g, h, i,...)
#define DUAL_CPP_CODE_SET_REG_R(a, b, c, d, e, f, g, h, i,...) (a)->h.Raw = 1;
/* a: add old, b: addr new, c: str name */
#define DUAL_CPP_CODE_FPRINTF(a, b, c, d, e, f, g, h, i,...) \
    {\
        unsigned int temp_addr = (size_t)(&(d)->h);\
        if ((a) < temp_addr)\
        {\
            if ((a) == (b))\
            {\
                (b) = temp_addr;\
                mtk_sprintf(c, sizeof(c), "\tCMD(a, b, c, 0x%08X, %s, %s)\\\r\n", temp_addr, #h, #i);\
            }\
            else\
            {\
                if (temp_addr < (b))\
                {\
                    b = temp_addr;\
                    mtk_sprintf(c, sizeof(c), "\tCMD(a, b, c, 0x%08X, %s, %s)\\\r\n", temp_addr, #h, #i);\
                }\
            }\
        }\
    }\

/* all data types must be int */
typedef struct DUAL_DEBUG_STRUCT
{
    int DUAL_LOG_EN;
    int DUAL_LOG_ID;
}DUAL_DEBUG_STRUCT;

/* all data types must be int */
typedef struct DUAL_SW_STRUCT
{
    int TWIN_MODE_SDBLK_XNUM_ALL;
    int TWIN_MODE_SDBLK_lWIDTH_ALL;
    int TWIN_RRZ_IN_CROP_HT;/* must be larger than one */
    int TWIN_RRZ_HORI_INT_OFST;
    int TWIN_RRZ_HORI_INT_OFST_LAST;
    int TWIN_RRZ_HORI_SUB_OFST;
    int TWIN_RRZ_OUT_WD;
    int TWIN_LSC_CROP_OFFX;/* by pixel, default zero */
    int TWIN_LSC_CROP_OFFY;/* by line, default zero */
}DUAL_SW_STRUCT;

typedef struct ISP_DUAL_IN_CONFIG_STRUCT
{
    DUAL_IN_OUT_STRUCT_LUT_SW(DUAL_IN_STRUCT_DECLARE,,,,,,)
    DUAL_IN_OUT_STRUCT_LUT_HW(DUAL_IN_STRUCT_DECLARE,,,,,,)
}ISP_DUAL_IN_CONFIG_STRUCT;

typedef struct ISP_DUAL_OUT_CONFIG_STRUCT
{
    DUAL_IN_OUT_STRUCT_LUT_SW(DUAL_OUT_STRUCT_DECLARE,,,,,,)
    DUAL_IN_OUT_STRUCT_LUT_HW(DUAL_OUT_STRUCT_DECLARE,,,,,,)
}ISP_DUAL_OUT_CONFIG_STRUCT;

extern int dual_cal_platform(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
                ISP_DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config,
                const isp_reg_t *ptr_isp_reg, const char *dir_path);
extern int dual_print_platform_config(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config);
#endif
