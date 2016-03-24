#ifndef __TPIPE_CONFIG_H__
#define __TPIPE_CONFIG_H__

/* following define can be changed with rebuilding tpipe_driver.c */
#define LOG_REMARK_MUST_FILL_STR "<must>" /* can be changed */
#define LOG_REMARK_NOT_FOUND_STR "<no config>?" /* can be changed */
#define LOG_REMARK_HW_NOT_FOUND_STR "<no hw>?" /* can be changed */
#define LOG_REMARK_HW_REG_STR "<hw>" /* can be changed */
#define LOG_REMARK_DEBUG_STR "<debug>" /* can be changed */
#define LOG_REMARK_HW_DIFF_STR "<hw diff> " /* can be changed */
#define LOG_REMARK_HW_LOG_STR "<hw log>" /* can be changed */
#define TPIPE_DEBUG_DUMP_HEADER "[ISP][tpipe_dump]"
#define TPIPE_DEBUG_DUMP_START "start MT6593"
#define TPIPE_DEBUG_DUMP_END "end MT6593"
#define TPIPE_LOG_PARSE_FILE_NAME "tpipe_log_parse_"
#define TPIPE_LOG_PARSE_FILE_EXT ".txt"
#define TPIPE_REG_DUMP_HEADER "0x"
#define TPIPE_REG_DUMP_SKIP_NO (4096)
#define TPIPE_ISP_BASE_HW     0x15000000

#define MAX_TILE_TOT_NO (2400)
#define MAX_ISP_DUMP_HEX_PER_TILE (36)
#define MAX_ISP_TILE_TDR_HEX_NO (MAX_TILE_TOT_NO*MAX_ISP_DUMP_HEX_PER_TILE)

#define ISP_ERROR_MESSAGE_DATA(n, CMD) \
    CMD(n, ISP_TPIPE_MESSAGE_OK)\
    CMD(n, ISP_TPIPE_MESSAGE_FAIL)\
    /* final count, can not be changed */\
    CMD(n, ISP_TPIPE_MESSAGE_MAX_NO)\

#define ISP_TPIPE_ENUM_DECLARE(a, b) b,
#define ISP_TPIPE_ENUM_STRING(n, a) if ((a) == (n)) return #a;

#define GET_ISP_ERROR_NAME(n) \
    if (0 == (n)) return "ISP_TPIPE_MESSAGE_UNKNOWN";\
    ISP_ERROR_MESSAGE_DATA(n, ISP_TPIPE_ENUM_STRING)\
    return "";

//need equal to ISP_DRV_P2_CQ_ENUM
typedef enum
{
    ISP_TPIPE_P2_CQ1 = 0,
    ISP_TPIPE_P2_CQ2,
    ISP_TPIPE_P2_CQ3,
    ISP_TPIPE_P2_CQ_NUM
}ISP_TPIPE_P2_CQ_ENUM;

/* error enum */
typedef enum ISP_TPIPE_MESSAGE_ENUM
{
    ISP_TPIPE_MESSAGE_UNKNOWN=0,
    ISP_ERROR_MESSAGE_DATA(,ISP_TPIPE_ENUM_DECLARE)
}ISP_TPIPE_MESSAGE_ENUM;

/* tpipe_irq_mode */
typedef enum TPIPE_IRQ_MODE_ENUM
{
    TPIPE_IRQ_FRAME_STOP=0,
    TPIPE_IRQ_LINE_END,
    TPIPE_IRQ_PER_TPIPE,
    TPIPE_IRQ_MODE_MAX_NO
}TPIPE_IRQ_MODE_ENUM;

/* f: reg_map_struct name*/
/* g: must and enable/disble*/
/* h: variable offset in isp_reg*/
/* i: variable mask in isp_reg*/
/* j: variable lsb in isp_reg*/
/* k: variable name in isp_reg*/
/* l: variable name in isp_reg*/
#define HW_ISP_REG_CMP(CMD, a, b, c, d, e) \
    /* Dump ISP registers - start, replaced with auto-code gen */\
