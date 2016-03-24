#include "MyUtils.h"

/* define to replace macro */
#define print_error_message(...) dual_cal_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_cal_get_current_file_name(__VA_ARGS__)
/* func prototype */
static const char *dual_cal_print_error_message(ISP_MESSAGE_ENUM n);
static const char *dual_cal_get_current_file_name(const char *filename);
static ISP_MESSAGE_ENUM dual_init_param(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_dmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_obc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_bnr(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_rmm(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_lsc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_bmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_rpg(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_rrz(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_rmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
/* bits field will always calculate by unsigned int */

ISP_MESSAGE_ENUM dual_cal_main(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param,
                               ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param && ptr_out_param)
    {
        result = dual_init_param(ptr_in_param, ptr_out_param);
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_dmx(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_obc(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_bnr(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_rmm(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_lsc(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_bmx(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_rpg(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_rrz(ptr_in_param, ptr_out_param);
        }
        if (ISP_MESSAGE_OK == result)
        {
            result = dual_cal_rmx(ptr_in_param, ptr_out_param);
        }
    }
    else
    {
        result = ISP_MESSAGE_NULL_PTR_ERROR;
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_init_param(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        /* copy default RW in to out */
        //DUAL_HW_LUT(DUAL_COPY_RW_REG, ptr_out_param, ptr_in_param,,,,);
        DUAL_IN_OUT_STRUCT_LUT_HW(DUAL_IN_OUT_COPY, ptr_out_param, ptr_in_param,,,,);
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_dmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        int bin_sel_h_size = ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_E -
            ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_S;
        ptr_out_param->CAM_DMX_CTL.Bits.DMX_EDGE = 0xF;
        /* left side */
        ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X = 0;
        ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X = ((unsigned int)bin_sel_h_size>>1) + (((unsigned int)bin_sel_h_size>>1) & 0x1) +
            (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) +
            (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ:0) - 1;
        /* right side */
        ptr_out_param->CAM_CTL_EN_P1_D.Bits.DMX_D_EN = 1;
        ptr_out_param->CAM_DMX_D_CTL.Bits.DMX_EDGE = 0xF;
        ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X =  ((unsigned int)bin_sel_h_size>>1) + (((unsigned int)bin_sel_h_size>>1) & 0x1) -
            (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) -
            (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ:0);
        ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X = bin_sel_h_size - 1;
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DMX_VSIZE.Bits.DMX_HT, CAM_DMX_D_VSIZE.Bits.DMX_HT);
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_obc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        if (ptr_in_param->CAM_CTL_EN_P1.Bits.OB_EN)
        {
            /* update right by left */
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST0.Bits.OBC_OFST_B, CAM_OBC_D_OFFST0.Bits.OBC_D_OFST_B);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST1.Bits.OBC_OFST_GR, CAM_OBC_D_OFFST1.Bits.OBC_D_OFST_GR);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST2.Bits.OBC_OFST_GB, CAM_OBC_D_OFFST2.Bits.OBC_D_OFST_GB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST3.Bits.OBC_OFST_R, CAM_OBC_D_OFFST3.Bits.OBC_D_OFST_R);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN0.Bits.OBC_GAIN_B, CAM_OBC_D_GAIN0.Bits.OBC_D_GAIN_B);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN1.Bits.OBC_GAIN_GR, CAM_OBC_D_GAIN1.Bits.OBC_D_GAIN_GR);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN2.Bits.OBC_GAIN_GB, CAM_OBC_D_GAIN2.Bits.OBC_D_GAIN_GB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN3.Bits.OBC_GAIN_R, CAM_OBC_D_GAIN3.Bits.OBC_D_GAIN_R);
        }
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.OB_EN, CAM_CTL_EN_P1_D.Bits.OB_D_EN);
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_bnr(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        if (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN)
        {
            int dmx_h_size = ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X - ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
            int dmx_v_size = ptr_in_param->CAM_TG_SEN_GRAB_LIN.Bits.LIN_E - ptr_in_param->CAM_TG_SEN_GRAB_LIN.Bits.LIN_S;
            int dmx_d_h_size = ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X - ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
            /* left side */
            ptr_out_param->CAM_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X;
            ptr_out_param->CAM_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
            ptr_out_param->CAM_BPC_TBLI2.Bits.BPC_XSIZE = dmx_h_size - 1;
            ptr_out_param->CAM_BPC_TBLI2.Bits.BPC_YSIZE = dmx_v_size - 1;
            /* right side */
            ptr_out_param->CAM_BPC_D_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X;
            ptr_out_param->CAM_BPC_D_TBLI1.Bits.BPC_YOFFSET = 0;
            ptr_out_param->CAM_BPC_D_TBLI2.Bits.BPC_XSIZE = dmx_d_h_size - 1;
            ptr_out_param->CAM_BPC_D_TBLI2.Bits.BPC_YSIZE = dmx_v_size - 1;
            /* copy */
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_CON.Bits.BPC_TABLE_END_MODE, CAM_BPC_D_CON.Bits.BPC_TABLE_END_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_CON.Bits.BPC_DTC_MODE, CAM_BPC_D_CON.Bits.BPC_DTC_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_CON.Bits.BPC_CS_MODE, CAM_BPC_D_CON.Bits.BPC_CS_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_CON.Bits.BPC_CRC_MODE, CAM_BPC_D_CON.Bits.BPC_CRC_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH1.Bits.BPC_TH_LWB, CAM_BPC_D_TH1.Bits.BPC_TH_LWB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH1.Bits.BPC_TH_Y, CAM_BPC_D_TH1.Bits.BPC_TH_Y);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH2.Bits.BPC_TH_UPB, CAM_BPC_D_TH2.Bits.BPC_TH_UPB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3.Bits.BPC_TH_XA, CAM_BPC_D_TH3.Bits.BPC_TH_XA);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3.Bits.BPC_TH_XB, CAM_BPC_D_TH3.Bits.BPC_TH_XB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3.Bits.BPC_TH_SLA, CAM_BPC_D_TH3.Bits.BPC_TH_SLA);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3.Bits.BPC_TH_SLB, CAM_BPC_D_TH3.Bits.BPC_TH_SLB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH4.Bits.BPC_DK_TH_XA, CAM_BPC_D_TH4.Bits.BPC_DK_TH_XA);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH4.Bits.BPC_DK_TH_XB, CAM_BPC_D_TH4.Bits.BPC_DK_TH_XB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH4.Bits.BPC_DK_TH_SLA, CAM_BPC_D_TH4.Bits.BPC_DK_TH_SLA);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH4.Bits.BPC_DK_TH_SLB, CAM_BPC_D_TH4.Bits.BPC_DK_TH_SLB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_DTC.Bits.BPC_RNG, CAM_BPC_D_DTC.Bits.BPC_RNG);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_DTC.Bits.BPC_CS_RNG, CAM_BPC_D_DTC.Bits.BPC_CS_RNG);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_DTC.Bits.BPC_CT_LV, CAM_BPC_D_DTC.Bits.BPC_CT_LV);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_DTC.Bits.BPC_TH_MUL, CAM_BPC_D_DTC.Bits.BPC_TH_MUL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_DTC.Bits.BPC_NO_LV, CAM_BPC_D_DTC.Bits.BPC_NO_LV);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_COR.Bits.BPC_DIR_MAX, CAM_BPC_D_COR.Bits.BPC_DIR_MAX);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_COR.Bits.BPC_DIR_TH, CAM_BPC_D_COR.Bits.BPC_DIR_TH);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_COR.Bits.BPC_RANK_IDX, CAM_BPC_D_COR.Bits.BPC_RANK_IDX);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_COR.Bits.BPC_DIR_TH2, CAM_BPC_D_COR.Bits.BPC_DIR_TH2);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_NR1_CON.Bits.NR1_CT_EN, CAM_NR1_D_CON.Bits.NR1_CT_EN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_NR1_CT_CON.Bits.NR1_CT_MD, CAM_NR1_D_CT_CON.Bits.NR1_CT_MD);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_NR1_CT_CON.Bits.NR1_CT_MD2, CAM_NR1_D_CT_CON.Bits.NR1_CT_MD2);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_NR1_CT_CON.Bits.NR1_CT_THRD, CAM_NR1_D_CT_CON.Bits.NR1_CT_THRD);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_NR1_CT_CON.Bits.NR1_MBND, CAM_NR1_D_CT_CON.Bits.NR1_MBND);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_NR1_CT_CON.Bits.NR1_CT_SLOPE, CAM_NR1_D_CT_CON.Bits.NR1_CT_SLOPE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_NR1_CT_CON.Bits.NR1_CT_DIV, CAM_NR1_D_CT_CON.Bits.NR1_CT_DIV);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM1.Bits.BPC_RMM_OSC_TH, CAM_BPC_D_RMM1.Bits.BPC_RMM_OSC_TH);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM1.Bits.BPC_RMM_SEDIR_SL, CAM_BPC_D_RMM1.Bits.BPC_RMM_SEDIR_SL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM1.Bits.BPC_RMM_SEBLD_WD, CAM_BPC_D_RMM1.Bits.BPC_RMM_SEBLD_WD);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM1.Bits.BPC_RMM_LEBLD_WD, CAM_BPC_D_RMM1.Bits.BPC_RMM_LEBLD_WD);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM2.Bits.BPC_RMM_MO_MODE, CAM_BPC_D_RMM2.Bits.BPC_RMM_MO_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM2.Bits.BPC_RMM_MOBLD_FT, CAM_BPC_D_RMM2.Bits.BPC_RMM_MOBLD_FT);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM2.Bits.BPC_RMM_MOTH_RATIO, CAM_BPC_D_RMM2.Bits.BPC_RMM_MOTH_RATIO);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM2.Bits.BPC_RMM_HORI_ADDWT, CAM_BPC_D_RMM2.Bits.BPC_RMM_HORI_ADDWT);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH1_C.Bits.BPC_C_TH_LWB, CAM_BPC_D_TH1_C.Bits.BPC_C_TH_LWB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH1_C.Bits.BPC_C_TH_Y, CAM_BPC_D_TH1_C.Bits.BPC_C_TH_Y);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH2_C.Bits.BPC_C_TH_UPB, CAM_BPC_D_TH2_C.Bits.BPC_C_TH_UPB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3_C.Bits.BPC_C_TH_XA, CAM_BPC_D_TH3_C.Bits.BPC_C_TH_XA);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3_C.Bits.BPC_C_TH_XB, CAM_BPC_D_TH3_C.Bits.BPC_C_TH_XB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3_C.Bits.BPC_C_TH_SLA, CAM_BPC_D_TH3_C.Bits.BPC_C_TH_SLA);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_TH3_C.Bits.BPC_C_TH_SLB, CAM_BPC_D_TH3_C.Bits.BPC_C_TH_SLB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM_REVG_1.Bits.BPC_RMM_REVG_R, CAM_BPC_D_RMM_REVG_1.Bits.BPC_RMM_REVG_R);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM_REVG_1.Bits.BPC_RMM_REVG_GR, CAM_BPC_D_RMM_REVG_1.Bits.BPC_RMM_REVG_GR);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM_REVG_2.Bits.BPC_RMM_REVG_B, CAM_BPC_D_RMM_REVG_2.Bits.BPC_RMM_REVG_B);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM_REVG_2.Bits.BPC_RMM_REVG_GB, CAM_BPC_D_RMM_REVG_2.Bits.BPC_RMM_REVG_GB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM_LEOS.Bits.BPC_RMM_LEOS_GRAY, CAM_BPC_D_RMM_LEOS.Bits.BPC_RMM_LEOS_GRAY);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_RMM_GCNT.Bits.BPC_RMM_GRAY_CNT, CAM_BPC_D_RMM_GCNT.Bits.BPC_RMM_GRAY_CNT);
            /* assign common config, BPCI */
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_OFST_ADDR.Bits.OFFSET_ADDR, CAM_BPCI_D_OFST_ADDR.Bits.OFFSET_ADDR);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_XSIZE.Bits.XSIZE, CAM_BPCI_D_XSIZE.Bits.XSIZE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_YSIZE.Bits.YSIZE, CAM_BPCI_D_YSIZE.Bits.YSIZE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_STRIDE.Bits.STRIDE, CAM_BPCI_D_STRIDE.Bits.STRIDE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_STRIDE.Bits.BUS_SIZE, CAM_BPCI_D_STRIDE.Bits.BUS_SIZE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_STRIDE.Bits.FORMAT, CAM_BPCI_D_STRIDE.Bits.FORMAT);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_STRIDE.Bits.FORMAT_EN, CAM_BPCI_D_STRIDE.Bits.FORMAT_EN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_STRIDE.Bits.BUS_SIZE_EN, CAM_BPCI_D_STRIDE.Bits.BUS_SIZE_EN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_STRIDE.Bits.SWAP, CAM_BPCI_D_STRIDE.Bits.SWAP);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Bits.FIFO_SIZE, CAM_BPCI_D_CON.Bits.FIFO_SIZE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Bits.FIFO_PRI_THRL, CAM_BPCI_D_CON.Bits.FIFO_PRI_THRL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Bits.FIFO_PRI_THRH, CAM_BPCI_D_CON.Bits.FIFO_PRI_THRH);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Bits.FIFO_SIZE_MSB, CAM_BPCI_D_CON.Bits.FIFO_SIZE_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Bits.FIFO_PRI_THRL_MSB, CAM_BPCI_D_CON.Bits.FIFO_PRI_THRL_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Bits.FIFO_PRI_THRH_MSB, CAM_BPCI_D_CON.Bits.FIFO_PRI_THRH_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Bits.MAX_BURST_LEN, CAM_BPCI_D_CON.Bits.MAX_BURST_LEN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON2.Bits.FIFO_PRE_PRI_THRL, CAM_BPCI_D_CON2.Bits.FIFO_PRE_PRI_THRL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON2.Bits.FIFO_PRE_PRI_THRH, CAM_BPCI_D_CON2.Bits.FIFO_PRE_PRI_THRH);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON2.Bits.FIFO_PRE_PRI_THRL_MSB, CAM_BPCI_D_CON2.Bits.FIFO_PRE_PRI_THRL_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON2.Bits.FIFO_PRE_PRI_THRH_MSB, CAM_BPCI_D_CON2.Bits.FIFO_PRE_PRI_THRH_MSB);
        }
        else
        {
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_XSIZE.Bits.XSIZE, CAM_BPCI_D_XSIZE.Bits.XSIZE);
        }
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.BNR_EN, CAM_CTL_EN_P1_D.Bits.BNR_D_EN);
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_CON.Bits.BPC_EN, CAM_BPC_D_CON.Bits.BPC_EN);
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPC_CON.Bits.BPC_LUT_EN, CAM_BPC_D_CON.Bits.BPC_LUT_EN);
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_rmm(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        if (ptr_in_param->CAM_CTL_EN_P1.Bits.RMG_EN)
        {
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_IHDR.Bits.IHDR_GAIN, CAM_CTL_IHDR_D.Bits.IHDR_D_GAIN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_IHDR.Bits.IHDR_MODE, CAM_CTL_IHDR_D.Bits.IHDR_D_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_IHDR.Bits.IHDR_LE_FIRST, CAM_CTL_IHDR_D.Bits.IHDR_D_LE_FIRST);
        }
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.RMG_EN, CAM_CTL_EN_P1_D.Bits.RMG_D_EN);
    }
    return result;
}
static ISP_MESSAGE_ENUM dual_cal_lsc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        if (ptr_in_param->CAM_CTL_EN_P1.Bits.LSC_EN)
        {
            int blkx_start, blkx_end;
            int bit_per_pix = 512;
            /* left side */
            blkx_start = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X)/
                (2*ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
            if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
            {
                blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
            }
            blkx_end = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X)/
                (2*ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
            ptr_out_param->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
            if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
            {
                blkx_end = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
                ptr_out_param->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
            }
            ptr_out_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_XNUM = blkx_end - blkx_start - 1;
            ptr_out_param->CAM_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_X =
                (unsigned int)(ptr_in_param->SW.TWIN_LSC_CROP_OFFX + ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X -
                blkx_start*2*ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH)>>1;
            ptr_out_param->CAM_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_X = ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
                ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
            ptr_out_param->CAM_LSCI_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)(blkx_start*bit_per_pix)>>3;
            ptr_out_param->CAM_LSCI_XSIZE.Bits.XSIZE =  ((unsigned int)((blkx_end - blkx_start)*bit_per_pix)>>3)- 1;

            /* right side */
            blkx_start = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X)/
                (2*ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
            if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
            {
                blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
            }
            blkx_end = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X)/
                (2*ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
            ptr_out_param->CAM_LSC_D_LBLOCK.Bits.LSC_D_SDBLK_lWIDTH = ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
            if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
            {
                blkx_end = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
                ptr_out_param->CAM_LSC_D_LBLOCK.Bits.LSC_D_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
            }
            ptr_out_param->CAM_LSC_D_CTL2.Bits.LSC_D_SDBLK_XNUM = blkx_end - blkx_start - 1;
            ptr_out_param->CAM_LSC_D_TPIPE_OFST.Bits.LSC_D_TPIPE_OFST_X =
                (unsigned int)(ptr_in_param->SW.TWIN_LSC_CROP_OFFX +  ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X -
                blkx_start*2*ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH)>>1;
            ptr_out_param->CAM_LSC_D_TPIPE_SIZE.Bits.LSC_D_TPIPE_SIZE_X =
                ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X - ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
            ptr_out_param->CAM_LSCI_D_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)(blkx_start*bit_per_pix)>>3;
            ptr_out_param->CAM_LSCI_D_XSIZE.Bits.XSIZE =  ((unsigned int)((blkx_end - blkx_start)*bit_per_pix)>>3)- 1;

            /* assign common config, LSCI */
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_YSIZE.Bits.YSIZE, CAM_LSCI_D_YSIZE.Bits.YSIZE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_STRIDE.Bits.STRIDE, CAM_LSCI_D_STRIDE.Bits.STRIDE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_STRIDE.Bits.BUS_SIZE, CAM_LSCI_D_STRIDE.Bits.BUS_SIZE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_STRIDE.Bits.FORMAT, CAM_LSCI_D_STRIDE.Bits.FORMAT);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_STRIDE.Bits.FORMAT_EN, CAM_LSCI_D_STRIDE.Bits.FORMAT_EN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_STRIDE.Bits.BUS_SIZE_EN, CAM_LSCI_D_STRIDE.Bits.BUS_SIZE_EN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_STRIDE.Bits.SWAP, CAM_LSCI_D_STRIDE.Bits.SWAP);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Bits.FIFO_SIZE, CAM_LSCI_D_CON.Bits.FIFO_SIZE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Bits.FIFO_PRI_THRL, CAM_LSCI_D_CON.Bits.FIFO_PRI_THRL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Bits.FIFO_PRI_THRH, CAM_LSCI_D_CON.Bits.FIFO_PRI_THRH);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Bits.FIFO_SIZE_MSB, CAM_LSCI_D_CON.Bits.FIFO_SIZE_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Bits.FIFO_PRI_THRL_MSB, CAM_LSCI_D_CON.Bits.FIFO_PRI_THRL_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Bits.FIFO_PRI_THRH_MSB, CAM_LSCI_D_CON.Bits.FIFO_PRI_THRH_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Bits.MAX_BURST_LEN, CAM_LSCI_D_CON.Bits.MAX_BURST_LEN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON2.Bits.FIFO_PRE_PRI_THRL, CAM_LSCI_D_CON2.Bits.FIFO_PRE_PRI_THRL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON2.Bits.FIFO_PRE_PRI_THRH, CAM_LSCI_D_CON2.Bits.FIFO_PRE_PRI_THRH);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON2.Bits.FIFO_PRE_PRI_THRL_MSB, CAM_LSCI_D_CON2.Bits.FIFO_PRE_PRI_THRL_MSB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON2.Bits.FIFO_PRE_PRI_THRH_MSB, CAM_LSCI_D_CON2.Bits.FIFO_PRE_PRI_THRH_MSB);
            /* assign common config, LSC */
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL1.Bits.SDBLK_XOFST, CAM_LSC_D_CTL1.Bits.SDBLK_XOFST);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL1.Bits.SDBLK_YOFST, CAM_LSC_D_CTL1.Bits.SDBLK_YOFST);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL1.Bits.SD_COEFRD_MODE, CAM_LSC_D_CTL1.Bits.SD_COEFRD_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL1.Bits.SD_ULTRA_MODE, CAM_LSC_D_CTL1.Bits.SD_ULTRA_MODE);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH, CAM_LSC_D_CTL2.Bits.LSC_D_SDBLK_WIDTH);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL2.Bits.LSC_OFLN, CAM_LSC_D_CTL2.Bits.LSC_D_OFLN);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL3.Bits.LSC_SDBLK_HEIGHT, CAM_LSC_D_CTL3.Bits.LSC_D_SDBLK_HEIGHT);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL3.Bits.LSC_SDBLK_YNUM, CAM_LSC_D_CTL3.Bits.LSC_D_SDBLK_YNUM);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lHEIGHT, CAM_LSC_D_LBLOCK.Bits.LSC_D_SDBLK_lHEIGHT);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_RATIO.Bits.LSC_RA3, CAM_LSC_D_RATIO.Bits.LSC_D_RA3);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_RATIO.Bits.LSC_RA2, CAM_LSC_D_RATIO.Bits.LSC_D_RA2);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_RATIO.Bits.LSC_RA1, CAM_LSC_D_RATIO.Bits.LSC_D_RA1);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_RATIO.Bits.LSC_RA0, CAM_LSC_D_RATIO.Bits.LSC_D_RA0);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_Y, CAM_LSC_D_TPIPE_OFST.Bits.LSC_D_TPIPE_OFST_Y);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_Y, CAM_LSC_D_TPIPE_SIZE.Bits.LSC_D_TPIPE_SIZE_Y);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_GAIN_TH.Bits.LSC_GAIN_TH2, CAM_LSC_D_GAIN_TH.Bits.LSC_D_GAIN_TH2);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_GAIN_TH.Bits.LSC_GAIN_TH1, CAM_LSC_D_GAIN_TH.Bits.LSC_D_GAIN_TH1);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_GAIN_TH.Bits.LSC_GAIN_TH0, CAM_LSC_D_GAIN_TH.Bits.LSC_D_GAIN_TH0);
        }
        else
        {
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_XSIZE.Bits.XSIZE, CAM_LSCI_D_XSIZE.Bits.XSIZE);
        }
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.LSC_EN, CAM_CTL_EN_P1_D.Bits.LSC_D_EN);
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_bmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        int dmx_h_size = ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X - ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
        int dmx_d_h_size = ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X - ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
        /* left side */
        ptr_out_param->CAM_BMX_CTL.Bits.BMX_EDGE = 0xF;
        ptr_out_param->CAM_BMX_CROP.Bits.BMX_STR_X = 0;
        ptr_out_param->CAM_BMX_CROP.Bits.BMX_END_X = dmx_h_size -
            (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) -
            (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ:0) - 1;
        ptr_out_param->CAM_BMX_VSIZE.Bits.BMX_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
        /* right side */
        ptr_out_param->CAM_CTL_EN_P1_D.Bits.BMX_D_EN = 1;
        ptr_out_param->CAM_BMX_D_CTL.Bits.BMX_EDGE = 0xF;
        ptr_out_param->CAM_BMX_D_CROP.Bits.BMX_STR_X =  (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ:0) +
            (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0);
        ptr_out_param->CAM_BMX_D_CROP.Bits.BMX_END_X = dmx_d_h_size - 1;
        ptr_out_param->CAM_BMX_D_VSIZE.Bits.BMX_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_rpg(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        if (ptr_in_param->CAM_CTL_EN_P1.Bits.RPG_EN)
        {
            /* assign common config, RPG */
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_SATU_1.Bits.RPG_SATU_B, CAM_RPG_D_SATU_1.Bits.RPG_SATU_B);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_SATU_1.Bits.RPG_SATU_GB, CAM_RPG_D_SATU_1.Bits.RPG_SATU_GB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_SATU_2.Bits.RPG_SATU_GR, CAM_RPG_D_SATU_2.Bits.RPG_SATU_GR);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_SATU_2.Bits.RPG_SATU_R, CAM_RPG_D_SATU_2.Bits.RPG_SATU_R);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_GAIN_1.Bits.RPG_GAIN_B, CAM_RPG_D_GAIN_1.Bits.RPG_GAIN_B);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_GAIN_1.Bits.RPG_GAIN_GB, CAM_RPG_D_GAIN_1.Bits.RPG_GAIN_GB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_GAIN_2.Bits.RPG_GAIN_GR, CAM_RPG_D_GAIN_2.Bits.RPG_GAIN_GR);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_GAIN_2.Bits.RPG_GAIN_R, CAM_RPG_D_GAIN_2.Bits.RPG_GAIN_R);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_OFST_1.Bits.RPG_OFST_B, CAM_RPG_D_OFST_1.Bits.RPG_OFST_B);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_OFST_1.Bits.RPG_OFST_GB, CAM_RPG_D_OFST_1.Bits.RPG_OFST_GB);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_OFST_2.Bits.RPG_OFST_GR, CAM_RPG_D_OFST_2.Bits.RPG_OFST_GR);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_OFST_2.Bits.RPG_OFST_R, CAM_RPG_D_OFST_2.Bits.RPG_OFST_R);
        }
        DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.RPG_EN, CAM_CTL_EN_P1_D.Bits.RPG_D_EN);
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_rrz(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        if (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN)
        {
            if(    ((ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST) > 1)
                || ((ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST) < -1))
            {
                int rrz_in_w = ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_E - ptr_in_param-> CAM_TG_SEN_GRAB_PXL.Bits.PXL_S;
                bool RMX_disable = false;
                bool RMX_D_disable = false;
                if(rrz_in_w < 8)
                {
                    result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }

                if ((ptr_in_param->SW.TWIN_RRZ_OUT_WD <= 1) || (rrz_in_w <= 1 + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST +
                    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST))
                {
                    result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }
                /* left side */
                ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = (long long)(rrz_in_w - 1 -
                    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)*
                    DUAL_RRZ_PREC/(ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1);
                if (ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP < DUAL_RRZ_PREC)
                {
                    result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }
                ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD = ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
                    ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
                /* check RMX or RMX_D disabled */
                if (2*ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST > rrz_in_w)
                {
                    /* RMX disabled */
                    RMX_disable = true;
                    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_OUT size */
                    ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
                    ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
                    /* RMX_D full out */
                    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
                }
                else if (rrz_in_w < 2*ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)
                {
                    /* RMX full out */
                    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
                    ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST;
                    ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
                    /* RMX_D disabled */
                    RMX_D_disable = true;
                    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_D_OUT size */
                    ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
                    ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
                }
                else
                {
                    long long end_temp = ((long long)ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD - 1 -
                        (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) - 4 -
                        ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST)*DUAL_RRZ_PREC - ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
                    int n = (int)(end_temp/ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP);
                    if ((long long)n*ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP == end_temp)
                    {
                        n -= 1;
                    }
                    if (n & 0x1)
                    {
                        ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = n - 1;
                    }
                    else
                    {
                        ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = n;
                    }
                    if (ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD <= 0)
                    {
                        /* RMX disabled */
                        RMX_disable = true;
                        ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_OUT size */
                        ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
                        ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
                        /* RMX_D full out */
                        ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
                    }
                    else
                    {
                        /* RMX enabled */
                        ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST;
                        ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
                        if (ptr_in_param->SW.TWIN_RRZ_OUT_WD > (int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD)
                        {
                            ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD -
                                ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
                        }
                        else
                        {
                            ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
                            /* RMX_D disabled */
                            RMX_D_disable = true;
                            ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_D_OUT size */
                            ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
                            ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
                        }
                    }
                    if (false == RMX_disable)
                    {
                        /* check xe of left RRZ is valid */
                        if ((long long)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X*DUAL_RRZ_PREC <=
                            (long long)(ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1)*
                            ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP +
                            (long long)(PREC_RRZ_RIGHT_TILE_LOSS + (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) +
                            ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST)*DUAL_RRZ_PREC +
                            (long long)ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST)
                        {
                            result = ISP_MESSAGE_RRZ_XE_CAL_ERROR;
                            dual_driver_printf("Error: %s\n", print_error_message(result));
                            return result;
                        }
                    }
                }
                /* right side */
                ptr_out_param->CAM_RRZ_D_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
                ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD = ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X -
                    ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
                if (false == RMX_D_disable)
                {
                    /* RMX_D enabled */
                    long long start_offset;
                    if (false == RMX_disable)/* RMX enabled */
                    {
                        /* check xs of right RRZ is valid */
                        if ((long long)(ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + PREC_RRZ_LEFT_TILE_LOSS +
                            (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0))*DUAL_RRZ_PREC >
                            (long long)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*
                            ptr_out_param->CAM_RRZ_D_HORI_STEP.Bits.RRZ_HORI_STEP +
                            (long long)ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
                            ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST)
                        {
                            result = ISP_MESSAGE_RRZ_D_XS_CAL_ERROR;
                            dual_driver_printf("Error: %s\n", print_error_message(result));
                            return result;
                        }
                        /* RMX enabled */
                        start_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
                            ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST +
                            (long long)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP -
                            (long long)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X*DUAL_RRZ_PREC;
                    }
                    else
                    {
                        /* check xs of right RRZ is valid */
                        if ((long long)(ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + PREC_RRZ_LEFT_TILE_LOSS +
                            (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0))*DUAL_RRZ_PREC >
                            (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
                            ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST)
                        {
                            result = ISP_MESSAGE_RRZ_D_XS_CAL_ERROR;
                            dual_driver_printf("Error: %s\n", print_error_message(result));
                            return result;
                        }
                        /* RMX disabled */
                        start_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
                            ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST -
                            (long long)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X*DUAL_RRZ_PREC;
                    }
                    ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)start_offset>>15);
                    ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(start_offset -
                        (long long)ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
                }
                /* check disable */
                if (RMX_disable && RMX_D_disable)
                {
                    result = ISP_MESSAGE_RRZ_DISABLE_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }
                else
                {
                    ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = RMX_D_disable?1:0;
                    ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = RMX_disable?1:0;
                }
                /* copy from RRZ_IN_HT & RRZ_D_IN_HT from DMX & DMX_D */
                ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
                ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
                /* cal vertical step */
                if (ptr_in_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP)
                {
                    /* RTL just copy */
                    DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP, CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP);
                    DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP, CAM_RRZ_D_VERT_STEP.Bits.RRZ_VERT_STEP);
                }
                else
                {
                    if (ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT <= 1)
                    {
                        result = ISP_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR;
                        dual_driver_printf("Error: %s\n", print_error_message(result));
                        return result;
                    }
                    else
                    {
                        /* re-cal RRZ_VERT_STEP */
                        ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = (long long)(ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT - 1)*
                            DUAL_RRZ_PREC/(ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);
                        if (ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP < DUAL_RRZ_PREC)
                        {
                            result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
                            dual_driver_printf("Error: %s\n", print_error_message(result));
                            return result;
                        }
                        /* copy RRZ_D_VERT_STEP by RRZ_VERT_STEP */
                        ptr_out_param->CAM_RRZ_D_VERT_STEP.Bits.RRZ_VERT_STEP = ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP;
                    }
                }
            }
            else
            {
            int rrz_in_w = ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_E - ptr_in_param-> CAM_TG_SEN_GRAB_PXL.Bits.PXL_S;
            long long temp_offset;
            if ((ptr_in_param->SW.TWIN_RRZ_OUT_WD <= 1) ||
                (rrz_in_w <= 1 + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST +
                ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST))
            {
                result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
                dual_driver_printf("Error: %s\n", print_error_message(result));
                return result;
            }
            /* left side */
                ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = (long long)(rrz_in_w - 1 -
                    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)*
                    DUAL_RRZ_PREC/(ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1);
            if (ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP < DUAL_RRZ_PREC)
            {
                result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
                dual_driver_printf("Error: %s\n", print_error_message(result));
                return result;
            }
            ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD = ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
                ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
            ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = ((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD>>1) +
                (((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD>>1)  & 0x1);
            ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST;
            ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
            /* check xe of left RRZ is valid */
            if ((long long)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X*DUAL_RRZ_PREC <=
                (long long)(ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1)*
                ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP +
                (long long)(PREC_RRZ_RIGHT_TILE_LOSS + (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) +
                ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST)*DUAL_RRZ_PREC +
                (long long)ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST)
            {
                result = ISP_MESSAGE_RRZ_LEFT_SHIFT_ERROR;
                dual_driver_printf("Error: %s\n", print_error_message(result));
                return result;
            }
            /* right side */
            ptr_out_param->CAM_RRZ_D_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
            ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD = ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X -
                ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
            ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD>>1) -
                (((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD>>1)  & 0x1);

            temp_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
                ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST +
                (long long)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP -
                (long long)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X*DUAL_RRZ_PREC;

            ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset>>15);
            ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
                (long long)ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
            /* check xs of right RRZ is valid */
            if ((long long)(ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + PREC_RRZ_LEFT_TILE_LOSS +
                (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0))*DUAL_RRZ_PREC >
                (long long)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*
                ptr_out_param->CAM_RRZ_D_HORI_STEP.Bits.RRZ_HORI_STEP +
                (long long)ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
                (long long)ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST)
            {
                result = ISP_MESSAGE_RRZ_RIGHT_SHIFT_ERROR;
                dual_driver_printf("Error: %s\n", print_error_message(result));
                return result;
            }
            /* copy from RRZ_IN_HT & RRZ_D_IN_HT from DMX & DMX_D */
            ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
            ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
            /* cal vertical step */
            if (ptr_in_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP)
            {
                /* RTL just copy */
                DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP, CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP);
                DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP, CAM_RRZ_D_VERT_STEP.Bits.RRZ_VERT_STEP);
            }
            else
            {
                if (ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT <= 1)
                {
                    result = ISP_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }
                else
                {
                    /* re-cal RRZ_VERT_STEP */
                        ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = (long long)(ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT - 1)*
                            DUAL_RRZ_PREC/(ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);
                    if (ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP < DUAL_RRZ_PREC)
                    {
                        result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
                        dual_driver_printf("Error: %s\n", print_error_message(result));
                        return result;
                    }
                    /* copy RRZ_D_VERT_STEP by RRZ_VERT_STEP */
                    ptr_out_param->CAM_RRZ_D_VERT_STEP.Bits.RRZ_VERT_STEP = ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP;
                }
            }
                ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
                ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
            }

            /*  RRZ RRZ_D output width check. if left side expand, offset needs back extension, if right side expand, it's tail is safe,*/
            /*  crop and output width change in RMX */
            if(ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD < 4)
            {
                int start_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
                    ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST - (long long)(4 - ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD)*
                    ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
                ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = 4;
                ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)start_offset>>15);
                ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(start_offset -
                    (long long)ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
            }
            if (ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD < 4)
            {
                ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = 4;
            }
            /* assign common config, RRZ */
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT, CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_HT);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_CTL.Bits.RRZ_HORI_TBL_SEL, CAM_RRZ_D_CTL.Bits.RRZ_HORI_TBL_SEL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_CTL.Bits.RRZ_VERT_TBL_SEL, CAM_RRZ_D_CTL.Bits.RRZ_VERT_TBL_SEL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_INT_OFST.Bits.RRZ_VERT_INT_OFST, CAM_RRZ_D_VERT_INT_OFST.Bits.RRZ_VERT_INT_OFST);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_SUB_OFST.Bits.RRZ_VERT_SUB_OFST, CAM_RRZ_D_VERT_SUB_OFST.Bits.RRZ_VERT_SUB_OFST);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_TH.Bits.RRZ_TH_MD, CAM_RRZ_D_MODE_TH.Bits.RRZ_TH_MD);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_TH.Bits.RRZ_TH_HI, CAM_RRZ_D_MODE_TH.Bits.RRZ_TH_HI);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_TH.Bits.RRZ_TH_LO, CAM_RRZ_D_MODE_TH.Bits.RRZ_TH_LO);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_TH.Bits.RRZ_TH_MD2, CAM_RRZ_D_MODE_TH.Bits.RRZ_TH_MD2);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_CTL.Bits.RRZ_PRF_BLD, CAM_RRZ_D_MODE_CTL.Bits.RRZ_PRF_BLD);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_CTL.Bits.RRZ_PRF, CAM_RRZ_D_MODE_CTL.Bits.RRZ_PRF);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_CTL.Bits.RRZ_BLD_SL, CAM_RRZ_D_MODE_CTL.Bits.RRZ_BLD_SL);
            DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_CTL.Bits.RRZ_CR_MODE, CAM_RRZ_D_MODE_CTL.Bits.RRZ_CR_MODE);
        }
        else
        {
            ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD = ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
                ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
            ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD;
            ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD = ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X -
                ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
            ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD;
        }
        ptr_out_param->CAM_CTL_EN_P1_D.Bits.RRZ_D_EN = ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN;
    }
    return result;
}