CMD(a, b, c, d, e, top.scenario, true, 0x15004024, 0x00000007, 0, CAM_CTL_SCENARIO, SCENARIO)\
CMD(a, b, c, d, e, top.mode, true, 0x15004024, 0x00000070, 4, CAM_CTL_SCENARIO, SUB_MODE)\
CMD(a, b, c, d, e, top.pixel_id, true, 0x15004028, 0x00000003, 0, CAM_CTL_FMT_SEL_P1, PIX_ID)\
CMD(a, b, c, d, e, top.cam_in_fmt, true, 0x15004030, 0x0000000F, 0, CAM_CTL_FMT_SEL_P2, IN_FMT)\
CMD(a, b, c, d, e, top.ctl_extension_en, true, 0x1500407C, 0x40000000, 30, CAM_CTL_TILE, CTL_EXTENSION_EN)\
CMD(a, b, c, d, e, top.fg_mode, true, 0x15004030, 0x00010000, 16, CAM_CTL_FMT_SEL_P2, FG_MODE)\
CMD(a, b, c, d, e, top.ufdi_fmt, true, 0x15004030, 0x00001000, 12, CAM_CTL_FMT_SEL_P2, UFDI_FMT)\
CMD(a, b, c, d, e, top.vipi_fmt, true, 0x15004030, 0x00000700, 8, CAM_CTL_FMT_SEL_P2, VIPI_FMT)\
CMD(a, b, c, d, e, top.img3o_fmt, true, 0x15004030, 0x00000070, 4, CAM_CTL_FMT_SEL_P2, IMG3O_FMT)\
CMD(a, b, c, d, e, top.imgi_en, true, 0x1500401C, 0x00000001, 0, CAM_CTL_EN_P2_DMA, IMGI_EN)\
CMD(a, b, c, d, e, top.ufdi_en, true, 0x1500401C, 0x00000002, 1, CAM_CTL_EN_P2_DMA, UFDI_EN)\
CMD(a, b, c, d, e, top.unp_en, true, 0x15004018, 0x40000000, 30, CAM_CTL_EN_P2, UNP_EN)\
CMD(a, b, c, d, e, top.ufd_en, true, 0x15004018, 0x00000001, 0, CAM_CTL_EN_P2, UFD_EN)\
CMD(a, b, c, d, e, top.bnr_en, MODE_CMP_EQ(c, TPIPE_SCENARIO_IP, TPIPE_SUBMODE_IP_RAW), 0x15004004, 0x00000020, 5, CAM_CTL_EN_P1, BNR_EN)\
CMD(a, b, c, d, e, top.lsci_en, MODE_CMP_EQ(c, TPIPE_SCENARIO_IP, TPIPE_SUBMODE_IP_RAW), 0x15004008, 0x00000080, 7, CAM_CTL_EN_P1_DMA, LSCI_EN)\
CMD(a, b, c, d, e, top.lsc_en, MODE_CMP_EQ(c, TPIPE_SCENARIO_IP, TPIPE_SUBMODE_IP_RAW), 0x15004004, 0x00000040, 6, CAM_CTL_EN_P1, LSC_EN)\
CMD(a, b, c, d, e, top.sl2_en, true, 0x15004018, 0x00000004, 2, CAM_CTL_EN_P2, SL2_EN)\
CMD(a, b, c, d, e, top.cfa_en, true, 0x15004018, 0x00000008, 3, CAM_CTL_EN_P2, CFA_EN)\
CMD(a, b, c, d, e, top.c24_en, true, 0x15004018, 0x00000100, 8, CAM_CTL_EN_P2, C24_EN)\
CMD(a, b, c, d, e, top.vipi_en, true, 0x1500401C, 0x00000004, 2, CAM_CTL_EN_P2_DMA, VIPI_EN)\
CMD(a, b, c, d, e, top.vip2i_en, true, 0x1500401C, 0x00000008, 3, CAM_CTL_EN_P2_DMA, VIP2I_EN)\
CMD(a, b, c, d, e, top.vip3i_en, true, 0x1500401C, 0x00000010, 4, CAM_CTL_EN_P2_DMA, VIP3I_EN)\
CMD(a, b, c, d, e, top.mfb_en, true, 0x15004018, 0x00000080, 7, CAM_CTL_EN_P2, MFB_EN)\
CMD(a, b, c, d, e, top.mfbo_en, true, 0x1500401C, 0x00000040, 6, CAM_CTL_EN_P2_DMA, MFBO_EN)\
CMD(a, b, c, d, e, top.g2c_en, true, 0x15004018, 0x00000200, 9, CAM_CTL_EN_P2, G2C_EN)\
CMD(a, b, c, d, e, top.c42_en, true, 0x15004018, 0x00000400, 10, CAM_CTL_EN_P2, C42_EN)\
CMD(a, b, c, d, e, top.sl2b_en, true, 0x15004018, 0x00010000, 16, CAM_CTL_EN_P2, SL2B_EN)\
CMD(a, b, c, d, e, top.nbc_en, true, 0x15004018, 0x00000800, 11, CAM_CTL_EN_P2, NBC_EN)\
CMD(a, b, c, d, e, top.mix1_en, true, 0x15004018, 0x00200000, 21, CAM_CTL_EN_P2, MIX1_EN)\
CMD(a, b, c, d, e, top.mix2_en, true, 0x15004018, 0x00400000, 22, CAM_CTL_EN_P2, MIX2_EN)\
CMD(a, b, c, d, e, top.sl2c_en, true, 0x15004018, 0x00020000, 17, CAM_CTL_EN_P2, SL2C_EN)\
CMD(a, b, c, d, e, top.seee_en, true, 0x15004018, 0x00002000, 13, CAM_CTL_EN_P2, SEEE_EN)\
CMD(a, b, c, d, e, top.lcei_en, true, 0x1500401C, 0x00000020, 5, CAM_CTL_EN_P2_DMA, LCEI_EN)\
CMD(a, b, c, d, e, top.lce_en, true, 0x15004018, 0x00004000, 14, CAM_CTL_EN_P2, LCE_EN)\
CMD(a, b, c, d, e, top.mix3_en, true, 0x15004018, 0x00800000, 23, CAM_CTL_EN_P2, MIX3_EN)\
CMD(a, b, c, d, e, top.crz_en, true, 0x15004018, 0x00100000, 20, CAM_CTL_EN_P2, CRZ_EN)\
CMD(a, b, c, d, e, top.img2o_en, true, 0x1500401C, 0x00000080, 7, CAM_CTL_EN_P2_DMA, IMG2O_EN)\
CMD(a, b, c, d, e, top.srz1_en, true, 0x15004018, 0x00040000, 18, CAM_CTL_EN_P2, SRZ1_EN)\
CMD(a, b, c, d, e, top.fe_en, true, 0x15004018, 0x20000000, 29, CAM_CTL_EN_P2, FE_EN)\
CMD(a, b, c, d, e, top.feo_en, true, 0x1500401C, 0x00000800, 11, CAM_CTL_EN_P2_DMA, FEO_EN)\
CMD(a, b, c, d, e, top.c02_en, true, 0x15004018, 0x80000000, 31, CAM_CTL_EN_P2, C02_EN)\
CMD(a, b, c, d, e, top.nr3d_en, true, 0x15004018, 0x00008000, 15, CAM_CTL_EN_P2, NR3D_EN)\
CMD(a, b, c, d, e, top.crsp_en, true, 0x15004018, 0x04000000, 26, CAM_CTL_EN_P2, CRSP_EN)\
CMD(a, b, c, d, e, top.img3o_en, true, 0x1500401C, 0x00000100, 8, CAM_CTL_EN_P2_DMA, IMG3O_EN)\
CMD(a, b, c, d, e, top.img3bo_en, true, 0x1500401C, 0x00000200, 9, CAM_CTL_EN_P2_DMA, IMG3BO_EN)\
CMD(a, b, c, d, e, top.img3co_en, true, 0x1500401C, 0x00000400, 10, CAM_CTL_EN_P2_DMA, IMG3CO_EN)\
CMD(a, b, c, d, e, top.c24b_en, true, 0x15004018, 0x08000000, 27, CAM_CTL_EN_P2, C24B_EN)\
CMD(a, b, c, d, e, top.mdp_crop_en, true, 0x15004018, 0x10000000, 28, CAM_CTL_EN_P2, MDPCROP_EN)\
CMD(a, b, c, d, e, top.srz2_en, true, 0x15004018, 0x00080000, 19, CAM_CTL_EN_P2, SRZ2_EN)\
CMD(a, b, c, d, e, top.imgi_v_flip_en, true, 0x15007220, 0x00000002, 1, CAM_VERTICAL_FLIP_EN, IMGI_V_FLIP_EN)\
CMD(a, b, c, d, e, top.lcei_v_flip_en, true, 0x15007220, 0x00000040, 6, CAM_VERTICAL_FLIP_EN, LCEI_V_FLIP_EN)\
CMD(a, b, c, d, e, top.ufdi_v_flip_en, true, 0x15007220, 0x00000020, 5, CAM_VERTICAL_FLIP_EN, UFDI_V_FLIP_EN)\
CMD(a, b, c, d, e, top.ufd_sel, true, 0x1500403C, 0x00000010, 4, CAM_CTL_SEL_P2, UFD_SEL)\
CMD(a, b, c, d, e, top.ccl_sel, true, 0x1500403C, 0x00000001, 0, CAM_CTL_SEL_P2, CCL_SEL)\
CMD(a, b, c, d, e, top.ccl_sel_en, true, 0x1500403C, 0x00000002, 1, CAM_CTL_SEL_P2, CCL_SEL_EN)\
CMD(a, b, c, d, e, top.g2g_sel, true, 0x1500403C, 0x00000004, 2, CAM_CTL_SEL_P2, G2G_SEL)\
CMD(a, b, c, d, e, top.g2g_sel_en, true, 0x1500403C, 0x00000008, 3, CAM_CTL_SEL_P2, G2G_SEL_EN)\
CMD(a, b, c, d, e, top.c24_sel, true, 0x1500403C, 0x00000020, 5, CAM_CTL_SEL_P2, C24_SEL)\
CMD(a, b, c, d, e, top.srz1_sel, true, 0x1500403C, 0x00000040, 6, CAM_CTL_SEL_P2, SRZ1_SEL)\
CMD(a, b, c, d, e, top.mix1_sel, true, 0x1500403C, 0x00000080, 7, CAM_CTL_SEL_P2, MIX1_SEL)\
CMD(a, b, c, d, e, top.crz_sel, true, 0x1500403C, 0x00000300, 8, CAM_CTL_SEL_P2, CRZ_SEL)\
CMD(a, b, c, d, e, top.nr3d_sel, true, 0x1500403C, 0x00000400, 10, CAM_CTL_SEL_P2, NR3D_SEL)\
CMD(a, b, c, d, e, top.fe_sel, true, 0x1500403C, 0x00001000, 12, CAM_CTL_SEL_P2, FE_SEL)\
CMD(a, b, c, d, e, top.mdp_sel, true, 0x1500403C, 0x00000800, 11, CAM_CTL_SEL_P2, MDP_SEL)\
CMD(a, b, c, d, e, top.pca_sel, true, 0x1500403C, 0x00002000, 13, CAM_CTL_SEL_P2, PCA_SEL)\
CMD(a, b, c, d, e, top.interlace_mode, true, 0x1500722C, 0x40000000, 30, CAM_IMGI_SLOW_DOWN, INTERLACE_MODE)\
CMD(a, b, c, d, e, imgi.imgi_stride, REG_CMP_EQ(c, LOG_IMGI_EN, 1), 0x15007240, 0x00007FFF, 0, CAM_IMGI_STRIDE, STRIDE)\
CMD(a, b, c, d, e, ufdi.ufdi_stride, REG_CMP_EQ(c, LOG_UFDI_EN, 1), 0x15007298, 0x00003FFF, 0, CAM_UFDI_STRIDE, STRIDE)\
CMD(a, b, c, d, e, bnr.bpc_en, REG_CMP_EQ(c, LOG_BNR_EN, 1), 0x15004800, 0x00000001, 0, CAM_BPC_CON, BPC_EN)\
CMD(a, b, c, d, e, bnr.bpc_tbl_en, REG_CMP_EQ(c, LOG_BNR_EN, 1), 0x15004800, 0x00000010, 4, CAM_BPC_CON, BPC_LUT_EN)\
CMD(a, b, c, d, e, lsci.lsci_stride, REG_CMP_EQ(c, LOG_LSCI_EN, 1), 0x1500727C, 0x00003FFF, 0, CAM_LSCI_STRIDE, STRIDE)\
CMD(a, b, c, d, e, sl2.sl2_hrz_comp, REG_CMP_EQ(c, LOG_SL2_EN, 1), 0x15004F50, 0x00003FFF, 0, CAM_SL2_HRZ, SL2_HRZ_COMP)\
CMD(a, b, c, d, e, cfa.bayer_bypass, REG_CMP_EQ(c, LOG_CFA_EN, 1), 0x150048A0, 0x00000001, 0, CAM_DM_O_BYP, DM_BYP)\
CMD(a, b, c, d, e, cfa.dm_fg_mode, REG_CMP_EQ(c, LOG_CFA_EN, 1), 0x150048A0, 0x00000010, 4, CAM_DM_O_BYP, DM_FG_MODE)\
CMD(a, b, c, d, e, vipi.vipi_stride, REG_CMP_EQ(c, LOG_VIPI_EN, 1), 0x150072D0, 0x00003FFF, 0, CAM_VIPI_STRIDE, STRIDE)\
CMD(a, b, c, d, e, vip2i.vip2i_stride, REG_CMP_EQ(c, LOG_VIP2I_EN, 1), 0x150072F0, 0x00003FFF, 0, CAM_VIP2I_STRIDE, STRIDE)\
CMD(a, b, c, d, e, vip3i.vip3i_stride, REG_CMP_EQ(c, LOG_VIP3I_EN, 1), 0x150073B4, 0x00003FFF, 0, CAM_VIP3I_STRIDE, STRIDE)\
CMD(a, b, c, d, e, mfb.bld_mode, REG_CMP_EQ(c, LOG_MFB_EN, 1), 0x15004960, 0x00000007, 0, CAM_MFB_CON, BLD_MODE)\
CMD(a, b, c, d, e, mfb.bld_deblock_en, REG_CMP_EQ(c, LOG_MFB_EN, 1), 0x1500496C, 0x00000010, 4, CAM_MFB_LL_CON3, BLD_LL_DB_EN)\
CMD(a, b, c, d, e, mfb.bld_brz_en, REG_CMP_EQ(c, LOG_MFB_EN, 1), 0x1500496C, 0x00000001, 0, CAM_MFB_LL_CON3, BLD_LL_BRZ_EN)\
CMD(a, b, c, d, e, mfbo.mfbo_stride, REG_CMP_EQ(c, LOG_MFBO_EN, 1), 0x150073F0, 0x00003FFF, 0, CAM_MFBO_STRIDE, STRIDE)\
CMD(a, b, c, d, e, g2c.g2c_shade_en, REG_CMP_EQ(c, LOG_G2C_EN, 1), 0x15004BB8, 0x10000000, 28, CAM_G2C_SHADE_CON_1, G2C_SHADE_EN)\
CMD(a, b, c, d, e, sl2b.sl2b_hrz_comp, REG_CMP_EQ(c, LOG_SL2B_EN, 1), 0x15004F70, 0x00003FFF, 0, CAM_SL2B_HRZ, SL2B_HRZ_COMP)\
CMD(a, b, c, d, e, nbc.anr_eny, REG_CMP_EQ(c, LOG_NBC_EN, 1), 0x15004A20, 0x00000002, 1, CAM_ANR_CON1, ANR_ENY)\
CMD(a, b, c, d, e, nbc.anr_enc, REG_CMP_EQ(c, LOG_NBC_EN, 1), 0x15004A20, 0x00000001, 0, CAM_ANR_CON1, ANR_ENC)\
CMD(a, b, c, d, e, nbc.anr_iir_mode, REG_CMP_EQ(c, LOG_NBC_EN, 1), 0x15004A20, 0x00000700, 8, CAM_ANR_CON1, ANR_FLT_MODE)\
CMD(a, b, c, d, e, nbc.anr_scale_mode, REG_CMP_EQ(c, LOG_NBC_EN, 1), 0x15004A20, 0x00000030, 4, CAM_ANR_CON1, ANR_SCALE_MODE)\
CMD(a, b, c, d, e, sl2c.sl2c_hrz_comp, REG_CMP_EQ(c, LOG_SL2C_EN, 1), 0x15004FD0, 0x00003FFF, 0, CAM_SL2C_HRZ, SL2C_HRZ_COMP)\
CMD(a, b, c, d, e, seee.se_edge, REG_CMP_EQ(c, LOG_SEEE_EN, 1), 0x15004ACC, 0x00000003, 0, CAM_SEEE_OUT_EDGE_CTRL, SEEE_OUT_EDGE_SEL)\
CMD(a, b, c, d, e, lcei.lcei_stride, REG_CMP_EQ(c, LOG_LCEI_EN, 1), 0x150072B4, 0x00003FFF, 0, CAM_LCEI_STRIDE, STRIDE)\
CMD(a, b, c, d, e, lce.lce_bc_mag_kubnx, REG_CMP_EQ(c, LOG_LCE_EN, 1), 0x150049C4, 0x00007FFF, 0, CAM_LCE_ZR, LCE_BCMK_X)\
CMD(a, b, c, d, e, lce.lce_bc_mag_kubny, REG_CMP_EQ(c, LOG_LCE_EN, 1), 0x150049C4, 0x7FFF0000, 16, CAM_LCE_ZR, LCE_BCMK_Y)\
CMD(a, b, c, d, e, cdrz.cdrz_horizontal_luma_algorithm, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), 0x15004B00, 0x00000060, 5, CAM_CRZ_CONTROL, CRZ_HORI_ALGO)\
CMD(a, b, c, d, e, cdrz.cdrz_vertical_luma_algorithm, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), 0x15004B00, 0x00000180, 7, CAM_CRZ_CONTROL, CRZ_VERT_ALGO)\
CMD(a, b, c, d, e, cdrz.cdrz_horizontal_coeff_step, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), 0x15004B0C, 0x007FFFFF, 0, CAM_CRZ_HORI_STEP, CRZ_HORI_STEP)\
CMD(a, b, c, d, e, cdrz.cdrz_vertical_coeff_step, REG_CMP_EQ(c, LOG_CDRZ_EN, 1), 0x15004B10, 0x007FFFFF, 0, CAM_CRZ_VERT_STEP, CRZ_VERT_STEP)\
CMD(a, b, c, d, e, img2o.img2o_stride, REG_CMP_EQ(c, LOG_IMG2O_EN, 1), 0x15007450, 0x00003FFF, 0, CAM_IMG2O_STRIDE, STRIDE)\
CMD(a, b, c, d, e, srz1.srz_horizontal_coeff_step, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), 0x15004C3C, 0x0007FFFF, 0, CAM_SRZ1_HORI_STEP, SRZ1_HORI_STEP)\
CMD(a, b, c, d, e, srz1.srz_vertical_coeff_step, REG_CMP_EQ(c, LOG_SRZ1_EN, 1), 0x15004C40, 0x0007FFFF, 0, CAM_SRZ1_VERT_STEP, SRZ1_VERT_STEP)\
CMD(a, b, c, d, e, srz2.srz_horizontal_coeff_step, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), 0x15004C6C, 0x0007FFFF, 0, CAM_SRZ2_HORI_STEP, SRZ2_HORI_STEP)\
CMD(a, b, c, d, e, srz2.srz_vertical_coeff_step, REG_CMP_EQ(c, LOG_SRZ2_EN, 1), 0x15004C70, 0x0007FFFF, 0, CAM_SRZ2_VERT_STEP, SRZ2_VERT_STEP)\
CMD(a, b, c, d, e, fe.fe_mode, REG_CMP_EQ(c, LOG_FE_EN, 1), 0x15004C20, 0x00000003, 0, CAM_FE_CTRL, FE_MODE)\
CMD(a, b, c, d, e, feo.feo_stride, REG_CMP_EQ(c, LOG_FEO_EN, 1), 0x15007490, 0x0000FFFF, 0, CAM_FEO_STRIDE, STRIDE)\
CMD(a, b, c, d, e, nr3d.nr3d_on_en, REG_CMP_EQ(c, LOG_NR3D_EN, 1), 0x15004CF8, 0x00000010, 4, CAM_NR3D_CTRL, NR3D_ON_EN)\
CMD(a, b, c, d, e, img3o.img3o_stride, REG_CMP_EQ(c, LOG_IMG3O_EN, 1), 0x15007470, 0x00003FFF, 0, CAM_IMG3O_STRIDE, STRIDE)\
CMD(a, b, c, d, e, img3bo.img3bo_stride, REG_CMP_EQ(c, LOG_IMG3BO_EN, 1), 0x15007410, 0x00003FFF, 0, CAM_IMG3BO_STRIDE, STRIDE)\
CMD(a, b, c, d, e, img3co.img3co_stride, REG_CMP_EQ(c, LOG_IMG3CO_EN, 1), 0x15007430, 0x00003FFF, 0, CAM_IMG3CO_STRIDE, STRIDE)\
    /* Dump ISP registers - end */\
    /* Manually insert more ISP registers below */\

typedef struct ISP_TPIPE_CONFIG_TOP_STRUCT
{
    int scenario;
    int mode;
    int pixel_id;
    int cam_in_fmt;
    int ctl_extension_en;
    int fg_mode;
    int ufdi_fmt;
    int vipi_fmt;
    int img3o_fmt;
    int imgi_en;
    int ufdi_en;
    int unp_en;
    int ufd_en;
    int ufd_crop_en;
    int bnr_en;
    int lsci_en;
    int lsc_en;
    int c24_en;
    int sl2_en;
    int cfa_en;
    int vipi_en;
    int vip2i_en;
    int vip3i_en;
    int mfb_en;
    int mfbo_en;
    int g2c_en;
    int c42_en;
    int sl2b_en;
    int nbc_en;
    int srz1_en;
    int mix1_en;
    int srz2_en;
    int mix2_en;
    int sl2c_en;
    int seee_en;
    int lcei_en;
    int lce_en;
    int mix3_en;
    int crz_en;
    int img2o_en;
    int fe_en;
    int feo_en;
    int c02_en;
    int nr3d_en;
    int crsp_en;
    int img3o_en;
    int img3bo_en;
    int img3co_en;
    int c24b_en;
    int mdp_crop_en;
    int interlace_mode;
    int ufd_sel;
    int ccl_sel;
    int ccl_sel_en;
    int g2g_sel;
    int g2g_sel_en;
    int c24_sel;
    int srz1_sel;
    int mix1_sel;
    int crz_sel;
    int nr3d_sel;
    int fe_sel;
    int mdp_sel;
    int pca_sel;
    int imgi_v_flip_en;
    int lcei_v_flip_en;
    int ufdi_v_flip_en;
    int pca_en;
}ISP_TPIPE_CONFIG_TOP_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SW_STRUCT
{
    int log_en;
    int src_width;
    int src_height;
    int tpipe_irq_mode;
    int tpipe_width;
    int tpipe_height;
}ISP_TPIPE_CONFIG_SW_STRUCT;