static ISP_MESSAGE_ENUM dual_cal_rmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
    if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
        ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
    {
        /* left side */
        ptr_out_param->CAM_RMX_CTL.Bits.RMX_EDGE = 0xF;
        if((0 == ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_1) &&
            (0 == ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_2) &&
            (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN))
        {
            if (((int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + (int)ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD) >
                ptr_in_param->SW.TWIN_RRZ_OUT_WD)
            {
                if (4 == ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD)
                {
                    ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 2;
                    ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = 3;
                    ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
                    ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD - 1;
                }
                else if (4 == ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD)
                {
                    ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
                    ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = 1;
        ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 0;
        ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
                }
                else
                {
                    result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }
            }
            else
            {
                ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 0;
                ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
                ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
                ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD - 1;
            }
        }
        else
        {
            ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 0;
            ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
            ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
            ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD - 1;
        }
        ptr_out_param->CAM_RMX_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
        /* right side */
        ptr_out_param->CAM_CTL_EN_P1_D.Bits.RMX_D_EN = 1;
        ptr_out_param->CAM_RMX_D_CTL.Bits.RMX_EDGE = 0xF;
        ptr_out_param->CAM_RMX_D_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
    }
    return result;
}

static const char *dual_cal_print_error_message(ISP_MESSAGE_ENUM n)
{
    GET_ERROR_NAME(n);
}

static const char *dual_cal_get_current_file_name(const char *filename)
{
    char *ptr = strrchr((char *)filename, FOLDER_SYMBOL_CHAR);
    if (NULL == ptr)
    {
        return filename;
    }
    else
    {
        return (const char *)(ptr + 1);
    }
}