typedef struct ISP_DRIVER_CONFIG_STRUCT
{
    unsigned int DesCqPa;
    unsigned int* DesCqVa; // for debugging
    unsigned int* VirCqVa; // for debugging
    unsigned int tpipeTablePa;
    unsigned int *tpipeTableVa;
    unsigned int dupCqIdx;
    ISP_TPIPE_P2_CQ_ENUM p2Cq;
    unsigned int debugRegDump; // dump isp reg from GCE
}ISP_DRIVER_CONFIG_STRUCT;


typedef struct ISP_TPIPE_CONFIG_IMGI_STRUCT
{
    int imgi_stride;
}ISP_TPIPE_CONFIG_IMGI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_UFDI_STRUCT
{
    int ufdi_stride;
}ISP_TPIPE_CONFIG_UFDI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_BNR_STRUCT
{
    int bpc_en;
    int bpc_tbl_en;
}ISP_TPIPE_CONFIG_BNR_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LSCI_STRUCT
{
    int lsci_stride;
}ISP_TPIPE_CONFIG_LSCI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LSC_STRUCT
{
    int sdblk_width;
    int sdblk_xnum;
    int sdblk_last_width;
    int sdblk_height;
    int sdblk_ynum;
    int sdblk_last_height;
}ISP_TPIPE_CONFIG_LSC_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SL2_STRUCT
{
    int sl2_hrz_comp;
}ISP_TPIPE_CONFIG_SL2_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CFA_STRUCT
{
    int bayer_bypass;
    int dm_fg_mode;
}ISP_TPIPE_CONFIG_CFA_STRUCT;

typedef struct ISP_TPIPE_CONFIG_VIPI_STRUCT
{
    int vipi_xsize;
    int vipi_ysize;
    int vipi_stride;
}ISP_TPIPE_CONFIG_VIPI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_VIP2I_STRUCT
{
    int vip2i_xsize;
    int vip2i_ysize;
    int vip2i_stride;
}ISP_TPIPE_CONFIG_VIP2I_STRUCT;

typedef struct ISP_TPIPE_CONFIG_VIP3I_STRUCT
{
    int vip3i_xsize;
    int vip3i_ysize;
    int vip3i_stride;
}ISP_TPIPE_CONFIG_VIP3I_STRUCT;

typedef struct ISP_TPIPE_CONFIG_MFB_STRUCT
{
    int bld_mode;
    int bld_deblock_en;
    int bld_brz_en;
}ISP_TPIPE_CONFIG_MFB_STRUCT;

typedef struct ISP_TPIPE_CONFIG_MFBO_STRUCT
{
    int mfbo_stride;
    int mfbo_xoffset;
    int mfbo_yoffset;
    int mfbo_xsize;
    int mfbo_ysize;
}ISP_TPIPE_CONFIG_MFBO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_G2C_STRUCT
{
    int g2c_shade_en;
}ISP_TPIPE_CONFIG_G2C_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SL2B_STRUCT
{
    int sl2b_hrz_comp;
}ISP_TPIPE_CONFIG_SL2B_STRUCT;

typedef struct ISP_TPIPE_CONFIG_NBC_STRUCT
{
    int anr_eny;
    int anr_enc;
    int anr_iir_mode;
    int anr_scale_mode;
}ISP_TPIPE_CONFIG_NBC_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SL2C_STRUCT
{
    int sl2c_hrz_comp;
}ISP_TPIPE_CONFIG_SL2C_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SEEE_STRUCT
{
    int se_edge;
    int usm_over_shrink_en;
}ISP_TPIPE_CONFIG_SEEE_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LCEI_STRUCT
{
    int lcei_stride;
}ISP_TPIPE_CONFIG_LCEI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LCE_STRUCT
{
    int lce_bc_mag_kubnx;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_slm_height;
}ISP_TPIPE_CONFIG_LCE_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CDRZ_STRUCT
{
    int cdrz_input_crop_width;
    int cdrz_input_crop_height;
    int cdrz_output_width;
    int cdrz_output_height;
    int cdrz_luma_horizontal_integer_offset;/* pixel base */
    int cdrz_luma_horizontal_subpixel_offset;/* 20 bits base */
    int cdrz_luma_vertical_integer_offset;/* pixel base */
    int cdrz_luma_vertical_subpixel_offset;/* 20 bits base */
    int cdrz_horizontal_luma_algorithm;
    int cdrz_vertical_luma_algorithm;
    int cdrz_horizontal_coeff_step;
    int cdrz_vertical_coeff_step;
}ISP_TPIPE_CONFIG_CDRZ_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMG2O_STRUCT
{
    int img2o_stride;
    int img2o_xoffset;
    int img2o_yoffset;
    int img2o_xsize;
    int img2o_ysize;
}ISP_TPIPE_CONFIG_IMG2O_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SRZ_STRUCT
{
    int srz_input_crop_width;
    int srz_input_crop_height;
    int srz_output_width;
    int srz_output_height;
    int srz_luma_horizontal_integer_offset;/* pixel base */
    int srz_luma_horizontal_subpixel_offset;/* 20 bits base */
    int srz_luma_vertical_integer_offset;/* pixel base */
    int srz_luma_vertical_subpixel_offset;/* 20 bits base */
    int srz_horizontal_coeff_step;
    int srz_vertical_coeff_step;
}ISP_TPIPE_CONFIG_SRZ_STRUCT;

typedef struct ISP_TPIPE_CONFIG_FE_STRUCT
{
    int fe_mode;
}ISP_TPIPE_CONFIG_FE_STRUCT;

typedef struct ISP_TPIPE_CONFIG_FEO_STRUCT
{
    int feo_stride;
}ISP_TPIPE_CONFIG_FEO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_NR3D_STRUCT
{
    int nr3d_on_en;
    int nr3d_on_xoffset;
    int nr3d_on_yoffset;
    int nr3d_on_width;
    int nr3d_on_height;
}ISP_TPIPE_CONFIG_NR3D_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CRSP_STRUCT
{
    int crsp_ystep;
    int crsp_xoffset;
    int crsp_yoffset;
}ISP_TPIPE_CONFIG_CRSP_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMG3O_STRUCT
{
    int img3o_stride;
    int img3o_xoffset;
    int img3o_yoffset;
    int img3o_xsize;
    int img3o_ysize;
}ISP_TPIPE_CONFIG_IMG3O_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMG3BO_STRUCT
{
    int img3bo_stride;
    int img3bo_xsize;
    int img3bo_ysize;
}ISP_TPIPE_CONFIG_IMG3BO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMG3CO_STRUCT
{
    int img3co_stride;
    int img3co_xsize;
    int img3co_ysize;
}ISP_TPIPE_CONFIG_IMG3CO_STRUCT;

typedef enum TPIPE_Pass2CmdqNum_ENUM
{
    TPIPE_PASS2_CMDQ_NONE=0,
    TPIPE_PASS2_CMDQ_1,
    TPIPE_PASS2_CMDQ_2,
    TPIPE_PASS2_CMDQ_3,
      TPIPE_PASS2_CMDQ_NUM
}TPIPE_Pass2CmdqNum_ENUM;

typedef enum TPIPE_Pass2CmdqPrior_ENUM
{
    TPIPE_PASS2_CMDQ_PRIOR_LOW=1,
    TPIPE_PASS2_CMDQ_PRIOR_HIGH = 10
}TPIPE_Pass2CmdqPrior_ENUM;

typedef struct ISP_TPIPE_CONFIG_PASS2_STRUCT
{
    TPIPE_Pass2CmdqNum_ENUM Pass2CmdqNum;
    TPIPE_Pass2CmdqPrior_ENUM Pass2CmdqPriority;
}ISP_TPIPE_CONFIG_PASS2_STRUCT;

typedef struct ISP_TPIPE_CONFIG_STRUCT
{
    ISP_TPIPE_CONFIG_TOP_STRUCT top;
    ISP_TPIPE_CONFIG_SW_STRUCT sw;
    ISP_TPIPE_CONFIG_IMGI_STRUCT imgi;
    ISP_TPIPE_CONFIG_UFDI_STRUCT ufdi;
    ISP_TPIPE_CONFIG_BNR_STRUCT bnr;
    ISP_TPIPE_CONFIG_LSCI_STRUCT lsci;
    ISP_TPIPE_CONFIG_LSC_STRUCT lsc;
    ISP_TPIPE_CONFIG_SL2_STRUCT sl2;
    ISP_TPIPE_CONFIG_CFA_STRUCT cfa;
    ISP_TPIPE_CONFIG_VIPI_STRUCT vipi;
    ISP_TPIPE_CONFIG_VIP2I_STRUCT vip2i;
    ISP_TPIPE_CONFIG_VIP3I_STRUCT vip3i;
    ISP_TPIPE_CONFIG_MFB_STRUCT mfb;
    ISP_TPIPE_CONFIG_MFBO_STRUCT mfbo;
    ISP_TPIPE_CONFIG_G2C_STRUCT g2c;
    ISP_TPIPE_CONFIG_SL2B_STRUCT sl2b;
    ISP_TPIPE_CONFIG_NBC_STRUCT nbc;
    ISP_TPIPE_CONFIG_SL2C_STRUCT sl2c;
    ISP_TPIPE_CONFIG_SEEE_STRUCT seee;
    ISP_TPIPE_CONFIG_LCEI_STRUCT lcei;
    ISP_TPIPE_CONFIG_LCE_STRUCT lce;
    ISP_TPIPE_CONFIG_CDRZ_STRUCT cdrz;
    ISP_TPIPE_CONFIG_IMG2O_STRUCT img2o;
    ISP_TPIPE_CONFIG_SRZ_STRUCT srz1;
    ISP_TPIPE_CONFIG_SRZ_STRUCT srz2;
    ISP_TPIPE_CONFIG_FE_STRUCT fe;
    ISP_TPIPE_CONFIG_FEO_STRUCT feo;
    ISP_TPIPE_CONFIG_NR3D_STRUCT nr3d;
    ISP_TPIPE_CONFIG_CRSP_STRUCT crsp;
    ISP_TPIPE_CONFIG_IMG3O_STRUCT img3o;
    ISP_TPIPE_CONFIG_IMG3BO_STRUCT img3bo;
    ISP_TPIPE_CONFIG_IMG3CO_STRUCT img3co;
    ISP_TPIPE_CONFIG_PASS2_STRUCT pass2;
    ISP_DRIVER_CONFIG_STRUCT drvinfo;
}ISP_TPIPE_CONFIG_STRUCT;

typedef struct ISP_TPIPE_INFORMATION_STRUCT
{
    unsigned int pos_xs;/* tpipe start */
    unsigned int pos_xe;/* tpipe end */
    unsigned int pos_ys;/* tpipe start */
    unsigned int pos_ye;/* tpipe end */
    unsigned int tpipe_stop_flag;/* stop flag */
    unsigned int dump_offset_no;/* word offset */
}ISP_TPIPE_INFORMATION_STRUCT;

typedef struct ISP_TPIPE_DESCRIPTOR_STRUCT
{
    unsigned int used_word_no;
    unsigned int total_word_no;
    unsigned int config_no_per_tpipe;
    unsigned int used_tpipe_no;
    unsigned int total_tpipe_no;
    unsigned int horizontal_tpipe_no;
    unsigned int curr_horizontal_tpipe_no;
    unsigned int curr_vertical_tpipe_no;
    unsigned int *tdr_disable_flag;
    unsigned int *last_irq_flag;
    unsigned int *tpipe_config;
    ISP_TPIPE_INFORMATION_STRUCT *tpipe_info;
}ISP_TPIPE_DESCRIPTOR_STRUCT;

extern int tpipe_main_query_platform_working_buffer_size(int tpipe_no);
extern ISP_TPIPE_MESSAGE_ENUM tpipe_main_platform(const ISP_TPIPE_CONFIG_STRUCT *ptr_tpipe_config,
                ISP_TPIPE_DESCRIPTOR_STRUCT *ptr_isp_tpipe_descriptor,
                char *ptr_working_buffer, int buffer_size, void *ptr_isp_reg);
extern ISP_TPIPE_MESSAGE_ENUM tpipe_print_platform_config(
    const ISP_TPIPE_CONFIG_STRUCT *ptr_tpipe_config);
#endif
