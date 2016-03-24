#ifndef _ISP_REG_H_
#define _ISP_REG_H_

// ----------------- CAM Bit Field Definitions -------------------

#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.Raw)
#define ISP_BASE_OFFSET 0x4000
//#define ISP_BASE_HW     0x15004000
#define ISP_BASE_HW     0x15000000
//#define ISP_BASE_RANGE  0x10000
#define ISP_BASE_RANGE  0xA000

typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* start MT6593_000_cam_ctl.xml*/
typedef volatile union _CAM_REG_CTL_START_
{
    volatile struct
    {
        FIELD PASS2_START               : 1;
        FIELD PASS2B_START              : 1;
        FIELD rsv_2                     : 2;
        FIELD PASS2C_START              : 1;
        FIELD CQ0_START                 : 1;
        FIELD CQ0B_START                : 1;
        FIELD rsv_7                     : 1;
        FIELD LSC_PREFETCH_TRIG         : 1;
        FIELD LSC_PREFETCH_TRIG_D       : 1;
        FIELD rsv_10                    : 2;
        FIELD CQ0_D_START               : 1;
        FIELD CQ0B_D_START              : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_START;

typedef volatile union _CAM_REG_CTL_EN_P1_
{
    volatile struct
    {
        FIELD TG1_EN                    : 1;
        FIELD DMX_EN                    : 1;
        FIELD WBN_EN                    : 1;
        FIELD W2G_EN                    : 1;
        FIELD OB_EN                     : 1;
        FIELD BNR_EN                    : 1;
        FIELD LSC_EN                    : 1;
        FIELD RPG_EN                    : 1;
        FIELD RRZ_EN                    : 1;
        FIELD RMX_EN                    : 1;
        FIELD PAKG_EN                   : 1;
        FIELD BMX_EN                    : 1;
        FIELD PAK_EN                    : 1;
        FIELD UFE_EN                    : 1;
        FIELD LCS_EN                    : 1;
        FIELD SGG1_EN                   : 1;
        FIELD AF_EN                     : 1;
        FIELD FLK_EN                    : 1;
        FIELD HBIN_EN                   : 1;
        FIELD AA_EN                     : 1;
        FIELD SGG2_EN                   : 1;
        FIELD EIS_EN                    : 1;
        FIELD RMG_EN                    : 1;
        FIELD CPG_EN                    : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN_P1;

typedef volatile union _CAM_REG_CTL_EN_P1_DMA_
{
    volatile struct
    {
        FIELD IMGO_EN                   : 1;
        FIELD UFEO_EN                   : 1;
        FIELD RRZO_EN                   : 1;
        FIELD ESFKO_EN                  : 1;
        FIELD LCSO_EN                   : 1;
        FIELD AAO_EN                    : 1;
        FIELD BPCI_EN                   : 1;
        FIELD LSCI_EN                   : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN_P1_DMA;

typedef volatile union _CAM_REG_CTL_EN_P1_D_
{
    volatile struct
    {
        FIELD TG1_D_EN                  : 1;
        FIELD DMX_D_EN                  : 1;
        FIELD WBN_D_EN                  : 1;
        FIELD W2G_D_EN                  : 1;
        FIELD OB_D_EN                   : 1;
        FIELD BNR_D_EN                  : 1;
        FIELD LSC_D_EN                  : 1;
        FIELD RPG_D_EN                  : 1;
        FIELD RRZ_D_EN                  : 1;
        FIELD RMX_D_EN                  : 1;
        FIELD PAKG_D_EN                 : 1;
        FIELD BMX_D_EN                  : 1;
        FIELD PAK_D_EN                  : 1;
        FIELD rsv_13                    : 1;
        FIELD LCS_D_EN                  : 1;
        FIELD SGG1_D_EN                 : 1;
        FIELD AF_D_EN                   : 1;
        FIELD rsv_17                    : 2;
        FIELD AA_D_EN                   : 1;
        FIELD rsv_20                    : 2;
        FIELD RMG_D_EN                  : 1;
        FIELD CPG_D_EN                  : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN_P1_D;

typedef volatile union _CAM_REG_CTL_EN_P1_DMA_D_
{
    volatile struct
    {
        FIELD IMGO_D_EN                 : 1;
        FIELD rsv_1                     : 1;
        FIELD RRZO_D_EN                 : 1;
        FIELD AFO_D_EN                  : 1;
        FIELD LCSO_D_EN                 : 1;
        FIELD AAO_D_EN                  : 1;
        FIELD BPCI_D_EN                 : 1;
        FIELD LSCI_D_EN                 : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN_P1_DMA_D;

typedef volatile union _CAM_REG_CTL_EN_P2_
{
    volatile struct
    {
        FIELD UFD_EN                    : 1;
        FIELD PGN_EN                    : 1;
        FIELD SL2_EN                    : 1;
        FIELD CFA_EN                    : 1;
        FIELD CCL_EN                    : 1;
        FIELD G2G_EN                    : 1;
        FIELD GGM_EN                    : 1;
        FIELD MFB_EN                    : 1;
        FIELD C24_EN                    : 1;
        FIELD G2C_EN                    : 1;
        FIELD C42_EN                    : 1;
        FIELD NBC_EN                    : 1;
        FIELD PCA_EN                    : 1;
        FIELD SEEE_EN                   : 1;
        FIELD LCE_EN                    : 1;
        FIELD NR3D_EN                   : 1;
        FIELD SL2B_EN                   : 1;
        FIELD SL2C_EN                   : 1;
        FIELD SRZ1_EN                   : 1;
        FIELD SRZ2_EN                   : 1;
        FIELD CRZ_EN                    : 1;
        FIELD MIX1_EN                   : 1;
        FIELD MIX2_EN                   : 1;
        FIELD MIX3_EN                   : 1;
        FIELD rsv_24                    : 2;
        FIELD CRSP_EN                   : 1;
        FIELD C24B_EN                   : 1;
        FIELD MDPCROP_EN                : 1;
        FIELD FE_EN                     : 1;
        FIELD UNP_EN                    : 1;
        FIELD C02_EN                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN_P2;

typedef volatile union _CAM_REG_CTL_EN_P2_DMA_
{
    volatile struct
    {
        FIELD IMGI_EN                   : 1;
        FIELD UFDI_EN                   : 1;
        FIELD VIPI_EN                   : 1;
        FIELD VIP2I_EN                  : 1;
        FIELD VIP3I_EN                  : 1;
        FIELD LCEI_EN                   : 1;
        FIELD MFBO_EN                   : 1;
        FIELD IMG2O_EN                  : 1;
        FIELD IMG3O_EN                  : 1;
        FIELD IMG3BO_EN                 : 1;
        FIELD IMG3CO_EN                 : 1;
        FIELD FEO_EN                    : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_EN_P2_DMA;

typedef volatile union _CAM_REG_CTL_CQ_EN_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD CQ0A_MASK_CQ0B_D_EN       : 1;
        FIELD CQ0A_MASK_CQ0C_D_EN       : 1;
        FIELD CQ0_CONT_D                : 1;
        FIELD CQ0B_CONT_D               : 1;
        FIELD CQ0C_IMGO_D_SEL           : 1;
        FIELD CQ0B_D_SEL                : 1;
        FIELD CQ0_D_MODE                : 1;
        FIELD CQ0C_D_RRZO_SEL           : 1;
        FIELD CQ0B_D_MODE               : 1;
        FIELD CQ0C_D_EN                 : 1;
        FIELD CQ0B_D_EN                 : 1;
        FIELD rsv_12                    : 3;
        FIELD CQ0_D_EN                  : 1;
        FIELD CQ_APB_2T                 : 1;
        FIELD CQ0A_MASK_CQ0B_EN         : 1;
        FIELD CQ0A_MASK_CQ0C_EN         : 1;
        FIELD CQ0_CONT                  : 1;
        FIELD CQ0B_CONT                 : 1;
        FIELD CQ0C_IMGO_SEL             : 1;
        FIELD CQ0B_SEL                  : 1;
        FIELD CQ0_MODE                  : 1;
        FIELD CQ0C_RRZO_SEL             : 1;
        FIELD CQ0B_MODE                 : 1;
        FIELD CQ0C_EN                   : 1;
        FIELD CQ0B_EN                   : 1;
        FIELD CQ0_EN                    : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ_EN;

typedef volatile union _CAM_REG_CTL_SCENARIO_
{
    volatile struct
    {
        FIELD SCENARIO                  : 3;
        FIELD rsv_3                     : 1;
        FIELD SUB_MODE                  : 3;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SCENARIO;

typedef volatile union _CAM_REG_CTL_FMT_SEL_P1_
{
    volatile struct
    {
        FIELD PIX_ID                    : 2;
        FIELD rsv_2                     : 2;
        FIELD PAKG_FMT                  : 2;
        FIELD rsv_6                     : 6;
        FIELD OUT_FMT                   : 4;
        FIELD TG1_FMT                   : 3;
        FIELD rsv_19                    : 4;
        FIELD TWO_PIX_BMX               : 1;
        FIELD TWO_PIX                   : 1;
        FIELD TWO_PIX_RMX               : 1;
        FIELD TWO_PIX_DMX               : 1;
        FIELD rsv_27                    : 1;
        FIELD TG1_SW                    : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_FMT_SEL_P1;

typedef volatile union _CAM_REG_CTL_FMT_SEL_P1_D_
{
    volatile struct
    {
        FIELD PIX_ID_D                  : 2;
        FIELD rsv_2                     : 2;
        FIELD PAKG_FMT_D                : 2;
        FIELD rsv_6                     : 6;
        FIELD OUT_FMT_D                 : 4;
        FIELD TG1_FMT_D                 : 3;
        FIELD rsv_19                    : 4;
        FIELD TWO_PIX_BMX_D             : 1;
        FIELD TWO_PIX_D                 : 1;
        FIELD TWO_PIX_RMX_D             : 1;
        FIELD TWO_PIX_DMX_D             : 1;
        FIELD rsv_27                    : 1;
        FIELD TG1_SW_D                  : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_FMT_SEL_P1_D;

typedef volatile union _CAM_REG_CTL_FMT_SEL_P2_
{
    volatile struct
    {
        FIELD IN_FMT                    : 4;
        FIELD IMG3O_FMT                 : 3;
        FIELD rsv_7                     : 1;
        FIELD VIPI_FMT                  : 3;
        FIELD rsv_11                    : 1;
        FIELD UFDI_FMT                  : 1;
        FIELD rsv_13                    : 1;
        FIELD PIX_ID_P2                 : 2;
        FIELD FG_MODE                   : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_FMT_SEL_P2;

typedef volatile union _CAM_REG_CTL_SEL_P1_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD SGG_SEL                   : 2;
        FIELD SGG_SEL_EN                : 1;
        FIELD LCS_SEL                   : 1;
        FIELD LCS_SEL_EN                : 1;
        FIELD IMG_SEL                   : 1;
        FIELD rsv_7                     : 1;
        FIELD UFE_SEL                   : 2;
        FIELD EIS_SEL                   : 2;
        FIELD W2G_SEL                   : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_P1;

typedef volatile union _CAM_REG_CTL_SEL_P1_D_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD SGG_SEL_D                 : 2;
        FIELD SGG_SEL_EN_D              : 1;
        FIELD LCS_SEL_D                 : 1;
        FIELD LCS_SEL_EN_D              : 1;
        FIELD IMG_SEL_D                 : 1;
        FIELD rsv_7                     : 1;
        FIELD UFE_SEL_D                 : 2;
        FIELD EIS_SEL_D                 : 2;
        FIELD W2G_SEL_D                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_P1_D;

typedef volatile union _CAM_REG_CTL_SEL_P2_
{
    volatile struct
    {
        FIELD CCL_SEL                   : 1;
        FIELD CCL_SEL_EN                : 1;
        FIELD G2G_SEL                   : 1;
        FIELD G2G_SEL_EN                : 1;
        FIELD UFD_SEL                   : 1;
        FIELD C24_SEL                   : 1;
        FIELD SRZ1_SEL                  : 1;
        FIELD MIX1_SEL                  : 1;
        FIELD CRZ_SEL                   : 2;
        FIELD NR3D_SEL                  : 1;
        FIELD MDP_SEL                   : 1;
        FIELD FE_SEL                    : 1;
        FIELD PCA_SEL                   : 1;
        FIELD MFB_MODE                  : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_P2;

typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_
{
    volatile struct
    {
        FIELD PASS1_DB_EN               : 1;
        FIELD PASS1_D_DB_EN             : 1;
        FIELD rsv_2                     : 1;
        FIELD APB_CLK_GATE_BYPASS       : 1;
        FIELD rsv_4                     : 1;
        FIELD DUAL_MODE                 : 1;
        FIELD BIN_SEL                   : 1;
        FIELD BIN_SEL_EN                : 1;
        FIELD PREGAIN_SEL               : 1;
        FIELD PREGAIN_SEL_EN            : 1;
        FIELD LSC_EARLY_SOF_EN          : 1;
        FIELD W2G_MODE                  : 1;
        FIELD rsv_12                    : 2;
        FIELD INT_MRG                   : 1;
        FIELD BIN_SEL_D                 : 1;
        FIELD BIN_SEL_EN_D              : 1;
        FIELD PREGAIN_SEL_D             : 1;
        FIELD PREGAIN_SEL_EN_D          : 1;
        FIELD LSC_EARLY_SOF_EN_D        : 1;
        FIELD W2G_MODE_D                : 1;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_GLOBAL;

typedef volatile union _CAM_REG_CTL_INT_P1_EN_
{
    volatile struct
    {
        FIELD VS1_INT_EN                : 1;
        FIELD TG1_INT1_EN               : 1;
        FIELD TG1_INT2_EN               : 1;
        FIELD EXPDON1_EN                : 1;
        FIELD TG1_ERR_EN                : 1;
        FIELD TG1_GBERR_EN              : 1;
        FIELD CQ0_ERR_EN                : 1;
        FIELD CQ0_VS_ERR_EN             : 1;
        FIELD IMGO_DROP_FRAME_EN        : 1;
        FIELD RRZO_DROP_FRAME_EN        : 1;
        FIELD PASS1_DON_EN              : 1;
        FIELD rsv_11                    : 1;
        FIELD SOF1_INT_EN               : 1;
        FIELD rsv_13                    : 3;
        FIELD AF_DON_EN                 : 1;
        FIELD FLK_DON_EN                : 1;
        FIELD rsv_18                    : 2;
        FIELD IMGO_ERR_EN               : 1;
        FIELD AAO_ERR_EN                : 1;
        FIELD LCSO_ERR_EN               : 1;
        FIELD RRZO_ERR_EN               : 1;
        FIELD ESFKO_ERR_EN              : 1;
        FIELD FLK_ERR_EN                : 1;
        FIELD LSC_ERR_EN                : 1;
        FIELD FBC_RRZO_DON_EN           : 1;
        FIELD FBC_IMGO_DON_EN           : 1;
        FIELD rsv_29                    : 1;
        FIELD DMA_ERR_EN                : 1;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_EN;

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS_
{
    volatile struct
    {
        FIELD VS1_INT_ST                : 1;
        FIELD TG1_INT1_ST               : 1;
        FIELD TG1_INT2_ST               : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG1_ERR_ST                : 1;
        FIELD TG1_GBERR                 : 1;
        FIELD CQ0_ERR                   : 1;
        FIELD CQ0_VS_ERR_ST             : 1;
        FIELD IMGO_DROP_FRAME_ST        : 1;
        FIELD RRZO_DROP_FRAME_ST        : 1;
        FIELD PASS1_DON_ST              : 1;
        FIELD rsv_11                    : 1;
        FIELD SOF1_INT_ST               : 1;
        FIELD rsv_13                    : 3;
        FIELD AF_DON_ST                 : 1;
        FIELD FLK_DON_ST                : 1;
        FIELD rsv_18                    : 2;
        FIELD IMGO_ERR_ST               : 1;
        FIELD AAO_ERR_ST                : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD RRZO_ERR_ST               : 1;
        FIELD ESFKO_ERR_ST              : 1;
        FIELD FLK_ERR_ST                : 1;
        FIELD LSC_ERR_ST                : 1;
        FIELD FBC_RRZO_DON_ST           : 1;
        FIELD FBC_IMGO_DON_ST           : 1;
        FIELD rsv_29                    : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_STATUS;

typedef volatile union _CAM_REG_CTL_INT_P1_EN2_
{
    volatile struct
    {
        FIELD IMGO_DONE_ST_EN           : 1;
        FIELD UFEO_DONE_ST_EN           : 1;
        FIELD RRZO_DONE_ST_EN           : 1;
        FIELD ESFKO_DONE_ST_EN          : 1;
        FIELD LCSO_DONE_ST_EN           : 1;
        FIELD AAO_DONE_ST_EN            : 1;
        FIELD BPCI_DONE_ST_EN           : 1;
        FIELD LSCI_DONE_ST_EN           : 1;
        FIELD AF_TAR_DONE_ST_EN         : 1;
        FIELD AF_FLO1_DONE_ST_EN        : 1;
        FIELD AF_FLO2_DONE_ST_EN        : 1;
        FIELD AF_FLO3_DONE_ST_EN        : 1;
        FIELD CQ0_DONE_ST_EN            : 1;
        FIELD CQ0B_DONE_ST_EN           : 1;
        FIELD CQ0C_DONE_ST_EN           : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_EN2;

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS2_
{
    volatile struct
    {
        FIELD IMGO_DONE_ST              : 1;
        FIELD UFEO_DONE_ST              : 1;
        FIELD RRZO_DONE_ST              : 1;
        FIELD ESFKO_DONE_ST             : 1;
        FIELD LCSO_DONE_ST              : 1;
        FIELD AAO_DONE_ST               : 1;
        FIELD BPCI_DONE_ST              : 1;
        FIELD LSCI_DONE_ST              : 1;
        FIELD AF_TAR_DONE_ST            : 1;
        FIELD AF_FLO1_DONE_ST           : 1;
        FIELD AF_FLO2_DONE_ST           : 1;
        FIELD AF_FLO3_DONE_ST           : 1;
        FIELD CQ0_DONE_ST               : 1;
        FIELD CQ0B_DONE_ST              : 1;
        FIELD CQ0C_DONE_ST              : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_STATUS2;

typedef volatile union _CAM_REG_CTL_INT_P1_EN_D_
{
    volatile struct
    {
        FIELD VS1_INT_EN                : 1;
        FIELD TG1_INT1_EN               : 1;
        FIELD TG1_INT2_EN               : 1;
        FIELD EXPDON1_EN                : 1;
        FIELD TG1_ERR_EN                : 1;
        FIELD TG1_GBERR_EN              : 1;
        FIELD CQ0_ERR_EN                : 1;
        FIELD CQ0_VS_ERR_EN             : 1;
        FIELD IMGO_DROP_FRAME_EN        : 1;
        FIELD RRZO_DROP_FRAME_EN        : 1;
        FIELD PASS1_DON_EN              : 1;
        FIELD rsv_11                    : 1;
        FIELD SOF1_INT_EN               : 1;
        FIELD rsv_13                    : 3;
        FIELD AF_DON_EN                 : 1;
        FIELD rsv_17                    : 3;
        FIELD IMGO_ERR_EN               : 1;
        FIELD AAO_ERR_EN                : 1;
        FIELD LCSO_ERR_EN               : 1;
        FIELD RRZO_ERR_EN               : 1;
        FIELD AFO_ERR_EN                : 1;
        FIELD rsv_25                    : 1;
        FIELD LSC_ERR_EN                : 1;
        FIELD FBC_RRZO_DON_EN           : 1;
        FIELD FBC_IMGO_DON_EN           : 1;
        FIELD rsv_29                    : 1;
        FIELD DMA_ERR_EN                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_EN_D;

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS_D_
{
    volatile struct
    {
        FIELD VS1_INT_ST                : 1;
        FIELD TG1_INT1_ST               : 1;
        FIELD TG1_INT2_ST               : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG1_ERR_ST                : 1;
        FIELD TG1_GBERR                 : 1;
        FIELD CQ0_ERR                   : 1;
        FIELD CQ0_VS_ERR_ST             : 1;
        FIELD IMGO_DROP_FRAME_ST        : 1;
        FIELD RRZO_DROP_FRAME_ST        : 1;
        FIELD PASS1_DON_ST              : 1;
        FIELD rsv_11                    : 1;
        FIELD SOF1_INT_ST               : 1;
        FIELD rsv_13                    : 3;
        FIELD AF_DON_ST                 : 1;
        FIELD rsv_17                    : 3;
        FIELD IMGO_ERR_ST               : 1;
        FIELD AAO_ERR_ST                : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD RRZO_ERR_ST               : 1;
        FIELD AFO_ERR_ST                : 1;
        FIELD rsv_25                    : 1;
        FIELD LSC_ERR_ST                : 1;
        FIELD FBC_RRZO_DON_ST           : 1;
        FIELD FBC_IMGO_DON_ST           : 1;
        FIELD rsv_29                    : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_STATUS_D;

typedef volatile union _CAM_REG_CTL_INT_P1_EN2_D_
{
    volatile struct
    {
        FIELD IMGO_D_DONE_ST_EN         : 1;
        FIELD rsv_1                     : 1;
        FIELD RRZO_D_DONE_ST_EN         : 1;
        FIELD AFO_D_DONE_ST_EN          : 1;
        FIELD LCSO_D_DONE_ST_EN         : 1;
        FIELD AAO_D_DONE_ST_EN          : 1;
        FIELD BPCI_D_DONE_ST_EN         : 1;
        FIELD LSCI_D_DONE_ST_EN         : 1;
        FIELD AF_TAR_DONE_EN            : 1;
        FIELD AF_FLO1_DONE_EN           : 1;
        FIELD AF_FLO2_DONE_EN           : 1;
        FIELD AF_FLO3_DONE_EN           : 1;
        FIELD CQ0_DONE_ST_EN            : 1;
        FIELD CQ0B_DONE_ST_EN           : 1;
        FIELD CQ0C_DONE_ST_EN           : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_EN2_D;

typedef volatile union _CAM_REG_CTL_INT_P1_STATUS2_D_
{
    volatile struct
    {
        FIELD IMGO_D_DONE_ST            : 1;
        FIELD rsv_1                     : 1;
        FIELD RRZO_D_DONE_ST            : 1;
        FIELD AFO_D_DONE_ST             : 1;
        FIELD LCSO_D_DONE_ST            : 1;
        FIELD AAO_D_DONE_ST             : 1;
        FIELD BPCI_D_DONE_ST            : 1;
        FIELD LSCI_D_DONE_ST            : 1;
        FIELD AF_TAR_DONE_ST            : 1;
        FIELD AF_FLO1_DONE_ST           : 1;
        FIELD AF_FLO2_DONE_ST           : 1;
        FIELD AF_FLO3_DONE_ST           : 1;
        FIELD CQ0_DONE_ST               : 1;
        FIELD CQ0B_DONE_ST              : 1;
        FIELD CQ0C_DONE_ST              : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P1_STATUS2_D;

typedef volatile union _CAM_REG_CTL_INT_P2_EN_
{
    volatile struct
    {
        FIELD CQ_ERR_EN                 : 1;
        FIELD PASS2_DON_EN              : 1;
        FIELD TILE_DON_EN               : 1;
        FIELD CQ_DON_EN                 : 1;
        FIELD TDR_ERR_EN                : 1;
        FIELD PASS2A_DONE_ST_EN         : 1;
        FIELD PASS2B_DONE_ST_EN         : 1;
        FIELD PASS2C_DONE_ST_EN         : 1;
        FIELD CQ1_DONE_ST_EN            : 1;
        FIELD CQ2_DONE_ST_EN            : 1;
        FIELD CQ3_DONE_ST_EN            : 1;
        FIELD PASS2A_ERR_TRIG_ST_EN     : 1;
        FIELD PASS2B_ERR_TRIG_ST_EN     : 1;
        FIELD PASS2C_ERR_TRIG_ST_EN     : 1;
        FIELD rsv_14                    : 1;
        FIELD IMGI_DONE_ST_EN           : 1;
        FIELD UFDI_DONE_ST_EN           : 1;
        FIELD VIPI_DONE_ST_EN           : 1;
        FIELD VIP2I_DONE_ST_EN          : 1;
        FIELD VIP3I_DONE_ST_EN          : 1;
        FIELD LCEI_DONE_ST_EN           : 1;
        FIELD MFBO_DONE_ST_EN           : 1;
        FIELD IMG2O_DONE_ST_EN          : 1;
        FIELD IMG3O_DONE_ST_EN          : 1;
        FIELD IMG3BO_DONE_ST_EN         : 1;
        FIELD IMG3CO_DONE_ST_EN         : 1;
        FIELD FEO_DONE_ST_EN            : 1;
        FIELD DMA_ERR_EN                : 1;
        FIELD GGM_COLLISION_ST_EN       : 1;
        FIELD PCA_COLLISION_ST_EN       : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P2_EN;

typedef volatile union _CAM_REG_CTL_INT_P2_STATUS_
{
    volatile struct
    {
        FIELD CQ_ERR_ST                 : 1;
        FIELD PASS2_DON_ST              : 1;
        FIELD TILE_DON_ST               : 1;
        FIELD CQ_DON_ST                 : 1;
        FIELD TDR_ERR_ST                : 1;
        FIELD PASS2A_DONE_ST            : 1;
        FIELD PASS2B_DONE_ST            : 1;
        FIELD PASS2C_DONE_ST            : 1;
        FIELD CQ1_DONE_ST               : 1;
        FIELD CQ2_DONE_ST               : 1;
        FIELD CQ3_DONE_ST               : 1;
        FIELD PASS2A_ERR_TRIG_ST        : 1;
        FIELD PASS2B_ERR_TRIG_ST        : 1;
        FIELD PASS2C_ERR_TRIG_ST        : 1;
        FIELD rsv_14                    : 1;
        FIELD IMGI_DONE_ST              : 1;
        FIELD UFDI_DONE_ST              : 1;
        FIELD VIPI_DONE_ST              : 1;
        FIELD VIP2I_DONE_ST             : 1;
        FIELD VIP3I_DONE_ST             : 1;
        FIELD LCEI_DONE_ST              : 1;
        FIELD MFBO_DONE_ST              : 1;
        FIELD IMG2O_DONE_ST             : 1;
        FIELD IMG3O_DONE_ST             : 1;
        FIELD IMG3BO_DONE_ST            : 1;
        FIELD IMG3CO_DONE_ST            : 1;
        FIELD FEO_DONE_ST               : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD GGM_COLLISION_ST          : 1;
        FIELD PCA_COLLISION_ST          : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_P2_STATUS;

typedef volatile union _CAM_REG_CTL_INT_STATUSX_
{
    volatile struct
    {
        FIELD VS1_INT_ST                : 1;
        FIELD TG1_INT1_ST               : 1;
        FIELD TG1_INT2_ST               : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG1_ERR_ST                : 1;
        FIELD TG1_GBERR                 : 1;
        FIELD CQ0_ERR                   : 1;
        FIELD CQ0_VS_ERR_ST             : 1;
        FIELD IMGO_DROP_FRAME_ST        : 1;
        FIELD RRZO_DROP_FRAME_ST        : 1;
        FIELD PASS1_DON_ST              : 1;
        FIELD rsv_11                    : 1;
        FIELD SOF1_INT_ST               : 1;
        FIELD CQ_ERR_ST                 : 1;
        FIELD PASS2_DON_ST              : 1;
        FIELD TILE_DON_ST               : 1;
        FIELD AF_DON_ST                 : 1;
        FIELD FLK_DON_ST                : 1;
        FIELD rsv_18                    : 1;
        FIELD CQ_DON_ST                 : 1;
        FIELD IMGO_ERR_ST               : 1;
        FIELD AAO_ERR_ST                : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD RRZO_ERR_ST               : 1;
        FIELD ESFKO_ERR_ST              : 1;
        FIELD FLK_ERR_ST                : 1;
        FIELD LSC_ERR_ST                : 1;
        FIELD FBC_RRZO_DON_ST           : 1;
        FIELD FBC_IMGO_DON_ST           : 1;
        FIELD rsv_29                    : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_STATUSX;

typedef volatile union _CAM_REG_CTL_INT_STATUS2X_
{
    volatile struct
    {
        FIELD VS1_INT_ST                : 1;
        FIELD TG1_INT1_ST               : 1;
        FIELD TG1_INT2_ST               : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG1_ERR_ST                : 1;
        FIELD TG1_GBERR                 : 1;
        FIELD CQ0_ERR                   : 1;
        FIELD CQ0_VS_ERR_ST             : 1;
        FIELD IMGO_DROP_FRAME_ST        : 1;
        FIELD RRZO_DROP_FRAME_ST        : 1;
        FIELD PASS1_DON_ST              : 1;
        FIELD rsv_11                    : 1;
        FIELD SOF1_INT_ST               : 1;
        FIELD rsv_13                    : 3;
        FIELD AF_DON_ST                 : 1;
        FIELD rsv_17                    : 3;
        FIELD IMGO_ERR_ST               : 1;
        FIELD AAO_ERR_ST                : 1;
        FIELD LCSO_ERR_ST               : 1;
        FIELD RRZO_ERR_ST               : 1;
        FIELD AFO_ERR_ST                : 1;
        FIELD rsv_25                    : 1;
        FIELD LSC_ERR_ST                : 1;
        FIELD FBC_RRZO_DON_ST           : 1;
        FIELD FBC_IMGO_DON_ST           : 1;
        FIELD rsv_29                    : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_STATUS2X;

typedef volatile union _CAM_REG_CTL_INT_STATUS3X_
{
    volatile struct
    {
        FIELD IMGO_DONE_ST              : 1;
        FIELD UFEO_DONE_ST              : 1;
        FIELD RRZO_DONE_ST              : 1;
        FIELD ESFKO_DONE_ST             : 1;
        FIELD LCSO_DONE_ST              : 1;
        FIELD AAO_DONE_ST               : 1;
        FIELD BPCI_DONE_ST              : 1;
        FIELD LSCI_DONE_ST              : 1;
        FIELD IMGO_D_DONE_ST            : 1;
        FIELD RRZO_D_DONE_ST            : 1;
        FIELD AFO_D_DONE_ST             : 1;
        FIELD LCSO_D_DONE_ST            : 1;
        FIELD AAO_D_DONE_ST             : 1;
        FIELD BPCI_D_DONE_ST            : 1;
        FIELD LSCI_D_DONE_ST            : 1;
        FIELD IMGI_DONE_ST              : 1;
        FIELD UFDI_DONE_ST              : 1;
        FIELD VIPI_DONE_ST              : 1;
        FIELD VIP2I_DONE_ST             : 1;
        FIELD VIP3I_DONE_ST             : 1;
        FIELD LCEI_DONE_ST              : 1;
        FIELD MFBO_DONE_ST              : 1;
        FIELD IMG2O_DONE_ST             : 1;
        FIELD IMG3O_DONE_ST             : 1;
        FIELD IMG3BO_DONE_ST            : 1;
        FIELD IMG3CO_DONE_ST            : 1;
        FIELD FEO_DONE_ST               : 1;
        FIELD DMA_ERR_ST                : 1;
        FIELD GGM_COLLISION_ST          : 1;
        FIELD PCA_COLLISION_ST          : 1;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_INT_STATUS3X;

typedef volatile union _CAM_REG_CTL_TILE_
{
    volatile struct
    {
        FIELD TILE_EDGE                 : 4;
        FIELD TILE_IRQ                  : 1;
        FIELD LAST_TILE                 : 1;
        FIELD C02_TCM_EN                : 1;
        FIELD C24B_TCM_EN               : 1;
        FIELD CRZ_EDGE                  : 4;
        FIELD LCEI_TCM_EN               : 1;
        FIELD UFDI_TCM_EN               : 1;
        FIELD VIPI_TCM_EN               : 1;
        FIELD VIP2I_TCM_EN              : 1;
        FIELD VIP3I_TCM_EN              : 1;
        FIELD FEO_TCM_EN                : 1;
        FIELD IMG2O_TCM_EN              : 1;
        FIELD IMG3O_TCM_EN              : 1;
        FIELD IMG3BO_TCM_EN             : 1;
        FIELD IMG3CO_TCM_EN             : 1;
        FIELD FE_TCM_EN                 : 1;
        FIELD CRZ_TCM_EN                : 1;
        FIELD MDPCROP_TCM_EN            : 1;
        FIELD LCE_TCM_EN                : 1;
        FIELD SRZ1_TCM_EN               : 1;
        FIELD SRZ2_TCM_EN               : 1;
        FIELD CRSP_TCM_EN               : 1;
        FIELD NR3D_TCM_EN               : 1;
        FIELD CTL_EXTENSION_EN          : 1;
        FIELD MIX3_TCM_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TILE;

typedef volatile union _CAM_REG_CTL_TDR_EN_STATUS_
{
    volatile struct
    {
        FIELD CTL_TDR_EN_STATUS         : 30;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TDR_EN_STATUS;

typedef volatile union _CAM_REG_CTL_TCM_EN_
{
    volatile struct
    {
        FIELD TILE_WIDTH                : 10;
        FIELD TILE_HEIGHT               : 13;
        FIELD SEEE_TCM_EN               : 1;
        FIELD SL2C_TCM_EN               : 1;
        FIELD MIX2_TCM_EN               : 1;
        FIELD PCA_TCM_EN                : 1;
        FIELD MIX1_TCM_EN               : 1;
        FIELD MFBO_TCM_EN               : 1;
        FIELD TDR_SOF_RST_EN            : 1;
        FIELD TDR_SZ_DET                : 1;
        FIELD TDR_EN                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TCM_EN;

typedef volatile union _CAM_REG_CTL_TDR_DBG_STATUS_
{
    volatile struct
    {
        FIELD TPIPE_CNT                 : 8;
        FIELD LOAD_SIZE                 : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TDR_DBG_STATUS;

typedef volatile union _CAM_REG_CTL_SW_CTL_
{
    volatile struct
    {
        FIELD SW_RST_Trig               : 1;
        FIELD SW_RST_ST                 : 1;
        FIELD HW_RST                    : 1;
        FIELD rsv_3                     : 1;
        FIELD SW_RST_P2_Trig            : 1;
        FIELD SW_RST_P2_ST              : 1;
        FIELD HW_RST_P2                 : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SW_CTL;

typedef volatile union _CAM_REG_CTL_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE0;

typedef volatile union _CAM_REG_CTL_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE1;

typedef volatile union _CAM_REG_CTL_SPARE2_
{
    volatile struct
    {
        FIELD SPARE2                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE2;

typedef volatile union _CAM_REG_CTL_CQ1_BASEADDR_
{
    volatile struct
    {
        FIELD CQ1_BASEADDR              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ1_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ2_BASEADDR_
{
    volatile struct
    {
        FIELD CQ2_BASEADDR              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ2_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ3_BASEADDR_
{
    volatile struct
    {
        FIELD CQ3_BASEADDR              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ3_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0_BASEADDR_
{
    volatile struct
    {
        FIELD CQ0_BASEADDR              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0B_BASEADDR_
{
    volatile struct
    {
        FIELD CQ0B_BASEADDR             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0B_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0C_BASEADDR_
{
    volatile struct
    {
        FIELD CQ0C_BASEADDR             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0C_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0_BASEADDR_
{
    volatile struct
    {
        FIELD CUR_CQ0_BASEADDR          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0B_BASEADDR_
{
    volatile struct
    {
        FIELD CUR_CQ0B_BASEADDR         : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0B_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0C_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CUR_CQ0C_BASEADDR     : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0C_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0_D_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ0_D_BASEADDR        : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0_D_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0B_D_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ0B_D_BASEADDR       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0B_D_BASEADDR;

typedef volatile union _CAM_REG_CTL_CQ0C_D_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CQ0C_D_BASEADDR       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ0C_D_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0_D_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CUR_CQ0_D_BASEADDR    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0_D_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0B_D_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CUR_CQ0B_D_BASEADDR   : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0B_D_BASEADDR;

typedef volatile union _CAM_REG_CTL_CUR_CQ0C_D_BASEADDR_
{
    volatile struct
    {
        FIELD CTL_CUR_CQ0C_D_BASEADDR   : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CUR_CQ0C_D_BASEADDR;

typedef volatile union _CAM_REG_CTL_DB_LOAD_D_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD DB_LOAD_HOLD_D            : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DB_LOAD_D;

typedef volatile union _CAM_REG_CTL_DB_LOAD_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD DB_LOAD_HOLD              : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DB_LOAD;

typedef volatile union _CAM_REG_CTL_P1_DONE_BYP_
{
    volatile struct
    {
        FIELD IMGO_DONE_BYP             : 1;
        FIELD UFEO_DONE_BYP             : 1;
        FIELD RRZO_DONE_BYP             : 1;
        FIELD ESFKO_DONE_BYP            : 1;
        FIELD LCSO_DONE_BYP             : 1;
        FIELD AAO_DONE_BYP              : 1;
        FIELD BPCI_DONE_BYP             : 1;
        FIELD LSCI_DONE_BYP             : 1;
        FIELD TG1_DONE_BYP              : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_P1_DONE_BYP;

typedef volatile union _CAM_REG_CTL_P1_DONE_BYP_D_
{
    volatile struct
    {
        FIELD IMGO_DONE_BYP_D           : 1;
        FIELD rsv_1                     : 1;
        FIELD RRZO_DONE_BYP_D           : 1;
        FIELD AFO_DONE_BYP_D            : 1;
        FIELD LCSO_DONE_BYP_D           : 1;
        FIELD AAO_DONE_BYP_D            : 1;
        FIELD BPCI_DONE_BYP_D           : 1;
        FIELD LSCI_DONE_BYP_D           : 1;
        FIELD TG1_DONE_BYP_D            : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_P1_DONE_BYP_D;

typedef volatile union _CAM_REG_CTL_P2_DONE_BYP_
{
    volatile struct
    {
        FIELD IMGI_DONE_BYP             : 1;
        FIELD UFDI_DONE_BYP             : 1;
        FIELD VIPI_DONE_BYP             : 1;
        FIELD VIP2I_DONE_BYP            : 1;
        FIELD VIP3I_DONE_BYP            : 1;
        FIELD LCEI_DONE_BYP             : 1;
        FIELD MFBO_DONE_BYP             : 1;
        FIELD IMG2O_DONE_BYP            : 1;
        FIELD IMG3O_DONE_BYP            : 1;
        FIELD IMG3BO_DONE_BYP           : 1;
        FIELD IMG3CO_DONE_BYP           : 1;
        FIELD FEO_DONE_BYP              : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_P2_DONE_BYP;

typedef volatile union _CAM_REG_CTL_IMGO_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD rsv_4                     : 7;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGO_FBC;

typedef volatile union _CAM_REG_CTL_RRZO_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD rsv_4                     : 7;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RRZO_FBC;

typedef volatile union _CAM_REG_CTL_IMGO_D_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD rsv_4                     : 7;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGO_D_FBC;

typedef volatile union _CAM_REG_CTL_RRZO_D_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD rsv_4                     : 7;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RRZO_D_FBC;

typedef volatile union _CAM_REG_CTL_SPARE3_
{
    volatile struct
    {
        FIELD SPARE3                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SPARE3;

typedef volatile union _CAM_REG_CTL_IHDR_
{
    volatile struct
    {
        FIELD IHDR_GAIN                 : 9;
        FIELD rsv_9                     : 3;
        FIELD IHDR_MODE                 : 1;
        FIELD IHDR_LE_FIRST             : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IHDR;

typedef volatile union _CAM_REG_CTL_IHDR_D_
{
    volatile struct
    {
        FIELD IHDR_D_GAIN               : 9;
        FIELD rsv_9                     : 3;
        FIELD IHDR_D_MODE               : 1;
        FIELD IHDR_D_LE_FIRST           : 1;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IHDR_D;

typedef volatile union _CAM_REG_CTL_CQ_EN_P2_
{
    volatile struct
    {
        FIELD CQ1_EN                    : 1;
        FIELD CQ2_EN                    : 1;
        FIELD CQ3_EN                    : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CQ_EN_P2;

typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_P2_
{
    volatile struct
    {
        FIELD PASS2_DB_EN               : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_GLOBAL_P2;

typedef volatile union _CAM_REG_CTL_DBG_SET_
{
    volatile struct
    {
        FIELD DEBUG_MOD_SEL             : 8;
        FIELD DEBUG_SEL                 : 4;
        FIELD DEBUG_TOP_SEL             : 4;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DBG_SET;

typedef volatile union _CAM_REG_CTL_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DBG_PORT;

typedef volatile union _CAM_REG_CTL_IMGI_CHECK_
{
    volatile struct
    {
        FIELD CTL_IMGI_CHECK            : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGI_CHECK;

typedef volatile union _CAM_REG_CTL_IMGO_CHECK_
{
    volatile struct
    {
        FIELD CTL_IMGO_CHECK            : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGO_CHECK;

typedef volatile union _CAM_REG_CTL_CLK_EN_
{
    volatile struct
    {
        FIELD RAW_DP_CK_EN              : 1;
        FIELD RAW_D_DP_CK_EN            : 1;
        FIELD DIP_DP_CK_EN              : 1;
        FIELD DMA_DP_CK_EN              : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_CLK_EN;

typedef volatile union _CAM_REG_CTL_DATE_CODE_
{
    volatile struct
    {
        FIELD CTL_DATE_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DATE_CODE;

typedef volatile union _CAM_REG_CTL_PROJ_CODE_
{
    volatile struct
    {
        FIELD CTL_PROJ_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_PROJ_CODE;

typedef volatile union _CAM_REG_CTL_RAW_DCM_DIS_
{
    volatile struct
    {
        FIELD UFD_DCM_DIS               : 1;
        FIELD PGN_DCM_DIS               : 1;
        FIELD UNP_DCM_DIS               : 1;
        FIELD TG1_DCM_DIS               : 1;
        FIELD DMX_DCM_DIS               : 1;
        FIELD WBN_DCM_DIS               : 1;
        FIELD W2G_DCM_DIS               : 1;
        FIELD OB_DCM_DIS                : 1;
        FIELD BNR_DCM_DIS               : 1;
        FIELD LSC_DCM_DIS               : 1;
        FIELD RPG_DCM_DIS               : 1;
        FIELD RRZ_DCM_DIS               : 1;
        FIELD RMX_DCM_DIS               : 1;
        FIELD PAKG_DCM_DIS              : 1;
        FIELD BMX_DCM_DIS               : 1;
        FIELD PAK_DCM_DIS               : 1;
        FIELD UFE_DCM_DIS               : 1;
        FIELD LCS_DCM_DIS               : 1;
        FIELD SGG1_DCM_DIS              : 1;
        FIELD AF_DCM_DIS                : 1;
        FIELD FLK_DCM_DIS               : 1;
        FIELD HBIN_DCM_DIS              : 1;
        FIELD AA_DCM_DIS                : 1;
        FIELD SGG2_DCM_DIS              : 1;
        FIELD EIS_DCM_DIS               : 1;
        FIELD RMG_DCM_DIS               : 1;
        FIELD CPG_DCM_DIS               : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_DCM_DIS;

typedef volatile union _CAM_REG_CTL_RAW_D_DCM_DIS_
{
    volatile struct
    {
        FIELD rsv_0                     : 3;
        FIELD TG1_D_DCM_DIS             : 1;
        FIELD rsv_4                     : 1;
        FIELD WBN_D_DCM_DIS             : 1;
        FIELD W2G_D_DCM_DIS             : 1;
        FIELD OB_D_DCM_DIS              : 1;
        FIELD BNR_D_DCM_DIS             : 1;
        FIELD LSC_D_DCM_DIS             : 1;
        FIELD RPG_D_DCM_DIS             : 1;
        FIELD RRZ_D_DCM_DIS             : 1;
        FIELD rsv_12                    : 1;
        FIELD PAKG_D_DCM_DIS            : 1;
        FIELD rsv_14                    : 1;
        FIELD PAK_D_DCM_DIS             : 1;
        FIELD rsv_16                    : 1;
        FIELD LCS_D_DCM_DIS             : 1;
        FIELD SGG1_D_DCM_DIS            : 1;
        FIELD AF_D_DCM_DIS              : 1;
        FIELD rsv_20                    : 2;
        FIELD AA_D_DCM_DIS              : 1;
        FIELD rsv_23                    : 2;
        FIELD RMG_D_DCM_DIS             : 1;
        FIELD CPG_D_DCM_DIS             : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_D_DCM_DIS;

typedef volatile union _CAM_REG_CTL_DMA_DCM_DIS_
{
    volatile struct
    {
        FIELD IMGI_DCM_DIS              : 1;
        FIELD UFDI_DCM_DIS              : 1;
        FIELD VIPI_DCM_DIS              : 1;
        FIELD VIP2I_DCM_DIS             : 1;
        FIELD VIP3I_DCM_DIS             : 1;
        FIELD LCEI_DCM_DIS              : 1;
        FIELD MFBO_DCM_DIS              : 1;
        FIELD IMG2O_DCM_DIS             : 1;
        FIELD IMG3O_DCM_DIS             : 1;
        FIELD IMG3BO_DCM_DIS            : 1;
        FIELD IMG3CO_DCM_DIS            : 1;
        FIELD FEO_DCM_DIS               : 1;
        FIELD IMGO_D_DCM_DIS            : 1;
        FIELD RRZO_D_DCM_DIS            : 1;
        FIELD AFO_D_DCM_DIS             : 1;
        FIELD LCSO_D_DCM_DIS            : 1;
        FIELD AAO_D_DCM_DIS             : 1;
        FIELD BPCI_D_DCM_DIS            : 1;
        FIELD LSCI_D_DCM_DIS            : 1;
        FIELD IMGO_DCM_DIS              : 1;
        FIELD UFEO_DCM_DIS              : 1;
        FIELD RRZO_DCM_DIS              : 1;
        FIELD ESFKO_DCM_DIS             : 1;
        FIELD LCSO_DCM_DIS              : 1;
        FIELD AAO_DCM_DIS               : 1;
        FIELD BPCI_DCM_DIS              : 1;
        FIELD LSCI_DCM_DIS              : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_DCM_DIS;

typedef volatile union _CAM_REG_CTL_RGB_DCM_DIS_
{
    volatile struct
    {
        FIELD SL2_DCM_DIS               : 1;
        FIELD CFA_DCM_DIS               : 1;
        FIELD CCL_DCM_DIS               : 1;
        FIELD G2G_DCM_DIS               : 1;
        FIELD C24_DCM_DIS               : 1;
        FIELD MFB_DCM_DIS               : 1;
        FIELD GGM_DCM_DIS               : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RGB_DCM_DIS;

typedef volatile union _CAM_REG_CTL_YUV_DCM_DIS_
{
    volatile struct
    {
        FIELD C02_DCM_DIS               : 1;
        FIELD G2C_DCM_DIS               : 1;
        FIELD C42_DCM_DIS               : 1;
        FIELD NBC_DCM_DIS               : 1;
        FIELD PCA_DCM_DIS               : 1;
        FIELD SEEE_DCM_DIS              : 1;
        FIELD LCE_DCM_DIS               : 1;
        FIELD NR3D_DCM_DIS              : 1;
        FIELD SL2B_DCM_DIS              : 1;
        FIELD SL2C_DCM_DIS              : 1;
        FIELD SRZ1_DCM_DIS              : 1;
        FIELD SRZ2_DCM_DIS              : 1;
        FIELD CRZ_DCM_DIS               : 1;
        FIELD MIX1_DCM_DIS              : 1;
        FIELD MIX2_DCM_DIS              : 1;
        FIELD MIX3_DCM_DIS              : 1;
        FIELD CRSP_DCM_DIS              : 1;
        FIELD C24B_DCM_DIS              : 1;
        FIELD MDPCROP_DCM_DIS           : 1;
        FIELD FE_DCM_DIS                : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_YUV_DCM_DIS;

typedef volatile union _CAM_REG_CTL_TOP_DCM_DIS_
{
    volatile struct
    {
        FIELD TOP_DCM_DIS               : 1;
        FIELD CQ_DCM_DIS                : 1;
        FIELD FBC_DCM_DIS               : 1;
        FIELD TDR_DCM_DIS               : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TOP_DCM_DIS;

typedef volatile union _CAM_REG_CTL_RAW_DCM_STATUS_
{
    volatile struct
    {
        FIELD UFD_DCM_STATUS            : 1;
        FIELD PGN_DCM_STATUS            : 1;
        FIELD UNP_DCM_STATUS            : 1;
        FIELD TG1_DCM_STATUS            : 1;
        FIELD DMX_DCM_STATUS            : 1;
        FIELD WBN_DCM_STATUS            : 1;
        FIELD W2G_DCM_STATUS            : 1;
        FIELD OB_DCM_STATUS             : 1;
        FIELD BNR_DCM_STATUS            : 1;
        FIELD LSC_DCM_STATUS            : 1;
        FIELD RPG_DCM_STATUS            : 1;
        FIELD RRZ_DCM_STATUS            : 1;
        FIELD RMX_DCM_STATUS            : 1;
        FIELD PAKG_DCM_STATUS           : 1;
        FIELD BMX_DCM_STATUS            : 1;
        FIELD PAK_DCM_STATUS            : 1;
        FIELD UFE_DCM_STATUS            : 1;
        FIELD LCS_DCM_STATUS            : 1;
        FIELD SGG1_DCM_STATUS           : 1;
        FIELD AF_DCM_STATUS             : 1;
        FIELD FLK_DCM_STATUS            : 1;
        FIELD HBIN_DCM_STATUS           : 1;
        FIELD AA_DCM_STATUS             : 1;
        FIELD SGG2_DCM_STATUS           : 1;
        FIELD EIS_DCM_STATUS            : 1;
        FIELD RMG_DCM_STATUS            : 1;
        FIELD CPG_DCM_STATUS            : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_RAW_D_DCM_STATUS_
{
    volatile struct
    {
        FIELD rsv_0                     : 3;
        FIELD TG1_D_DCM_STATUS          : 1;
        FIELD rsv_4                     : 1;
        FIELD WBN_D_DCM_STATUS          : 1;
        FIELD W2G_D_DCM_STATUS          : 1;
        FIELD OB_D_DCM_STATUS           : 1;
        FIELD BNR_D_DCM_STATUS          : 1;
        FIELD LSC_D_DCM_STATUS          : 1;
        FIELD RPG_D_DCM_STATUS          : 1;
        FIELD RRZ_D_DCM_STATUS          : 1;
        FIELD rsv_12                    : 1;
        FIELD PAKG_D_DCM_STATUS         : 1;
        FIELD rsv_14                    : 1;
        FIELD PAK_D_DCM_STATUS          : 1;
        FIELD rsv_16                    : 1;
        FIELD LCS_D_DCM_STATUS          : 1;
        FIELD SGG1_D_DCM_STATUS         : 1;
        FIELD AF_D_DCM_STATUS           : 1;
        FIELD rsv_20                    : 2;
        FIELD AA_D_DCM_STATUS           : 1;
        FIELD rsv_23                    : 2;
        FIELD RMG_D_DCM_STATUS          : 1;
        FIELD CPG_D_DCM_STATUS          : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_D_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_DMA_DCM_STATUS_
{
    volatile struct
    {
        FIELD IMGI_DCM_STATUS           : 1;
        FIELD UFDI_DCM_STATUS           : 1;
        FIELD VIPI_DCM_STATUS           : 1;
        FIELD VIP2I_DCM_STATUS          : 1;
        FIELD VIP3I_DCM_STATUS          : 1;
        FIELD LCEI_DCM_STATUS           : 1;
        FIELD MFBO_DCM_STATUS           : 1;
        FIELD IMG2O_DCM_STATUS          : 1;
        FIELD IMG3O_DCM_STATUS          : 1;
        FIELD IMG3BO_DCM_STATUS         : 1;
        FIELD IMG3CO_DCM_STATUS         : 1;
        FIELD FEO_DCM_STATUS            : 1;
        FIELD IMGO_D_DCM_STATUS         : 1;
        FIELD RRZO_D_DCM_STATUS         : 1;
        FIELD AFO_D_DCM_STATUS          : 1;
        FIELD LCSO_D_DCM_STATUS         : 1;
        FIELD AAO_D_DCM_STATUS          : 1;
        FIELD BPCI_D_DCM_STATUS         : 1;
        FIELD LSCI_D_DCM_STATUS         : 1;
        FIELD IMGO_DCM_STATUS           : 1;
        FIELD UFEO_DCM_STATUS           : 1;
        FIELD RRZO_DCM_STATUS           : 1;
        FIELD ESFKO_DCM_STATUS          : 1;
        FIELD LCSO_DCM_STATUS           : 1;
        FIELD AAO_DCM_STATUS            : 1;
        FIELD BPCI_DCM_STATUS           : 1;
        FIELD LSCI_DCM_STATUS           : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_RGB_DCM_STATUS_
{
    volatile struct
    {
        FIELD SL2_DCM_STATUS            : 1;
        FIELD CFA_DCM_STATUS            : 1;
        FIELD CCL_DCM_STATUS            : 1;
        FIELD G2G_DCM_STATUS            : 1;
        FIELD C24_DCM_STATUS            : 1;
        FIELD MFB_DCM_STATUS            : 1;
        FIELD GGM_DCM_STATUS            : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RGB_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_YUV_DCM_STATUS_
{
    volatile struct
    {
        FIELD C02_DCM_STATUS            : 1;
        FIELD G2C_DCM_STATUS            : 1;
        FIELD C42_DCM_STATUS            : 1;
        FIELD NBC_DCM_STATUS            : 1;
        FIELD PCA_DCM_STATUS            : 1;
        FIELD SEEE_DCM_STATUS           : 1;
        FIELD LCE_DCM_STATUS            : 1;
        FIELD NR3D_DCM_STATUS           : 1;
        FIELD SL2B_DCM_STATUS           : 1;
        FIELD SL2C_DCM_STATUS           : 1;
        FIELD SRZ1_DCM_STATUS           : 1;
        FIELD SRZ2_DCM_STATUS           : 1;
        FIELD CRZ_DCM_STATUS            : 1;
        FIELD MIX1_DCM_STATUS           : 1;
        FIELD MIX2_DCM_STATUS           : 1;
        FIELD MIX3_DCM_STATUS           : 1;
        FIELD CRSP_DCM_STATUS           : 1;
        FIELD C24B_DCM_STATUS           : 1;
        FIELD MDPCROP_DCM_STATUS        : 1;
        FIELD FE_DCM_STATUS             : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_YUV_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_TOP_DCM_STATUS_
{
    volatile struct
    {
        FIELD TOP_DCM_STATUS            : 1;
        FIELD CQ_DCM_STATUS             : 1;
        FIELD FBC_DCM_STATUS            : 1;
        FIELD TDR_DCM_STATUS            : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_TOP_DCM_STATUS;

typedef volatile union _CAM_REG_CTL_RAW_REQ_STATUS_
{
    volatile struct
    {
        FIELD UFD_REQ_STATUS            : 1;
        FIELD PGN_REQ_STATUS            : 1;
        FIELD UNP_REQ_STATUS            : 1;
        FIELD TG1_REQ_STATUS            : 1;
        FIELD DMX_REQ_STATUS            : 1;
        FIELD WBN_REQ_STATUS            : 1;
        FIELD W2G_REQ_STATUS            : 1;
        FIELD OB_REQ_STATUS             : 1;
        FIELD BNR_REQ_STATUS            : 1;
        FIELD LSC_REQ_STATUS            : 1;
        FIELD RPG_REQ_STATUS            : 1;
        FIELD RRZ_REQ_STATUS            : 1;
        FIELD RMX_REQ_STATUS            : 1;
        FIELD PAKG_REQ_STATUS           : 1;
        FIELD BMX_REQ_STATUS            : 1;
        FIELD PAK_REQ_STATUS            : 1;
        FIELD UFE_REQ_STATUS            : 1;
        FIELD LCS_REQ_STATUS            : 1;
        FIELD SGG1_REQ_STATUS           : 1;
        FIELD AF_REQ_STATUS             : 1;
        FIELD FLK_REQ_STATUS            : 1;
        FIELD HBIN_REQ_STATUS           : 1;
        FIELD AA_REQ_STATUS             : 1;
        FIELD SGG2_REQ_STATUS           : 1;
        FIELD EIS_REQ_STATUS            : 1;
        FIELD RMG_REQ_STATUS            : 1;
        FIELD CPG_REQ_STATUS            : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_REQ_STATUS;

typedef volatile union _CAM_REG_CTL_RAW_D_REQ_STATUS_
{
    volatile struct
    {
        FIELD rsv_0                     : 3;
        FIELD TG1_D_REQ_STATUS          : 1;
        FIELD rsv_4                     : 1;
        FIELD WBN_D_REQ_STATUS          : 1;
        FIELD W2G_D_REQ_STATUS          : 1;
        FIELD OB_D_REQ_STATUS           : 1;
        FIELD BNR_D_REQ_STATUS          : 1;
        FIELD LSC_D_REQ_STATUS          : 1;
        FIELD RPG_D_REQ_STATUS          : 1;
        FIELD RRZ_D_REQ_STATUS          : 1;
        FIELD rsv_12                    : 1;
        FIELD PAKG_D_REQ_STATUS         : 1;
        FIELD rsv_14                    : 1;
        FIELD PAK_D_REQ_STATUS          : 1;
        FIELD rsv_16                    : 1;
        FIELD LCS_D_REQ_STATUS          : 1;
        FIELD SGG1_D_REQ_STATUS         : 1;
        FIELD AF_D_REQ_STATUS           : 1;
        FIELD rsv_20                    : 2;
        FIELD AA_D_REQ_STATUS           : 1;
        FIELD rsv_23                    : 2;
        FIELD RMG_D_REQ_STATUS          : 1;
        FIELD CPG_D_REQ_STATUS          : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_D_REQ_STATUS;

typedef volatile union _CAM_REG_CTL_DMA_REQ_STATUS_
{
    volatile struct
    {
        FIELD IMGI_REQ_STATUS           : 1;
        FIELD UFDI_REQ_STATUS           : 1;
        FIELD VIPI_REQ_STATUS           : 1;
        FIELD VIP2I_REQ_STATUS          : 1;
        FIELD VIP3I_REQ_STATUS          : 1;
        FIELD LCEI_REQ_STATUS           : 1;
        FIELD MFBO_REQ_STATUS           : 1;
        FIELD IMG2O_REQ_STATUS          : 1;
        FIELD IMG3O_REQ_STATUS          : 1;
        FIELD IMG3BO_REQ_STATUS         : 1;
        FIELD IMG3CO_REQ_STATUS         : 1;
        FIELD FEO_REQ_STATUS            : 1;
        FIELD IMGO_D_REQ_STATUS         : 1;
        FIELD RRZO_D_REQ_STATUS         : 1;
        FIELD AFO_D_REQ_STATUS          : 1;
        FIELD LCSO_D_REQ_STATUS         : 1;
        FIELD AAO_D_REQ_STATUS          : 1;
        FIELD BPCI_D_REQ_STATUS         : 1;
        FIELD LSCI_D_REQ_STATUS         : 1;
        FIELD IMGO_REQ_STATUS           : 1;
        FIELD UFEO_REQ_STATUS           : 1;
        FIELD RRZO_REQ_STATUS           : 1;
        FIELD ESFKO_REQ_STATUS          : 1;
        FIELD LCSO_REQ_STATUS           : 1;
        FIELD AAO_REQ_STATUS            : 1;
        FIELD BPCI_REQ_STATUS           : 1;
        FIELD LSCI_REQ_STATUS           : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_REQ_STATUS;

typedef volatile union _CAM_REG_CTL_RGB_REQ_STATUS_
{
    volatile struct
    {
        FIELD SL2_REQ_STATUS            : 1;
        FIELD CFA_REQ_STATUS            : 1;
        FIELD CCL_REQ_STATUS            : 1;
        FIELD G2G_REQ_STATUS            : 1;
        FIELD C24_REQ_STATUS            : 1;
        FIELD MFB_REQ_STATUS            : 1;
        FIELD GGM_REQ_STATUS            : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RGB_REQ_STATUS;

typedef volatile union _CAM_REG_CTL_YUV_REQ_STATUS_
{
    volatile struct
    {
        FIELD C02_REQ_STATUS            : 1;
        FIELD G2C_REQ_STATUS            : 1;
        FIELD C42_REQ_STATUS            : 1;
        FIELD NBC_REQ_STATUS            : 1;
        FIELD PCA_REQ_STATUS            : 1;
        FIELD SEEE_REQ_STATUS           : 1;
        FIELD LCE_REQ_STATUS            : 1;
        FIELD NR3D_REQ_STATUS           : 1;
        FIELD SL2B_REQ_STATUS           : 1;
        FIELD SL2C_REQ_STATUS           : 1;
        FIELD SRZ1_REQ_STATUS           : 1;
        FIELD SRZ2_REQ_STATUS           : 1;
        FIELD CRZ_REQ_STATUS            : 1;
        FIELD MIX1_REQ_STATUS           : 1;
        FIELD MIX2_REQ_STATUS           : 1;
        FIELD MIX3_REQ_STATUS           : 1;
        FIELD CRSP_REQ_STATUS           : 1;
        FIELD C24B_REQ_STATUS           : 1;
        FIELD MDPCROP_REQ_STATUS        : 1;
        FIELD FE_REQ_STATUS             : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_YUV_REQ_STATUS;

typedef volatile union _CAM_REG_CTL_RAW_RDY_STATUS_
{
    volatile struct
    {
        FIELD UFD_RDY_STATUS            : 1;
        FIELD PGN_RDY_STATUS            : 1;
        FIELD UNP_RDY_STATUS            : 1;
        FIELD TG1_RDY_STATUS            : 1;
        FIELD DMX_RDY_STATUS            : 1;
        FIELD WBN_RDY_STATUS            : 1;
        FIELD W2G_RDY_STATUS            : 1;
        FIELD OB_RDY_STATUS             : 1;
        FIELD BNR_RDY_STATUS            : 1;
        FIELD LSC_RDY_STATUS            : 1;
        FIELD RPG_RDY_STATUS            : 1;
        FIELD RRZ_RDY_STATUS            : 1;
        FIELD RMX_RDY_STATUS            : 1;
        FIELD PAKG_RDY_STATUS           : 1;
        FIELD BMX_RDY_STATUS            : 1;
        FIELD PAK_RDY_STATUS            : 1;
        FIELD UFE_RDY_STATUS            : 1;
        FIELD LCS_RDY_STATUS            : 1;
        FIELD SGG1_RDY_STATUS           : 1;
        FIELD AF_RDY_STATUS             : 1;
        FIELD FLK_RDY_STATUS            : 1;
        FIELD HBIN_RDY_STATUS           : 1;
        FIELD AA_RDY_STATUS             : 1;
        FIELD SGG2_RDY_STATUS           : 1;
        FIELD EIS_RDY_STATUS            : 1;
        FIELD RMG_RDY_STATUS            : 1;
        FIELD CPG_RDY_STATUS            : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_RDY_STATUS;

typedef volatile union _CAM_REG_CTL_RAW_D_RDY_STATUS_
{
    volatile struct
    {
        FIELD rsv_0                     : 3;
        FIELD TG1_D_RDY_STATUS          : 1;
        FIELD rsv_4                     : 1;
        FIELD WBN_D_RDY_STATUS          : 1;
        FIELD W2G_D_RDY_STATUS          : 1;
        FIELD OB_D_RDY_STATUS           : 1;
        FIELD BNR_D_RDY_STATUS          : 1;
        FIELD LSC_D_RDY_STATUS          : 1;
        FIELD RPG_D_RDY_STATUS          : 1;
        FIELD RRZ_D_RDY_STATUS          : 1;
        FIELD rsv_12                    : 1;
        FIELD PAKG_D_RDY_STATUS         : 1;
        FIELD rsv_14                    : 1;
        FIELD PAK_D_RDY_STATUS          : 1;
        FIELD rsv_16                    : 1;
        FIELD LCS_D_RDY_STATUS          : 1;
        FIELD SGG1_D_RDY_STATUS         : 1;
        FIELD AF_D_RDY_STATUS           : 1;
        FIELD rsv_20                    : 2;
        FIELD AA_D_RDY_STATUS           : 1;
        FIELD rsv_23                    : 2;
        FIELD RMG_D_RDY_STATUS          : 1;
        FIELD CPG_D_RDY_STATUS          : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RAW_D_RDY_STATUS;

typedef volatile union _CAM_REG_CTL_DMA_RDY_STATUS_
{
    volatile struct
    {
        FIELD IMGI_RDY_STATUS           : 1;
        FIELD UFDI_RDY_STATUS           : 1;
        FIELD VIPI_RDY_STATUS           : 1;
        FIELD VIP2I_RDY_STATUS          : 1;
        FIELD VIP3I_RDY_STATUS          : 1;
        FIELD LCEI_RDY_STATUS           : 1;
        FIELD MFBO_RDY_STATUS           : 1;
        FIELD IMG2O_RDY_STATUS          : 1;
        FIELD IMG3O_RDY_STATUS          : 1;
        FIELD IMG3BO_RDY_STATUS         : 1;
        FIELD IMG3CO_RDY_STATUS         : 1;
        FIELD FEO_RDY_STATUS            : 1;
        FIELD IMGO_D_RDY_STATUS         : 1;
        FIELD RRZO_D_RDY_STATUS         : 1;
        FIELD AFO_D_RDY_STATUS          : 1;
        FIELD LCSO_D_RDY_STATUS         : 1;
        FIELD AAO_D_RDY_STATUS          : 1;
        FIELD BPCI_D_RDY_STATUS         : 1;
        FIELD LSCI_D_RDY_STATUS         : 1;
        FIELD IMGO_RDY_STATUS           : 1;
        FIELD UFEO_RDY_STATUS           : 1;
        FIELD RRZO_RDY_STATUS           : 1;
        FIELD ESFKO_RDY_STATUS          : 1;
        FIELD LCSO_RDY_STATUS           : 1;
        FIELD AAO_RDY_STATUS            : 1;
        FIELD BPCI_RDY_STATUS           : 1;
        FIELD LSCI_RDY_STATUS           : 1;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_DMA_RDY_STATUS;

typedef volatile union _CAM_REG_CTL_RGB_RDY_STATUS_
{
    volatile struct
    {
        FIELD SL2_RDY_STATUS            : 1;
        FIELD CFA_RDY_STATUS            : 1;
        FIELD CCL_RDY_STATUS            : 1;
        FIELD G2G_RDY_STATUS            : 1;
        FIELD C24_RDY_STATUS            : 1;
        FIELD MFB_RDY_STATUS            : 1;
        FIELD GGM_RDY_STATUS            : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_RGB_RDY_STATUS;

typedef volatile union _CAM_REG_CTL_YUV_RDY_STATUS_
{
    volatile struct
    {
        FIELD C02_RDY_STATUS            : 1;
        FIELD G2C_RDY_STATUS            : 1;
        FIELD C42_RDY_STATUS            : 1;
        FIELD NBC_RDY_STATUS            : 1;
        FIELD PCA_RDY_STATUS            : 1;
        FIELD SEEE_RDY_STATUS           : 1;
        FIELD LCE_RDY_STATUS            : 1;
        FIELD NR3D_RDY_STATUS           : 1;
        FIELD SL2B_RDY_STATUS           : 1;
        FIELD SL2C_RDY_STATUS           : 1;
        FIELD SRZ1_RDY_STATUS           : 1;
        FIELD SRZ2_RDY_STATUS           : 1;
        FIELD CRZ_RDY_STATUS            : 1;
        FIELD MIX1_RDY_STATUS           : 1;
        FIELD MIX2_RDY_STATUS           : 1;
        FIELD MIX3_RDY_STATUS           : 1;
        FIELD CRSP_RDY_STATUS           : 1;
        FIELD C24B_RDY_STATUS           : 1;
        FIELD MDPCROP_RDY_STATUS        : 1;
        FIELD FE_RDY_STATUS             : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_YUV_RDY_STATUS;

/* end MT6593_000_cam_ctl.xml*/

/* start MT6593_201_raw_tg.xml*/
typedef volatile union _CAM_REG_TG_SEN_MODE_
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_MODE;

typedef volatile union _CAM_REG_TG_VF_CON_
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_VF_CON;

typedef volatile union _CAM_REG_TG_SEN_GRAB_PXL_
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_GRAB_PXL;

typedef volatile union _CAM_REG_TG_SEN_GRAB_LIN_
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SEN_GRAB_LIN;

typedef volatile union _CAM_REG_TG_PATH_CFG_
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_PATH_CFG;

typedef volatile union _CAM_REG_TG_MEMIN_CTL_
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MEMIN_CTL;

typedef volatile union _CAM_REG_TG_INT1_
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INT1;

typedef volatile union _CAM_REG_TG_INT2_
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INT2;

typedef volatile union _CAM_REG_TG_SOF_CNT_
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SOF_CNT;

typedef volatile union _CAM_REG_TG_SOT_CNT_
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_SOT_CNT;

typedef volatile union _CAM_REG_TG_EOT_CNT_
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_EOT_CNT;

typedef volatile union _CAM_REG_TG_ERR_CTL_
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_ERR_CTL;

typedef volatile union _CAM_REG_TG_DAT_NO_
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_DAT_NO;

typedef volatile union _CAM_REG_TG_FRM_CNT_ST_
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FRM_CNT_ST;

typedef volatile union _CAM_REG_TG_FRMSIZE_ST_
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FRMSIZE_ST;

typedef volatile union _CAM_REG_TG_INTER_ST_
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_INTER_ST;

typedef volatile union _CAM_REG_TG_FLASHA_CTL_
{
    volatile struct
    {
        FIELD FLASHA_EN                 : 1;
        FIELD FLASH_EN                  : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHA_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHA_END_FRM            : 3;
        FIELD rsv_11                    : 1;
        FIELD FLASH_POL                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_CTL;

typedef volatile union _CAM_REG_TG_FLASHA_LINE_CNT_
{
    volatile struct
    {
        FIELD FLASHA_LUNIT_NO           : 20;
        FIELD FLASHA_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_LINE_CNT;

typedef volatile union _CAM_REG_TG_FLASHA_POS_
{
    volatile struct
    {
        FIELD FLASHA_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHA_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHA_POS;

typedef volatile union _CAM_REG_TG_FLASHB_CTL_
{
    volatile struct
    {
        FIELD FLASHB_EN                 : 1;
        FIELD FLASHB_TRIG_SRC           : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHB_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHB_START_FRM          : 4;
        FIELD FLASHB_CONT_FRM           : 3;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_CTL;

typedef volatile union _CAM_REG_TG_FLASHB_LINE_CNT_
{
    volatile struct
    {
        FIELD FLASHB_LUNIT_NO           : 20;
        FIELD FLASHB_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_LINE_CNT;

typedef volatile union _CAM_REG_TG_FLASHB_POS_
{
    volatile struct
    {
        FIELD FLASHB_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHB_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_POS;

typedef volatile union _CAM_REG_TG_FLASHB_POS1_
{
    volatile struct
    {
        FIELD FLASHB_CYC_CNT            : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_FLASHB_POS1;

typedef volatile union _CAM_REG_TG_GSCTRL_CTL_
{
    volatile struct
    {
        FIELD GSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD GSCTRL_POL                : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_GSCTRL_CTL;

typedef volatile union _CAM_REG_TG_GSCTRL_TIME_
{
    volatile struct
    {
        FIELD GS_EPTIME                 : 23;
        FIELD rsv_23                    : 1;
        FIELD GSMS_TIMEU                : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_GSCTRL_TIME;

typedef volatile union _CAM_REG_TG_MS_PHASE_
{
    volatile struct
    {
        FIELD MSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD MSCTRL_VSPOL              : 1;
        FIELD MSCTRL_OPEN_TRSRC         : 1;
        FIELD rsv_6                     : 2;
        FIELD MSCTRL_TRSRC              : 2;
        FIELD rsv_10                    : 6;
        FIELD MSCP1_PH0                 : 1;
        FIELD MSCP1_PH1                 : 1;
        FIELD MSCP1_PH2                 : 1;
        FIELD rsv_19                    : 1;
        FIELD MSOP1_PH0                 : 1;
        FIELD MSOP1_PH1                 : 1;
        FIELD rsv_22                    : 1;
        FIELD MSP1_RST                  : 1;
        FIELD MSCP2_PH0                 : 1;
        FIELD MSCP2_PH1                 : 1;
        FIELD MSCP2_PH2                 : 1;
        FIELD rsv_27                    : 1;
        FIELD MSOP2_PH0                 : 1;
        FIELD MSOP2_PH1                 : 1;
        FIELD rsv_30                    : 1;
        FIELD MSP2_RST                  : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_PHASE;

typedef volatile union _CAM_REG_TG_MS_CL_TIME_
{
    volatile struct
    {
        FIELD MS_TCLOSE                 : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_CL_TIME;

typedef volatile union _CAM_REG_TG_MS_OP_TIME_
{
    volatile struct
    {
        FIELD MS_TOPEN                  : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_OP_TIME;

typedef volatile union _CAM_REG_TG_MS_CLPH_TIME_
{
    volatile struct
    {
        FIELD MS_CL_T1                  : 16;
        FIELD MS_CL_T2                  : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_CLPH_TIME;

typedef volatile union _CAM_REG_TG_MS_OPPH_TIME_
{
    volatile struct
    {
        FIELD MS_OP_T3                  : 16;
        FIELD MS_OP_T4                  : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG_MS_OPPH_TIME;

/* end MT6593_201_raw_tg.xml*/

/* start MT6593_202_raw_bin.xml*/
typedef volatile union _CAM_REG_HBN_SIZE_
{
    volatile struct
    {
        FIELD HBN_IN_H                  : 13;
        FIELD rsv_13                    : 3;
        FIELD HBN_IN_V                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_HBN_SIZE;

typedef volatile union _CAM_REG_HBN_MODE_
{
    volatile struct
    {
        FIELD HBIN_ACC                  : 2;
        FIELD rsv_2                     : 2;
        FIELD HBIN_EN                   : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_HBN_MODE;

/* end MT6593_202_raw_bin.xml*/

/* start MT6593_203_raw_obc.xml*/
typedef volatile union _CAM_REG_OBC_OFFST0_
{
    volatile struct
    {
        FIELD OBC_OFST_B                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST0;

typedef volatile union _CAM_REG_OBC_OFFST1_
{
    volatile struct
    {
        FIELD OBC_OFST_GR               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST1;

typedef volatile union _CAM_REG_OBC_OFFST2_
{
    volatile struct
    {
        FIELD OBC_OFST_GB               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST2;

typedef volatile union _CAM_REG_OBC_OFFST3_
{
    volatile struct
    {
        FIELD OBC_OFST_R                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_OFFST3;

typedef volatile union _CAM_REG_OBC_GAIN0_
{
    volatile struct
    {
        FIELD OBC_GAIN_B                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN0;

typedef volatile union _CAM_REG_OBC_GAIN1_
{
    volatile struct
    {
        FIELD OBC_GAIN_GR               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN1;

typedef volatile union _CAM_REG_OBC_GAIN2_
{
    volatile struct
    {
        FIELD OBC_GAIN_GB               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN2;

typedef volatile union _CAM_REG_OBC_GAIN3_
{
    volatile struct
    {
        FIELD OBC_GAIN_R                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_GAIN3;

/* end MT6593_203_raw_obc.xml*/

/* start MT6593_204_raw_lsc.xml*/
typedef volatile union _CAM_REG_LSC_CTL1_
{
    volatile struct
    {
        FIELD SDBLK_YOFST               : 6;
        FIELD rsv_6                     : 10;
        FIELD SDBLK_XOFST               : 6;
        FIELD rsv_22                    : 2;
        FIELD SD_COEFRD_MODE            : 1;
        FIELD rsv_25                    : 3;
        FIELD SD_ULTRA_MODE             : 1;
        FIELD LSC_PRC_MODE              : 1;
        FIELD LSC_SPARE2                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_CTL1;

typedef volatile union _CAM_REG_LSC_CTL2_
{
    volatile struct
    {
        FIELD LSC_SDBLK_WIDTH           : 12;
        FIELD LSC_SDBLK_XNUM            : 5;
        FIELD LSC_OFLN                  : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_CTL2;

typedef volatile union _CAM_REG_LSC_CTL3_
{
    volatile struct
    {
        FIELD LSC_SDBLK_HEIGHT          : 12;
        FIELD LSC_SDBLK_YNUM            : 5;
        FIELD LSC_SPARE                 : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_CTL3;

typedef volatile union _CAM_REG_LSC_LBLOCK_
{
    volatile struct
    {
        FIELD LSC_SDBLK_lHEIGHT         : 12;
        FIELD rsv_12                    : 4;
        FIELD LSC_SDBLK_lWIDTH          : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_LBLOCK;

typedef volatile union _CAM_REG_LSC_RATIO_
{
    volatile struct
    {
        FIELD LSC_RA3                   : 6;
        FIELD rsv_6                     : 2;
        FIELD LSC_RA2                   : 6;
        FIELD rsv_14                    : 2;
        FIELD LSC_RA1                   : 6;
        FIELD rsv_22                    : 2;
        FIELD LSC_RA0                   : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_RATIO;

typedef volatile union _CAM_REG_LSC_TPIPE_OFST_
{
    volatile struct
    {
        FIELD LSC_TPIPE_OFST_Y          : 12;
        FIELD rsv_12                    : 4;
        FIELD LSC_TPIPE_OFST_X          : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_TPIPE_OFST;

typedef volatile union _CAM_REG_LSC_TPIPE_SIZE_
{
    volatile struct
    {
        FIELD LSC_TPIPE_SIZE_Y          : 13;
        FIELD rsv_13                    : 3;
        FIELD LSC_TPIPE_SIZE_X          : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_TPIPE_SIZE;

typedef volatile union _CAM_REG_LSC_GAIN_TH_
{
    volatile struct
    {
        FIELD LSC_GAIN_TH2              : 9;
        FIELD rsv_9                     : 1;
        FIELD LSC_GAIN_TH1              : 9;
        FIELD rsv_19                    : 1;
        FIELD LSC_GAIN_TH0              : 9;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_GAIN_TH;

/* end MT6593_204_raw_lsc.xml*/

/* start MT6593_205_raw_rpg.xml*/
typedef volatile union _CAM_REG_RPG_SATU_1_
{
    volatile struct
    {
        FIELD RPG_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_SATU_1;

typedef volatile union _CAM_REG_RPG_SATU_2_
{
    volatile struct
    {
        FIELD RPG_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_SATU_2;

typedef volatile union _CAM_REG_RPG_GAIN_1_
{
    volatile struct
    {
        FIELD RPG_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_GAIN_1;

typedef volatile union _CAM_REG_RPG_GAIN_2_
{
    volatile struct
    {
        FIELD RPG_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_GAIN_2;

typedef volatile union _CAM_REG_RPG_OFST_1_
{
    volatile struct
    {
        FIELD RPG_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_OFST_1;

typedef volatile union _CAM_REG_RPG_OFST_2_
{
    volatile struct
    {
        FIELD RPG_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_OFST_2;

/* end MT6593_205_raw_rpg.xml*/

/* start MT6593_206_raw_sgg.xml*/
typedef volatile union _CAM_REG_SGG3_PGN_
{
    volatile struct
    {
        FIELD SGG3_GAIN                 : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG3_PGN;

typedef volatile union _CAM_REG_SGG3_GMRC_1_
{
    volatile struct
    {
        FIELD SGG3_GMR_1                : 8;
        FIELD SGG3_GMR_2                : 8;
        FIELD SGG3_GMR_3                : 8;
        FIELD SGG3_GMR_4                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG3_GMRC_1;

typedef volatile union _CAM_REG_SGG3_GMRC_2_
{
    volatile struct
    {
        FIELD SGG3_GMR_5                : 8;
        FIELD SGG3_GMR_6                : 8;
        FIELD SGG3_GMR_7                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG3_GMRC_2;

/* end MT6593_206_raw_sgg.xml*/

/* start MT6593_207_raw_sgg.xml*/
typedef volatile union _CAM_REG_SGG2_PGN_
{
    volatile struct
    {
        FIELD SGG2_GAIN                 : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG2_PGN;

typedef volatile union _CAM_REG_SGG2_GMRC_1_
{
    volatile struct
    {
        FIELD SGG2_GMR_1                : 8;
        FIELD SGG2_GMR_2                : 8;
        FIELD SGG2_GMR_3                : 8;
        FIELD SGG2_GMR_4                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG2_GMRC_1;

typedef volatile union _CAM_REG_SGG2_GMRC_2_
{
    volatile struct
    {
        FIELD SGG2_GMR_5                : 8;
        FIELD SGG2_GMR_6                : 8;
        FIELD SGG2_GMR_7                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG2_GMRC_2;

/* end MT6593_207_raw_sgg.xml*/

/* start MT6593_2091_raw_awb.xml*/
typedef volatile union _CAM_REG_AWB_WIN_ORG_
{
    volatile struct
    {
        FIELD AWB_W_HORG                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VORG                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_ORG;

typedef volatile union _CAM_REG_AWB_WIN_SIZE_
{
    volatile struct
    {
        FIELD AWB_W_HSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VSIZE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_SIZE;

typedef volatile union _CAM_REG_AWB_WIN_PIT_
{
    volatile struct
    {
        FIELD AWB_W_HPIT                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VPIT                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_PIT;

typedef volatile union _CAM_REG_AWB_WIN_NUM_
{
    volatile struct
    {
        FIELD AWB_W_HNUM                : 8;
        FIELD rsv_8                     : 8;
        FIELD AWB_W_VNUM                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_WIN_NUM;

typedef volatile union _CAM_REG_AWB_GAIN1_0_
{
    volatile struct
    {
        FIELD AWB_GAIN1_R               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_GAIN1_G               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_GAIN1_0;

typedef volatile union _CAM_REG_AWB_GAIN1_1_
{
    volatile struct
    {
        FIELD AWB_GAIN1_B               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_GAIN1_1;

typedef volatile union _CAM_REG_AWB_LMT1_0_
{
    volatile struct
    {
        FIELD AWB_LMT1_R                : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_LMT1_G                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_LMT1_0;

typedef volatile union _CAM_REG_AWB_LMT1_1_
{
    volatile struct
    {
        FIELD AWB_LMT1_B                : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_LMT1_1;

typedef volatile union _CAM_REG_AWB_LOW_THR_
{
    volatile struct
    {
        FIELD AWB_LOW_THR0              : 8;
        FIELD AWB_LOW_THR1              : 8;
        FIELD AWB_LOW_THR2              : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_LOW_THR;

typedef volatile union _CAM_REG_AWB_HI_THR_
{
    volatile struct
    {
        FIELD AWB_HI_THR0               : 8;
        FIELD AWB_HI_THR1               : 8;
        FIELD AWB_HI_THR2               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_HI_THR;

typedef volatile union _CAM_REG_AWB_PIXEL_CNT0_
{
    volatile struct
    {
        FIELD AWB_PIXEL_CNT0            : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_PIXEL_CNT0;

typedef volatile union _CAM_REG_AWB_PIXEL_CNT1_
{
    volatile struct
    {
        FIELD AWB_PIXEL_CNT1            : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_PIXEL_CNT1;

typedef volatile union _CAM_REG_AWB_PIXEL_CNT2_
{
    volatile struct
    {
        FIELD AWB_PIXEL_CNT2            : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_PIXEL_CNT2;

typedef volatile union _CAM_REG_AWB_ERR_THR_
{
    volatile struct
    {
        FIELD AWB_ERR_THR               : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_ERR_SFT               : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_ERR_THR;

typedef volatile union _CAM_REG_AWB_ROT_
{
    volatile struct
    {
        FIELD AWB_C                     : 10;
        FIELD rsv_10                    : 6;
        FIELD AWB_S                     : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_ROT;

typedef volatile union _CAM_REG_AWB_L0_X_
{
    volatile struct
    {
        FIELD AWB_L0_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L0_X;

typedef volatile union _CAM_REG_AWB_L0_Y_
{
    volatile struct
    {
        FIELD AWB_L0_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L0_Y;

typedef volatile union _CAM_REG_AWB_L1_X_
{
    volatile struct
    {
        FIELD AWB_L1_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L1_X;

typedef volatile union _CAM_REG_AWB_L1_Y_
{
    volatile struct
    {
        FIELD AWB_L1_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L1_Y;

typedef volatile union _CAM_REG_AWB_L2_X_
{
    volatile struct
    {
        FIELD AWB_L2_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L2_X;

typedef volatile union _CAM_REG_AWB_L2_Y_
{
    volatile struct
    {
        FIELD AWB_L2_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L2_Y;

typedef volatile union _CAM_REG_AWB_L3_X_
{
    volatile struct
    {
        FIELD AWB_L3_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L3_X;

typedef volatile union _CAM_REG_AWB_L3_Y_
{
    volatile struct
    {
        FIELD AWB_L3_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L3_Y;

typedef volatile union _CAM_REG_AWB_L4_X_
{
    volatile struct
    {
        FIELD AWB_L4_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L4_X;

typedef volatile union _CAM_REG_AWB_L4_Y_
{
    volatile struct
    {
        FIELD AWB_L4_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L4_Y;

typedef volatile union _CAM_REG_AWB_L5_X_
{
    volatile struct
    {
        FIELD AWB_L5_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L5_X;

typedef volatile union _CAM_REG_AWB_L5_Y_
{
    volatile struct
    {
        FIELD AWB_L5_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L5_Y;

typedef volatile union _CAM_REG_AWB_L6_X_
{
    volatile struct
    {
        FIELD AWB_L6_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L6_X;

typedef volatile union _CAM_REG_AWB_L6_Y_
{
    volatile struct
    {
        FIELD AWB_L6_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L6_Y;

typedef volatile union _CAM_REG_AWB_L7_X_
{
    volatile struct
    {
        FIELD AWB_L7_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L7_X;

typedef volatile union _CAM_REG_AWB_L7_Y_
{
    volatile struct
    {
        FIELD AWB_L7_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L7_Y;

typedef volatile union _CAM_REG_AWB_L8_X_
{
    volatile struct
    {
        FIELD AWB_L8_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L8_X;

typedef volatile union _CAM_REG_AWB_L8_Y_
{
    volatile struct
    {
        FIELD AWB_L8_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L8_Y;

typedef volatile union _CAM_REG_AWB_L9_X_
{
    volatile struct
    {
        FIELD AWB_L9_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L9_X;

typedef volatile union _CAM_REG_AWB_L9_Y_
{
    volatile struct
    {
        FIELD AWB_L9_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_L9_Y;

typedef volatile union _CAM_REG_AWB_SPARE_
{
    volatile struct
    {
        FIELD AWB_SPARE                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_SPARE;

/* end MT6593_2091_raw_awb.xml*/

/* start MT6593_2092_raw_ae.xml*/
typedef volatile union _CAM_REG_AE_HST_CTL_
{
    volatile struct
    {
        FIELD AE_HST0_EN                : 1;
        FIELD AE_HST1_EN                : 1;
        FIELD AE_HST2_EN                : 1;
        FIELD AE_HST3_EN                : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST_CTL;

typedef volatile union _CAM_REG_AE_GAIN2_0_
{
    volatile struct
    {
        FIELD AE_GAIN2_R                : 12;
        FIELD rsv_12                    : 4;
        FIELD AE_GAIN2_G                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_GAIN2_0;

typedef volatile union _CAM_REG_AE_GAIN2_1_
{
    volatile struct
    {
        FIELD AE_GAIN2_B                : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_GAIN2_1;

typedef volatile union _CAM_REG_AE_LMT2_0_
{
    volatile struct
    {
        FIELD AE_LMT2_R                 : 12;
        FIELD rsv_12                    : 4;
        FIELD AE_LMT2_G                 : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_LMT2_0;

typedef volatile union _CAM_REG_AE_LMT2_1_
{
    volatile struct
    {
        FIELD AE_LMT2_B                 : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_LMT2_1;

typedef volatile union _CAM_REG_AE_RC_CNV_0_
{
    volatile struct
    {
        FIELD AE_RC_CNV00               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV01               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RC_CNV_0;

typedef volatile union _CAM_REG_AE_RC_CNV_1_
{
    volatile struct
    {
        FIELD AE_RC_CNV02               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV10               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RC_CNV_1;

typedef volatile union _CAM_REG_AE_RC_CNV_2_
{
    volatile struct
    {
        FIELD AE_RC_CNV11               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV12               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RC_CNV_2;

typedef volatile union _CAM_REG_AE_RC_CNV_3_
{
    volatile struct
    {
        FIELD AE_RC_CNV20               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV21               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RC_CNV_3;

typedef volatile union _CAM_REG_AE_RC_CNV_4_
{
    volatile struct
    {
        FIELD AE_RC_CNV22               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_ACC                 : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_RC_CNV_4;

typedef volatile union _CAM_REG_AE_YGAMMA_0_
{
    volatile struct
    {
        FIELD Y_GMR1                    : 8;
        FIELD Y_GMR2                    : 8;
        FIELD Y_GMR3                    : 8;
        FIELD Y_GMR4                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_YGAMMA_0;

typedef volatile union _CAM_REG_AE_YGAMMA_1_
{
    volatile struct
    {
        FIELD Y_GMR5                    : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_YGAMMA_1;

typedef volatile union _CAM_REG_AE_HST_SET_
{
    volatile struct
    {
        FIELD AE_BIN_MODE_0             : 2;
        FIELD rsv_2                     : 2;
        FIELD AE_BIN_MODE_1             : 2;
        FIELD rsv_6                     : 2;
        FIELD AE_BIN_MODE_2             : 2;
        FIELD rsv_10                    : 2;
        FIELD AE_BIN_MODE_3             : 2;
        FIELD rsv_14                    : 2;
        FIELD AE_COLOR_MODE_0           : 3;
        FIELD rsv_19                    : 1;
        FIELD AE_COLOR_MODE_1           : 3;
        FIELD rsv_23                    : 1;
        FIELD AE_COLOR_MODE_2           : 3;
        FIELD rsv_27                    : 1;
        FIELD AE_COLOR_MODE_3           : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST_SET;

typedef volatile union _CAM_REG_AE_HST0_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_0                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_0                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_0                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_0                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST0_RNG;

typedef volatile union _CAM_REG_AE_HST1_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_1                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_1                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_1                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_1                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST1_RNG;

typedef volatile union _CAM_REG_AE_HST2_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_2                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_2                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_2                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_2                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST2_RNG;

typedef volatile union _CAM_REG_AE_HST3_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_3                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_3                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_3                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_3                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_HST3_RNG;

typedef volatile union _CAM_REG_AE_SPARE_
{
    volatile struct
    {
        FIELD AE_SPARE                  : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_SPARE;

/* end MT6593_2092_raw_ae.xml*/

/* start MT6593_210_raw_sgg.xml*/
typedef volatile union _CAM_REG_SGG1_PGN_
{
    volatile struct
    {
        FIELD SGG1_GAIN                 : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG1_PGN;

typedef volatile union _CAM_REG_SGG1_GMRC_1_
{
    volatile struct
    {
        FIELD SGG1_GMR_1                : 8;
        FIELD SGG1_GMR_2                : 8;
        FIELD SGG1_GMR_3                : 8;
        FIELD SGG1_GMR_4                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG1_GMRC_1;

typedef volatile union _CAM_REG_SGG1_GMRC_2_
{
    volatile struct
    {
        FIELD SGG1_GMR_5                : 8;
        FIELD SGG1_GMR_6                : 8;
        FIELD SGG1_GMR_7                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG1_GMRC_2;

/* end MT6593_210_raw_sgg.xml*/

/* start MT6593_211_raw_af.xml*/
typedef volatile union _CAM_REG_AF_CON_
{
    volatile struct
    {
        FIELD AF_DECI                   : 2;
        FIELD AF_ZZ                     : 1;
        FIELD AF_ODD                    : 1;
        FIELD AF_TARX                   : 3;
        FIELD rsv_7                     : 1;
        FIELD AF_TARY                   : 3;
        FIELD rsv_11                    : 17;
        FIELD RESERVED                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_CON;

typedef volatile union _CAM_REG_AF_WINX_1_
{
    volatile struct
    {
        FIELD AF_WINX_0                 : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINX_1                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINX_1;

typedef volatile union _CAM_REG_AF_WINX_2_
{
    volatile struct
    {
        FIELD AF_WINX_2                 : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINX_3                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINX_2;

typedef volatile union _CAM_REG_AF_WINX_3_
{
    volatile struct
    {
        FIELD AF_WINX_4                 : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINX_5                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINX_3;

typedef volatile union _CAM_REG_AF_WINY_1_
{
    volatile struct
    {
        FIELD AF_WINY_0                 : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINY_1                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINY_1;

typedef volatile union _CAM_REG_AF_WINY_2_
{
    volatile struct
    {
        FIELD AF_WINY_2                 : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINY_3                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINY_2;

typedef volatile union _CAM_REG_AF_WINY_3_
{
    volatile struct
    {
        FIELD AF_WINY_4                 : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_WINY_5                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_WINY_3;

typedef volatile union _CAM_REG_AF_SIZE_
{
    volatile struct
    {
        FIELD AF_WIN_WD                 : 10;
        FIELD rsv_10                    : 6;
        FIELD AF_WIN_HT                 : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_SIZE;

typedef volatile union _CAM_REG_AF_FLT_1_
{
    volatile struct
    {
        FIELD AF_FLT_P_1                : 8;
        FIELD AF_FLT_P_2                : 8;
        FIELD AF_FLT_P_3                : 8;
        FIELD AF_FLT_P_4                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLT_1;

typedef volatile union _CAM_REG_AF_FLT_2_
{
    volatile struct
    {
        FIELD AF_FLT_P_5                : 8;
        FIELD AF_FLT_P_6                : 8;
        FIELD AF_FLT_P_7                : 8;
        FIELD AF_FLT_P_8                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLT_2;

typedef volatile union _CAM_REG_AF_FLT_3_
{
    volatile struct
    {
        FIELD AF_FLT_P_9                : 8;
        FIELD AF_FLT_P_10               : 8;
        FIELD AF_FLT_P_11               : 8;
        FIELD AF_FLT_P_12               : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLT_3;

typedef volatile union _CAM_REG_AF_TH_
{
    volatile struct
    {
        FIELD AF_TH_0                   : 8;
        FIELD rsv_8                     : 8;
        FIELD AF_TH_1                   : 8;
        FIELD AF_TH_MODE                : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_TH;

typedef volatile union _CAM_REG_AF_FLO_WIN_1_
{
    volatile struct
    {
        FIELD AF_FLO_WINX_1             : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_FLO_WINY_1             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_WIN_1;

typedef volatile union _CAM_REG_AF_FLO_SIZE_1_
{
    volatile struct
    {
        FIELD AF_FLO_WD_1               : 12;
        FIELD rsv_12                    : 4;
        FIELD AF_FLO_HT_1               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_SIZE_1;

typedef volatile union _CAM_REG_AF_FLO_WIN_2_
{
    volatile struct
    {
        FIELD AF_FLO_WINX_2             : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_FLO_WINY_2             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_WIN_2;

typedef volatile union _CAM_REG_AF_FLO_SIZE_2_
{
    volatile struct
    {
        FIELD AF_FLO_WD_2               : 12;
        FIELD rsv_12                    : 4;
        FIELD AF_FLO_HT_2               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_SIZE_2;

typedef volatile union _CAM_REG_AF_FLO_WIN_3_
{
    volatile struct
    {
        FIELD AF_FLO_WINX_3             : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_FLO_WINY_3             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_WIN_3;

typedef volatile union _CAM_REG_AF_FLO_SIZE_3_
{
    volatile struct
    {
        FIELD AF_FLO_WD_3               : 12;
        FIELD rsv_12                    : 4;
        FIELD AF_FLO_HT_3               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_SIZE_3;

typedef volatile union _CAM_REG_AF_FLO_TH_
{
    volatile struct
    {
        FIELD AF_FLO_TH_0               : 8;
        FIELD rsv_8                     : 8;
        FIELD AF_FLO_TH_1               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_TH;

typedef volatile union _CAM_REG_AF_IMAGE_SIZE_
{
    volatile struct
    {
        FIELD AF_IMAGE_WD               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_IMAGE_SIZE;

typedef volatile union _CAM_REG_AF_FLT_4_
{
    volatile struct
    {
        FIELD AF_FLT_X_0                : 11;
        FIELD rsv_11                    : 5;
        FIELD AF_FLT_X_1                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLT_4;

typedef volatile union _CAM_REG_AF_FLT_5_
{
    volatile struct
    {
        FIELD AF_FLT_X_2                : 11;
        FIELD rsv_11                    : 5;
        FIELD AF_FLT_X_3                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLT_5;

typedef volatile union _CAM_REG_AF_STAT_L_
{
    volatile struct
    {
        FIELD AF_STAT_LSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_STAT_L;

typedef volatile union _CAM_REG_AF_STAT_M_
{
    volatile struct
    {
        FIELD AF_STAT_MSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_STAT_M;

typedef volatile union _CAM_REG_AF_FLO_STAT_1L_
{
    volatile struct
    {
        FIELD AF_STAT_LSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_1L;

typedef volatile union _CAM_REG_AF_FLO_STAT_1M_
{
    volatile struct
    {
        FIELD AF_STAT_MSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_1M;

typedef volatile union _CAM_REG_AF_FLO_STAT_1V_
{
    volatile struct
    {
        FIELD AF_STAT_VER               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_1V;

typedef volatile union _CAM_REG_AF_FLO_STAT_2L_
{
    volatile struct
    {
        FIELD AF_STAT_LSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_2L;

typedef volatile union _CAM_REG_AF_FLO_STAT_2M_
{
    volatile struct
    {
        FIELD AF_STAT_MSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_2M;

typedef volatile union _CAM_REG_AF_FLO_STAT_2V_
{
    volatile struct
    {
        FIELD AF_STAT_VER               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_2V;

typedef volatile union _CAM_REG_AF_FLO_STAT_3L_
{
    volatile struct
    {
        FIELD AF_STAT_LSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_3L;

typedef volatile union _CAM_REG_AF_FLO_STAT_3M_
{
    volatile struct
    {
        FIELD AF_STAT_MSB               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_3M;

typedef volatile union _CAM_REG_AF_FLO_STAT_3V_
{
    volatile struct
    {
        FIELD AF_STAT_VER               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_FLO_STAT_3V;

/* end MT6593_211_raw_af.xml*/

/* start MT6593_2111_raw_w2g.xml*/
typedef volatile union _CAM_REG_W2G_BLD_
{
    volatile struct
    {
        FIELD W2G_BLD_SL                : 5;
        FIELD rsv_5                     : 3;
        FIELD W2G_EXT_SL                : 5;
        FIELD rsv_13                    : 3;
        FIELD W2G_EXT_DSL               : 5;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_BLD;

typedef volatile union _CAM_REG_W2G_TH_1_
{
    volatile struct
    {
        FIELD W2G_DPF                   : 1;
        FIELD rsv_1                     : 3;
        FIELD W2G_FTH                   : 12;
        FIELD W2G_NTH                   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_TH_1;

typedef volatile union _CAM_REG_W2G_TH_2_
{
    volatile struct
    {
        FIELD W2G_OFST                  : 12;
        FIELD W2G_DTH                   : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_TH_2;

typedef volatile union _CAM_REG_W2G_CTL_OFT_
{
    volatile struct
    {
        FIELD W2G_H_START               : 13;
        FIELD rsv_13                    : 3;
        FIELD W2G_H_END                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_CTL_OFT;

/* end MT6593_2111_raw_w2g.xml*/

/* start MT6593_2112_raw_wbn.xml*/
typedef volatile union _CAM_REG_WBN_SIZE_
{
    volatile struct
    {
        FIELD WBN_IN_H                  : 13;
        FIELD rsv_13                    : 3;
        FIELD WBN_IN_V                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_WBN_SIZE;

typedef volatile union _CAM_REG_WBN_MODE_
{
    volatile struct
    {
        FIELD WBN_ACC                   : 3;
        FIELD rsv_3                     : 1;
        FIELD WBN_RGBC_ID               : 2;
        FIELD rsv_6                     : 2;
        FIELD WBN_RGBC_DIR              : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_WBN_MODE;

/* end MT6593_2112_raw_wbn.xml*/

/* start MT6593_212_raw_flk.xml*/
typedef volatile union _CAM_REG_FLK_CON_
{
    volatile struct
    {
        FIELD RESERVED                  : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_CON;

typedef volatile union _CAM_REG_FLK_OFST_
{
    volatile struct
    {
        FIELD FLK_OFST_X                : 12;
        FIELD rsv_12                    : 4;
        FIELD FLK_OFST_Y                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_OFST;

typedef volatile union _CAM_REG_FLK_SIZE_
{
    volatile struct
    {
        FIELD FLK_SIZE_X                : 12;
        FIELD rsv_12                    : 4;
        FIELD FLK_SIZE_Y                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_SIZE;

typedef volatile union _CAM_REG_FLK_NUM_
{
    volatile struct
    {
        FIELD FLK_NUM_X                 : 3;
        FIELD rsv_3                     : 1;
        FIELD FLK_NUM_Y                 : 3;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_FLK_NUM;

/* end MT6593_212_raw_flk.xml*/

/* start MT6593_213_raw_lcs.xml*/
typedef volatile union _CAM_REG_LCS_CON_
{
    volatile struct
    {
        FIELD LCS_LG2                   : 1;
        FIELD rsv_1                     : 15;
        FIELD LCS_OUT_WD                : 7;
        FIELD rsv_23                    : 1;
        FIELD LCS_OUT_HT                : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_CON;

typedef volatile union _CAM_REG_LCS_ST_
{
    volatile struct
    {
        FIELD LCS_START_J               : 13;
        FIELD rsv_13                    : 3;
        FIELD LCS_START_I               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_ST;

typedef volatile union _CAM_REG_LCS_AWS_
{
    volatile struct
    {
        FIELD LCS_IN_WD                 : 13;
        FIELD rsv_13                    : 3;
        FIELD LCS_IN_HT                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_AWS;

typedef volatile union _CAM_REG_LCS_FLR_
{
    volatile struct
    {
        FIELD LCS_FLR_OFST              : 7;
        FIELD LCS_FLR_GAIN              : 9;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_FLR;

typedef volatile union _CAM_REG_LCS_LRZR_1_
{
    volatile struct
    {
        FIELD LCS_LRZR_X                : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_LRZR_1;

typedef volatile union _CAM_REG_LCS_LRZR_2_
{
    volatile struct
    {
        FIELD LCS_LRZR_Y                : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_LRZR_2;

/* end MT6593_213_raw_lcs.xml*/

/* start MT6593_214_raw_rrz.xml*/
typedef volatile union _CAM_REG_RRZ_CTL_
{
    volatile struct
    {
        FIELD RRZ_HORI_EN               : 1;
        FIELD RRZ_VERT_EN               : 1;
        FIELD rsv_2                     : 14;
        FIELD RRZ_HORI_TBL_SEL          : 5;
        FIELD RRZ_VERT_TBL_SEL          : 5;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_CTL;

typedef volatile union _CAM_REG_RRZ_IN_IMG_
{
    volatile struct
    {
        FIELD RRZ_IN_WD                 : 13;
        FIELD rsv_13                    : 3;
        FIELD RRZ_IN_HT                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_IN_IMG;

typedef volatile union _CAM_REG_RRZ_OUT_IMG_
{
    volatile struct
    {
        FIELD RRZ_OUT_WD                : 13;
        FIELD rsv_13                    : 3;
        FIELD RRZ_OUT_HT                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_OUT_IMG;

typedef volatile union _CAM_REG_RRZ_HORI_STEP_
{
    volatile struct
    {
        FIELD RRZ_HORI_STEP             : 17;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_HORI_STEP;

typedef volatile union _CAM_REG_RRZ_VERT_STEP_
{
    volatile struct
    {
        FIELD RRZ_VERT_STEP             : 17;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_VERT_STEP;

typedef volatile union _CAM_REG_RRZ_HORI_INT_OFST_
{
    volatile struct
    {
        FIELD RRZ_HORI_INT_OFST         : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_HORI_INT_OFST;

typedef volatile union _CAM_REG_RRZ_HORI_SUB_OFST_
{
    volatile struct
    {
        FIELD RRZ_HORI_SUB_OFST         : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_HORI_SUB_OFST;

typedef volatile union _CAM_REG_RRZ_VERT_INT_OFST_
{
    volatile struct
    {
        FIELD RRZ_VERT_INT_OFST         : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_VERT_INT_OFST;

typedef volatile union _CAM_REG_RRZ_VERT_SUB_OFST_
{
    volatile struct
    {
        FIELD RRZ_VERT_SUB_OFST         : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_VERT_SUB_OFST;

typedef volatile union _CAM_REG_RRZ_MODE_TH_
{
    volatile struct
    {
        FIELD RRZ_TH_MD                 : 8;
        FIELD RRZ_TH_HI                 : 8;
        FIELD RRZ_TH_LO                 : 8;
        FIELD RRZ_TH_MD2                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_MODE_TH;

typedef volatile union _CAM_REG_RRZ_MODE_CTL_
{
    volatile struct
    {
        FIELD RRZ_PRF_BLD               : 9;
        FIELD RRZ_PRF                   : 2;
        FIELD RRZ_BLD_SL                : 5;
        FIELD RRZ_CR_MODE               : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_MODE_CTL;

/* end MT6593_214_raw_rrz.xml*/

/* start MT6593_216_raw_bpc.xml*/
typedef volatile union _CAM_REG_BPC_CON_
{
    volatile struct
    {
        FIELD BPC_EN                    : 1;
        FIELD rsv_1                     : 3;
        FIELD BPC_LUT_EN                : 1;
        FIELD BPC_TABLE_END_MODE        : 1;
        FIELD rsv_6                     : 6;
        FIELD BPC_DTC_MODE              : 2;
        FIELD BPC_CS_MODE               : 2;
        FIELD BPC_CRC_MODE              : 2;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_CON;

typedef volatile union _CAM_REG_BPC_TH1_
{
    volatile struct
    {
        FIELD BPC_TH_LWB                : 12;
        FIELD BPC_TH_Y                  : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TH1;

typedef volatile union _CAM_REG_BPC_TH2_
{
    volatile struct
    {
        FIELD BPC_TH_UPB                : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TH2;

typedef volatile union _CAM_REG_BPC_TH3_
{
    volatile struct
    {
        FIELD BPC_TH_XA                 : 12;
        FIELD BPC_TH_XB                 : 12;
        FIELD BPC_TH_SLA                : 4;
        FIELD BPC_TH_SLB                : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TH3;

typedef volatile union _CAM_REG_BPC_TH4_
{
    volatile struct
    {
        FIELD BPC_DK_TH_XA              : 12;
        FIELD BPC_DK_TH_XB              : 12;
        FIELD BPC_DK_TH_SLA             : 4;
        FIELD BPC_DK_TH_SLB             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TH4;

typedef volatile union _CAM_REG_BPC_DTC_
{
    volatile struct
    {
        FIELD BPC_RNG                   : 4;
        FIELD BPC_CS_RNG                : 3;
        FIELD rsv_7                     : 1;
        FIELD BPC_CT_LV                 : 4;
        FIELD rsv_12                    : 4;
        FIELD BPC_TH_MUL                : 4;
        FIELD rsv_20                    : 4;
        FIELD BPC_NO_LV                 : 3;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_DTC;

typedef volatile union _CAM_REG_BPC_COR_
{
    volatile struct
    {
        FIELD BPC_DIR_MAX               : 8;
        FIELD BPC_DIR_TH                : 8;
        FIELD BPC_RANK_IDX              : 3;
        FIELD rsv_19                    : 5;
        FIELD BPC_DIR_TH2               : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_COR;

typedef volatile union _CAM_REG_BPC_TBLI1_
{
    volatile struct
    {
        FIELD BPC_XOFFSET               : 13;
        FIELD rsv_13                    : 3;
        FIELD BPC_YOFFSET               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TBLI1;

typedef volatile union _CAM_REG_BPC_TBLI2_
{
    volatile struct
    {
        FIELD BPC_XSIZE                 : 13;
        FIELD rsv_13                    : 3;
        FIELD BPC_YSIZE                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TBLI2;

typedef volatile union _CAM_REG_BPC_TH1_C_
{
    volatile struct
    {
        FIELD BPC_C_TH_LWB              : 12;
        FIELD BPC_C_TH_Y                : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TH1_C;

typedef volatile union _CAM_REG_BPC_TH2_C_
{
    volatile struct
    {
        FIELD BPC_C_TH_UPB              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TH2_C;

typedef volatile union _CAM_REG_BPC_TH3_C_
{
    volatile struct
    {
        FIELD BPC_C_TH_XA               : 12;
        FIELD BPC_C_TH_XB               : 12;
        FIELD BPC_C_TH_SLA              : 4;
        FIELD BPC_C_TH_SLB              : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_TH3_C;

typedef volatile union _CAM_REG_BPC_RMM1_
{
    volatile struct
    {
        FIELD BPC_RMM_OSC_TH            : 12;
        FIELD BPC_RMM_SEDIR_SL          : 3;
        FIELD rsv_15                    : 1;
        FIELD BPC_RMM_SEBLD_WD          : 3;
        FIELD rsv_19                    : 1;
        FIELD BPC_RMM_LEBLD_WD          : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_RMM1;

typedef volatile union _CAM_REG_BPC_RMM2_
{
    volatile struct
    {
        FIELD BPC_RMM_MO_MODE           : 1;
        FIELD rsv_1                     : 3;
        FIELD BPC_RMM_MOBLD_FT          : 3;
        FIELD rsv_7                     : 1;
        FIELD BPC_RMM_MOTH_RATIO        : 4;
        FIELD rsv_12                    : 4;
        FIELD BPC_RMM_HORI_ADDWT        : 5;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_RMM2;

typedef volatile union _CAM_REG_BPC_RMM_REVG_1_
{
    volatile struct
    {
        FIELD BPC_RMM_REVG_R            : 11;
        FIELD rsv_11                    : 5;
        FIELD BPC_RMM_REVG_GR           : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_RMM_REVG_1;

typedef volatile union _CAM_REG_BPC_RMM_REVG_2_
{
    volatile struct
    {
        FIELD BPC_RMM_REVG_B            : 11;
        FIELD rsv_11                    : 5;
        FIELD BPC_RMM_REVG_GB           : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_RMM_REVG_2;

typedef volatile union _CAM_REG_BPC_RMM_LEOS_
{
    volatile struct
    {
        FIELD BPC_RMM_LEOS_GRAY         : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_RMM_LEOS;

typedef volatile union _CAM_REG_BPC_RMM_GCNT_
{
    volatile struct
    {
        FIELD BPC_RMM_GRAY_CNT          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_RMM_GCNT;

/* end MT6593_216_raw_bpc.xml*/

/* start MT6593_216_raw_ct.xml*/
typedef volatile union _CAM_REG_NR1_CON_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD NR1_CT_EN                 : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR1_CON;

typedef volatile union _CAM_REG_NR1_CT_CON_
{
    volatile struct
    {
        FIELD NR1_CT_MD                 : 2;
        FIELD NR1_CT_MD2                : 2;
        FIELD NR1_CT_THRD               : 10;
        FIELD rsv_14                    : 2;
        FIELD NR1_MBND                  : 10;
        FIELD rsv_26                    : 2;
        FIELD NR1_CT_SLOPE              : 2;
        FIELD NR1_CT_DIV                : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR1_CT_CON;

typedef volatile union _CAM_REG_BNR_RSV1_
{
    volatile struct
    {
        FIELD RSV1                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BNR_RSV1;

typedef volatile union _CAM_REG_BNR_RSV2_
{
    volatile struct
    {
        FIELD RSV2                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BNR_RSV2;

/* end MT6593_216_raw_ct.xml*/

/* start MT6593_217_raw_pgn.xml*/
typedef volatile union _CAM_REG_PGN_SATU_1_
{
    volatile struct
    {
        FIELD PGN_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_SATU_1;

typedef volatile union _CAM_REG_PGN_SATU_2_
{
    volatile struct
    {
        FIELD PGN_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_SATU_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_SATU_2;

typedef volatile union _CAM_REG_PGN_GAIN_1_
{
    volatile struct
    {
        FIELD PGN_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_GAIN_1;

typedef volatile union _CAM_REG_PGN_GAIN_2_
{
    volatile struct
    {
        FIELD PGN_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD PGN_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_GAIN_2;

typedef volatile union _CAM_REG_PGN_OFST_1_
{
    volatile struct
    {
        FIELD PGN_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_OFST_1;

typedef volatile union _CAM_REG_PGN_OFST_2_
{
    volatile struct
    {
        FIELD PGN_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD PGN_OFST_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_PGN_OFST_2;

/* end MT6593_217_raw_pgn.xml*/

/* start MT6593_300_rgb_cfa.xml*/
typedef volatile union _CAM_REG_DM_O_BYP_
{
    volatile struct
    {
        FIELD DM_BYP                    : 1;
        FIELD DM_DEBUG_MODE             : 2;
        FIELD DM_HF_LSC_MAX_BYP         : 1;
        FIELD DM_FG_MODE                : 1;
        FIELD DM_SPARE1                 : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_BYP;

typedef volatile union _CAM_REG_DM_O_ED_FLAT_
{
    volatile struct
    {
        FIELD DM_FLAT_DET_MODE          : 1;
        FIELD DM_STEP_DET_MODE          : 1;
        FIELD rsv_2                     : 6;
        FIELD DM_FLAT_TH                : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_ED_FLAT;

typedef volatile union _CAM_REG_DM_O_ED_NYQ_
{
    volatile struct
    {
        FIELD DM_NYQ_TH_1               : 8;
        FIELD DM_NYQ_TH_2               : 8;
        FIELD DM_NYQ_TH_3               : 8;
        FIELD DM_HF_NYQ_GAIN            : 2;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_ED_NYQ;

typedef volatile union _CAM_REG_DM_O_ED_STEP_
{
    volatile struct
    {
        FIELD DM_STEP_TH_1              : 8;
        FIELD DM_STEP_TH_2              : 8;
        FIELD DM_STEP_TH_3              : 8;
        FIELD rsv_24                    : 7;
        FIELD DM_RB_MODE                : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_ED_STEP;

typedef volatile union _CAM_REG_DM_O_RGB_HF_
{
    volatile struct
    {
        FIELD DM_HF_CORE_GAIN           : 5;
        FIELD DM_ROUGH_RB_F             : 5;
        FIELD DM_ROUGH_RB_D             : 5;
        FIELD DM_ROUGH_G_F              : 5;
        FIELD DM_ROUGH_G_D              : 5;
        FIELD DM_RB_MODE_F              : 2;
        FIELD DM_RB_MODE_D              : 2;
        FIELD DM_RB_MODE_HV             : 2;
        FIELD DM_SSG_MODE               : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_RGB_HF;

typedef volatile union _CAM_REG_DM_O_DOT_
{
    volatile struct
    {
        FIELD DM_DOT_B_TH               : 8;
        FIELD DM_DOT_W_TH               : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_DOT;

typedef volatile union _CAM_REG_DM_O_F1_ACT_
{
    volatile struct
    {
        FIELD DM_F1_TH_1                : 9;
        FIELD DM_F1_TH_2                : 9;
        FIELD DM_F1_SLOPE_1             : 2;
        FIELD DM_F1_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F1_ACT;

typedef volatile union _CAM_REG_DM_O_F2_ACT_
{
    volatile struct
    {
        FIELD DM_F2_TH_1                : 9;
        FIELD DM_F2_TH_2                : 9;
        FIELD DM_F2_SLOPE_1             : 2;
        FIELD DM_F2_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F2_ACT;

typedef volatile union _CAM_REG_DM_O_F3_ACT_
{
    volatile struct
    {
        FIELD DM_F3_TH_1                : 9;
        FIELD DM_F3_TH_2                : 9;
        FIELD DM_F3_SLOPE_1             : 2;
        FIELD DM_F3_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F3_ACT;

typedef volatile union _CAM_REG_DM_O_F4_ACT_
{
    volatile struct
    {
        FIELD DM_F4_TH_1                : 9;
        FIELD DM_F4_TH_2                : 9;
        FIELD DM_F4_SLOPE_1             : 2;
        FIELD DM_F4_SLOPE_2             : 2;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F4_ACT;

typedef volatile union _CAM_REG_DM_O_F1_L_
{
    volatile struct
    {
        FIELD DM_F1_LLUT_Y0             : 5;
        FIELD DM_F1_LLUT_Y1             : 5;
        FIELD DM_F1_LLUT_Y2             : 5;
        FIELD DM_F1_LLUT_Y3             : 5;
        FIELD DM_F1_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F1_L;

typedef volatile union _CAM_REG_DM_O_F2_L_
{
    volatile struct
    {
        FIELD DM_F2_LLUT_Y0             : 5;
        FIELD DM_F2_LLUT_Y1             : 5;
        FIELD DM_F2_LLUT_Y2             : 5;
        FIELD DM_F2_LLUT_Y3             : 5;
        FIELD DM_F2_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F2_L;

typedef volatile union _CAM_REG_DM_O_F3_L_
{
    volatile struct
    {
        FIELD DM_F3_LLUT_Y0             : 5;
        FIELD DM_F3_LLUT_Y1             : 5;
        FIELD DM_F3_LLUT_Y2             : 5;
        FIELD DM_F3_LLUT_Y3             : 5;
        FIELD DM_F3_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F3_L;

typedef volatile union _CAM_REG_DM_O_F4_L_
{
    volatile struct
    {
        FIELD DM_F4_LLUT_Y0             : 5;
        FIELD DM_F4_LLUT_Y1             : 5;
        FIELD DM_F4_LLUT_Y2             : 5;
        FIELD DM_F4_LLUT_Y3             : 5;
        FIELD DM_F4_LLUT_Y4             : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_F4_L;

typedef volatile union _CAM_REG_DM_O_HF_RB_
{
    volatile struct
    {
        FIELD DM_RB_DIFF_TH             : 10;
        FIELD DM_HF_CLIP                : 9;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_HF_RB;

typedef volatile union _CAM_REG_DM_O_HF_GAIN_
{
    volatile struct
    {
        FIELD DM_HF_GLOBL_GAIN          : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_HF_GAIN;

typedef volatile union _CAM_REG_DM_O_HF_COMP_
{
    volatile struct
    {
        FIELD DM_HF_LSC_GAIN0           : 4;
        FIELD DM_HF_LSC_GAIN1           : 4;
        FIELD DM_HF_LSC_GAIN2           : 4;
        FIELD DM_HF_LSC_GAIN3           : 4;
        FIELD DM_HF_UND_TH              : 8;
        FIELD DM_HF_UND_ACT_TH          : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_HF_COMP;

typedef volatile union _CAM_REG_DM_O_HF_CORIN_TH_
{
    volatile struct
    {
        FIELD DM_HF_CORIN_TH            : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_HF_CORIN_TH;

typedef volatile union _CAM_REG_DM_O_ACT_LUT_
{
    volatile struct
    {
        FIELD DM_ACT_LUT_Y0             : 5;
        FIELD DM_ACT_LUT_Y1             : 5;
        FIELD DM_ACT_LUT_Y2             : 5;
        FIELD DM_ACT_LUT_Y3             : 5;
        FIELD DM_ACT_LUT_Y4             : 5;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_ACT_LUT;

typedef volatile union _CAM_REG_DM_O_SPARE_
{
    volatile struct
    {
        FIELD DM_O_SPARE                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_SPARE;

typedef volatile union _CAM_REG_DM_O_BB_
{
    volatile struct
    {
        FIELD DM_BB_TH_1                : 9;
        FIELD rsv_9                     : 3;
        FIELD DM_BB_TH_2                : 9;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_DM_O_BB;

/* end MT6593_300_rgb_cfa.xml*/

/* start MT6593_301_rgb_ccl.xml*/
typedef volatile union _CAM_REG_CCL_GTC_
{
    volatile struct
    {
        FIELD CCL_GLOBL_TH              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCL_GTC;

typedef volatile union _CAM_REG_CCL_ADC_
{
    volatile struct
    {
        FIELD CCL_ADSAT_TH              : 12;
        FIELD rsv_12                    : 4;
        FIELD CCL_ADSAT_SLOPE           : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCL_ADC;

typedef volatile union _CAM_REG_CCL_BAC_
{
    volatile struct
    {
        FIELD CCL_B_OFST                : 12;
        FIELD CCL_B_SLOPE               : 4;
        FIELD CCL_B_DIFF                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCL_BAC;

/* end MT6593_301_rgb_ccl.xml*/

/* start MT6593_302_rgb_g2g.xml*/
typedef volatile union _CAM_REG_G2G_CNV_1_
{
    volatile struct
    {
        FIELD G2G_CNV_00                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_01                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CNV_1;

typedef volatile union _CAM_REG_G2G_CNV_2_
{
    volatile struct
    {
        FIELD G2G_CNV_02                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CNV_2;

typedef volatile union _CAM_REG_G2G_CNV_3_
{
    volatile struct
    {
        FIELD G2G_CNV_10                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_11                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CNV_3;

typedef volatile union _CAM_REG_G2G_CNV_4_
{
    volatile struct
    {
        FIELD G2G_CNV_12                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CNV_4;

typedef volatile union _CAM_REG_G2G_CNV_5_
{
    volatile struct
    {
        FIELD G2G_CNV_20                : 13;
        FIELD rsv_13                    : 3;
        FIELD G2G_CNV_21                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CNV_5;

typedef volatile union _CAM_REG_G2G_CNV_6_
{
    volatile struct
    {
        FIELD G2G_CNV_22                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CNV_6;

typedef volatile union _CAM_REG_G2G_CTRL_
{
    volatile struct
    {
        FIELD G2G_ACC                   : 4;
        FIELD G2G_MOFST_R               : 1;
        FIELD G2G_POFST_R               : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2G_CTRL;

/* end MT6593_302_rgb_g2g.xml*/

/* start MT6593_304_raw_unp.xml*/
typedef volatile union _CAM_REG_UNP_OFST_
{
    volatile struct
    {
        FIELD UNP_OFST_STB              : 5;
        FIELD rsv_5                     : 3;
        FIELD UNP_OFST_EDB              : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_UNP_OFST;

/* end MT6593_304_raw_unp.xml*/

/* start MT6593_305_yuv_c02.xml*/
typedef volatile union _CAM_REG_C02_CON_
{
    volatile struct
    {
        FIELD C02_TPIPE_EDGE            : 4;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_C02_CON;

typedef volatile union _CAM_REG_C02_CROP_CON1_
{
    volatile struct
    {
        FIELD C02_CROP_XSTART           : 13;
        FIELD rsv_13                    : 3;
        FIELD C02_CROP_XEND             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_C02_CROP_CON1;

typedef volatile union _CAM_REG_C02_CROP_CON2_
{
    volatile struct
    {
        FIELD C02_CROP_YSTART           : 13;
        FIELD rsv_13                    : 3;
        FIELD C02_CROP_YEND             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_C02_CROP_CON2;

/* end MT6593_305_yuv_c02.xml*/

/* start MT6593_306_rgb_mfb.xml*/
typedef volatile union _CAM_REG_MFB_CON_
{
    volatile struct
    {
        FIELD BLD_MODE                  : 3;
        FIELD rsv_3                     : 1;
        FIELD RESERVED                  : 4;
        FIELD rsv_8                     : 16;
        FIELD BLD_MM_TH                 : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_CON;

typedef volatile union _CAM_REG_MFB_LL_CON1_
{
    volatile struct
    {
        FIELD BLD_LL_OUT_XSIZE          : 13;
        FIELD rsv_13                    : 2;
        FIELD BLD_LL_OUT_XOFST          : 1;
        FIELD BLD_LL_OUT_YSIZE          : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_LL_CON1;

typedef volatile union _CAM_REG_MFB_LL_CON2_
{
    volatile struct
    {
        FIELD BLD_LL_MAX_WT             : 3;
        FIELD rsv_3                     : 5;
        FIELD BLD_LL_DT1                : 8;
        FIELD BLD_LL_TH1                : 8;
        FIELD BLD_LL_TH2                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_LL_CON2;

typedef volatile union _CAM_REG_MFB_LL_CON3_
{
    volatile struct
    {
        FIELD BLD_LL_BRZ_EN             : 1;
        FIELD rsv_1                     : 3;
        FIELD BLD_LL_DB_EN              : 1;
        FIELD rsv_5                     : 3;
        FIELD BLD_LL_DB_XDIST           : 4;
        FIELD BLD_LL_DB_YDIST           : 4;
        FIELD BLD_LL_TH_E               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_LL_CON3;

typedef volatile union _CAM_REG_MFB_LL_CON4_
{
    volatile struct
    {
        FIELD BLD_LL_FLT_MODE           : 2;
        FIELD rsv_2                     : 2;
        FIELD BLD_LL_FLT_WT_MODE1       : 3;
        FIELD rsv_7                     : 1;
        FIELD BLD_LL_FLT_WT_MODE2       : 2;
        FIELD rsv_10                    : 6;
        FIELD BLD_LL_CLIP_TH1           : 8;
        FIELD BLD_LL_CLIP_TH2           : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_LL_CON4;

typedef volatile union _CAM_REG_MFB_LL_CON5_
{
    volatile struct
    {
        FIELD BLD_LL_MX_M0              : 8;
        FIELD BLD_LL_MX_M1              : 8;
        FIELD BLD_LL_MX_DT              : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_LL_CON5;

typedef volatile union _CAM_REG_MFB_LL_CON6_
{
    volatile struct
    {
        FIELD BLD_LL_MX_B0              : 8;
        FIELD BLD_LL_MX_B1              : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFB_LL_CON6;

typedef volatile union _CAM_REG_CPG_SATU_1_
{
    volatile struct
    {
        FIELD CPG_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_SATU_1;

typedef volatile union _CAM_REG_CPG_SATU_2_
{
    volatile struct
    {
        FIELD CPG_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_SATU_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_SATU_2;

typedef volatile union _CAM_REG_CPG_GAIN_1_
{
    volatile struct
    {
        FIELD CPG_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD CPG_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_GAIN_1;

typedef volatile union _CAM_REG_CPG_GAIN_2_
{
    volatile struct
    {
        FIELD CPG_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD CPG_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_GAIN_2;

typedef volatile union _CAM_REG_CPG_OFST_1_
{
    volatile struct
    {
        FIELD CPG_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_OFST_1;

typedef volatile union _CAM_REG_CPG_OFST_2_
{
    volatile struct
    {
        FIELD CPG_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_OFST_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_OFST_2;


/* start MT6593_307_yuv_lce.xml*/
typedef volatile union _CAM_REG_LCE_CON_
{
    volatile struct
    {
        FIELD LCE_GLOB                  : 1;
        FIELD LCE_GLOB_VHALF            : 7;
        FIELD rsv_8                     : 20;
        FIELD RESERVED                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_CON;

typedef volatile union _CAM_REG_LCE_ZR_
{
    volatile struct
    {
        FIELD LCE_BCMK_X                : 15;
        FIELD rsv_15                    : 1;
        FIELD LCE_BCMK_Y                : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_ZR;

typedef volatile union _CAM_REG_LCE_QUA_
{
    volatile struct
    {
        FIELD LCE_PA                    : 7;
        FIELD LCE_PB                    : 9;
        FIELD LCE_BA                    : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_QUA;

typedef volatile union _CAM_REG_LCE_DGC_1_
{
    volatile struct
    {
        FIELD LCE_DG_1                  : 8;
        FIELD LCE_DG_2                  : 8;
        FIELD LCE_DG_3                  : 8;
        FIELD LCE_DG_4                  : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_DGC_1;

typedef volatile union _CAM_REG_LCE_DGC_2_
{
    volatile struct
    {
        FIELD LCE_DG_5                  : 8;
        FIELD LCE_DG_6                  : 8;
        FIELD LCE_DG_7                  : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_DGC_2;

typedef volatile union _CAM_REG_LCE_GM_
{
    volatile struct
    {
        FIELD LCE_GM_LB                 : 8;
        FIELD LCE_GM_TH                 : 6;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_GM;

typedef volatile union _CAM_REG_LCE_SLM_LB_
{
    volatile struct
    {
        FIELD LCE_SLM_LB                : 7;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_SLM_LB;

typedef volatile union _CAM_REG_LCE_SLM_SIZE_
{
    volatile struct
    {
        FIELD LCE_SLM_WD                : 7;
        FIELD rsv_7                     : 1;
        FIELD LCE_SLM_HT                : 7;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_SLM_SIZE;

typedef volatile union _CAM_REG_LCE_OFST_
{
    volatile struct
    {
        FIELD LCE_OFST_X                : 15;
        FIELD rsv_15                    : 1;
        FIELD LCE_OFST_Y                : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_OFST;

typedef volatile union _CAM_REG_LCE_BIAS_
{
    volatile struct
    {
        FIELD LCE_BIAS_X                : 3;
        FIELD rsv_3                     : 5;
        FIELD LCE_BIAS_Y                : 2;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_BIAS;

typedef volatile union _CAM_REG_LCE_IMAGE_SIZE_
{
    volatile struct
    {
        FIELD LCE_IMAGE_WD              : 13;
        FIELD rsv_13                    : 3;
        FIELD LCE_IMAGE_HT              : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCE_IMAGE_SIZE;

/* end MT6593_307_yuv_lce.xml*/

/* start MT6593_401_yuv_c42.xml*/
typedef volatile union _CAM_REG_C42_CON_
{
    volatile struct
    {
        FIELD C42_FILT_DIS              : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_C42_CON;

/* end MT6593_401_yuv_c42.xml*/

/* start MT6593_402_yuv_nbc.xml*/
typedef volatile union _CAM_REG_ANR_CON1_
{
    volatile struct
    {
        FIELD ANR_ENC                   : 1;
        FIELD ANR_ENY                   : 1;
        FIELD rsv_2                     : 2;
        FIELD ANR_SCALE_MODE            : 2;
        FIELD rsv_6                     : 2;
        FIELD ANR_FLT_MODE              : 3;
        FIELD rsv_11                    : 1;
        FIELD ANR_MODE                  : 1;
        FIELD rsv_13                    : 3;
        FIELD ANR_Y_LUMA_SCALE          : 4;
        FIELD rsv_20                    : 4;
        FIELD ANR_LCE_LINK              : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CON1;

typedef volatile union _CAM_REG_ANR_CON2_
{
    volatile struct
    {
        FIELD ANR_GNY                   : 6;
        FIELD rsv_6                     : 2;
        FIELD ANR_GNC                   : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CON2;

typedef volatile union _CAM_REG_ANR_CON3_
{
    volatile struct
    {
        FIELD ANR_IMPL_MODE             : 2;
        FIELD rsv_2                     : 2;
        FIELD ANR_C_MED_EN              : 2;
        FIELD rsv_6                     : 2;
        FIELD ANR_C_SM_EDGE             : 1;
        FIELD rsv_9                     : 3;
        FIELD ANR_QEC                   : 1;
        FIELD rsv_13                    : 3;
        FIELD ANR_QEC_VAL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CON3;

typedef volatile union _CAM_REG_ANR_YAD1_
{
    volatile struct
    {
        FIELD ANR_CEN_GAIN_LO_TH        : 5;
        FIELD rsv_5                     : 3;
        FIELD ANR_CEN_GAIN_HI_TH        : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_K_LO_TH               : 4;
        FIELD rsv_20                    : 4;
        FIELD ANR_K_HI_TH               : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_YAD1;

typedef volatile union _CAM_REG_ANR_YAD2_
{
    volatile struct
    {
        FIELD ANR_PTY_VGAIN             : 4;
        FIELD rsv_4                     : 4;
        FIELD ANR_PTY_GAIN_TH           : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_YAD2;

typedef volatile union _CAM_REG_ANR_4LUT1_
{
    volatile struct
    {
        FIELD ANR_Y_CPX1                : 8;
        FIELD ANR_Y_CPX2                : 8;
        FIELD ANR_Y_CPX3                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_4LUT1;

typedef volatile union _CAM_REG_ANR_4LUT2_
{
    volatile struct
    {
        FIELD ANR_Y_SCALE_CPY0          : 5;
        FIELD rsv_5                     : 3;
        FIELD ANR_Y_SCALE_CPY1          : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_Y_SCALE_CPY2          : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_Y_SCALE_CPY3          : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_4LUT2;

typedef volatile union _CAM_REG_ANR_4LUT3_
{
    volatile struct
    {
        FIELD ANR_Y_SCALE_SP0           : 5;
        FIELD rsv_5                     : 3;
        FIELD ANR_Y_SCALE_SP1           : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_Y_SCALE_SP2           : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_Y_SCALE_SP3           : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_4LUT3;

typedef volatile union _CAM_REG_ANR_PTY_
{
    volatile struct
    {
        FIELD ANR_PTY1                  : 8;
        FIELD ANR_PTY2                  : 8;
        FIELD ANR_PTY3                  : 8;
        FIELD ANR_PTY4                  : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_PTY;

typedef volatile union _CAM_REG_ANR_CAD_
{
    volatile struct
    {
        FIELD ANR_PTC_VGAIN             : 4;
        FIELD rsv_4                     : 4;
        FIELD ANR_PTC_GAIN_TH           : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_C_L_DIFF_TH           : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_CAD;

typedef volatile union _CAM_REG_ANR_PTC_
{
    volatile struct
    {
        FIELD ANR_PTC1                  : 8;
        FIELD ANR_PTC2                  : 8;
        FIELD ANR_PTC3                  : 8;
        FIELD ANR_PTC4                  : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_PTC;

typedef volatile union _CAM_REG_ANR_LCE1_
{
    volatile struct
    {
        FIELD ANR_LCE_C_GAIN            : 4;
        FIELD ANR_LCE_SCALE_GAIN        : 3;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_LCE1;

typedef volatile union _CAM_REG_ANR_LCE2_
{
    volatile struct
    {
        FIELD ANR_LCE_GAIN0             : 6;
        FIELD rsv_6                     : 2;
        FIELD ANR_LCE_GAIN1             : 6;
        FIELD rsv_14                    : 2;
        FIELD ANR_LCE_GAIN2             : 6;
        FIELD rsv_22                    : 2;
        FIELD ANR_LCE_GAIN3             : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_LCE2;

typedef volatile union _CAM_REG_ANR_HP1_
{
    volatile struct
    {
        FIELD ANR_HP_A                  : 8;
        FIELD ANR_HP_B                  : 6;
        FIELD rsv_14                    : 2;
        FIELD ANR_HP_C                  : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_HP_D                  : 4;
        FIELD ANR_HP_E                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_HP1;

typedef volatile union _CAM_REG_ANR_HP2_
{
    volatile struct
    {
        FIELD ANR_HP_S1                 : 4;
        FIELD ANR_HP_S2                 : 4;
        FIELD ANR_HP_X1                 : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_HP_F                  : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_HP2;

typedef volatile union _CAM_REG_ANR_HP3_
{
    volatile struct
    {
        FIELD ANR_HP_Y_GAIN_CLIP        : 7;
        FIELD rsv_7                     : 1;
        FIELD ANR_HP_Y_SP               : 5;
        FIELD rsv_13                    : 3;
        FIELD ANR_HP_Y_LO               : 8;
        FIELD ANR_HP_CLIP               : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_HP3;

typedef volatile union _CAM_REG_ANR_ACTY_
{
    volatile struct
    {
        FIELD ANR_ACT_TH_Y              : 8;
        FIELD ANR_ACT_BLD_BASE_Y        : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_ACT_SLANT_Y           : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_ACT_BLD_TH_Y          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_ACTY;

typedef volatile union _CAM_REG_ANR_ACTC_
{
    volatile struct
    {
        FIELD ANR_ACT_TH_C              : 8;
        FIELD ANR_ACT_BLD_BASE_C        : 7;
        FIELD rsv_15                    : 1;
        FIELD ANR_ACT_SLANT_C           : 5;
        FIELD rsv_21                    : 3;
        FIELD ANR_ACT_BLD_TH_C          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_ACTC;

typedef volatile union _CAM_REG_ANR_RSV1_
{
    volatile struct
    {
        FIELD RSV1                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_RSV1;

typedef volatile union _CAM_REG_ANR_RSV2_
{
    volatile struct
    {
        FIELD RSV2                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ANR_RSV2;

typedef volatile union _CAM_REG_CCR_CON_
{
    volatile struct
    {
        FIELD CCR_EN                    : 1;
        FIELD rsv_1                     : 4;
        FIELD CCR_OR_MODE               : 1;
        FIELD rsv_6                     : 2;
        FIELD CCR_UV_GAIN_MODE          : 1;
        FIELD rsv_9                     : 7;
        FIELD CCR_UV_GAIN2              : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPX3                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_CON;

typedef volatile union _CAM_REG_CCR_YLUT_
{
    volatile struct
    {
        FIELD CCR_Y_CPX1                : 8;
        FIELD CCR_Y_CPX2                : 8;
        FIELD CCR_Y_SP1                 : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPY1                : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_YLUT;

typedef volatile union _CAM_REG_CCR_UVLUT_
{
    volatile struct
    {
        FIELD CCR_UV_X1                 : 8;
        FIELD CCR_UV_X2                 : 8;
        FIELD CCR_UV_X3                 : 8;
        FIELD CCR_UV_GAIN1              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_UVLUT;

typedef volatile union _CAM_REG_CCR_YLUT2_
{
    volatile struct
    {
        FIELD CCR_Y_SP0                 : 7;
        FIELD rsv_7                     : 1;
        FIELD CCR_Y_SP2                 : 7;
        FIELD rsv_15                    : 1;
        FIELD CCR_Y_CPY0                : 7;
        FIELD rsv_23                    : 1;
        FIELD CCR_Y_CPY2                : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_YLUT2;

typedef volatile union _CAM_REG_CCR_SAT_CTRL_
{
    volatile struct
    {
        FIELD CCR_MODE                  : 1;
        FIELD rsv_1                     : 3;
        FIELD CCR_CEN_U                 : 6;
        FIELD rsv_10                    : 2;
        FIELD CCR_CEN_V                 : 6;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_SAT_CTRL;

typedef volatile union _CAM_REG_CCR_UVLUT_SP_
{
    volatile struct
    {
        FIELD CCR_UV_GAIN_SP1           : 10;
        FIELD rsv_10                    : 6;
        FIELD CCR_UV_GAIN_SP2           : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_CCR_UVLUT_SP;

/* end MT6593_402_yuv_nbc.xml*/

/* start MT6593_403_yuv_seee.xml*/
typedef volatile union _CAM_REG_SEEE_SRK_CTRL_
{
    volatile struct
    {
        FIELD rsv_0                     : 28;
        FIELD RESERVED                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SRK_CTRL;

typedef volatile union _CAM_REG_SEEE_CLIP_CTRL_
{
    volatile struct
    {
        FIELD SEEE_OVRSH_CLIP_EN        : 1;
        FIELD SEEE_OVRSH_CLIP_STR       : 3;
        FIELD SEEE_DOT_REDUC_EN         : 1;
        FIELD SEEE_DOT_WF               : 3;
        FIELD SEEE_DOT_TH               : 8;
        FIELD SEEE_RESP_CLIP            : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_CLIP_CTRL;

typedef volatile union _CAM_REG_SEEE_FLT_CTRL_1_
{
    volatile struct
    {
        FIELD SEEE_FLT_TH               : 8;
        FIELD SEEE_FLT_AMP              : 3;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_FLT_CTRL_1;

typedef volatile union _CAM_REG_SEEE_FLT_CTRL_2_
{
    volatile struct
    {
        FIELD SEEE_FLT_G1               : 8;
        FIELD SEEE_FLT_G2               : 6;
        FIELD SEEE_FLT_G3               : 5;
        FIELD SEEE_FLT_G4               : 4;
        FIELD SEEE_FLT_G5               : 4;
        FIELD SEEE_FLT_G6               : 3;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_FLT_CTRL_2;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_01_
{
    volatile struct
    {
        FIELD SEEE_GLUT_X1              : 8;
        FIELD SEEE_GLUT_S1              : 8;
        FIELD SEEE_GLUT_Y1              : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_01;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_02_
{
    volatile struct
    {
        FIELD SEEE_GLUT_X2              : 8;
        FIELD SEEE_GLUT_S2              : 8;
        FIELD SEEE_GLUT_Y2              : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_02;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_03_
{
    volatile struct
    {
        FIELD SEEE_GLUT_X3              : 8;
        FIELD SEEE_GLUT_S3              : 8;
        FIELD SEEE_GLUT_Y3              : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_03;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_04_
{
    volatile struct
    {
        FIELD SEEE_GLUT_X4              : 8;
        FIELD SEEE_GLUT_S4              : 8;
        FIELD SEEE_GLUT_Y4              : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_04;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_05_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD SEEE_GLUT_S5              : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_05;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_06_
{
    volatile struct
    {
        FIELD SEEE_GLUT_TH_OVR          : 8;
        FIELD SEEE_GLUT_TH_UND          : 8;
        FIELD SEEE_GLUT_TH_MIN          : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_06;

typedef volatile union _CAM_REG_SEEE_EDTR_CTRL_
{
    volatile struct
    {
        FIELD SEEE_EDTR_DIAG_AMP        : 3;
        FIELD SEEE_EDTR_AMP             : 6;
        FIELD SEEE_EDTR_LV              : 3;
        FIELD SEEE_EDTR_FLT_MODE        : 2;
        FIELD SEEE_EDTR_FLT_2_EN        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_EDTR_CTRL;

typedef volatile union _CAM_REG_SEEE_OUT_EDGE_CTRL_
{
    volatile struct
    {
        FIELD SEEE_OUT_EDGE_SEL         : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_OUT_EDGE_CTRL;

typedef volatile union _CAM_REG_SEEE_SE_Y_CTRL_
{
    volatile struct
    {
        FIELD SEEE_SE_CONST_Y_EN        : 1;
        FIELD SEEE_SE_CONST_Y_VAL       : 8;
        FIELD rsv_9                     : 7;
        FIELD SEEE_SE_YOUT_QBIT         : 3;
        FIELD rsv_19                    : 1;
        FIELD SEEE_SE_COUT_QBIT         : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SE_Y_CTRL;

typedef volatile union _CAM_REG_SEEE_SE_EDGE_CTRL_1_
{
    volatile struct
    {
        FIELD SEEE_SE_HORI_EDGE_SEL     : 1;
        FIELD SEEE_SE_HORI_EDGE_GAIN_A  : 4;
        FIELD SEEE_SE_HORI_EDGE_GAIN_B  : 5;
        FIELD SEEE_SE_HORI_EDGE_GAIN_C  : 5;
        FIELD rsv_15                    : 1;
        FIELD SEEE_SE_VERT_EDGE_SEL     : 1;
        FIELD SEEE_SE_VERT_EDGE_GAIN_A  : 4;
        FIELD SEEE_SE_VERT_EDGE_GAIN_B  : 5;
        FIELD SEEE_SE_VERT_EDGE_GAIN_C  : 5;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SE_EDGE_CTRL_1;

typedef volatile union _CAM_REG_SEEE_SE_EDGE_CTRL_2_
{
    volatile struct
    {
        FIELD SEEE_SE_EDGE_DET_GAIN     : 5;
        FIELD SEEE_SE_BOSS_IN_SEL       : 1;
        FIELD SEEE_SE_BOSS_GAIN_OFF     : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SE_EDGE_CTRL_2;

typedef volatile union _CAM_REG_SEEE_SE_EDGE_CTRL_3_
{
    volatile struct
    {
        FIELD SEEE_SE_CONVT_FORCE_EN    : 1;
        FIELD SEEE_SE_CONVT_CORE_TH     : 7;
        FIELD SEEE_SE_CONVT_E_TH        : 8;
        FIELD SEEE_SE_CONVT_SLOPE_SEL   : 1;
        FIELD SEEE_SE_OIL_EN            : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SE_EDGE_CTRL_3;

typedef volatile union _CAM_REG_SEEE_SE_SPECL_CTRL_
{
    volatile struct
    {
        FIELD SEEE_SE_SPECL_HALF_MODE   : 2;
        FIELD SEEE_SE_SPECL_ABS         : 1;
        FIELD SEEE_SE_SPECL_INV         : 1;
        FIELD SEEE_SE_SPECL_GAIN        : 2;
        FIELD SEEE_SE_KNEE_SEL          : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SE_SPECL_CTRL;

typedef volatile union _CAM_REG_SEEE_SE_CORE_CTRL_1_
{
    volatile struct
    {
        FIELD SEEE_SE_CORE_HORI_X0      : 7;
        FIELD SEEE_SE_CORE_HORI_SUP     : 2;
        FIELD SEEE_SE_CORE_HORI_X2      : 8;
        FIELD SEEE_SE_CORE_HORI_SDN     : 2;
        FIELD SEEE_SE_CORE_HORI_Y5      : 6;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SE_CORE_CTRL_1;

typedef volatile union _CAM_REG_SEEE_SE_CORE_CTRL_2_
{
    volatile struct
    {
        FIELD SEEE_SE_CORE_VERT_X0      : 7;
        FIELD SEEE_SE_CORE_VERT_SUP     : 2;
        FIELD SEEE_SE_CORE_VERT_X2      : 8;
        FIELD SEEE_SE_CORE_VERT_SDN     : 2;
        FIELD SEEE_SE_CORE_VERT_Y5      : 6;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_SE_CORE_CTRL_2;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_07_
{
    volatile struct
    {
        FIELD SEEE_GLUT_X1_1            : 8;
        FIELD SEEE_GLUT_S1_1            : 8;
        FIELD SEEE_GLUT_S2_1            : 8;
        FIELD SEEE_GLUT_LLINK_EN        : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_07;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_08_
{
    volatile struct
    {
        FIELD SEEE_GLUT_X1_2            : 8;
        FIELD SEEE_GLUT_S1_2            : 8;
        FIELD SEEE_GLUT_S2_2            : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_08;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_09_
{
    volatile struct
    {
        FIELD SEEE_GLUT_X1_3            : 8;
        FIELD SEEE_GLUT_S1_3            : 8;
        FIELD SEEE_GLUT_S2_3            : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_09;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_10_
{
    volatile struct
    {
        FIELD SEEE_GLUT_TH_OVR_1        : 8;
        FIELD SEEE_GLUT_TH_UND_1        : 8;
        FIELD SEEE_GLUT_TH_OVR_2        : 8;
        FIELD SEEE_GLUT_TH_UND_2        : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_10;

typedef volatile union _CAM_REG_SEEE_GLUT_CTRL_11_
{
    volatile struct
    {
        FIELD SEEE_GLUT_TH_OVR_3        : 8;
        FIELD SEEE_GLUT_TH_UND_3        : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_SEEE_GLUT_CTRL_11;

/* end MT6593_403_yuv_seee.xml*/

/* start MT6593_500_yuv_crz.xml*/
typedef volatile union _CAM_REG_CRZ_CONTROL_
{
    volatile struct
    {
        FIELD CRZ_HORI_EN               : 1;
        FIELD CRZ_VERT_EN               : 1;
        FIELD rsv_2                     : 2;
        FIELD CRZ_VERT_FIRST            : 1;
        FIELD CRZ_HORI_ALGO             : 2;
        FIELD CRZ_VERT_ALGO             : 2;
        FIELD CRZ_DER_EN                : 1;
        FIELD CRZ_TRNC_HORI             : 3;
        FIELD CRZ_TRNC_VERT             : 3;
        FIELD CRZ_HORI_TBL_SEL          : 5;
        FIELD CRZ_VERT_TBL_SEL          : 5;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_CONTROL;

typedef volatile union _CAM_REG_CRZ_IN_IMG_
{
    volatile struct
    {
        FIELD CRZ_IN_WD                 : 13;
        FIELD rsv_13                    : 3;
        FIELD CRZ_IN_HT                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_IN_IMG;

typedef volatile union _CAM_REG_CRZ_OUT_IMG_
{
    volatile struct
    {
        FIELD CRZ_OUT_WD                : 13;
        FIELD rsv_13                    : 3;
        FIELD CRZ_OUT_HT                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_OUT_IMG;

typedef volatile union _CAM_REG_CRZ_HORI_STEP_
{
    volatile struct
    {
        FIELD CRZ_HORI_STEP             : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_HORI_STEP;

typedef volatile union _CAM_REG_CRZ_VERT_STEP_
{
    volatile struct
    {
        FIELD CRZ_VERT_STEP             : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_VERT_STEP;

typedef volatile union _CAM_REG_CRZ_LUMA_HORI_INT_OFST_
{
    volatile struct
    {
        FIELD CRZ_LUMA_HORI_INT_OFST    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_LUMA_HORI_INT_OFST;

typedef volatile union _CAM_REG_CRZ_LUMA_HORI_SUB_OFST_
{
    volatile struct
    {
        FIELD CRZ_LUMA_HORI_SUB_OFST    : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_LUMA_HORI_SUB_OFST;

typedef volatile union _CAM_REG_CRZ_LUMA_VERT_INT_OFST_
{
    volatile struct
    {
        FIELD CRZ_LUMA_VERT_INT_OFST    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_LUMA_VERT_INT_OFST;

typedef volatile union _CAM_REG_CRZ_LUMA_VERT_SUB_OFST_
{
    volatile struct
    {
        FIELD CRZ_LUMA_VERT_SUB_OFST    : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_LUMA_VERT_SUB_OFST;

typedef volatile union _CAM_REG_CRZ_CHRO_HORI_INT_OFST_
{
    volatile struct
    {
        FIELD CRZ_CHRO_HORI_INT_OFST    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_CHRO_HORI_INT_OFST;

typedef volatile union _CAM_REG_CRZ_CHRO_HORI_SUB_OFST_
{
    volatile struct
    {
        FIELD CRZ_CHRO_HORI_SUB_OFST    : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_CHRO_HORI_SUB_OFST;

typedef volatile union _CAM_REG_CRZ_CHRO_VERT_INT_OFST_
{
    volatile struct
    {
        FIELD CRZ_CHRO_VERT_INT_OFST    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_CHRO_VERT_INT_OFST;

typedef volatile union _CAM_REG_CRZ_CHRO_VERT_SUB_OFST_
{
    volatile struct
    {
        FIELD CRZ_CHRO_VERT_SUB_OFST    : 21;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_CHRO_VERT_SUB_OFST;

typedef volatile union _CAM_REG_CRZ_DER_1_
{
    volatile struct
    {
        FIELD CRZ_SPARE_1V              : 4;
        FIELD rsv_4                     : 12;
        FIELD CRZ_SPARE_1H              : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_DER_1;

typedef volatile union _CAM_REG_CRZ_DER_2_
{
    volatile struct
    {
        FIELD CRZ_SPARE_2V              : 9;
        FIELD rsv_9                     : 7;
        FIELD CRZ_SPARE_2H              : 9;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRZ_DER_2;

/* end MT6593_500_yuv_crz.xml*/

/* start MT6593_502_yuv_g2c.xml*/
typedef volatile union _CAM_REG_G2C_CONV_0A_
{
    volatile struct
    {
        FIELD G2C_CNV_00                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_01                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_0A;

typedef volatile union _CAM_REG_G2C_CONV_0B_
{
    volatile struct
    {
        FIELD G2C_CNV_02                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_Y_OFST                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_0B;

typedef volatile union _CAM_REG_G2C_CONV_1A_
{
    volatile struct
    {
        FIELD G2C_CNV_10                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_11                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_1A;

typedef volatile union _CAM_REG_G2C_CONV_1B_
{
    volatile struct
    {
        FIELD G2C_CNV_12                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_U_OFST                : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_1B;

typedef volatile union _CAM_REG_G2C_CONV_2A_
{
    volatile struct
    {
        FIELD G2C_CNV_20                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_CNV_21                : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_2A;

typedef volatile union _CAM_REG_G2C_CONV_2B_
{
    volatile struct
    {
        FIELD G2C_CNV_22                : 11;
        FIELD rsv_11                    : 5;
        FIELD G2C_V_OFST                : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_CONV_2B;

typedef volatile union _CAM_REG_G2C_SHADE_CON_1_
{
    volatile struct
    {
        FIELD G2C_SHADE_VAR             : 16;
        FIELD G2C_SHADE_P0              : 11;
        FIELD rsv_27                    : 1;
        FIELD G2C_SHADE_EN              : 1;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_SHADE_CON_1;

typedef volatile union _CAM_REG_G2C_SHADE_CON_2_
{
    volatile struct
    {
        FIELD G2C_SHADE_P1              : 11;
        FIELD rsv_11                    : 1;
        FIELD G2C_SHADE_P2              : 11;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_SHADE_CON_2;

typedef volatile union _CAM_REG_G2C_SHADE_CON_3_
{
    volatile struct
    {
        FIELD G2C_SHADE_UB              : 9;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_SHADE_CON_3;

typedef volatile union _CAM_REG_G2C_SHADE_TAR_
{
    volatile struct
    {
        FIELD G2C_SHADE_XMID            : 13;
        FIELD rsv_13                    : 3;
        FIELD G2C_SHADE_YMID            : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_SHADE_TAR;

typedef volatile union _CAM_REG_G2C_SHADE_SP_
{
    volatile struct
    {
        FIELD G2C_SHADE_XSP             : 13;
        FIELD rsv_13                    : 3;
        FIELD G2C_SHADE_YSP             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_G2C_SHADE_SP;

/* end MT6593_502_yuv_g2c.xml*/

/* start MT6593_504_cdp_fe.xml*/
typedef volatile union _CAM_REG_FE_CTRL_
{
    volatile struct
    {
        FIELD FE_MODE                   : 2;
        FIELD FE_PARAM                  : 3;
        FIELD FE_FLT_EN                 : 1;
        FIELD FE_TH_G                   : 8;
        FIELD FE_TH_C                   : 8;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_CTRL;

typedef volatile union _CAM_REG_FE_IDX_CTRL_
{
    volatile struct
    {
        FIELD FE_XIDX                   : 13;
        FIELD rsv_13                    : 3;
        FIELD FE_YIDX                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_IDX_CTRL;

typedef volatile union _CAM_REG_FE_CROP_CTRL1_
{
    volatile struct
    {
        FIELD FE_START_X                : 11;
        FIELD rsv_11                    : 5;
        FIELD FE_START_Y                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_CROP_CTRL1;

typedef volatile union _CAM_REG_FE_CROP_CTRL2_
{
    volatile struct
    {
        FIELD FE_IN_WD                  : 11;
        FIELD rsv_11                    : 5;
        FIELD FE_IN_HT                  : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_FE_CROP_CTRL2;

/* end MT6593_504_cdp_fe.xml*/

/* start MT6593_505_yuv_srz1.xml*/
typedef volatile union _CAM_REG_SRZ1_CONTROL_
{
    volatile struct
    {
        FIELD SRZ1_HORI_EN              : 1;
        FIELD SRZ1_VERT_EN              : 1;
        FIELD rsv_2                     : 2;
        FIELD SRZ1_VERT_FIRST           : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_CONTROL;

typedef volatile union _CAM_REG_SRZ1_IN_IMG_
{
    volatile struct
    {
        FIELD SRZ1_IN_WD                : 13;
        FIELD rsv_13                    : 3;
        FIELD SRZ1_IN_HT                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_IN_IMG;

typedef volatile union _CAM_REG_SRZ1_OUT_IMG_
{
    volatile struct
    {
        FIELD SRZ1_OUT_WD               : 13;
        FIELD rsv_13                    : 3;
        FIELD SRZ1_OUT_HT               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_OUT_IMG;

typedef volatile union _CAM_REG_SRZ1_HORI_STEP_
{
    volatile struct
    {
        FIELD SRZ1_HORI_STEP            : 19;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_HORI_STEP;

typedef volatile union _CAM_REG_SRZ1_VERT_STEP_
{
    volatile struct
    {
        FIELD SRZ1_VERT_STEP            : 19;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_VERT_STEP;

typedef volatile union _CAM_REG_SRZ1_HORI_INT_OFST_
{
    volatile struct
    {
        FIELD SRZ1_HORI_INT_OFST        : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_HORI_INT_OFST;

typedef volatile union _CAM_REG_SRZ1_HORI_SUB_OFST_
{
    volatile struct
    {
        FIELD SRZ1_HORI_SUB_OFST        : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_HORI_SUB_OFST;

typedef volatile union _CAM_REG_SRZ1_VERT_INT_OFST_
{
    volatile struct
    {
        FIELD SRZ1_VERT_INT_OFST        : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_VERT_INT_OFST;

typedef volatile union _CAM_REG_SRZ1_VERT_SUB_OFST_
{
    volatile struct
    {
        FIELD SRZ1_VERT_SUB_OFST        : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ1_VERT_SUB_OFST;

/* end MT6593_505_yuv_srz1.xml*/

/* start MT6593_506_yuv_srz2.xml*/
typedef volatile union _CAM_REG_SRZ2_CONTROL_
{
    volatile struct
    {
        FIELD SRZ2_HORI_EN              : 1;
        FIELD SRZ2_VERT_EN              : 1;
        FIELD rsv_2                     : 2;
        FIELD SRZ2_VERT_FIRST           : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_CONTROL;

typedef volatile union _CAM_REG_SRZ2_IN_IMG_
{
    volatile struct
    {
        FIELD SRZ2_IN_WD                : 13;
        FIELD rsv_13                    : 3;
        FIELD SRZ2_IN_HT                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_IN_IMG;

typedef volatile union _CAM_REG_SRZ2_OUT_IMG_
{
    volatile struct
    {
        FIELD SRZ2_OUT_WD               : 13;
        FIELD rsv_13                    : 3;
        FIELD SRZ2_OUT_HT               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_OUT_IMG;

typedef volatile union _CAM_REG_SRZ2_HORI_STEP_
{
    volatile struct
    {
        FIELD SRZ2_HORI_STEP            : 19;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_HORI_STEP;

typedef volatile union _CAM_REG_SRZ2_VERT_STEP_
{
    volatile struct
    {
        FIELD SRZ2_VERT_STEP            : 19;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_VERT_STEP;

typedef volatile union _CAM_REG_SRZ2_HORI_INT_OFST_
{
    volatile struct
    {
        FIELD SRZ2_HORI_INT_OFST        : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_HORI_INT_OFST;

typedef volatile union _CAM_REG_SRZ2_HORI_SUB_OFST_
{
    volatile struct
    {
        FIELD SRZ2_HORI_SUB_OFST        : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_HORI_SUB_OFST;

typedef volatile union _CAM_REG_SRZ2_VERT_INT_OFST_
{
    volatile struct
    {
        FIELD SRZ2_VERT_INT_OFST        : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_VERT_INT_OFST;

typedef volatile union _CAM_REG_SRZ2_VERT_SUB_OFST_
{
    volatile struct
    {
        FIELD SRZ2_VERT_SUB_OFST        : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_SRZ2_VERT_SUB_OFST;

/* end MT6593_506_yuv_srz2.xml*/

/* start MT6593_507_yuv_mix1.xml*/
typedef volatile union _CAM_REG_MIX1_CTRL_0_
{
    volatile struct
    {
        FIELD MIX1_WT_SEL               : 1;
        FIELD rsv_1                     : 7;
        FIELD MIX1_B0                   : 8;
        FIELD MIX1_B1                   : 8;
        FIELD MIX1_DT                   : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX1_CTRL_0;

typedef volatile union _CAM_REG_MIX1_CTRL_1_
{
    volatile struct
    {
        FIELD MIX1_M0                   : 8;
        FIELD rsv_8                     : 8;
        FIELD MIX1_M1                   : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX1_CTRL_1;

typedef volatile union _CAM_REG_MIX1_SPARE_
{
    volatile struct
    {
        FIELD MIX1_SPARE                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX1_SPARE;

/* end MT6593_507_yuv_mix1.xml*/

/* start MT6593_508_yuv_mix2.xml*/
typedef volatile union _CAM_REG_MIX2_CTRL_0_
{
    volatile struct
    {
        FIELD MIX2_WT_SEL               : 1;
        FIELD rsv_1                     : 7;
        FIELD MIX2_B0                   : 8;
        FIELD MIX2_B1                   : 8;
        FIELD MIX2_DT                   : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX2_CTRL_0;

typedef volatile union _CAM_REG_MIX2_CTRL_1_
{
    volatile struct
    {
        FIELD MIX2_M0                   : 8;
        FIELD rsv_8                     : 8;
        FIELD MIX2_M1                   : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX2_CTRL_1;

typedef volatile union _CAM_REG_MIX2_SPARE_
{
    volatile struct
    {
        FIELD MIX2_SPARE                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX2_SPARE;

/* end MT6593_508_yuv_mix2.xml*/

/* start MT6593_509_yuv_mix3.xml*/
typedef volatile union _CAM_REG_MIX3_CTRL_0_
{
    volatile struct
    {
        FIELD MIX3_WT_SEL               : 1;
        FIELD rsv_1                     : 7;
        FIELD MIX3_B0                   : 8;
        FIELD MIX3_B1                   : 8;
        FIELD MIX3_DT                   : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX3_CTRL_0;

typedef volatile union _CAM_REG_MIX3_CTRL_1_
{
    volatile struct
    {
        FIELD MIX3_M0                   : 8;
        FIELD rsv_8                     : 8;
        FIELD MIX3_M1                   : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX3_CTRL_1;

typedef volatile union _CAM_REG_MIX3_SPARE_
{
    volatile struct
    {
        FIELD MIX3_SPARE                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_MIX3_SPARE;

/* end MT6593_509_yuv_mix3.xml*/

/* start MT6593_510_yuv_3dnr.xml*/
typedef volatile union _CAM_REG_NR3D_BLEND_
{
    volatile struct
    {
        FIELD NR3D_GAIN                 : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_RND_Y                : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_RND_U                : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_RND_V                : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_BLEND;

typedef volatile union _CAM_REG_NR3D_FBCNT_OFF_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD NR3D_FB_CNT_XOFF          : 13;
        FIELD rsv_14                    : 2;
        FIELD NR3D_FB_CNT_YOFF          : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_FBCNT_OFF;

typedef volatile union _CAM_REG_NR3D_FBCNT_SIZ_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD NR3D_FB_CNT_XSIZ          : 13;
        FIELD rsv_14                    : 2;
        FIELD NR3D_FB_CNT_YSIZ          : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_FBCNT_SIZ;

typedef volatile union _CAM_REG_NR3D_FB_COUNT_
{
    volatile struct
    {
        FIELD NR3D_FB_CNT               : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_FB_COUNT;

typedef volatile union _CAM_REG_NR3D_LMT_CPX_
{
    volatile struct
    {
        FIELD NR3D_LMT_CPX1             : 8;
        FIELD NR3D_LMT_CPX2             : 8;
        FIELD NR3D_LMT_CPX3             : 8;
        FIELD NR3D_LMT_CPX4             : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_CPX;

typedef volatile union _CAM_REG_NR3D_LMT_Y_CON1_
{
    volatile struct
    {
        FIELD NR3D_LMT_Y0               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_Y0_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_Y1               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_Y1_TH            : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_Y_CON1;

typedef volatile union _CAM_REG_NR3D_LMT_Y_CON2_
{
    volatile struct
    {
        FIELD NR3D_LMT_Y2               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_Y2_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_Y3               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_Y3_TH            : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_Y_CON2;

typedef volatile union _CAM_REG_NR3D_LMT_Y_CON3_
{
    volatile struct
    {
        FIELD NR3D_LMT_Y4               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_Y4_TH            : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_Y_CON3;

typedef volatile union _CAM_REG_NR3D_LMT_U_CON1_
{
    volatile struct
    {
        FIELD NR3D_LMT_U0               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_U0_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_U1               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_U1_TH            : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_U_CON1;

typedef volatile union _CAM_REG_NR3D_LMT_U_CON2_
{
    volatile struct
    {
        FIELD NR3D_LMT_U2               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_U2_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_U3               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_U3_TH            : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_U_CON2;

typedef volatile union _CAM_REG_NR3D_LMT_U_CON3_
{
    volatile struct
    {
        FIELD NR3D_LMT_U4               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_U4_TH            : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_U_CON3;

typedef volatile union _CAM_REG_NR3D_LMT_V_CON1_
{
    volatile struct
    {
        FIELD NR3D_LMT_V0               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_V0_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_V1               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_V1_TH            : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_V_CON1;

typedef volatile union _CAM_REG_NR3D_LMT_V_CON2_
{
    volatile struct
    {
        FIELD NR3D_LMT_V2               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_V2_TH            : 5;
        FIELD rsv_13                    : 3;
        FIELD NR3D_LMT_V3               : 5;
        FIELD rsv_21                    : 3;
        FIELD NR3D_LMT_V3_TH            : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_V_CON2;

typedef volatile union _CAM_REG_NR3D_LMT_V_CON3_
{
    volatile struct
    {
        FIELD NR3D_LMT_V4               : 5;
        FIELD rsv_5                     : 3;
        FIELD NR3D_LMT_V4_TH            : 5;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_LMT_V_CON3;

typedef volatile union _CAM_REG_NR3D_CTRL_
{
    volatile struct
    {
        FIELD NR3D_LMT_OUT_CNT_TH       : 2;
        FIELD rsv_2                     : 2;
        FIELD NR3D_ON_EN                : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_CTRL;

typedef volatile union _CAM_REG_NR3D_ON_OFF_
{
    volatile struct
    {
        FIELD NR3D_ON_OFST_X            : 14;
        FIELD rsv_14                    : 2;
        FIELD NR3D_ON_OFST_Y            : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_ON_OFF;

typedef volatile union _CAM_REG_NR3D_ON_SIZ_
{
    volatile struct
    {
        FIELD NR3D_ON_WD                : 14;
        FIELD rsv_14                    : 2;
        FIELD NR3D_ON_HT                : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_ON_SIZ;

typedef volatile union _CAM_REG_NR3D_SPARE0_
{
    volatile struct
    {
        FIELD NR3D_SPARE_0              : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR3D_SPARE0;

/* end MT6593_510_yuv_3dnr.xml*/

/* start MT6593_512_yuv_mdp_crop.xml*/
typedef volatile union _CAM_REG_MDP_CROP_X_
{
    volatile struct
    {
        FIELD MDP_CROP_STR_X            : 13;
        FIELD rsv_13                    : 3;
        FIELD MDP_CROP_END_X            : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_MDP_CROP_X;

typedef volatile union _CAM_REG_MDP_CROP_Y_
{
    volatile struct
    {
        FIELD MDP_CROP_STR_Y            : 13;
        FIELD rsv_13                    : 3;
        FIELD MDP_CROP_END_Y            : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_MDP_CROP_Y;

/* end MT6593_512_yuv_mdp_crop.xml*/

/* start MT6593_511_cdp_eis.xml*/
typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL1_
{
    volatile struct
    {
        FIELD EIS_OP_HORI               : 3;
        FIELD EIS_OP_VERT               : 3;
        FIELD EIS_SUBG_EN               : 1;
        FIELD rsv_7                     : 1;
        FIELD EIS_NUM_HRP               : 5;
        FIELD EIS_KNEE_2                : 4;
        FIELD EIS_KNEE_1                : 4;
        FIELD EIS_NUM_VRP               : 4;
        FIELD EIS_NUM_HWIN              : 3;
        FIELD EIS_NUM_VWIN              : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_PREP_ME_CTRL1;

typedef volatile union _CAM_REG_EIS_PREP_ME_CTRL2_
{
    volatile struct
    {
        FIELD EIS_PROC_GAIN             : 2;
        FIELD EIS_HORI_SHR              : 3;
        FIELD EIS_VERT_SHR              : 3;
        FIELD EIS_DC_DL                 : 6;
        FIELD EIS_RP_MODI               : 1;
        FIELD EIS_FIRST_FRM             : 1;
        FIELD EIS_SPARE                 : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_PREP_ME_CTRL2;

typedef volatile union _CAM_REG_EIS_LMV_TH_
{
    volatile struct
    {
        FIELD LMV_TH_Y_SURROUND         : 8;
        FIELD LMV_TH_Y_CENTER           : 8;
        FIELD LMV_TH_X_SOURROUND        : 8;
        FIELD LMV_TH_X_CENTER           : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_LMV_TH;

typedef volatile union _CAM_REG_EIS_FL_OFFSET_
{
    volatile struct
    {
        FIELD EIS_WIN_VOFST             : 12;
        FIELD rsv_12                    : 4;
        FIELD EIS_WIN_HOFST             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_FL_OFFSET;

typedef volatile union _CAM_REG_EIS_MB_OFFSET_
{
    volatile struct
    {
        FIELD EIS_RP_VOFST              : 12;
        FIELD rsv_12                    : 4;
        FIELD EIS_RP_HOFST              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_MB_OFFSET;

typedef volatile union _CAM_REG_EIS_MB_INTERVAL_
{
    volatile struct
    {
        FIELD EIS_WIN_VSIZE             : 12;
        FIELD rsv_12                    : 4;
        FIELD EIS_WIN_HSIZE             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_MB_INTERVAL;

typedef volatile union _CAM_REG_EIS_GMV_
{
    volatile struct
    {
        FIELD GMV_Y                     : 16;
        FIELD GMV_X                     : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_GMV;

typedef volatile union _CAM_REG_EIS_ERR_CTRL_
{
    volatile struct
    {
        FIELD ERR_STATUS                : 4;
        FIELD CHK_SUM_EN                : 1;
        FIELD rsv_5                     : 3;
        FIELD CHK_SUM_OUT               : 8;
        FIELD ERR_MASK                  : 4;
        FIELD rsv_20                    : 11;
        FIELD CLEAR_ERR                 : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_ERR_CTRL;

typedef volatile union _CAM_REG_EIS_IMAGE_CTRL_
{
    volatile struct
    {
        FIELD HEIGHT                    : 13;
        FIELD rsv_13                    : 3;
        FIELD WIDTH                     : 13;
        FIELD rsv_29                    : 2;
        FIELD PIPE_MODE                 : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_EIS_IMAGE_CTRL;

/* end MT6593_511_cdp_eis.xml*/

/* start MT6593_600_raw_dmx_bmx_rmx.xml*/
typedef volatile union _CAM_REG_DMX_CTL_
{
    volatile struct
    {
        FIELD DMX_IHDR_SEP_MODE         : 1;
        FIELD rsv_1                     : 3;
        FIELD DMX_IHDR_ORD              : 1;
        FIELD rsv_5                     : 11;
        FIELD DMX_EDGE                  : 4;
        FIELD rsv_20                    : 11;
        FIELD DMX_EDGE_SET              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMX_CTL;

typedef volatile union _CAM_REG_DMX_CROP_
{
    volatile struct
    {
        FIELD DMX_STR_X                 : 13;
        FIELD rsv_13                    : 3;
        FIELD DMX_END_X                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMX_CROP;

typedef volatile union _CAM_REG_DMX_VSIZE_
{
    volatile struct
    {
        FIELD DMX_HT                    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMX_VSIZE;

typedef volatile union _CAM_REG_BMX_CTL_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD BMX_SINGLE_MODE           : 1;
        FIELD rsv_5                     : 11;
        FIELD BMX_EDGE                  : 4;
        FIELD rsv_20                    : 11;
        FIELD BMX_EDGE_SET              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_BMX_CTL;

typedef volatile union _CAM_REG_BMX_CROP_
{
    volatile struct
    {
        FIELD BMX_STR_X                 : 13;
        FIELD rsv_13                    : 3;
        FIELD BMX_END_X                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BMX_CROP;

typedef volatile union _CAM_REG_BMX_VSIZE_
{
    volatile struct
    {
        FIELD BMX_HT                    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_BMX_VSIZE;

typedef volatile union _CAM_REG_RMX_CTL_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD RMX_SINGLE_MODE_1           : 1;
        FIELD rsv_9                     : 2;
        FIELD RMX_SINGLE_MODE_2           : 1;
        FIELD rsv_12                     : 4;
        FIELD RMX_EDGE                  : 4;
        FIELD rsv_20                    : 11;
        FIELD RMX_EDGE_SET              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_RMX_CTL;

typedef volatile union _CAM_REG_RMX_CROP_
{
    volatile struct
    {
        FIELD RMX_STR_X                 : 13;
        FIELD rsv_13                    : 3;
        FIELD RMX_END_X                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RMX_CROP;

typedef volatile union _CAM_REG_RMX_VSIZE_
{
    volatile struct
    {
        FIELD RMX_HT                    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RMX_VSIZE;

/* end MT6593_600_raw_dmx_bmx_rmx.xml*/

/* start MT6593_602_raw_ufe.xml*/
typedef volatile union _CAM_REG_UFE_CON_
{
    volatile struct
    {
        FIELD UFE_FORCE_PCM             : 1;
        FIELD rsv_1                     : 3;
        FIELD UFE_TCCT_BYP              : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFE_CON;

/* end MT6593_602_raw_ufe.xml*/

/* start MT6593_603_raw_ufd.xml*/
typedef volatile union _CAM_REG_UFD_CON_
{
    volatile struct
    {
        FIELD UFD_TCCT_BYP              : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFD_CON;

typedef volatile union _CAM_REG_UFD_SIZE_CON_
{
    volatile struct
    {
        FIELD UFD_WD                    : 13;
        FIELD rsv_13                    : 3;
        FIELD UFD_HT                    : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFD_SIZE_CON;

typedef volatile union _CAM_REG_UFD_AU_CON_
{
    volatile struct
    {
        FIELD UFD_AU_OFST               : 7;
        FIELD rsv_7                     : 9;
        FIELD UFD_AU_SIZE               : 7;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFD_AU_CON;

typedef volatile union _CAM_REG_UFD_CROP_CON1_
{
    volatile struct
    {
        FIELD UFD_X_START               : 13;
        FIELD rsv_13                    : 3;
        FIELD UFD_X_END                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFD_CROP_CON1;

typedef volatile union _CAM_REG_UFD_CROP_CON2_
{
    volatile struct
    {
        FIELD UFD_Y_START               : 13;
        FIELD rsv_13                    : 3;
        FIELD UFD_Y_END                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFD_CROP_CON2;

/* end MT6593_603_raw_ufd.xml*/

/* start MT6593_606_rgb_sl2.xml*/
typedef volatile union _CAM_REG_SL2_CEN_
{
    volatile struct
    {
        FIELD SL2_CENTR_X               : 13;
        FIELD rsv_13                    : 3;
        FIELD SL2_CENTR_Y               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2_CEN;

typedef volatile union _CAM_REG_SL2_MAX0_RR_
{
    volatile struct
    {
        FIELD SL2_RR_0                  : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2_MAX0_RR;

typedef volatile union _CAM_REG_SL2_MAX1_RR_
{
    volatile struct
    {
        FIELD SL2_RR_1                  : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2_MAX1_RR;

typedef volatile union _CAM_REG_SL2_MAX2_RR_
{
    volatile struct
    {
        FIELD SL2_RR_2                  : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2_MAX2_RR;

typedef volatile union _CAM_REG_SL2_HRZ_
{
    volatile struct
    {
        FIELD SL2_HRZ_COMP              : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2_HRZ;

typedef volatile union _CAM_REG_SL2_XOFF_
{
    volatile struct
    {
        FIELD SL2_X_OFST                : 27;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2_XOFF;

typedef volatile union _CAM_REG_SL2_YOFF_
{
    volatile struct
    {
        FIELD SL2_Y_OFST                : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2_YOFF;

/* end MT6593_606_rgb_sl2.xml*/

/* start MT6593_607_yuv_sl2b.xml*/
typedef volatile union _CAM_REG_SL2B_CEN_
{
    volatile struct
    {
        FIELD SL2B_CENTR_X              : 13;
        FIELD rsv_13                    : 3;
        FIELD SL2B_CENTR_Y              : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2B_CEN;

typedef volatile union _CAM_REG_SL2B_MAX0_RR_
{
    volatile struct
    {
        FIELD SL2B_RR_0                 : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2B_MAX0_RR;

typedef volatile union _CAM_REG_SL2B_MAX1_RR_
{
    volatile struct
    {
        FIELD SL2B_RR_1                 : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2B_MAX1_RR;

typedef volatile union _CAM_REG_SL2B_MAX2_RR_
{
    volatile struct
    {
        FIELD SL2B_RR_2                 : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2B_MAX2_RR;

typedef volatile union _CAM_REG_SL2B_HRZ_
{
    volatile struct
    {
        FIELD SL2B_HRZ_COMP             : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2B_HRZ;

typedef volatile union _CAM_REG_SL2B_XOFF_
{
    volatile struct
    {
        FIELD SL2B_X_OFST               : 27;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2B_XOFF;

typedef volatile union _CAM_REG_SL2B_YOFF_
{
    volatile struct
    {
        FIELD SL2B_Y_OFST               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2B_YOFF;

/* end MT6593_607_yuv_sl2b.xml*/

/* start MT6593_609_yuv_crsp.xml*/
typedef volatile union _CAM_REG_CRSP_CTRL_
{
    volatile struct
    {
        FIELD CRSP_HORI_EN              : 1;
        FIELD CRSP_VERT_EN              : 1;
        FIELD rsv_2                     : 2;
        FIELD CRSP_RSV_1                : 12;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRSP_CTRL;

typedef volatile union _CAM_REG_CRSP_OUT_IMG_
{
    volatile struct
    {
        FIELD CRSP_WD                   : 13;
        FIELD rsv_13                    : 3;
        FIELD CRSP_HT                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRSP_OUT_IMG;

typedef volatile union _CAM_REG_CRSP_STEP_OFST_
{
    volatile struct
    {
        FIELD CRSP_STEP_X               : 3;
        FIELD rsv_3                     : 5;
        FIELD CRSP_OFST_X               : 2;
        FIELD rsv_10                    : 6;
        FIELD CRSP_STEP_Y               : 3;
        FIELD rsv_19                    : 5;
        FIELD CRSP_OFST_Y               : 2;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRSP_STEP_OFST;

typedef volatile union _CAM_REG_CRSP_CROP_X_
{
    volatile struct
    {
        FIELD CRSP_CROP_STR_X           : 13;
        FIELD rsv_13                    : 3;
        FIELD CRSP_CROP_END_X           : 13;
        FIELD rsv_29                    : 2;
        FIELD CRSP_CROP_EN              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRSP_CROP_X;

typedef volatile union _CAM_REG_CRSP_CROP_Y_
{
    volatile struct
    {
        FIELD CRSP_CROP_STR_Y           : 13;
        FIELD rsv_13                    : 3;
        FIELD CRSP_CROP_END_Y           : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CRSP_CROP_Y;

/* end MT6593_609_yuv_crsp.xml*/

/* start MT6593_610_yuv_sl2c.xml*/
typedef volatile union _CAM_REG_SL2C_CEN_
{
    volatile struct
    {
        FIELD SL2C_CENTR_X              : 13;
        FIELD rsv_13                    : 3;
        FIELD SL2C_CENTR_Y              : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2C_CEN;

typedef volatile union _CAM_REG_SL2C_MAX0_RR_
{
    volatile struct
    {
        FIELD SL2C_RR_0                 : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2C_MAX0_RR;

typedef volatile union _CAM_REG_SL2C_MAX1_RR_
{
    volatile struct
    {
        FIELD SL2C_RR_1                 : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2C_MAX1_RR;

typedef volatile union _CAM_REG_SL2C_MAX2_RR_
{
    volatile struct
    {
        FIELD SL2C_RR_2                 : 26;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2C_MAX2_RR;

typedef volatile union _CAM_REG_SL2C_HRZ_
{
    volatile struct
    {
        FIELD SL2C_HRZ_COMP             : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2C_HRZ;

typedef volatile union _CAM_REG_SL2C_XOFF_
{
    volatile struct
    {
        FIELD SL2C_X_OFST               : 27;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2C_XOFF;

typedef volatile union _CAM_REG_SL2C_YOFF_
{
    volatile struct
    {
        FIELD SL2C_Y_OFST               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_SL2C_YOFF;

/* end MT6593_610_yuv_sl2c.xml*/

/* start MT6593_700_rgb_ggm.xml*/
typedef volatile union _CAM_REG_GGM_LUT_RB_
{
    volatile struct
    {
        FIELD GGM_R                     : 16;
        FIELD GGM_B                     : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GGM_LUT_RB;

typedef volatile union _CAM_REG_GGM_LUT_G_
{
    volatile struct
    {
        FIELD GGM_G                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_GGM_LUT_G;

typedef volatile union _CAM_REG_GGM_CTRL_
{
    volatile struct
    {
        FIELD GGM_EN                    : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} CAM_REG_GGM_CTRL;

/* end MT6593_700_rgb_ggm.xml*/

/* start MT6593_701_yuv_pca.xml*/
typedef volatile union _CAM_REG_PCA_TBL_
{
    volatile struct
    {
        FIELD PCA_LUMA_GAIN             : 8;
        FIELD PCA_SAT_GAIN              : 8;
        FIELD PCA_HUE_SHIFT             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_PCA_TBL;

typedef volatile union _CAM_REG_PCA_CON1_
{
    volatile struct
    {
        FIELD PCA_LUT_360               : 1;
        FIELD rsv_1                     : 3;
        FIELD RSV                       : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_PCA_CON1;

typedef volatile union _CAM_REG_PCA_CON2_
{
    volatile struct
    {
        FIELD PCA_C_TH                  : 5;
        FIELD rsv_5                     : 3;
        FIELD RSV                       : 2;
        FIELD rsv_10                    : 2;
        FIELD PCA_S_TH_EN               : 1;
        FIELD rsv_13                    : 3;
        FIELD PCA_S_TH                  : 8;
        FIELD PCA_Y2YLUT_EN             : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_PCA_CON2;

/* end MT6593_701_yuv_pca.xml*/

/* start MT6593_cam_verif.xml*/
typedef volatile union _CAM_REG_TILE_RING_CON1_
{
    volatile struct
    {
        FIELD TILE_IRQ_MODE             : 8;
        FIELD RING_BUFFER_MCU_NO        : 8;
        FIELD RING_BUFFER_MCU_Y_SIZE    : 8;
        FIELD EIS_FE_ONE_SEL            : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_TILE_RING_CON1;

typedef volatile union _CAM_REG_CTL_IMGI_SIZE_
{
    volatile struct
    {
        FIELD IMGI_YSIZE                : 13;
        FIELD rsv_13                    : 3;
        FIELD IMGI_XSIZE                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_IMGI_SIZE;

/* end MT6593_cam_verif.xml*/

/* start MT6593_201_raw_d_tg.xml*/
typedef volatile union _CAM_REG_TG2_SEN_MODE_
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_MODE;

typedef volatile union _CAM_REG_TG2_VF_CON_
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_VF_CON;

typedef volatile union _CAM_REG_TG2_SEN_GRAB_PXL_
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_GRAB_PXL;

typedef volatile union _CAM_REG_TG2_SEN_GRAB_LIN_
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SEN_GRAB_LIN;

typedef volatile union _CAM_REG_TG2_PATH_CFG_
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_PATH_CFG;

typedef volatile union _CAM_REG_TG2_MEMIN_CTL_
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_MEMIN_CTL;

typedef volatile union _CAM_REG_TG2_INT1_
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INT1;

typedef volatile union _CAM_REG_TG2_INT2_
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INT2;

typedef volatile union _CAM_REG_TG2_SOF_CNT_
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SOF_CNT;

typedef volatile union _CAM_REG_TG2_SOT_CNT_
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_SOT_CNT;

typedef volatile union _CAM_REG_TG2_EOT_CNT_
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_EOT_CNT;

typedef volatile union _CAM_REG_TG2_ERR_CTL_
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_ERR_CTL;

typedef volatile union _CAM_REG_TG2_DAT_NO_
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_DAT_NO;

typedef volatile union _CAM_REG_TG2_FRM_CNT_ST_
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_FRM_CNT_ST;

typedef volatile union _CAM_REG_TG2_FRMSIZE_ST_
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_FRMSIZE_ST;

typedef volatile union _CAM_REG_TG2_INTER_ST_
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_TG2_INTER_ST;

/* end MT6593_201_raw_d_tg.xml*/

/* start MT6593_203_raw_d_obc.xml*/
typedef volatile union _CAM_REG_OBC_D_OFFST0_
{
    volatile struct
    {
        FIELD OBC_D_OFST_B              : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_OFFST0;

typedef volatile union _CAM_REG_OBC_D_OFFST1_
{
    volatile struct
    {
        FIELD OBC_D_OFST_GR             : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_OFFST1;

typedef volatile union _CAM_REG_OBC_D_OFFST2_
{
    volatile struct
    {
        FIELD OBC_D_OFST_GB             : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_OFFST2;

typedef volatile union _CAM_REG_OBC_D_OFFST3_
{
    volatile struct
    {
        FIELD OBC_D_OFST_R              : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_OFFST3;

typedef volatile union _CAM_REG_OBC_D_GAIN0_
{
    volatile struct
    {
        FIELD OBC_D_GAIN_B              : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_GAIN0;

typedef volatile union _CAM_REG_OBC_D_GAIN1_
{
    volatile struct
    {
        FIELD OBC_D_GAIN_GR             : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_GAIN1;

typedef volatile union _CAM_REG_OBC_D_GAIN2_
{
    volatile struct
    {
        FIELD OBC_D_GAIN_GB             : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_GAIN2;

typedef volatile union _CAM_REG_OBC_D_GAIN3_
{
    volatile struct
    {
        FIELD OBC_D_GAIN_R              : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_OBC_D_GAIN3;

/* end MT6593_203_raw_d_obc.xml*/

/* start MT6593_204_raw_d_lsc.xml*/
typedef volatile union _CAM_REG_LSC_D_CTL1_
{
    volatile struct
    {
        FIELD SDBLK_YOFST               : 6;
        FIELD rsv_6                     : 10;
        FIELD SDBLK_XOFST               : 6;
        FIELD rsv_22                    : 2;
        FIELD SD_COEFRD_MODE            : 1;
        FIELD rsv_25                    : 3;
        FIELD SD_ULTRA_MODE             : 1;
        FIELD LSC_D_PRC_MODE            : 1;
        FIELD LSC_SPARE2                : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_CTL1;

typedef volatile union _CAM_REG_LSC_D_CTL2_
{
    volatile struct
    {
        FIELD LSC_D_SDBLK_WIDTH         : 12;
        FIELD LSC_D_SDBLK_XNUM          : 5;
        FIELD LSC_D_OFLN                : 1;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_CTL2;

typedef volatile union _CAM_REG_LSC_D_CTL3_
{
    volatile struct
    {
        FIELD LSC_D_SDBLK_HEIGHT        : 12;
        FIELD LSC_D_SDBLK_YNUM          : 5;
        FIELD LSC_SPARE                 : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_CTL3;

typedef volatile union _CAM_REG_LSC_D_LBLOCK_
{
    volatile struct
    {
        FIELD LSC_D_SDBLK_lHEIGHT       : 12;
        FIELD rsv_12                    : 4;
        FIELD LSC_D_SDBLK_lWIDTH        : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_LBLOCK;

typedef volatile union _CAM_REG_LSC_D_RATIO_
{
    volatile struct
    {
        FIELD LSC_D_RA3                 : 6;
        FIELD rsv_6                     : 2;
        FIELD LSC_D_RA2                 : 6;
        FIELD rsv_14                    : 2;
        FIELD LSC_D_RA1                 : 6;
        FIELD rsv_22                    : 2;
        FIELD LSC_D_RA0                 : 6;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_RATIO;

typedef volatile union _CAM_REG_LSC_D_TPIPE_OFST_
{
    volatile struct
    {
        FIELD LSC_D_TPIPE_OFST_Y        : 12;
        FIELD rsv_12                    : 4;
        FIELD LSC_D_TPIPE_OFST_X        : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_TPIPE_OFST;

typedef volatile union _CAM_REG_LSC_D_TPIPE_SIZE_
{
    volatile struct
    {
        FIELD LSC_D_TPIPE_SIZE_Y        : 13;
        FIELD rsv_13                    : 3;
        FIELD LSC_D_TPIPE_SIZE_X        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_TPIPE_SIZE;

typedef volatile union _CAM_REG_LSC_D_GAIN_TH_
{
    volatile struct
    {
        FIELD LSC_D_GAIN_TH2            : 9;
        FIELD rsv_9                     : 1;
        FIELD LSC_D_GAIN_TH1            : 9;
        FIELD rsv_19                    : 1;
        FIELD LSC_D_GAIN_TH0            : 9;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSC_D_GAIN_TH;

/* end MT6593_204_raw_d_lsc.xml*/

/* start MT6593_205_raw_d_rpg.xml*/
typedef volatile union _CAM_REG_RPG_D_SATU_1_
{
    volatile struct
    {
        FIELD RPG_SATU_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_D_SATU_1;

typedef volatile union _CAM_REG_RPG_D_SATU_2_
{
    volatile struct
    {
        FIELD RPG_SATU_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_SATU_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_D_SATU_2;

typedef volatile union _CAM_REG_RPG_D_GAIN_1_
{
    volatile struct
    {
        FIELD RPG_GAIN_B                : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_GB               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_D_GAIN_1;

typedef volatile union _CAM_REG_RPG_D_GAIN_2_
{
    volatile struct
    {
        FIELD RPG_GAIN_GR               : 13;
        FIELD rsv_13                    : 3;
        FIELD RPG_GAIN_R                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_D_GAIN_2;

typedef volatile union _CAM_REG_RPG_D_OFST_1_
{
    volatile struct
    {
        FIELD RPG_OFST_B                : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_GB               : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_D_OFST_1;

typedef volatile union _CAM_REG_RPG_D_OFST_2_
{
    volatile struct
    {
        FIELD RPG_OFST_GR               : 12;
        FIELD rsv_12                    : 4;
        FIELD RPG_OFST_R                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RPG_D_OFST_2;

/* end MT6593_205_raw_d_rpg.xml*/

/* start MT6593_2091_raw_d_awb.xml*/
typedef volatile union _CAM_REG_AWB_D_WIN_ORG_
{
    volatile struct
    {
        FIELD AWB_W_HORG                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VORG                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_WIN_ORG;

typedef volatile union _CAM_REG_AWB_D_WIN_SIZE_
{
    volatile struct
    {
        FIELD AWB_W_HSIZE               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VSIZE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_WIN_SIZE;

typedef volatile union _CAM_REG_AWB_D_WIN_PIT_
{
    volatile struct
    {
        FIELD AWB_W_HPIT                : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_W_VPIT                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_WIN_PIT;

typedef volatile union _CAM_REG_AWB_D_WIN_NUM_
{
    volatile struct
    {
        FIELD AWB_W_HNUM                : 8;
        FIELD rsv_8                     : 8;
        FIELD AWB_W_VNUM                : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_WIN_NUM;

typedef volatile union _CAM_REG_AWB_D_GAIN1_0_
{
    volatile struct
    {
        FIELD AWB_GAIN1_R               : 13;
        FIELD rsv_13                    : 3;
        FIELD AWB_GAIN1_G               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_GAIN1_0;

typedef volatile union _CAM_REG_AWB_D_GAIN1_1_
{
    volatile struct
    {
        FIELD AWB_GAIN1_B               : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_GAIN1_1;

typedef volatile union _CAM_REG_AWB_D_LMT1_0_
{
    volatile struct
    {
        FIELD AWB_LMT1_R                : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_LMT1_G                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_LMT1_0;

typedef volatile union _CAM_REG_AWB_D_LMT1_1_
{
    volatile struct
    {
        FIELD AWB_LMT1_B                : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_LMT1_1;

typedef volatile union _CAM_REG_AWB_D_LOW_THR_
{
    volatile struct
    {
        FIELD AWB_LOW_THR0              : 8;
        FIELD AWB_LOW_THR1              : 8;
        FIELD AWB_LOW_THR2              : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_LOW_THR;

typedef volatile union _CAM_REG_AWB_D_HI_THR_
{
    volatile struct
    {
        FIELD AWB_HI_THR0               : 8;
        FIELD AWB_HI_THR1               : 8;
        FIELD AWB_HI_THR2               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_HI_THR;

typedef volatile union _CAM_REG_AWB_D_PIXEL_CNT0_
{
    volatile struct
    {
        FIELD AWB_PIXEL_CNT0            : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_PIXEL_CNT0;

typedef volatile union _CAM_REG_AWB_D_PIXEL_CNT1_
{
    volatile struct
    {
        FIELD AWB_PIXEL_CNT1            : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_PIXEL_CNT1;

typedef volatile union _CAM_REG_AWB_D_PIXEL_CNT2_
{
    volatile struct
    {
        FIELD AWB_PIXEL_CNT2            : 24;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_PIXEL_CNT2;

typedef volatile union _CAM_REG_AWB_D_ERR_THR_
{
    volatile struct
    {
        FIELD AWB_ERR_THR               : 12;
        FIELD rsv_12                    : 4;
        FIELD AWB_ERR_SFT               : 3;
        FIELD rsv_19                    : 13;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_ERR_THR;

typedef volatile union _CAM_REG_AWB_D_ROT_
{
    volatile struct
    {
        FIELD AWB_C                     : 10;
        FIELD rsv_10                    : 6;
        FIELD AWB_S                     : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_ROT;

typedef volatile union _CAM_REG_AWB_D_L0_X_
{
    volatile struct
    {
        FIELD AWB_L0_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L0_X;

typedef volatile union _CAM_REG_AWB_D_L0_Y_
{
    volatile struct
    {
        FIELD AWB_L0_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L0_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L0_Y;

typedef volatile union _CAM_REG_AWB_D_L1_X_
{
    volatile struct
    {
        FIELD AWB_L1_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L1_X;

typedef volatile union _CAM_REG_AWB_D_L1_Y_
{
    volatile struct
    {
        FIELD AWB_L1_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L1_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L1_Y;

typedef volatile union _CAM_REG_AWB_D_L2_X_
{
    volatile struct
    {
        FIELD AWB_L2_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L2_X;

typedef volatile union _CAM_REG_AWB_D_L2_Y_
{
    volatile struct
    {
        FIELD AWB_L2_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L2_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L2_Y;

typedef volatile union _CAM_REG_AWB_D_L3_X_
{
    volatile struct
    {
        FIELD AWB_L3_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L3_X;

typedef volatile union _CAM_REG_AWB_D_L3_Y_
{
    volatile struct
    {
        FIELD AWB_L3_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L3_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L3_Y;

typedef volatile union _CAM_REG_AWB_D_L4_X_
{
    volatile struct
    {
        FIELD AWB_L4_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L4_X;

typedef volatile union _CAM_REG_AWB_D_L4_Y_
{
    volatile struct
    {
        FIELD AWB_L4_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L4_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L4_Y;

typedef volatile union _CAM_REG_AWB_D_L5_X_
{
    volatile struct
    {
        FIELD AWB_L5_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L5_X;

typedef volatile union _CAM_REG_AWB_D_L5_Y_
{
    volatile struct
    {
        FIELD AWB_L5_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L5_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L5_Y;

typedef volatile union _CAM_REG_AWB_D_L6_X_
{
    volatile struct
    {
        FIELD AWB_L6_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L6_X;

typedef volatile union _CAM_REG_AWB_D_L6_Y_
{
    volatile struct
    {
        FIELD AWB_L6_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L6_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L6_Y;

typedef volatile union _CAM_REG_AWB_D_L7_X_
{
    volatile struct
    {
        FIELD AWB_L7_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L7_X;

typedef volatile union _CAM_REG_AWB_D_L7_Y_
{
    volatile struct
    {
        FIELD AWB_L7_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L7_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L7_Y;

typedef volatile union _CAM_REG_AWB_D_L8_X_
{
    volatile struct
    {
        FIELD AWB_L8_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L8_X;

typedef volatile union _CAM_REG_AWB_D_L8_Y_
{
    volatile struct
    {
        FIELD AWB_L8_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L8_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L8_Y;

typedef volatile union _CAM_REG_AWB_D_L9_X_
{
    volatile struct
    {
        FIELD AWB_L9_X_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_X_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L9_X;

typedef volatile union _CAM_REG_AWB_D_L9_Y_
{
    volatile struct
    {
        FIELD AWB_L9_Y_LOW              : 14;
        FIELD rsv_14                    : 2;
        FIELD AWB_L9_Y_UP               : 14;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_L9_Y;

typedef volatile union _CAM_REG_AWB_D_SPARE_
{
    volatile struct
    {
        FIELD AWB_SPARE                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AWB_D_SPARE;

/* end MT6593_2091_raw_d_awb.xml*/

/* start MT6593_2092_raw_d_ae.xml*/
typedef volatile union _CAM_REG_AE_D_HST_CTL_
{
    volatile struct
    {
        FIELD AE_HST0_EN                : 1;
        FIELD AE_HST1_EN                : 1;
        FIELD AE_HST2_EN                : 1;
        FIELD AE_HST3_EN                : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_HST_CTL;

typedef volatile union _CAM_REG_AE_D_GAIN2_0_
{
    volatile struct
    {
        FIELD AE_GAIN2_R                : 12;
        FIELD rsv_12                    : 4;
        FIELD AE_GAIN2_G                : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_GAIN2_0;

typedef volatile union _CAM_REG_AE_D_GAIN2_1_
{
    volatile struct
    {
        FIELD AE_GAIN2_B                : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_GAIN2_1;

typedef volatile union _CAM_REG_AE_D_LMT2_0_
{
    volatile struct
    {
        FIELD AE_LMT2_R                 : 12;
        FIELD rsv_12                    : 4;
        FIELD AE_LMT2_G                 : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_LMT2_0;

typedef volatile union _CAM_REG_AE_D_LMT2_1_
{
    volatile struct
    {
        FIELD AE_LMT2_B                 : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_LMT2_1;

typedef volatile union _CAM_REG_AE_D_RC_CNV_0_
{
    volatile struct
    {
        FIELD AE_RC_CNV00               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV01               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_RC_CNV_0;

typedef volatile union _CAM_REG_AE_D_RC_CNV_1_
{
    volatile struct
    {
        FIELD AE_RC_CNV02               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV10               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_RC_CNV_1;

typedef volatile union _CAM_REG_AE_D_RC_CNV_2_
{
    volatile struct
    {
        FIELD AE_RC_CNV11               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV12               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_RC_CNV_2;

typedef volatile union _CAM_REG_AE_D_RC_CNV_3_
{
    volatile struct
    {
        FIELD AE_RC_CNV20               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_CNV21               : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_RC_CNV_3;

typedef volatile union _CAM_REG_AE_D_RC_CNV_4_
{
    volatile struct
    {
        FIELD AE_RC_CNV22               : 11;
        FIELD rsv_11                    : 5;
        FIELD AE_RC_ACC                 : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_RC_CNV_4;

typedef volatile union _CAM_REG_AE_D_YGAMMA_0_
{
    volatile struct
    {
        FIELD Y_GMR1                    : 8;
        FIELD Y_GMR2                    : 8;
        FIELD Y_GMR3                    : 8;
        FIELD Y_GMR4                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_YGAMMA_0;

typedef volatile union _CAM_REG_AE_D_YGAMMA_1_
{
    volatile struct
    {
        FIELD Y_GMR5                    : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_YGAMMA_1;

typedef volatile union _CAM_REG_AE_D_HST_SET_
{
    volatile struct
    {
        FIELD AE_BIN_MODE_0             : 2;
        FIELD rsv_2                     : 2;
        FIELD AE_BIN_MODE_1             : 2;
        FIELD rsv_6                     : 2;
        FIELD AE_BIN_MODE_2             : 2;
        FIELD rsv_10                    : 2;
        FIELD AE_BIN_MODE_3             : 2;
        FIELD rsv_14                    : 2;
        FIELD AE_COLOR_MODE_0           : 3;
        FIELD rsv_19                    : 1;
        FIELD AE_COLOR_MODE_1           : 3;
        FIELD rsv_23                    : 1;
        FIELD AE_COLOR_MODE_2           : 3;
        FIELD rsv_27                    : 1;
        FIELD AE_COLOR_MODE_3           : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_HST_SET;

typedef volatile union _CAM_REG_AE_D_HST0_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_0                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_0                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_0                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_0                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_HST0_RNG;

typedef volatile union _CAM_REG_AE_D_HST1_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_1                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_1                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_1                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_1                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_HST1_RNG;

typedef volatile union _CAM_REG_AE_D_HST2_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_2                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_2                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_2                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_2                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_HST2_RNG;

typedef volatile union _CAM_REG_AE_D_HST3_RNG_
{
    volatile struct
    {
        FIELD AE_X_LOW_3                : 7;
        FIELD rsv_7                     : 1;
        FIELD AE_X_HI_3                 : 7;
        FIELD rsv_15                    : 1;
        FIELD AE_Y_LOW_3                : 7;
        FIELD rsv_23                    : 1;
        FIELD AE_Y_HI_3                 : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_HST3_RNG;

typedef volatile union _CAM_REG_AE_D_SPARE_
{
    volatile struct
    {
        FIELD AE_SPARE                  : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AE_D_SPARE;

/* end MT6593_2092_raw_d_ae.xml*/

/* start MT6593_210_raw_d_sgg.xml*/
typedef volatile union _CAM_REG_SGG1_D_PGN_
{
    volatile struct
    {
        FIELD SGG1_D_GAIN               : 11;
        FIELD rsv_11                    : 21;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG1_D_PGN;

typedef volatile union _CAM_REG_SGG1_D_GMRC_1_
{
    volatile struct
    {
        FIELD SGG1_D_GMR_1              : 8;
        FIELD SGG1_D_GMR_2              : 8;
        FIELD SGG1_D_GMR_3              : 8;
        FIELD SGG1_D_GMR_4              : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG1_D_GMRC_1;

typedef volatile union _CAM_REG_SGG1_D_GMRC_2_
{
    volatile struct
    {
        FIELD SGG1_D_GMR_5              : 8;
        FIELD SGG1_D_GMR_6              : 8;
        FIELD SGG1_D_GMR_7              : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_SGG1_D_GMRC_2;

/* end MT6593_210_raw_d_sgg.xml*/

/* start MT6593_211_raw_d_af.xml*/
typedef volatile union _CAM_REG_AF_D_CON_
{
    volatile struct
    {
        FIELD AF_D_DECI                 : 2;
        FIELD AF_D_ZZ                   : 1;
        FIELD AF_D_ODD                  : 1;
        FIELD AF_D_TARX                 : 3;
        FIELD rsv_7                     : 1;
        FIELD AF_D_TARY                 : 3;
        FIELD rsv_11                    : 17;
        FIELD RESERVED                  : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_CON;

typedef volatile union _CAM_REG_AF_D_WINX_1_
{
    volatile struct
    {
        FIELD AF_D_WINX_0               : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_WINX_1               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_WINX_1;

typedef volatile union _CAM_REG_AF_D_WINX_2_
{
    volatile struct
    {
        FIELD AF_D_WINX_2               : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_WINX_3               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_WINX_2;

typedef volatile union _CAM_REG_AF_D_WINX_3_
{
    volatile struct
    {
        FIELD AF_D_WINX_4               : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_WINX_5               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_WINX_3;

typedef volatile union _CAM_REG_AF_D_WINY_1_
{
    volatile struct
    {
        FIELD AF_D_WINY_0               : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_WINY_1               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_WINY_1;

typedef volatile union _CAM_REG_AF_D_WINY_2_
{
    volatile struct
    {
        FIELD AF_D_WINY_2               : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_WINY_3               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_WINY_2;

typedef volatile union _CAM_REG_AF_D_WINY_3_
{
    volatile struct
    {
        FIELD AF_D_WINY_4               : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_WINY_5               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_WINY_3;

typedef volatile union _CAM_REG_AF_D_SIZE_
{
    volatile struct
    {
        FIELD AF_D_WIN_WD               : 10;
        FIELD rsv_10                    : 6;
        FIELD AF_D_WIN_HT               : 10;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_SIZE;

typedef volatile union _CAM_REG_AF_D_FLT_1_
{
    volatile struct
    {
        FIELD AF_D_FLT_P_1              : 8;
        FIELD AF_D_FLT_P_2              : 8;
        FIELD AF_D_FLT_P_3              : 8;
        FIELD AF_D_FLT_P_4              : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLT_1;

typedef volatile union _CAM_REG_AF_D_FLT_2_
{
    volatile struct
    {
        FIELD AF_D_FLT_P_5              : 8;
        FIELD AF_D_FLT_P_6              : 8;
        FIELD AF_D_FLT_P_7              : 8;
        FIELD AF_D_FLT_P_8              : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLT_2;

typedef volatile union _CAM_REG_AF_D_FLT_3_
{
    volatile struct
    {
        FIELD AF_D_FLT_P_9              : 8;
        FIELD AF_D_FLT_P_10             : 8;
        FIELD AF_D_FLT_P_11             : 8;
        FIELD AF_D_FLT_P_12             : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLT_3;

typedef volatile union _CAM_REG_AF_D_TH_
{
    volatile struct
    {
        FIELD AF_D_TH_0                 : 8;
        FIELD rsv_8                     : 8;
        FIELD AF_D_TH_1                 : 8;
        FIELD AF_D_TH_MODE              : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_TH;

typedef volatile union _CAM_REG_AF_D_FLO_WIN_1_
{
    volatile struct
    {
        FIELD AF_D_FLO_WINX_1           : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_FLO_WINY_1           : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_WIN_1;

typedef volatile union _CAM_REG_AF_D_FLO_SIZE_1_
{
    volatile struct
    {
        FIELD AF_D_FLO_WD_1             : 12;
        FIELD rsv_12                    : 4;
        FIELD AF_D_FLO_HT_1             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_SIZE_1;

typedef volatile union _CAM_REG_AF_D_FLO_WIN_2_
{
    volatile struct
    {
        FIELD AF_D_FLO_WINX_2           : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_FLO_WINY_2           : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_WIN_2;

typedef volatile union _CAM_REG_AF_D_FLO_SIZE_2_
{
    volatile struct
    {
        FIELD AF_D_FLO_WD_2             : 12;
        FIELD rsv_12                    : 4;
        FIELD AF_D_FLO_HT_2             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_SIZE_2;

typedef volatile union _CAM_REG_AF_D_FLO_WIN_3_
{
    volatile struct
    {
        FIELD AF_D_FLO_WINX_3           : 13;
        FIELD rsv_13                    : 3;
        FIELD AF_D_FLO_WINY_3           : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_WIN_3;

typedef volatile union _CAM_REG_AF_D_FLO_SIZE_3_
{
    volatile struct
    {
        FIELD AF_D_FLO_WD_3             : 12;
        FIELD rsv_12                    : 4;
        FIELD AF_D_FLO_HT_3             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_SIZE_3;

typedef volatile union _CAM_REG_AF_D_FLO_TH_
{
    volatile struct
    {
        FIELD AF_D_FLO_TH_0             : 8;
        FIELD rsv_8                     : 8;
        FIELD AF_D_FLO_TH_1             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_TH;

typedef volatile union _CAM_REG_AF_D_IMAGE_SIZE_
{
    volatile struct
    {
        FIELD AF_D_IMAGE_WD             : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_IMAGE_SIZE;

typedef volatile union _CAM_REG_AF_D_FLT_4_
{
    volatile struct
    {
        FIELD AF_D_FLT_X_0              : 11;
        FIELD rsv_11                    : 5;
        FIELD AF_D_FLT_X_1              : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLT_4;

typedef volatile union _CAM_REG_AF_D_FLT_5_
{
    volatile struct
    {
        FIELD AF_D_FLT_X_2              : 11;
        FIELD rsv_11                    : 5;
        FIELD AF_D_FLT_X_3              : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLT_5;

typedef volatile union _CAM_REG_AF_D_STAT_L_
{
    volatile struct
    {
        FIELD AF_D_STAT_LSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_STAT_L;

typedef volatile union _CAM_REG_AF_D_STAT_M_
{
    volatile struct
    {
        FIELD AF_D_STAT_MSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_STAT_M;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_1L_
{
    volatile struct
    {
        FIELD AF_D_STAT_LSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_1L;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_1M_
{
    volatile struct
    {
        FIELD AF_D_STAT_MSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_1M;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_1V_
{
    volatile struct
    {
        FIELD AF_D_STAT_VER             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_1V;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_2L_
{
    volatile struct
    {
        FIELD AF_D_STAT_LSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_2L;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_2M_
{
    volatile struct
    {
        FIELD AF_D_STAT_MSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_2M;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_2V_
{
    volatile struct
    {
        FIELD AF_D_STAT_VER             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_2V;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_3L_
{
    volatile struct
    {
        FIELD AF_D_STAT_LSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_3L;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_3M_
{
    volatile struct
    {
        FIELD AF_D_STAT_MSB             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_3M;

typedef volatile union _CAM_REG_AF_D_FLO_STAT_3V_
{
    volatile struct
    {
        FIELD AF_D_STAT_VER             : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AF_D_FLO_STAT_3V;

/* end MT6593_211_raw_d_af.xml*/

/* start MT6593_2111_raw_d_w2g.xml*/
typedef volatile union _CAM_REG_W2G_D_BLD_
{
    volatile struct
    {
        FIELD W2G_D_BLD_SL              : 5;
        FIELD rsv_5                     : 3;
        FIELD W2G_D_EXT_SL              : 5;
        FIELD rsv_13                    : 3;
        FIELD W2G_D_EXT_DSL             : 5;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_D_BLD;

typedef volatile union _CAM_REG_W2G_D_TH_1_
{
    volatile struct
    {
        FIELD W2G_D_DPF                 : 1;
        FIELD rsv_1                     : 3;
        FIELD W2G_D_FTH                 : 12;
        FIELD W2G_D_NTH                 : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_D_TH_1;

typedef volatile union _CAM_REG_W2G_D_TH_2_
{
    volatile struct
    {
        FIELD W2G_D_OFST                : 12;
        FIELD W2G_D_DTH                 : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_D_TH_2;

typedef volatile union _CAM_REG_W2G_D_CTL_OFT_
{
    volatile struct
    {
        FIELD W2G_D_H_START             : 13;
        FIELD rsv_13                    : 3;
        FIELD W2G_D_H_END               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_W2G_D_CTL_OFT;

/* end MT6593_2111_raw_d_w2g.xml*/

/* start MT6593_2112_raw_d_wbn.xml*/
typedef volatile union _CAM_REG_WBN_D_SIZE_
{
    volatile struct
    {
        FIELD WBN_D_IN_H                : 13;
        FIELD rsv_13                    : 3;
        FIELD WBN_D_IN_V                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_WBN_D_SIZE;

typedef volatile union _CAM_REG_WBN_D_MODE_
{
    volatile struct
    {
        FIELD WBN_D_ACC                 : 3;
        FIELD rsv_3                     : 1;
        FIELD WBN_D_RGBC_ID             : 2;
        FIELD rsv_6                     : 2;
        FIELD WBN_D_RGBC_DIR            : 1;
        FIELD rsv_9                     : 23;
    } Bits;
    UINT32 Raw;
} CAM_REG_WBN_D_MODE;

/* end MT6593_2112_raw_d_wbn.xml*/

/* start MT6593_213_raw_d_lcs.xml*/
typedef volatile union _CAM_REG_LCS_D_CON_
{
    volatile struct
    {
        FIELD LCS_D_LG2                 : 1;
        FIELD rsv_1                     : 15;
        FIELD LCS_D_OUT_WD              : 7;
        FIELD rsv_23                    : 1;
        FIELD LCS_D_OUT_HT              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_D_CON;

typedef volatile union _CAM_REG_LCS_D_ST_
{
    volatile struct
    {
        FIELD LCS_D_START_J             : 13;
        FIELD rsv_13                    : 3;
        FIELD LCS_D_START_I             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_D_ST;

typedef volatile union _CAM_REG_LCS_D_AWS_
{
    volatile struct
    {
        FIELD LCS_D_IN_WD               : 13;
        FIELD rsv_13                    : 3;
        FIELD LCS_D_IN_HT               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_D_AWS;

typedef volatile union _CAM_REG_LCS_D_FLR_
{
    volatile struct
    {
        FIELD LCS_D_FLR_OFST            : 7;
        FIELD LCS_D_FLR_GAIN            : 9;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_D_FLR;

typedef volatile union _CAM_REG_LCS_D_LRZR_1_
{
    volatile struct
    {
        FIELD LCS_D_LRZR_X              : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_D_LRZR_1;

typedef volatile union _CAM_REG_LCS_D_LRZR_2_
{
    volatile struct
    {
        FIELD LCS_D_LRZR_Y              : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCS_D_LRZR_2;

/* end MT6593_213_raw_d_lcs.xml*/

/* start MT6593_214_raw_d_rrz.xml*/
typedef volatile union _CAM_REG_RRZ_D_CTL_
{
    volatile struct
    {
        FIELD RRZ_HORI_EN               : 1;
        FIELD RRZ_VERT_EN               : 1;
        FIELD rsv_2                     : 14;
        FIELD RRZ_HORI_TBL_SEL          : 5;
        FIELD RRZ_VERT_TBL_SEL          : 5;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_CTL;

typedef volatile union _CAM_REG_RRZ_D_IN_IMG_
{
    volatile struct
    {
        FIELD RRZ_IN_WD                 : 13;
        FIELD rsv_13                    : 3;
        FIELD RRZ_IN_HT                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_IN_IMG;

typedef volatile union _CAM_REG_RRZ_D_OUT_IMG_
{
    volatile struct
    {
        FIELD RRZ_OUT_WD                : 13;
        FIELD rsv_13                    : 3;
        FIELD RRZ_OUT_HT                : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_OUT_IMG;

typedef volatile union _CAM_REG_RRZ_D_HORI_STEP_
{
    volatile struct
    {
        FIELD RRZ_HORI_STEP             : 17;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_HORI_STEP;

typedef volatile union _CAM_REG_RRZ_D_VERT_STEP_
{
    volatile struct
    {
        FIELD RRZ_VERT_STEP             : 17;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_VERT_STEP;

typedef volatile union _CAM_REG_RRZ_D_HORI_INT_OFST_
{
    volatile struct
    {
        FIELD RRZ_HORI_INT_OFST         : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_HORI_INT_OFST;

typedef volatile union _CAM_REG_RRZ_D_HORI_SUB_OFST_
{
    volatile struct
    {
        FIELD RRZ_HORI_SUB_OFST         : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_HORI_SUB_OFST;

typedef volatile union _CAM_REG_RRZ_D_VERT_INT_OFST_
{
    volatile struct
    {
        FIELD RRZ_VERT_INT_OFST         : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_VERT_INT_OFST;

typedef volatile union _CAM_REG_RRZ_D_VERT_SUB_OFST_
{
    volatile struct
    {
        FIELD RRZ_VERT_SUB_OFST         : 15;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_VERT_SUB_OFST;

typedef volatile union _CAM_REG_RRZ_D_MODE_TH_
{
    volatile struct
    {
        FIELD RRZ_TH_MD                 : 8;
        FIELD RRZ_TH_HI                 : 8;
        FIELD RRZ_TH_LO                 : 8;
        FIELD RRZ_TH_MD2                : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_MODE_TH;

typedef volatile union _CAM_REG_RRZ_D_MODE_CTL_
{
    volatile struct
    {
        FIELD RRZ_PRF_BLD               : 9;
        FIELD RRZ_PRF                   : 2;
        FIELD RRZ_BLD_SL                : 5;
        FIELD RRZ_CR_MODE               : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZ_D_MODE_CTL;

/* end MT6593_214_raw_d_rrz.xml*/

/* start MT6593_216_raw_d_bpc.xml*/
typedef volatile union _CAM_REG_BPC_D_CON_
{
    volatile struct
    {
        FIELD BPC_EN                    : 1;
        FIELD rsv_1                     : 3;
        FIELD BPC_LUT_EN                : 1;
        FIELD BPC_TABLE_END_MODE        : 1;
        FIELD rsv_6                     : 6;
        FIELD BPC_DTC_MODE              : 2;
        FIELD BPC_CS_MODE               : 2;
        FIELD BPC_CRC_MODE              : 2;
        FIELD rsv_18                    : 14;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_CON;

typedef volatile union _CAM_REG_BPC_D_TH1_
{
    volatile struct
    {
        FIELD BPC_TH_LWB                : 12;
        FIELD BPC_TH_Y                  : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TH1;

typedef volatile union _CAM_REG_BPC_D_TH2_
{
    volatile struct
    {
        FIELD BPC_TH_UPB                : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TH2;

typedef volatile union _CAM_REG_BPC_D_TH3_
{
    volatile struct
    {
        FIELD BPC_TH_XA                 : 12;
        FIELD BPC_TH_XB                 : 12;
        FIELD BPC_TH_SLA                : 4;
        FIELD BPC_TH_SLB                : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TH3;

typedef volatile union _CAM_REG_BPC_D_TH4_
{
    volatile struct
    {
        FIELD BPC_DK_TH_XA              : 12;
        FIELD BPC_DK_TH_XB              : 12;
        FIELD BPC_DK_TH_SLA             : 4;
        FIELD BPC_DK_TH_SLB             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TH4;

typedef volatile union _CAM_REG_BPC_D_DTC_
{
    volatile struct
    {
        FIELD BPC_RNG                   : 4;
        FIELD BPC_CS_RNG                : 3;
        FIELD rsv_7                     : 1;
        FIELD BPC_CT_LV                 : 4;
        FIELD rsv_12                    : 4;
        FIELD BPC_TH_MUL                : 4;
        FIELD rsv_20                    : 4;
        FIELD BPC_NO_LV                 : 3;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_DTC;

typedef volatile union _CAM_REG_BPC_D_COR_
{
    volatile struct
    {
        FIELD BPC_DIR_MAX               : 8;
        FIELD BPC_DIR_TH                : 8;
        FIELD BPC_RANK_IDX              : 3;
        FIELD rsv_19                    : 5;
        FIELD BPC_DIR_TH2               : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_COR;

typedef volatile union _CAM_REG_BPC_D_TBLI1_
{
    volatile struct
    {
        FIELD BPC_XOFFSET               : 13;
        FIELD rsv_13                    : 3;
        FIELD BPC_YOFFSET               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TBLI1;

typedef volatile union _CAM_REG_BPC_D_TBLI2_
{
    volatile struct
    {
        FIELD BPC_XSIZE                 : 13;
        FIELD rsv_13                    : 3;
        FIELD BPC_YSIZE                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TBLI2;

typedef volatile union _CAM_REG_BPC_D_TH1_C_
{
    volatile struct
    {
        FIELD BPC_C_TH_LWB              : 12;
        FIELD BPC_C_TH_Y                : 12;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TH1_C;

typedef volatile union _CAM_REG_BPC_D_TH2_C_
{
    volatile struct
    {
        FIELD BPC_C_TH_UPB              : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TH2_C;

typedef volatile union _CAM_REG_BPC_D_TH3_C_
{
    volatile struct
    {
        FIELD BPC_C_TH_XA               : 12;
        FIELD BPC_C_TH_XB               : 12;
        FIELD BPC_C_TH_SLA              : 4;
        FIELD BPC_C_TH_SLB              : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_TH3_C;

typedef volatile union _CAM_REG_CPG_D_SATU_1_
{
    volatile struct
    {
        FIELD CPG_D_SATU_B              : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_D_SATU_GB             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_D_SATU_1;

typedef volatile union _CAM_REG_CPG_D_SATU_2_
{
    volatile struct
    {
        FIELD CPG_D_SATU_GR             : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_D_SATU_R              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_D_SATU_2;






typedef volatile union _CAM_REG_CPG_D_GAIN_1_
{
    volatile struct
    {
        FIELD CPG_D_GAIN_B              : 13;
        FIELD rsv_13                    : 3;
        FIELD CPG_D_GAIN_GB             : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_D_GAIN_1;

typedef volatile union _CAM_REG_CPG_D_GAIN_2_
{
    volatile struct
    {
        FIELD CPG_D_GAIN_GR             : 13;
        FIELD rsv_13                    : 3;
        FIELD CPG_D_GAIN_R              : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_D_GAIN_2;


typedef volatile union _CAM_REG_CPG_D_OFST_1_
{
    volatile struct
    {
        FIELD CPG_D_OFST_B              : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_D_OFST_GB             : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_D_OFST_1;

typedef volatile union _CAM_REG_CPG_D_OFST_2_
{
    volatile struct
    {
        FIELD CPG_D_OFST_GR             : 12;
        FIELD rsv_12                    : 4;
        FIELD CPG_D_OFST_R              : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_CPG_D_OFST_2;

typedef volatile union _CAM_REG_BPC_D_RMM1_
{
    volatile struct
    {
        FIELD BPC_RMM_OSC_TH            : 12;
        FIELD BPC_RMM_SEDIR_SL          : 3;
        FIELD rsv_15                    : 1;
        FIELD BPC_RMM_SEBLD_WD          : 3;
        FIELD rsv_19                    : 1;
        FIELD BPC_RMM_LEBLD_WD          : 3;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_RMM1;

typedef volatile union _CAM_REG_BPC_D_RMM2_
{
    volatile struct
    {
        FIELD BPC_RMM_MO_MODE           : 1;
        FIELD rsv_1                     : 3;
        FIELD BPC_RMM_MOBLD_FT          : 3;
        FIELD rsv_7                     : 1;
        FIELD BPC_RMM_MOTH_RATIO        : 4;
        FIELD rsv_12                    : 4;
        FIELD BPC_RMM_HORI_ADDWT        : 5;
        FIELD rsv_21                    : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_RMM2;

typedef volatile union _CAM_REG_BPC_D_RMM_REVG_1_
{
    volatile struct
    {
        FIELD BPC_RMM_REVG_R            : 11;
        FIELD rsv_11                    : 5;
        FIELD BPC_RMM_REVG_GR           : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_RMM_REVG_1;

typedef volatile union _CAM_REG_BPC_D_RMM_REVG_2_
{
    volatile struct
    {
        FIELD BPC_RMM_REVG_B            : 11;
        FIELD rsv_11                    : 5;
        FIELD BPC_RMM_REVG_GB           : 11;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_RMM_REVG_2;

typedef volatile union _CAM_REG_BPC_D_RMM_LEOS_
{
    volatile struct
    {
        FIELD BPC_RMM_LEOS_GRAY         : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_RMM_LEOS;

typedef volatile union _CAM_REG_BPC_D_RMM_GCNT_
{
    volatile struct
    {
        FIELD BPC_RMM_GRAY_CNT          : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPC_D_RMM_GCNT;

/* end MT6593_216_raw_d_bpc.xml*/

/* start MT6593_216_raw_d_ct.xml*/
typedef volatile union _CAM_REG_NR1_D_CON_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD NR1_CT_EN                 : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR1_D_CON;

typedef volatile union _CAM_REG_NR1_D_CT_CON_
{
    volatile struct
    {
        FIELD NR1_CT_MD                 : 2;
        FIELD NR1_CT_MD2                : 2;
        FIELD NR1_CT_THRD               : 10;
        FIELD rsv_14                    : 2;
        FIELD NR1_MBND                  : 10;
        FIELD rsv_26                    : 2;
        FIELD NR1_CT_SLOPE              : 2;
        FIELD NR1_CT_DIV                : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_NR1_D_CT_CON;

typedef volatile union _CAM_REG_BNR_D_RSV1_
{
    volatile struct
    {
        FIELD RSV1                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BNR_D_RSV1;

typedef volatile union _CAM_REG_BNR_D_RSV2_
{
    volatile struct
    {
        FIELD RSV2                      : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BNR_D_RSV2;

/* end MT6593_216_raw_d_ct.xml*/

/* start MT6593_600_raw_d_dmx_bmx_rmx.xml*/
typedef volatile union _CAM_REG_DMX_D_CTL_
{
    volatile struct
    {
        FIELD DMX_IHDR_SEP_MODE         : 1;
        FIELD rsv_1                     : 3;
        FIELD DMX_IHDR_ORD              : 1;
        FIELD rsv_5                     : 11;
        FIELD DMX_EDGE                  : 4;
        FIELD rsv_20                    : 11;
        FIELD DMX_EDGE_SET              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMX_D_CTL;

typedef volatile union _CAM_REG_DMX_D_CROP_
{
    volatile struct
    {
        FIELD DMX_STR_X                 : 13;
        FIELD rsv_13                    : 3;
        FIELD DMX_END_X                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMX_D_CROP;

typedef volatile union _CAM_REG_DMX_D_VSIZE_
{
    volatile struct
    {
        FIELD DMX_HT                    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMX_D_VSIZE;

typedef volatile union _CAM_REG_BMX_D_CTL_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD BMX_SINGLE_MODE           : 1;
        FIELD rsv_5                     : 11;
        FIELD BMX_EDGE                  : 4;
        FIELD rsv_20                    : 11;
        FIELD BMX_EDGE_SET              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_BMX_D_CTL;

typedef volatile union _CAM_REG_BMX_D_CROP_
{
    volatile struct
    {
        FIELD BMX_STR_X                 : 13;
        FIELD rsv_13                    : 3;
        FIELD BMX_END_X                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_BMX_D_CROP;

typedef volatile union _CAM_REG_BMX_D_VSIZE_
{
    volatile struct
    {
        FIELD BMX_HT                    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_BMX_D_VSIZE;

typedef volatile union _CAM_REG_RMX_D_CTL_
{
    volatile struct
    {
        FIELD rsv_0                     : 4;
        FIELD RMX_SINGLE_MODE           : 1;
        FIELD rsv_5                     : 11;
        FIELD RMX_EDGE                  : 4;
        FIELD rsv_20                    : 11;
        FIELD RMX_EDGE_SET              : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_RMX_D_CTL;

typedef volatile union _CAM_REG_RMX_D_CROP_
{
    volatile struct
    {
        FIELD RMX_STR_X                 : 13;
        FIELD rsv_13                    : 3;
        FIELD RMX_END_X                 : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RMX_D_CROP;

typedef volatile union _CAM_REG_RMX_D_VSIZE_
{
    volatile struct
    {
        FIELD RMX_HT                    : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RMX_D_VSIZE;

/* end MT6593_600_raw_d_dmx_bmx_rmx.xml*/

/* start MT6593_100_dma.xml*/
typedef volatile union _CAM_REG_DMA_SOFT_RSTSTAT_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD IMGI_SOFT_RST_STAT        : 1;
        FIELD BPCI_SOFT_RST_STAT        : 1;
        FIELD rsv_3                     : 1;
        FIELD LSCI_SOFT_RST_STAT        : 1;
        FIELD UFDI_SOFT_RST_STAT        : 1;
        FIELD LCEI_SOFT_RST_STAT        : 1;
        FIELD VIPI_SOFT_RST_STAT        : 1;
        FIELD VIP2I_SOFT_RST_STAT       : 1;
        FIELD VIP3I_SOFT_RST_STAT       : 1;
        FIELD rsv_10                    : 2;
        FIELD IMGO_SOFT_RST_STAT        : 1;
        FIELD RRZO_SOFT_RST_STAT        : 1;
        FIELD LCSO_SOFT_RST_STAT        : 1;
        FIELD ESFKO_SOFT_RST_STAT       : 1;
        FIELD AAO_SOFT_RST_STAT         : 1;
        FIELD UFEO_SOFT_RST_STAT        : 1;
        FIELD MFBO_SOFT_RST_STAT        : 1;
        FIELD IMG3BO_SOFT_RST_STAT      : 1;
        FIELD IMG3CO_SOFT_RST_STAT      : 1;
        FIELD IMG2O_SOFT_RST_STAT       : 1;
        FIELD IMG3O_SOFT_RST_STAT       : 1;
        FIELD FEO_SOFT_RST_STAT         : 1;
        FIELD BPCI_D_SOFT_RST_STAT      : 1;
        FIELD LSCI_D_SOFT_RST_STAT      : 1;
        FIELD IMGO_D_SOFT_RST_STAT      : 1;
        FIELD RRZO_D_SOFT_RST_STAT      : 1;
        FIELD LCSO_D_SOFT_RST_STAT      : 1;
        FIELD AFO_D_SOFT_RST_STAT       : 1;
        FIELD AAO_D_SOFT_RST_STAT       : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_SOFT_RSTSTAT;

typedef volatile union _CAM_REG_TDRI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TDRI_BASE_ADDR;

typedef volatile union _CAM_REG_TDRI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_TDRI_OFST_ADDR;

typedef volatile union _CAM_REG_TDRI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_TDRI_XSIZE;

typedef volatile union _CAM_REG_CQ0I_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CQ0I_BASE_ADDR;

typedef volatile union _CAM_REG_CQ0I_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CQ0I_XSIZE;

typedef volatile union _CAM_REG_CQ0I_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_CQ0I_D_BASE_ADDR;

typedef volatile union _CAM_REG_CQ0I_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_CQ0I_D_XSIZE;

typedef volatile union _CAM_REG_VERTICAL_FLIP_EN_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD IMGI_V_FLIP_EN            : 1;
        FIELD BPCI_V_FLIP_EN            : 1;
        FIELD rsv_3                     : 1;
        FIELD LSCI_V_FLIP_EN            : 1;
        FIELD UFDI_V_FLIP_EN            : 1;
        FIELD LCEI_V_FLIP_EN            : 1;
        FIELD VIPI_V_FLIP_EN            : 1;
        FIELD VIP2I_V_FLIP_EN           : 1;
        FIELD VIP3I_V_FLIP_EN           : 1;
        FIELD rsv_10                    : 2;
        FIELD IMGO_V_FLIP_EN            : 1;
        FIELD RRZO_V_FLIP_EN            : 1;
        FIELD LCSO_V_FLIP_EN            : 1;
        FIELD ESFKO_V_FLIP_EN           : 1;
        FIELD AAO_V_FLIP_EN             : 1;
        FIELD UFEO_V_FLIP_EN            : 1;
        FIELD MFBO_V_FLIP_EN            : 1;
        FIELD IMG3BO_V_FLIP_EN          : 1;
        FIELD IMG3CO_V_FLIP_EN          : 1;
        FIELD IMG2O_V_FLIP_EN           : 1;
        FIELD IMG3O_V_FLIP_EN           : 1;
        FIELD FEO_V_FLIP_EN             : 1;
        FIELD BPCI_D_V_FLIP_EN          : 1;
        FIELD LSCI_D_V_FLIP_EN          : 1;
        FIELD IMGO_D_V_FLIP_EN          : 1;
        FIELD RRZO_D_V_FLIP_EN          : 1;
        FIELD LCSO_D_V_FLIP_EN          : 1;
        FIELD AFO_D_V_FLIP_EN           : 1;
        FIELD AAO_D_V_FLIP_EN           : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_VERTICAL_FLIP_EN;

typedef volatile union _CAM_REG_DMA_SOFT_RESET_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD IMGI_SOFT_RST             : 1;
        FIELD BPCI_SOFT_RST             : 1;
        FIELD rsv_3                     : 1;
        FIELD LSCI_SOFT_RST             : 1;
        FIELD UFDI_SOFT_RST             : 1;
        FIELD LCEI_SOFT_RST             : 1;
        FIELD VIPI_SOFT_RST             : 1;
        FIELD VIP2I_SOFT_RST            : 1;
        FIELD VIP3I_SOFT_RST            : 1;
        FIELD rsv_10                    : 2;
        FIELD IMGO_SOFT_RST             : 1;
        FIELD RRZO_SOFT_RST             : 1;
        FIELD LCSO_SOFT_RST             : 1;
        FIELD ESFKO_SOFT_RST            : 1;
        FIELD AAO_SOFT_RST              : 1;
        FIELD UFEO_SOFT_RST             : 1;
        FIELD MFBO_SOFT_RST             : 1;
        FIELD IMG3BO_SOFT_RST           : 1;
        FIELD IMG3CO_SOFT_RST           : 1;
        FIELD IMG2O_SOFT_RST            : 1;
        FIELD IMG3O_SOFT_RST            : 1;
        FIELD FEO_SOFT_RST              : 1;
        FIELD BPCI_D_SOFT_RST           : 1;
        FIELD LSCI_D_SOFT_RST           : 1;
        FIELD IMGO_D_SOFT_RST           : 1;
        FIELD RRZO_D_SOFT_RST           : 1;
        FIELD LCSO_D_SOFT_RST           : 1;
        FIELD AFO_D_SOFT_RST            : 1;
        FIELD AAO_D_SOFT_RST            : 1;
        FIELD SEPARATE_SOFT_RST_EN      : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_SOFT_RESET;

typedef volatile union _CAM_REG_LAST_ULTRA_EN_
{
    volatile struct
    {
        FIELD BPCI_LAST_ULTRA_EN        : 1;
        FIELD LSCI_LAST_ULTRA_EN        : 1;
        FIELD BPCI_D_LAST_ULTRA_EN      : 1;
        FIELD LSCI_D_LAST_ULTRA_EN      : 1;
        FIELD rsv_4                     : 8;
        FIELD IMGO_LAST_ULTRA_EN        : 1;
        FIELD RRZO_LAST_ULTRA_EN        : 1;
        FIELD LCSO_LAST_ULTRA_EN        : 1;
        FIELD ESFKO_LAST_ULTRA_EN       : 1;
        FIELD AAO_LAST_ULTRA_EN         : 1;
        FIELD UFEO_LAST_ULTRA_EN        : 1;
        FIELD MFBO_LAST_ULTRA_EN        : 1;
        FIELD IMG3BO_LAST_ULTRA_EN      : 1;
        FIELD IMG3CO_LAST_ULTRA_EN      : 1;
        FIELD IMG2O_LAST_ULTRA_EN       : 1;
        FIELD IMG3O_LAST_ULTRA_EN       : 1;
        FIELD FEO_LAST_ULTRA_EN         : 1;
        FIELD rsv_24                    : 2;
        FIELD IMGO_D_LAST_ULTRA_EN      : 1;
        FIELD RRZO_D_LAST_ULTRA_EN      : 1;
        FIELD LCSO_D_LAST_ULTRA_EN      : 1;
        FIELD AFO_D_LAST_ULTRA_EN       : 1;
        FIELD AAO_D_LAST_ULTRA_EN       : 1;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_LAST_ULTRA_EN;

typedef volatile union _CAM_REG_IMGI_SLOW_DOWN_
{
    volatile struct
    {
        FIELD SLOW_CNT                  : 16;
        FIELD IMGO_RRZO_SRAM_CHANGE_EN  : 1;
        FIELD rsv_17                    : 6;
        FIELD CONTINUOUS_COM_CON        : 2;
        FIELD CONTINUOUS_COM_EN         : 1;
        FIELD CQ_ULTRA_LSCI_EN          : 1;
        FIELD CQ_ULTRA_IMGI_EN          : 1;
        FIELD UFO_IMGO_EN               : 1;
        FIELD UFO_IMGI_EN               : 1;
        FIELD INTERLACE_MODE            : 1;
        FIELD SLOW_EN                   : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_SLOW_DOWN;

typedef volatile union _CAM_REG_IMGI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_BASE_ADDR;

typedef volatile union _CAM_REG_IMGI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_OFST_ADDR;

typedef volatile union _CAM_REG_IMGI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_XSIZE;

typedef volatile union _CAM_REG_IMGI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_YSIZE;

typedef volatile union _CAM_REG_IMGI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 15;
        FIELD rsv_15                    : 1;
        FIELD BUS_SIZE                  : 4;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_STRIDE;

typedef volatile union _CAM_REG_IMGI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_CON;

typedef volatile union _CAM_REG_IMGI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_CON2;

typedef volatile union _CAM_REG_BPCI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_BASE_ADDR;

typedef volatile union _CAM_REG_BPCI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_OFST_ADDR;

typedef volatile union _CAM_REG_BPCI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_XSIZE;

typedef volatile union _CAM_REG_BPCI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_YSIZE;

typedef volatile union _CAM_REG_BPCI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 3;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_STRIDE;

typedef volatile union _CAM_REG_BPCI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_CON;

typedef volatile union _CAM_REG_BPCI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_CON2;

typedef volatile union _CAM_REG_LSCI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_BASE_ADDR;

typedef volatile union _CAM_REG_LSCI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_OFST_ADDR;

typedef volatile union _CAM_REG_LSCI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_XSIZE;

typedef volatile union _CAM_REG_LSCI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_YSIZE;

typedef volatile union _CAM_REG_LSCI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 3;
        FIELD rsv_19                    : 1;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_STRIDE;

typedef volatile union _CAM_REG_LSCI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_CON;

typedef volatile union _CAM_REG_LSCI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_CON2;

typedef volatile union _CAM_REG_UFDI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_BASE_ADDR;

typedef volatile union _CAM_REG_UFDI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_OFST_ADDR;

typedef volatile union _CAM_REG_UFDI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_XSIZE;

typedef volatile union _CAM_REG_UFDI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_YSIZE;

typedef volatile union _CAM_REG_UFDI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 4;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_STRIDE;

typedef volatile union _CAM_REG_UFDI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_CON;

typedef volatile union _CAM_REG_UFDI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_CON2;

typedef volatile union _CAM_REG_LCEI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_BASE_ADDR;

typedef volatile union _CAM_REG_LCEI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_OFST_ADDR;

typedef volatile union _CAM_REG_LCEI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_XSIZE;

typedef volatile union _CAM_REG_LCEI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_YSIZE;

typedef volatile union _CAM_REG_LCEI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 3;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_STRIDE;

typedef volatile union _CAM_REG_LCEI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_CON;

typedef volatile union _CAM_REG_LCEI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_CON2;

typedef volatile union _CAM_REG_VIPI_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_BASE_ADDR;

typedef volatile union _CAM_REG_VIPI_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_OFST_ADDR;

typedef volatile union _CAM_REG_VIPI_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_XSIZE;

typedef volatile union _CAM_REG_VIPI_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_YSIZE;

typedef volatile union _CAM_REG_VIPI_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 3;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_STRIDE;

typedef volatile union _CAM_REG_VIPI_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_CON;

typedef volatile union _CAM_REG_VIPI_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_CON2;

typedef volatile union _CAM_REG_VIP2I_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_BASE_ADDR;

typedef volatile union _CAM_REG_VIP2I_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_OFST_ADDR;

typedef volatile union _CAM_REG_VIP2I_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_XSIZE;

typedef volatile union _CAM_REG_VIP2I_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_YSIZE;

typedef volatile union _CAM_REG_VIP2I_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 3;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_STRIDE;

typedef volatile union _CAM_REG_VIP2I_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_CON;

typedef volatile union _CAM_REG_VIP2I_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_CON2;

typedef volatile union _CAM_REG_IMGO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_BASE_ADDR;

typedef volatile union _CAM_REG_IMGO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_OFST_ADDR;

typedef volatile union _CAM_REG_IMGO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_XSIZE;

typedef volatile union _CAM_REG_IMGO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_YSIZE;

typedef volatile union _CAM_REG_IMGO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 2;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_STRIDE;

typedef volatile union _CAM_REG_IMGO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CON;

typedef volatile union _CAM_REG_IMGO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CON2;

typedef volatile union _CAM_REG_IMGO_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_CROP;

typedef volatile union _CAM_REG_RRZO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_BASE_ADDR;

typedef volatile union _CAM_REG_RRZO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_OFST_ADDR;

typedef volatile union _CAM_REG_RRZO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_XSIZE;

typedef volatile union _CAM_REG_RRZO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_YSIZE;

typedef volatile union _CAM_REG_RRZO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 3;
        FIELD rsv_19                    : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_STRIDE;

typedef volatile union _CAM_REG_RRZO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_CON;

typedef volatile union _CAM_REG_RRZO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_CON2;

typedef volatile union _CAM_REG_RRZO_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_CROP;

typedef volatile union _CAM_REG_LCSO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_BASE_ADDR;

typedef volatile union _CAM_REG_LCSO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_OFST_ADDR;

typedef volatile union _CAM_REG_LCSO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_XSIZE;

typedef volatile union _CAM_REG_LCSO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_YSIZE;

typedef volatile union _CAM_REG_LCSO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_STRIDE;

typedef volatile union _CAM_REG_LCSO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_CON;

typedef volatile union _CAM_REG_LCSO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_CON2;

typedef volatile union _CAM_REG_EISO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_EISO_BASE_ADDR;

typedef volatile union _CAM_REG_EISO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 10;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_EISO_XSIZE;

typedef volatile union _CAM_REG_AFO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_BASE_ADDR;

typedef volatile union _CAM_REG_AFO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 10;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_XSIZE;

typedef volatile union _CAM_REG_ESFKO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_BASE_ADDR;

typedef volatile union _CAM_REG_ESFKO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_XSIZE;

typedef volatile union _CAM_REG_ESFKO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_OFST_ADDR;

typedef volatile union _CAM_REG_ESFKO_YSIZE_
{
    volatile struct
    {
        FIELD RSV                       : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_YSIZE;

typedef volatile union _CAM_REG_ESFKO_STRIDE_
{
    volatile struct
    {
        FIELD RSV                       : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_STRIDE;

typedef volatile union _CAM_REG_ESFKO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_CON;

typedef volatile union _CAM_REG_ESFKO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_CON2;

typedef volatile union _CAM_REG_AAO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_BASE_ADDR;

typedef volatile union _CAM_REG_AAO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_OFST_ADDR;

typedef volatile union _CAM_REG_AAO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 17;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_XSIZE;

typedef volatile union _CAM_REG_AAO_YSIZE_
{
    volatile struct
    {
        FIELD RSV                       : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_YSIZE;

typedef volatile union _CAM_REG_AAO_STRIDE_
{
    volatile struct
    {
        FIELD RSV                       : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 6;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_STRIDE;

typedef volatile union _CAM_REG_AAO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_CON;

typedef volatile union _CAM_REG_AAO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_CON2;

typedef volatile union _CAM_REG_VIP3I_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_BASE_ADDR;

typedef volatile union _CAM_REG_VIP3I_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_OFST_ADDR;

typedef volatile union _CAM_REG_VIP3I_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_XSIZE;

typedef volatile union _CAM_REG_VIP3I_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_YSIZE;

typedef volatile union _CAM_REG_VIP3I_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 3;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_STRIDE;

typedef volatile union _CAM_REG_VIP3I_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_CON;

typedef volatile union _CAM_REG_VIP3I_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_CON2;

typedef volatile union _CAM_REG_UFEO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_BASE_ADDR;

typedef volatile union _CAM_REG_UFEO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_OFST_ADDR;

typedef volatile union _CAM_REG_UFEO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_XSIZE;

typedef volatile union _CAM_REG_UFEO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_YSIZE;

typedef volatile union _CAM_REG_UFEO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_STRIDE;

typedef volatile union _CAM_REG_UFEO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_CON;

typedef volatile union _CAM_REG_UFEO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_CON2;

typedef volatile union _CAM_REG_MFBO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_BASE_ADDR;

typedef volatile union _CAM_REG_MFBO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_OFST_ADDR;

typedef volatile union _CAM_REG_MFBO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_XSIZE;

typedef volatile union _CAM_REG_MFBO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_YSIZE;

typedef volatile union _CAM_REG_MFBO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_STRIDE;

typedef volatile union _CAM_REG_MFBO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_CON;

typedef volatile union _CAM_REG_MFBO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_CON2;

typedef volatile union _CAM_REG_MFBO_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_CROP;

typedef volatile union _CAM_REG_IMG3BO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_BASE_ADDR;

typedef volatile union _CAM_REG_IMG3BO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_OFST_ADDR;

typedef volatile union _CAM_REG_IMG3BO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_XSIZE;

typedef volatile union _CAM_REG_IMG3BO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_YSIZE;

typedef volatile union _CAM_REG_IMG3BO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 2;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_STRIDE;

typedef volatile union _CAM_REG_IMG3BO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_CON;

typedef volatile union _CAM_REG_IMG3BO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_CON2;

typedef volatile union _CAM_REG_IMG3BO_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_CROP;

typedef volatile union _CAM_REG_IMG3CO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_BASE_ADDR;

typedef volatile union _CAM_REG_IMG3CO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_OFST_ADDR;

typedef volatile union _CAM_REG_IMG3CO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_XSIZE;

typedef volatile union _CAM_REG_IMG3CO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_YSIZE;

typedef volatile union _CAM_REG_IMG3CO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 2;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_STRIDE;

typedef volatile union _CAM_REG_IMG3CO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_CON;

typedef volatile union _CAM_REG_IMG3CO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_CON2;

typedef volatile union _CAM_REG_IMG3CO_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_CROP;

typedef volatile union _CAM_REG_IMG2O_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_BASE_ADDR;

typedef volatile union _CAM_REG_IMG2O_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_OFST_ADDR;

typedef volatile union _CAM_REG_IMG2O_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_XSIZE;

typedef volatile union _CAM_REG_IMG2O_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_YSIZE;

typedef volatile union _CAM_REG_IMG2O_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 2;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_STRIDE;

typedef volatile union _CAM_REG_IMG2O_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_CON;

typedef volatile union _CAM_REG_IMG2O_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_CON2;

typedef volatile union _CAM_REG_IMG2O_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_CROP;

typedef volatile union _CAM_REG_IMG3O_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_BASE_ADDR;

typedef volatile union _CAM_REG_IMG3O_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_OFST_ADDR;

typedef volatile union _CAM_REG_IMG3O_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_XSIZE;

typedef volatile union _CAM_REG_IMG3O_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_YSIZE;

typedef volatile union _CAM_REG_IMG3O_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 2;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_STRIDE;

typedef volatile union _CAM_REG_IMG3O_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_CON;

typedef volatile union _CAM_REG_IMG3O_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_CON2;

typedef volatile union _CAM_REG_IMG3O_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_CROP;

typedef volatile union _CAM_REG_FEO_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_BASE_ADDR;

typedef volatile union _CAM_REG_FEO_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_OFST_ADDR;

typedef volatile union _CAM_REG_FEO_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_XSIZE;

typedef volatile union _CAM_REG_FEO_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_YSIZE;

typedef volatile union _CAM_REG_FEO_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_STRIDE;

typedef volatile union _CAM_REG_FEO_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_CON;

typedef volatile union _CAM_REG_FEO_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_CON2;

typedef volatile union _CAM_REG_BPCI_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_BASE_ADDR;

typedef volatile union _CAM_REG_BPCI_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_OFST_ADDR;

typedef volatile union _CAM_REG_BPCI_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_XSIZE;

typedef volatile union _CAM_REG_BPCI_D_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_YSIZE;

typedef volatile union _CAM_REG_BPCI_D_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 1;
        FIELD rsv_17                    : 3;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_STRIDE;

typedef volatile union _CAM_REG_BPCI_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_CON;

typedef volatile union _CAM_REG_BPCI_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_CON2;

typedef volatile union _CAM_REG_LSCI_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_BASE_ADDR;

typedef volatile union _CAM_REG_LSCI_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_OFST_ADDR;

typedef volatile union _CAM_REG_LSCI_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_XSIZE;

typedef volatile union _CAM_REG_LSCI_D_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_YSIZE;

typedef volatile union _CAM_REG_LSCI_D_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 3;
        FIELD rsv_19                    : 1;
        FIELD FORMAT                    : 1;
        FIELD rsv_21                    : 2;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 5;
        FIELD SWAP                      : 2;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_STRIDE;

typedef volatile union _CAM_REG_LSCI_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_CON;

typedef volatile union _CAM_REG_LSCI_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_CON2;

typedef volatile union _CAM_REG_IMGO_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_BASE_ADDR;

typedef volatile union _CAM_REG_IMGO_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_OFST_ADDR;

typedef volatile union _CAM_REG_IMGO_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_XSIZE;

typedef volatile union _CAM_REG_IMGO_D_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_YSIZE;

typedef volatile union _CAM_REG_IMGO_D_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 2;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_STRIDE;

typedef volatile union _CAM_REG_IMGO_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_CON;

typedef volatile union _CAM_REG_IMGO_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_CON2;

typedef volatile union _CAM_REG_IMGO_D_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_CROP;

typedef volatile union _CAM_REG_RRZO_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_BASE_ADDR;

typedef volatile union _CAM_REG_RRZO_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_OFST_ADDR;

typedef volatile union _CAM_REG_RRZO_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_XSIZE;

typedef volatile union _CAM_REG_RRZO_D_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_YSIZE;

typedef volatile union _CAM_REG_RRZO_D_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 3;
        FIELD rsv_19                    : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_STRIDE;

typedef volatile union _CAM_REG_RRZO_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_CON;

typedef volatile union _CAM_REG_RRZO_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_CON2;

typedef volatile union _CAM_REG_RRZO_D_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_CROP;

typedef volatile union _CAM_REG_LCSO_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_BASE_ADDR;

typedef volatile union _CAM_REG_LCSO_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_OFST_ADDR;

typedef volatile union _CAM_REG_LCSO_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_XSIZE;

typedef volatile union _CAM_REG_LCSO_D_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_YSIZE;

typedef volatile union _CAM_REG_LCSO_D_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_STRIDE;

typedef volatile union _CAM_REG_LCSO_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_CON;

typedef volatile union _CAM_REG_LCSO_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_CON2;

typedef volatile union _CAM_REG_AFO_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_BASE_ADDR;

typedef volatile union _CAM_REG_AFO_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_XSIZE;

typedef volatile union _CAM_REG_AFO_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_OFST_ADDR;

typedef volatile union _CAM_REG_AFO_D_YSIZE_
{
    volatile struct
    {
        FIELD RSV                       : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_YSIZE;

typedef volatile union _CAM_REG_AFO_D_STRIDE_
{
    volatile struct
    {
        FIELD RSV                       : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_STRIDE;

typedef volatile union _CAM_REG_AFO_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_CON;

typedef volatile union _CAM_REG_AFO_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_CON2;

typedef volatile union _CAM_REG_AAO_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_BASE_ADDR;

typedef volatile union _CAM_REG_AAO_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_OFST_ADDR;

typedef volatile union _CAM_REG_AAO_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 17;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_XSIZE;

typedef volatile union _CAM_REG_AAO_D_YSIZE_
{
    volatile struct
    {
        FIELD RSV                       : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_YSIZE;

typedef volatile union _CAM_REG_AAO_D_STRIDE_
{
    volatile struct
    {
        FIELD RSV                       : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 6;
        FIELD BUS_SIZE_EN               : 1;
        FIELD rsv_25                    : 7;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_STRIDE;

typedef volatile union _CAM_REG_AAO_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_CON;

typedef volatile union _CAM_REG_AAO_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_CON2;

typedef volatile union _CAM_REG_DMA_ERR_CTRL_
{
    volatile struct
    {
        FIELD rsv_0                     : 1;
        FIELD IMGI_ERR                  : 1;
        FIELD BPCI_ERR                  : 1;
        FIELD rsv_3                     : 1;
        FIELD LSCI_ERR                  : 1;
        FIELD UFDI_ERR                  : 1;
        FIELD LCEI_ERR                  : 1;
        FIELD VIPI_ERR                  : 1;
        FIELD VIP2I_ERR                 : 1;
        FIELD VIP3I_ERR                 : 1;
        FIELD rsv_10                    : 2;
        FIELD IMGO_ERR                  : 1;
        FIELD IMG2O_ERR                 : 1;
        FIELD LCSO_ERR                  : 1;
        FIELD ESFKO_ERR                 : 1;
        FIELD AAO_ERR                   : 1;
        FIELD UFEO_ERR                  : 1;
        FIELD MFBO_ERR                  : 1;
        FIELD IMG3BO_ERR                : 1;
        FIELD IMG3CO_ERR                : 1;
        FIELD RRZO_ERR                  : 1;
        FIELD IMG3O_ERR                 : 1;
        FIELD FEO_ERR                   : 1;
        FIELD BPCI_D_ERR                : 1;
        FIELD LSCI_D_ERR                : 1;
        FIELD IMGO_D_ERR                : 1;
        FIELD RRZO_D_ERR                : 1;
        FIELD LCSO_D_ERR                : 1;
        FIELD AFO_D_ERR                 : 1;
        FIELD AAO_D_ERR                 : 1;
        FIELD ERR_CLR_MD                : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_ERR_CTRL;

typedef volatile union _CAM_REG_IMGI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGI_ERR_STAT;

typedef volatile union _CAM_REG_BPCI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_ERR_STAT;

typedef volatile union _CAM_REG_LSCI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_ERR_STAT;

typedef volatile union _CAM_REG_UFDI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFDI_ERR_STAT;

typedef volatile union _CAM_REG_LCEI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCEI_ERR_STAT;

typedef volatile union _CAM_REG_VIPI_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIPI_ERR_STAT;

typedef volatile union _CAM_REG_VIP2I_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP2I_ERR_STAT;

typedef volatile union _CAM_REG_VIP3I_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_VIP3I_ERR_STAT;

typedef volatile union _CAM_REG_IMGO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_ERR_STAT;

typedef volatile union _CAM_REG_RRZO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_ERR_STAT;

typedef volatile union _CAM_REG_LCSO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_ERR_STAT;

typedef volatile union _CAM_REG_ESFKO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_ESFKO_ERR_STAT;

typedef volatile union _CAM_REG_AAO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_ERR_STAT;

typedef volatile union _CAM_REG_UFEO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_UFEO_ERR_STAT;

typedef volatile union _CAM_REG_MFBO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_MFBO_ERR_STAT;

typedef volatile union _CAM_REG_IMG3BO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3BO_ERR_STAT;

typedef volatile union _CAM_REG_IMG3CO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3CO_ERR_STAT;

typedef volatile union _CAM_REG_IMG2O_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG2O_ERR_STAT;

typedef volatile union _CAM_REG_IMG3O_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMG3O_ERR_STAT;

typedef volatile union _CAM_REG_FEO_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_FEO_ERR_STAT;

typedef volatile union _CAM_REG_BPCI_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_BPCI_D_ERR_STAT;

typedef volatile union _CAM_REG_LSCI_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LSCI_D_ERR_STAT;

typedef volatile union _CAM_REG_IMGO_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_IMGO_D_ERR_STAT;

typedef volatile union _CAM_REG_RRZO_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_RRZO_D_ERR_STAT;

typedef volatile union _CAM_REG_LCSO_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_LCSO_D_ERR_STAT;

typedef volatile union _CAM_REG_AFO_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_AFO_D_ERR_STAT;

typedef volatile union _CAM_REG_AAO_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_AAO_D_ERR_STAT;

typedef volatile union _CAM_REG_DMA_DEBUG_ADDR_
{
    volatile struct
    {
        FIELD DEBUG_ADDR                : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_DEBUG_ADDR;

typedef volatile union _CAM_REG_DMA_RSV1_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV1;

typedef volatile union _CAM_REG_DMA_RSV2_
{
    volatile struct
    {
        //FIELD RSV                       : 32;
        FIELD CROP_IN_W                 : 16;
        FIELD CROP_IN_H                 : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV2;

typedef volatile union _CAM_REG_DMA_RSV3_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV3;

typedef volatile union _CAM_REG_DMA_RSV4_
{
    volatile struct
    {
        //FIELD RSV                       : 32;
        FIELD CROP_IN_W                 : 16;
        FIELD CROP_IN_H                 : 16;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV4;

typedef volatile union _CAM_REG_DMA_RSV5_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV5;

typedef volatile union _CAM_REG_DMA_RSV6_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_RSV6;

typedef volatile union _CAM_REG_DMA_DEBUG_SEL_
{
    volatile struct
    {
        FIELD DMA_TOP_SEL               : 8;
        FIELD R_W_DMA_TOP_SEL           : 8;
        FIELD SUB_MODULE_SEL            : 8;
        FIELD rsv_24                    : 2;
        FIELD IMGI_ADDR_GEN_WAIT        : 1;
        FIELD IMGI_UFD_FIFO_FULL_OFFSET : 1;
        FIELD IMGI_UFD_FIFO_FULL_XSIZE  : 1;
        FIELD IMGO_UFE_FIFO_FULL_XSIZE  : 1;
        FIELD ARBITER_BVALID_FULL       : 1;
        FIELD ARBITER_COM_FULL          : 1;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_DEBUG_SEL;

typedef volatile union _CAM_REG_DMA_BW_SELF_TEST_
{
    volatile struct
    {
        FIELD BW_SELF_TEST_EN_IMGO      : 1;
        FIELD BW_SELF_TEST_EN_IMGO_D    : 1;
        FIELD BW_SELF_TEST_EN_UFEO      : 1;
        FIELD BW_SELF_TEST_EN_RRZO      : 1;
        FIELD BW_SELF_TEST_EN_RRZO_D    : 1;
        FIELD BW_SELF_TEST_EN_IMG3O     : 1;
        FIELD BW_SELF_TEST_EN_IMG3BO    : 1;
        FIELD BW_SELF_TEST_EN_IMG3CO    : 1;
        FIELD BW_SELF_TEST_EN_AAO       : 1;
        FIELD BW_SELF_TEST_EN_AAO_D     : 1;
        FIELD BW_SELF_TEST_EN_LCSO      : 1;
        FIELD BW_SELF_TEST_EN_LCSO_D    : 1;
        FIELD BW_SELF_TEST_EN_AFO_D     : 1;
        FIELD BW_SELF_TEST_EN_IMG2O     : 1;
        FIELD BW_SELF_TEST_EN_MFBO      : 1;
        FIELD BW_SELF_TEST_EN_FEO       : 1;
        FIELD BW_SELF_TEST_EN_ESFKO     : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} CAM_REG_DMA_BW_SELF_TEST;

/* end MT6593_100_dma.xml*/

/* start MT6593_camsv_csr.xml*/
typedef volatile union _CAMSV_REG_CAMSV_MODULE_EN_
{
    volatile struct
    {
        FIELD TG_EN                     : 1;
        FIELD rsv_1                     : 1;
        FIELD PAK_EN                    : 1;
        FIELD PAK_SEL                   : 1;
        FIELD IMGO_EN_                  : 1;
        FIELD rsv_5                     : 25;
        FIELD DB_EN                     : 1;
        FIELD DB_LOCK                   : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_MODULE_EN;

typedef volatile union _CAMSV_REG_CAMSV_FMT_SEL_
{
    volatile struct
    {
        FIELD TG1_FMT                   : 3;
        FIELD rsv_3                     : 1;
        FIELD TG1_SW                    : 2;
        FIELD rsv_6                     : 10;
        FIELD IMGO_FORMAT               : 2;
        FIELD rsv_18                    : 6;
        FIELD IMGO_BUS_SIZE             : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_FMT_SEL;

typedef volatile union _CAMSV_REG_CAMSV_INT_EN_
{
    volatile struct
    {
        FIELD VS1_INT_EN                : 1;
        FIELD TG_INT1_EN                : 1;
        FIELD TG_INT2_EN                : 1;
        FIELD EXPDON1_INT_EN            : 1;
        FIELD TG_ERR_INT_EN             : 1;
        FIELD TG_GBERR_INT_EN           : 1;
        FIELD TG_DROP_INT_EN            : 1;
        FIELD TG_SOF_INT_EN             : 1;
        FIELD rsv_8                     : 2;
        FIELD PASS1_DON_INT_EN          : 1;
        FIELD rsv_11                    : 5;
        FIELD IMGO_ERR_INT_EN           : 1;
        FIELD IMGO_OVERR_INT_EN         : 1;
        FIELD rsv_18                    : 1;
        FIELD IMGO_DROP_INT_EN          : 1;
        FIELD rsv_20                    : 11;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_INT_EN;

typedef volatile union _CAMSV_REG_CAMSV_INT_STATUS_
{
    volatile struct
    {
        FIELD VS1_ST                    : 1;
        FIELD TG_ST1                    : 1;
        FIELD TG_ST2                    : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG_ERR_ST                 : 1;
        FIELD TG_GBERR_ST               : 1;
        FIELD TG_DROP_INT_ST            : 1;
        FIELD TG_SOF1_INT_ST            : 1;
        FIELD rsv_8                     : 2;
        FIELD PASS1_DON_ST              : 1;
        FIELD rsv_11                    : 5;
        FIELD IMGO_ERR_ST               : 1;
        FIELD IMGO_OVERR_ST             : 1;
        FIELD rsv_18                    : 1;
        FIELD IMGO_DROP_ST              : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_INT_STATUS;

typedef volatile union _CAMSV_REG_CAMSV_SW_CTL_
{
    volatile struct
    {
        FIELD IMGO_RST_TRIG             : 1;
        FIELD IMGO_RST_ST               : 1;
        FIELD SW_RST                    : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_SW_CTL;

typedef volatile union _CAMSV_REG_CAMSV_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_SPARE0;

typedef volatile union _CAMSV_REG_CAMSV_SPARE1_
{
    volatile struct
    {
        FIELD CTL_SPARE1                : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_SPARE1;

typedef volatile union _CAMSV_REG_CAMSV_IMGO_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD rsv_4                     : 7;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_IMGO_FBC;

typedef volatile union _CAMSV_REG_CAMSV_CLK_EN_
{
    volatile struct
    {
        FIELD TG_DP_CK_EN               : 1;
        FIELD rsv_1                     : 1;
        FIELD PAK_DP_CK_EN              : 1;
        FIELD rsv_3                     : 12;
        FIELD DMA_DP_CK_EN              : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_CLK_EN;

typedef volatile union _CAMSV_REG_CAMSV_DBG_SET_
{
    volatile struct
    {
        FIELD DEBUG_MOD_SEL             : 4;
        FIELD rsv_4                     : 4;
        FIELD DEBUG_SEL                 : 12;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_DBG_SET;

typedef volatile union _CAMSV_REG_CAMSV_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_DBG_PORT;

typedef volatile union _CAMSV_REG_CAMSV_DATE_CODE_
{
    volatile struct
    {
        FIELD CTL_DATE_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_DATE_CODE;

typedef volatile union _CAMSV_REG_CAMSV_PROJ_CODE_
{
    volatile struct
    {
        FIELD CTL_PROJ_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_PROJ_CODE;

typedef volatile union _CAMSV_REG_CAMSV_DCM_DIS_
{
    volatile struct
    {
        FIELD PAK_DCM_DIS               : 1;
        FIELD IMGO_DCM_DIS              : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_DCM_DIS;

typedef volatile union _CAMSV_REG_CAMSV_DCM_STATUS_
{
    volatile struct
    {
        FIELD PAK_DCM_ST                : 1;
        FIELD IMGO_DCM_ST               : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_DCM_STATUS;

typedef volatile union _CAMSV_REG_CAMSV_PAK_
{
    volatile struct
    {
        FIELD PAK_MODE                  : 2;
        FIELD PAK_DBL_MODE              : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV_PAK;

typedef volatile union _CAMSV_REG_CAMSV2_MODULE_EN_
{
    volatile struct
    {
        FIELD TG_EN                     : 1;
        FIELD rsv_1                     : 1;
        FIELD PAK_EN                    : 1;
        FIELD PAK_SEL                   : 1;
        FIELD IMGO_EN_                  : 1;
        FIELD rsv_5                     : 25;
        FIELD DB_EN                     : 1;
        FIELD DB_LOCK                   : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_MODULE_EN;

typedef volatile union _CAMSV_REG_CAMSV2_FMT_SEL_
{
    volatile struct
    {
        FIELD TG1_FMT                   : 3;
        FIELD rsv_3                     : 1;
        FIELD TG1_SW                    : 2;
        FIELD rsv_6                     : 10;
        FIELD IMGO_FORMAT               : 2;
        FIELD rsv_18                    : 6;
        FIELD IMGO_BUS_SIZE             : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_FMT_SEL;

typedef volatile union _CAMSV_REG_CAMSV2_INT_EN_
{
    volatile struct
    {
        FIELD VS1_INT_EN                : 1;
        FIELD TG_INT1_EN                : 1;
        FIELD TG_INT2_EN                : 1;
        FIELD EXPDON1_INT_EN            : 1;
        FIELD TG_ERR_INT_EN             : 1;
        FIELD TG_GBERR_INT_EN           : 1;
        FIELD TG_DROP_INT_EN            : 1;
        FIELD TG_SOF_INT_EN             : 1;
        FIELD rsv_8                     : 2;
        FIELD PASS1_DON_INT_EN          : 1;
        FIELD rsv_11                    : 5;
        FIELD IMGO_ERR_INT_EN           : 1;
        FIELD IMGO_OVERR_INT_EN         : 1;
        FIELD rsv_18                    : 1;
        FIELD IMGO_DROP_INT_EN          : 1;
        FIELD rsv_20                    : 11;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_INT_EN;

typedef volatile union _CAMSV_REG_CAMSV2_INT_STATUS_
{
    volatile struct
    {
        FIELD VS1_ST                    : 1;
        FIELD TG_ST1                    : 1;
        FIELD TG_ST2                    : 1;
        FIELD EXPDON1_ST                : 1;
        FIELD TG_ERR_ST                 : 1;
        FIELD TG_GBERR_ST               : 1;
        FIELD TG_DROP_INT_ST            : 1;
        FIELD TG_SOF1_INT_ST            : 1;
        FIELD rsv_8                     : 2;
        FIELD PASS1_DON_ST              : 1;
        FIELD rsv_11                    : 5;
        FIELD IMGO_ERR_ST               : 1;
        FIELD IMGO_OVERR_ST             : 1;
        FIELD rsv_18                    : 1;
        FIELD IMGO_DROP_ST              : 1;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_INT_STATUS;

typedef volatile union _CAMSV_REG_CAMSV2_SW_CTL_
{
    volatile struct
    {
        FIELD IMGO_RST_TRIG             : 1;
        FIELD IMGO_RST_ST               : 1;
        FIELD SW_RST                    : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_SW_CTL;

typedef volatile union _CAMSV_REG_CAMSV2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_SPARE0;

typedef volatile union _CAMSV_REG_CAMSV2_SPARE1_
{
    volatile struct
    {
        FIELD CTL_SPARE1                : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_SPARE1;

typedef volatile union _CAMSV_REG_CAMSV2_IMGO_FBC_
{
    volatile struct
    {
        FIELD FBC_CNT                   : 4;
        FIELD rsv_4                     : 7;
        FIELD RCNT_INC                  : 1;
        FIELD rsv_12                    : 2;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_IMGO_FBC;

typedef volatile union _CAMSV_REG_CAMSV2_CLK_EN_
{
    volatile struct
    {
        FIELD TG_DP_CK_EN               : 1;
        FIELD rsv_1                     : 1;
        FIELD PAK_DP_CK_EN              : 1;
        FIELD rsv_3                     : 12;
        FIELD DMA_DP_CK_EN              : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_CLK_EN;

typedef volatile union _CAMSV_REG_CAMSV2_DBG_SET_
{
    volatile struct
    {
        FIELD DEBUG_MOD_SEL             : 4;
        FIELD rsv_4                     : 4;
        FIELD DEBUG_SEL                 : 12;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_DBG_SET;

typedef volatile union _CAMSV_REG_CAMSV2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_DBG_PORT;

typedef volatile union _CAMSV_REG_CAMSV2_DATE_CODE_
{
    volatile struct
    {
        FIELD CTL_DATE_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_DATE_CODE;

typedef volatile union _CAMSV_REG_CAMSV2_PROJ_CODE_
{
    volatile struct
    {
        FIELD CTL_PROJ_CODE             : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_PROJ_CODE;

typedef volatile union _CAMSV_REG_CAMSV2_DCM_DIS_
{
    volatile struct
    {
        FIELD PAK_DCM_DIS               : 1;
        FIELD IMGO_DCM_DIS              : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_DCM_DIS;

typedef volatile union _CAMSV_REG_CAMSV2_DCM_STATUS_
{
    volatile struct
    {
        FIELD PAK_DCM_ST                : 1;
        FIELD IMGO_DCM_ST               : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_DCM_STATUS;

typedef volatile union _CAMSV_REG_CAMSV2_PAK_
{
    volatile struct
    {
        FIELD PAK_MODE                  : 2;
        FIELD PAK_DBL_MODE              : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_CAMSV2_PAK;

/* end MT6593_camsv_csr.xml*/

/* start MT6593_dma_sv.xml*/
typedef volatile union _CAMSV_REG_DMA_SOFT_RSTSTAT_SV_
{
    volatile struct
    {
        FIELD IMGO_SV_SOFT_RST_STAT     : 1;
        FIELD IMGO_SV_D_SOFT_RST_STAT   : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_SOFT_RSTSTAT_SV;

typedef volatile union _CAMSV_REG_LAST_ULTRA_EN_SV_
{
    volatile struct
    {
        FIELD IMGO_SV_LAST_ULTRA_EN     : 1;
        FIELD IMGO_SV_D_LAST_ULTRA_EN   : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_LAST_ULTRA_EN_SV;

typedef volatile union _CAMSV_REG_IMGO_SV_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_BASE_ADDR;

typedef volatile union _CAMSV_REG_IMGO_SV_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_OFST_ADDR;

typedef volatile union _CAMSV_REG_IMGO_SV_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_XSIZE;

typedef volatile union _CAMSV_REG_IMGO_SV_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_YSIZE;

typedef volatile union _CAMSV_REG_IMGO_SV_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_STRIDE;

typedef volatile union _CAMSV_REG_IMGO_SV_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD MAX_BURST_LEN             : 5;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_CON;

typedef volatile union _CAMSV_REG_IMGO_SV_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_CON2;

typedef volatile union _CAMSV_REG_IMGO_SV_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_CROP;

typedef volatile union _CAMSV_REG_IMGO_SV_D_BASE_ADDR_
{
    volatile struct
    {
        FIELD BASE_ADDR                 : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_BASE_ADDR;

typedef volatile union _CAMSV_REG_IMGO_SV_D_OFST_ADDR_
{
    volatile struct
    {
        FIELD OFFSET_ADDR               : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_OFST_ADDR;

typedef volatile union _CAMSV_REG_IMGO_SV_D_XSIZE_
{
    volatile struct
    {
        FIELD XSIZE                     : 14;
        FIELD rsv_14                    : 18;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_XSIZE;

typedef volatile union _CAMSV_REG_IMGO_SV_D_YSIZE_
{
    volatile struct
    {
        FIELD YSIZE                     : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_YSIZE;

typedef volatile union _CAMSV_REG_IMGO_SV_D_STRIDE_
{
    volatile struct
    {
        FIELD STRIDE                    : 14;
        FIELD rsv_14                    : 2;
        FIELD BUS_SIZE                  : 2;
        FIELD rsv_18                    : 1;
        FIELD BUS_SIZE_EN               : 1;
        FIELD FORMAT                    : 2;
        FIELD rsv_22                    : 1;
        FIELD FORMAT_EN                 : 1;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_STRIDE;

typedef volatile union _CAMSV_REG_IMGO_SV_D_CON_
{
    volatile struct
    {
        FIELD FIFO_SIZE                 : 8;
        FIELD FIFO_PRI_THRL             : 8;
        FIELD FIFO_PRI_THRH             : 8;
        FIELD FIFO_SIZE_MSB             : 1;
        FIELD FIFO_PRI_THRL_MSB         : 1;
        FIELD FIFO_PRI_THRH_MSB         : 1;
        FIELD rsv_27                    : 1;
        FIELD MAX_BURST_LEN             : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_CON;

typedef volatile union _CAMSV_REG_IMGO_SV_D_CON2_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD FIFO_PRE_PRI_THRL         : 8;
        FIELD FIFO_PRE_PRI_THRH         : 8;
        FIELD FIFO_PRE_PRI_THRL_MSB     : 1;
        FIELD FIFO_PRE_PRI_THRH_MSB     : 1;
        FIELD rsv_26                    : 6;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_CON2;

typedef volatile union _CAMSV_REG_IMGO_SV_D_CROP_
{
    volatile struct
    {
        FIELD XOFFSET                   : 14;
        FIELD rsv_14                    : 2;
        FIELD YOFFSET                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_CROP;

typedef volatile union _CAMSV_REG_DMA_ERR_CTRL_SV_
{
    volatile struct
    {
        FIELD IMGO_SV_ERR               : 1;
        FIELD IMGO_SV_D_ERR             : 1;
        FIELD rsv_2                     : 29;
        FIELD ERR_CLR_MD_SV             : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_ERR_CTRL_SV;

typedef volatile union _CAMSV_REG_IMGO_SV_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_ERR_STAT;

typedef volatile union _CAMSV_REG_IMGO_SV_D_ERR_STAT_
{
    volatile struct
    {
        FIELD ERR_STAT                  : 16;
        FIELD ERR_EN                    : 16;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_IMGO_SV_D_ERR_STAT;

typedef volatile union _CAMSV_REG_DMA_DEBUG_ADDR_SV_
{
    volatile struct
    {
        FIELD DEBUG_ADDR_SV             : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_DEBUG_ADDR_SV;

typedef volatile union _CAMSV_REG_DMA_RSV1_SV_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_RSV1_SV;

typedef volatile union _CAMSV_REG_DMA_RSV2_SV_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_RSV2_SV;

typedef volatile union _CAMSV_REG_DMA_RSV3_SV_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_RSV3_SV;

typedef volatile union _CAMSV_REG_DMA_RSV4_SV_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_RSV4_SV;

typedef volatile union _CAMSV_REG_DMA_RSV5_SV_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_RSV5_SV;

typedef volatile union _CAMSV_REG_DMA_RSV6_SV_
{
    volatile struct
    {
        FIELD RSV                       : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_DMA_RSV6_SV;

/* end MT6593_dma_sv.xml*/

/* start MT6593_camsv_tg.xml*/
typedef volatile union _CAMSV_REG_TG_SEN_MODE_
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_SEN_MODE;

typedef volatile union _CAMSV_REG_TG_VF_CON_
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_VF_CON;

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_PXL_
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_SEN_GRAB_PXL;

typedef volatile union _CAMSV_REG_TG_SEN_GRAB_LIN_
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_SEN_GRAB_LIN;

typedef volatile union _CAMSV_REG_TG_PATH_CFG_
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_PATH_CFG;

typedef volatile union _CAMSV_REG_TG_MEMIN_CTL_
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_MEMIN_CTL;

typedef volatile union _CAMSV_REG_TG_INT1_
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_INT1;

typedef volatile union _CAMSV_REG_TG_INT2_
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_INT2;

typedef volatile union _CAMSV_REG_TG_SOF_CNT_
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_SOF_CNT;

typedef volatile union _CAMSV_REG_TG_SOT_CNT_
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_SOT_CNT;

typedef volatile union _CAMSV_REG_TG_EOT_CNT_
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_EOT_CNT;

typedef volatile union _CAMSV_REG_TG_ERR_CTL_
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_ERR_CTL;

typedef volatile union _CAMSV_REG_TG_DAT_NO_
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_DAT_NO;

typedef volatile union _CAMSV_REG_TG_FRM_CNT_ST_
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FRM_CNT_ST;

typedef volatile union _CAMSV_REG_TG_FRMSIZE_ST_
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FRMSIZE_ST;

typedef volatile union _CAMSV_REG_TG_INTER_ST_
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_INTER_ST;

typedef volatile union _CAMSV_REG_TG_FLASHA_CTL_
{
    volatile struct
    {
        FIELD FLASHA_EN                 : 1;
        FIELD FLASH_EN                  : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHA_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHA_END_FRM            : 3;
        FIELD rsv_11                    : 1;
        FIELD FLASH_POL                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FLASHA_CTL;

typedef volatile union _CAMSV_REG_TG_FLASHA_LINE_CNT_
{
    volatile struct
    {
        FIELD FLASHA_LUNIT_NO           : 20;
        FIELD FLASHA_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FLASHA_LINE_CNT;

typedef volatile union _CAMSV_REG_TG_FLASHA_POS_
{
    volatile struct
    {
        FIELD FLASHA_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHA_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FLASHA_POS;

typedef volatile union _CAMSV_REG_TG_FLASHB_CTL_
{
    volatile struct
    {
        FIELD FLASHB_EN                 : 1;
        FIELD FLASHB_TRIG_SRC           : 1;
        FIELD rsv_2                     : 2;
        FIELD FLASHB_STARTPNT           : 2;
        FIELD rsv_6                     : 2;
        FIELD FLASHB_START_FRM          : 4;
        FIELD FLASHB_CONT_FRM           : 3;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FLASHB_CTL;

typedef volatile union _CAMSV_REG_TG_FLASHB_LINE_CNT_
{
    volatile struct
    {
        FIELD FLASHB_LUNIT_NO           : 20;
        FIELD FLASHB_LUNIT              : 4;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FLASHB_LINE_CNT;

typedef volatile union _CAMSV_REG_TG_FLASHB_POS_
{
    volatile struct
    {
        FIELD FLASHB_PXL                : 15;
        FIELD rsv_15                    : 1;
        FIELD FLASHB_LINE               : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FLASHB_POS;

typedef volatile union _CAMSV_REG_TG_FLASHB_POS1_
{
    volatile struct
    {
        FIELD FLASHB_CYC_CNT            : 20;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_FLASHB_POS1;

typedef volatile union _CAMSV_REG_TG_GSCTRL_CTL_
{
    volatile struct
    {
        FIELD GSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD GSCTRL_POL                : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_GSCTRL_CTL;

typedef volatile union _CAMSV_REG_TG_GSCTRL_TIME_
{
    volatile struct
    {
        FIELD GS_EPTIME                 : 23;
        FIELD rsv_23                    : 1;
        FIELD GSMS_TIMEU                : 4;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_GSCTRL_TIME;

typedef volatile union _CAMSV_REG_TG_MS_PHASE_
{
    volatile struct
    {
        FIELD MSCTRL_EN                 : 1;
        FIELD rsv_1                     : 3;
        FIELD MSCTRL_VSPOL              : 1;
        FIELD MSCTRL_OPEN_TRSRC         : 1;
        FIELD rsv_6                     : 2;
        FIELD MSCTRL_TRSRC              : 2;
        FIELD rsv_10                    : 6;
        FIELD MSCP1_PH0                 : 1;
        FIELD MSCP1_PH1                 : 1;
        FIELD MSCP1_PH2                 : 1;
        FIELD rsv_19                    : 1;
        FIELD MSOP1_PH0                 : 1;
        FIELD MSOP1_PH1                 : 1;
        FIELD rsv_22                    : 1;
        FIELD MSP1_RST                  : 1;
        FIELD MSCP2_PH0                 : 1;
        FIELD MSCP2_PH1                 : 1;
        FIELD MSCP2_PH2                 : 1;
        FIELD rsv_27                    : 1;
        FIELD MSOP2_PH0                 : 1;
        FIELD MSOP2_PH1                 : 1;
        FIELD rsv_30                    : 1;
        FIELD MSP2_RST                  : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_MS_PHASE;

typedef volatile union _CAMSV_REG_TG_MS_CL_TIME_
{
    volatile struct
    {
        FIELD MS_TCLOSE                 : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_MS_CL_TIME;

typedef volatile union _CAMSV_REG_TG_MS_OP_TIME_
{
    volatile struct
    {
        FIELD MS_TOPEN                  : 23;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_MS_OP_TIME;

typedef volatile union _CAMSV_REG_TG_MS_CLPH_TIME_
{
    volatile struct
    {
        FIELD MS_CL_T1                  : 16;
        FIELD MS_CL_T2                  : 16;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_MS_CLPH_TIME;

typedef volatile union _CAMSV_REG_TG_MS_OPPH_TIME_
{
    volatile struct
    {
        FIELD MS_OP_T3                  : 16;
        FIELD MS_OP_T4                  : 16;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG_MS_OPPH_TIME;

typedef volatile union _CAMSV_REG_TG2_SEN_MODE_
{
    volatile struct
    {
        FIELD CMOS_EN                   : 1;
        FIELD DBL_DATA_BUS              : 1;
        FIELD SOT_MODE                  : 1;
        FIELD SOT_CLR_MODE              : 1;
        FIELD rsv_4                     : 4;
        FIELD SOF_SRC                   : 2;
        FIELD EOF_SRC                   : 2;
        FIELD PXL_CNT_RST_SRC           : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_SEN_MODE;

typedef volatile union _CAMSV_REG_TG2_VF_CON_
{
    volatile struct
    {
        FIELD VFDATA_EN                 : 1;
        FIELD SINGLE_MODE               : 1;
        FIELD rsv_2                     : 2;
        FIELD FR_CON                    : 3;
        FIELD rsv_7                     : 1;
        FIELD SP_DELAY                  : 3;
        FIELD rsv_11                    : 1;
        FIELD SPDELAY_MODE              : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_VF_CON;

typedef volatile union _CAMSV_REG_TG2_SEN_GRAB_PXL_
{
    volatile struct
    {
        FIELD PXL_S                     : 15;
        FIELD rsv_15                    : 1;
        FIELD PXL_E                     : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_SEN_GRAB_PXL;

typedef volatile union _CAMSV_REG_TG2_SEN_GRAB_LIN_
{
    volatile struct
    {
        FIELD LIN_S                     : 13;
        FIELD rsv_13                    : 3;
        FIELD LIN_E                     : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_SEN_GRAB_LIN;

typedef volatile union _CAMSV_REG_TG2_PATH_CFG_
{
    volatile struct
    {
        FIELD SEN_IN_LSB                : 2;
        FIELD rsv_2                     : 2;
        FIELD JPGINF_EN                 : 1;
        FIELD MEMIN_EN                  : 1;
        FIELD rsv_6                     : 1;
        FIELD JPG_LINEND_EN             : 1;
        FIELD DB_LOAD_DIS               : 1;
        FIELD DB_LOAD_SRC               : 1;
        FIELD DB_LOAD_VSPOL             : 1;
        FIELD RCNT_INC                  : 1;
        FIELD YUV_U2S_DIS               : 1;
        FIELD YUV_BIN_EN                : 1;
        FIELD FBC_EN                    : 1;
        FIELD LOCK_EN                   : 1;
        FIELD FB_NUM                    : 4;
        FIELD RCNT                      : 4;
        FIELD WCNT                      : 4;
        FIELD DROP_CNT                  : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_PATH_CFG;

typedef volatile union _CAMSV_REG_TG2_MEMIN_CTL_
{
    volatile struct
    {
        FIELD MEMIN_DUMMYPXL            : 8;
        FIELD MEMIN_DUMMYLIN            : 5;
        FIELD rsv_13                    : 3;
        FIELD FBC_CNT                   : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_MEMIN_CTL;

typedef volatile union _CAMSV_REG_TG2_INT1_
{
    volatile struct
    {
        FIELD TG_INT1_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT1_PXLNO             : 15;
        FIELD VSYNC_INT_POL             : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_INT1;

typedef volatile union _CAMSV_REG_TG2_INT2_
{
    volatile struct
    {
        FIELD TG_INT2_LINENO            : 13;
        FIELD rsv_13                    : 3;
        FIELD TG_INT2_PXLNO             : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_INT2;

typedef volatile union _CAMSV_REG_TG2_SOF_CNT_
{
    volatile struct
    {
        FIELD SOF_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_SOF_CNT;

typedef volatile union _CAMSV_REG_TG2_SOT_CNT_
{
    volatile struct
    {
        FIELD SOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_SOT_CNT;

typedef volatile union _CAMSV_REG_TG2_EOT_CNT_
{
    volatile struct
    {
        FIELD EOT_CNT                   : 28;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_EOT_CNT;

typedef volatile union _CAMSV_REG_TG2_ERR_CTL_
{
    volatile struct
    {
        FIELD GRAB_ERR_FLIMIT_NO        : 4;
        FIELD GRAB_ERR_FLIMIT_EN        : 1;
        FIELD GRAB_ERR_EN               : 1;
        FIELD rsv_6                     : 2;
        FIELD REZ_OVRUN_FLIMIT_NO       : 4;
        FIELD REZ_OVRUN_FLIMIT_EN       : 1;
        FIELD rsv_13                    : 3;
        FIELD DBG_SRC_SEL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_ERR_CTL;

typedef volatile union _CAMSV_REG_TG2_DAT_NO_
{
    volatile struct
    {
        FIELD DAT_NO                    : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_DAT_NO;

typedef volatile union _CAMSV_REG_TG2_FRM_CNT_ST_
{
    volatile struct
    {
        FIELD REZ_OVRUN_FCNT            : 4;
        FIELD rsv_4                     : 4;
        FIELD GRAB_ERR_FCNT             : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_FRM_CNT_ST;

typedef volatile union _CAMSV_REG_TG2_FRMSIZE_ST_
{
    volatile struct
    {
        FIELD LINE_CNT                  : 13;
        FIELD rsv_13                    : 3;
        FIELD PXL_CNT                   : 15;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_FRMSIZE_ST;

typedef volatile union _CAMSV_REG_TG2_INTER_ST_
{
    volatile struct
    {
        FIELD SYN_VF_DATA_EN            : 1;
        FIELD OUT_RDY                   : 1;
        FIELD OUT_REQ                   : 1;
        FIELD rsv_3                     : 5;
        FIELD TG_CAM_CS                 : 6;
        FIELD rsv_14                    : 2;
        FIELD CAM_FRM_CNT               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} CAMSV_REG_TG2_INTER_ST;

/* end MT6593_camsv_tg.xml*/

/* start MT6593_camsv_top_csr.xml*/
typedef volatile union _CAMSV_TOP_REG_CAMSV_TOP_DEBUG_
{
    volatile struct
    {
        FIELD DEBUG                     : 32;
    } Bits;
    UINT32 Raw;
} CAMSV_TOP_REG_CAMSV_TOP_DEBUG;

/* end MT6593_camsv_top_csr.xml*/

/* manual add */
typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_SET_
{
    volatile struct
    {
        FIELD PASS1_DB_EN_SET               : 1;
        FIELD PASS1_D_DB_EN_SET             : 1;
        FIELD rsv_2                         : 1;
        FIELD APB_CLK_GATE_BYPASS_SET       : 1;
        FIELD rsv_4                         : 1;
        FIELD DUAL_MODE_SET                 : 1;
        FIELD BIN_SEL_SET                   : 1;
        FIELD BIN_SEL_EN_SET                : 1;
        FIELD PREGAIN_SEL_SET               : 1;
        FIELD PREGAIN_SEL_EN_SET            : 1;
        FIELD LSC_EARLY_SOF_EN_SET          : 1;
        FIELD W2G_MODE_SET                  : 1;
        FIELD rsv_12                        : 1;
        FIELD MFB_MODE_SET                  : 1;
        FIELD INT_MRG_SET                   : 1;
        FIELD BIN_SEL_D_SET                 : 1;
        FIELD BIN_SEL_EN_D_SET              : 1;
        FIELD PREGAIN_SEL_D_SET             : 1;
        FIELD PREGAIN_SEL_EN_D_SET          : 1;
        FIELD LSC_EARLY_SOF_EN_D_SET        : 1;
        FIELD W2G_MODE_D_SET                : 1;
        FIELD rsv_21                        : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_GLOBAL_SET;

typedef volatile union _CAM_REG_CTL_SEL_GLOBAL_CLR_
{
    volatile struct
    {
        FIELD PASS1_DB_EN_SET               : 1;
        FIELD PASS1_D_DB_EN_SET             : 1;
        FIELD rsv_2                         : 1;
        FIELD APB_CLK_GATE_BYPASS_SET       : 1;
        FIELD rsv_4                         : 1;
        FIELD DUAL_MODE_SET                 : 1;
        FIELD BIN_SEL_SET                   : 1;
        FIELD BIN_SEL_EN_SET                : 1;
        FIELD PREGAIN_SEL_SET               : 1;
        FIELD PREGAIN_SEL_EN_SET            : 1;
        FIELD LSC_EARLY_SOF_EN_SET          : 1;
        FIELD W2G_MODE_SET                  : 1;
        FIELD rsv_12                        : 1;
        FIELD MFB_MODE_SET                  : 1;
        FIELD INT_MRG_SET                   : 1;
        FIELD BIN_SEL_D_SET                 : 1;
        FIELD BIN_SEL_EN_D_SET              : 1;
        FIELD PREGAIN_SEL_D_SET             : 1;
        FIELD PREGAIN_SEL_EN_D_SET          : 1;
        FIELD LSC_EARLY_SOF_EN_D_SET        : 1;
        FIELD W2G_MODE_D_SET                : 1;
        FIELD rsv_21                        : 11;
    } Bits;
    UINT32 Raw;
} CAM_REG_CTL_SEL_GLOBAL_CLR;

/**/

// ----------------- CAM  Grouping Definitions -------------------
// ----------------- CAM Register Definition -------------------
typedef volatile struct _isp_reg_t_
{
     UINT32                          rsv_0000[4096];           // 0000..3FFC
     CAM_REG_CTL_START               CAM_CTL_START;                  //4000
     CAM_REG_CTL_EN_P1               CAM_CTL_EN_P1;                  //4004
     CAM_REG_CTL_EN_P1_DMA           CAM_CTL_EN_P1_DMA;              //4008
     UINT32                          rsv_400C;                       //400C
     CAM_REG_CTL_EN_P1_D             CAM_CTL_EN_P1_D;                //4010
     CAM_REG_CTL_EN_P1_DMA_D         CAM_CTL_EN_P1_DMA_D;            //4014
     CAM_REG_CTL_EN_P2               CAM_CTL_EN_P2;                  //4018
     CAM_REG_CTL_EN_P2_DMA           CAM_CTL_EN_P2_DMA;              //401C
     CAM_REG_CTL_CQ_EN               CAM_CTL_CQ_EN;                  //4020
     CAM_REG_CTL_SCENARIO            CAM_CTL_SCENARIO;               //4024
     CAM_REG_CTL_FMT_SEL_P1          CAM_CTL_FMT_SEL_P1;             //4028
     CAM_REG_CTL_FMT_SEL_P1_D        CAM_CTL_FMT_SEL_P1_D;           //402C
     CAM_REG_CTL_FMT_SEL_P2          CAM_CTL_FMT_SEL_P2;             //4030
     CAM_REG_CTL_SEL_P1              CAM_CTL_SEL_P1;                 //4034
     CAM_REG_CTL_SEL_P1_D            CAM_CTL_SEL_P1_D;               //4038
     CAM_REG_CTL_SEL_P2              CAM_CTL_SEL_P2;                 //403C
     CAM_REG_CTL_SEL_GLOBAL          CAM_CTL_SEL_GLOBAL;             //4040
     UINT32                          rsv_4044;                       //4044
     CAM_REG_CTL_INT_P1_EN           CAM_CTL_INT_P1_EN;              //4048
     CAM_REG_CTL_INT_P1_STATUS       CAM_CTL_INT_P1_STATUS;          //404C
     CAM_REG_CTL_INT_P1_EN2          CAM_CTL_INT_P1_EN2;             //4050
     CAM_REG_CTL_INT_P1_STATUS2      CAM_CTL_INT_P1_STATUS2;         //4054
     CAM_REG_CTL_INT_P1_EN_D         CAM_CTL_INT_P1_EN_D;            //4058
     CAM_REG_CTL_INT_P1_STATUS_D     CAM_CTL_INT_P1_STATUS_D;        //405C
     CAM_REG_CTL_INT_P1_EN2_D        CAM_CTL_INT_P1_EN2_D;           //4060
     CAM_REG_CTL_INT_P1_STATUS2_D    CAM_CTL_INT_P1_STATUS2_D;       //4064
     CAM_REG_CTL_INT_P2_EN           CAM_CTL_INT_P2_EN;              //4068
     CAM_REG_CTL_INT_P2_STATUS       CAM_CTL_INT_P2_STATUS;          //406C
     CAM_REG_CTL_INT_STATUSX         CAM_CTL_INT_STATUSX;            //4070
     CAM_REG_CTL_INT_STATUS2X        CAM_CTL_INT_STATUS2X;           //4074
     CAM_REG_CTL_INT_STATUS3X        CAM_CTL_INT_STATUS3X;           //4078
     CAM_REG_CTL_TILE                CAM_CTL_TILE;                   //407C
     CAM_REG_CTL_TDR_EN_STATUS       CAM_CTL_TDR_EN_STATUS;          //4080
     CAM_REG_CTL_TCM_EN              CAM_CTL_TCM_EN;                 //4084
     CAM_REG_CTL_TDR_DBG_STATUS      CAM_CTL_TDR_DBG_STATUS;         //4088
     CAM_REG_CTL_SW_CTL              CAM_CTL_SW_CTL;                 //408C
     CAM_REG_CTL_SPARE0              CAM_CTL_SPARE0;                 //4090
     CAM_REG_CTL_SPARE1              CAM_RRZ_OUT_W;                  //4094
     UINT32                          rsv_4098;                       //4098
     CAM_REG_CTL_SPARE2              CAM_RRZ_OUT_W_D;                //409C
     CAM_REG_CTL_CQ1_BASEADDR        CAM_CTL_CQ1_BASEADDR;           //40A0
     CAM_REG_CTL_CQ2_BASEADDR        CAM_CTL_CQ2_BASEADDR;           //40A4
     CAM_REG_CTL_CQ3_BASEADDR        CAM_CTL_CQ3_BASEADDR;           //40A8
     CAM_REG_CTL_CQ0_BASEADDR        CAM_CTL_CQ0_BASEADDR;           //40AC
     CAM_REG_CTL_CQ0B_BASEADDR       CAM_CTL_CQ0B_BASEADDR;          //40B0
     CAM_REG_CTL_CQ0C_BASEADDR       CAM_CTL_CQ0C_BASEADDR;          //40B4
     CAM_REG_CTL_CUR_CQ0_BASEADDR    CAM_CTL_CUR_CQ0_BASEADDR;       //40B8
     CAM_REG_CTL_CUR_CQ0B_BASEADDR   CAM_CTL_CUR_CQ0B_BASEADDR;      //40BC
     CAM_REG_CTL_CUR_CQ0C_BASEADDR   CAM_CTL_CUR_CQ0C_BASEADDR;      //40C0
     CAM_REG_CTL_CQ0_D_BASEADDR      CAM_CTL_CQ0_D_BASEADDR;         //40C4
     CAM_REG_CTL_CQ0B_D_BASEADDR     CAM_CTL_CQ0B_D_BASEADDR;        //40C8
     CAM_REG_CTL_CQ0C_D_BASEADDR     CAM_CTL_CQ0C_D_BASEADDR;        //40CC
     CAM_REG_CTL_CUR_CQ0_D_BASEADDR  CAM_CTL_CUR_CQ0_D_BASEADDR;     //40D0
     CAM_REG_CTL_CUR_CQ0B_D_BASEADDR CAM_CTL_CUR_CQ0B_D_BASEADDR;    //40D4
     CAM_REG_CTL_CUR_CQ0C_D_BASEADDR CAM_CTL_CUR_CQ0C_D_BASEADDR;    //40D8
     CAM_REG_CTL_DB_LOAD_D           CAM_CTL_DB_LOAD_D;              //40DC
     CAM_REG_CTL_DB_LOAD             CAM_CTL_DB_LOAD;                //40E0
     CAM_REG_CTL_P1_DONE_BYP         CAM_CTL_P1_DONE_BYP;            //40E4
     CAM_REG_CTL_P1_DONE_BYP_D       CAM_CTL_P1_DONE_BYP_D;          //40E8
     CAM_REG_CTL_P2_DONE_BYP         CAM_CTL_P2_DONE_BYP;            //40EC
     CAM_REG_CTL_IMGO_FBC            CAM_CTL_IMGO_FBC;               //40F0
     CAM_REG_CTL_RRZO_FBC            CAM_CTL_RRZO_FBC;               //40F4
     CAM_REG_CTL_IMGO_D_FBC          CAM_CTL_IMGO_D_FBC;             //40F8
     CAM_REG_CTL_RRZO_D_FBC          CAM_CTL_RRZO_D_FBC;             //40FC
     CAM_REG_CTL_SPARE3              CAM_CTL_SPARE3;                 //4100
     CAM_REG_CTL_IHDR                CAM_CTL_IHDR;                   //4104
     CAM_REG_CTL_IHDR_D              CAM_CTL_IHDR_D;                 //4108
     CAM_REG_CTL_CQ_EN_P2            CAM_CTL_CQ_EN_P2;               //410C
     CAM_REG_CTL_SEL_GLOBAL_P2       CAM_CTL_SEL_GLOBAL_P2;          //4110
     UINT32                          rsv_4114[19];                   //4114...415C
     CAM_REG_CTL_DBG_SET             CAM_CTL_DBG_SET;                //4160
     CAM_REG_CTL_DBG_PORT            CAM_CTL_DBG_PORT;               //4164
     CAM_REG_CTL_IMGI_CHECK          CAM_CTL_IMGI_CHECK;             //4168
     CAM_REG_CTL_IMGO_CHECK          CAM_CTL_IMGO_CHECK;             //416C
     CAM_REG_CTL_CLK_EN              CAM_CTL_CLK_EN;                 //4170
     UINT32                          rsv_4174[3];                    //4174...417C
     CAM_REG_CTL_DATE_CODE           CAM_CTL_DATE_CODE;              //4180
     CAM_REG_CTL_PROJ_CODE           CAM_CTL_PROJ_CODE;              //4184
     CAM_REG_CTL_RAW_DCM_DIS         CAM_CTL_RAW_DCM_DIS;            //4188
     CAM_REG_CTL_RAW_D_DCM_DIS       CAM_CTL_RAW_D_DCM_DIS;          //418C
     CAM_REG_CTL_DMA_DCM_DIS         CAM_CTL_DMA_DCM_DIS;            //4190
     CAM_REG_CTL_RGB_DCM_DIS         CAM_CTL_RGB_DCM_DIS;            //4194
     CAM_REG_CTL_YUV_DCM_DIS         CAM_CTL_YUV_DCM_DIS;            //4198
     CAM_REG_CTL_TOP_DCM_DIS         CAM_CTL_TOP_DCM_DIS;            //419C
     CAM_REG_CTL_RAW_DCM_STATUS      CAM_CTL_RAW_DCM_STATUS;         //41A0
     CAM_REG_CTL_RAW_D_DCM_STATUS    CAM_CTL_RAW_D_DCM_STATUS;       //41A4
     CAM_REG_CTL_DMA_DCM_STATUS      CAM_CTL_DMA_DCM_STATUS;         //41A8
     CAM_REG_CTL_RGB_DCM_STATUS      CAM_CTL_RGB_DCM_STATUS;         //41AC
     CAM_REG_CTL_YUV_DCM_STATUS      CAM_CTL_YUV_DCM_STATUS;         //41B0
     CAM_REG_CTL_TOP_DCM_STATUS      CAM_CTL_TOP_DCM_STATUS;         //41B4
     CAM_REG_CTL_RAW_REQ_STATUS      CAM_CTL_RAW_REQ_STATUS;         //41B8
     CAM_REG_CTL_RAW_D_REQ_STATUS    CAM_CTL_RAW_D_REQ_STATUS;       //41BC
     CAM_REG_CTL_DMA_REQ_STATUS      CAM_CTL_DMA_REQ_STATUS;         //41C0
     CAM_REG_CTL_RGB_REQ_STATUS      CAM_CTL_RGB_REQ_STATUS;         //41C4
     CAM_REG_CTL_YUV_REQ_STATUS      CAM_CTL_YUV_REQ_STATUS;         //41C8
     CAM_REG_CTL_RAW_RDY_STATUS      CAM_CTL_RAW_RDY_STATUS;         //41CC
     CAM_REG_CTL_RAW_D_RDY_STATUS    CAM_CTL_RAW_D_RDY_STATUS;       //41D0
     CAM_REG_CTL_DMA_RDY_STATUS      CAM_CTL_DMA_RDY_STATUS;         //41D4
     CAM_REG_CTL_RGB_RDY_STATUS      CAM_CTL_RGB_RDY_STATUS;         //41D8
     CAM_REG_CTL_YUV_RDY_STATUS      CAM_CTL_YUV_RDY_STATUS;         //41DC
     UINT32                          rsv_41E0[140];                  //41E0...440C
     CAM_REG_TG_SEN_MODE             CAM_TG_SEN_MODE;                //4410
     CAM_REG_TG_VF_CON               CAM_TG_VF_CON;                  //4414
     CAM_REG_TG_SEN_GRAB_PXL         CAM_TG_SEN_GRAB_PXL;            //4418
     CAM_REG_TG_SEN_GRAB_LIN         CAM_TG_SEN_GRAB_LIN;            //441C
     CAM_REG_TG_PATH_CFG             CAM_TG_PATH_CFG;                //4420
     CAM_REG_TG_MEMIN_CTL            CAM_TG_MEMIN_CTL;               //4424
     CAM_REG_TG_INT1                 CAM_TG_INT1;                    //4428
     CAM_REG_TG_INT2                 CAM_TG_INT2;                    //442C
     CAM_REG_TG_SOF_CNT              CAM_TG_SOF_CNT;                 //4430
     CAM_REG_TG_SOT_CNT              CAM_TG_SOT_CNT;                 //4434
     CAM_REG_TG_EOT_CNT              CAM_TG_EOT_CNT;                 //4438
     CAM_REG_TG_ERR_CTL              CAM_TG_ERR_CTL;                 //443C
     CAM_REG_TG_DAT_NO               CAM_TG_DAT_NO;                  //4440
     CAM_REG_TG_FRM_CNT_ST           CAM_TG_FRM_CNT_ST;              //4444
     CAM_REG_TG_FRMSIZE_ST           CAM_TG_FRMSIZE_ST;              //4448
     CAM_REG_TG_INTER_ST             CAM_TG_INTER_ST;                //444C
     UINT32                          rsv_4450[4];                    //4450...445C
     CAM_REG_TG_FLASHA_CTL           CAM_TG_FLASHA_CTL;              //4460
     CAM_REG_TG_FLASHA_LINE_CNT      CAM_TG_FLASHA_LINE_CNT;         //4464
     CAM_REG_TG_FLASHA_POS           CAM_TG_FLASHA_POS;              //4468
     CAM_REG_TG_FLASHB_CTL           CAM_TG_FLASHB_CTL;              //446C
     CAM_REG_TG_FLASHB_LINE_CNT      CAM_TG_FLASHB_LINE_CNT;         //4470
     CAM_REG_TG_FLASHB_POS           CAM_TG_FLASHB_POS;              //4474
     CAM_REG_TG_FLASHB_POS1          CAM_TG_FLASHB_POS1;             //4478
     CAM_REG_TG_GSCTRL_CTL           CAM_TG_GSCTRL_CTL;              //447C
     CAM_REG_TG_GSCTRL_TIME          CAM_TG_GSCTRL_TIME;             //4480
     CAM_REG_TG_MS_PHASE             CAM_TG_MS_PHASE;                //4484
     CAM_REG_TG_MS_CL_TIME           CAM_TG_MS_CL_TIME;              //4488
     CAM_REG_TG_MS_OP_TIME           CAM_TG_MS_OP_TIME;              //448C
     CAM_REG_TG_MS_CLPH_TIME         CAM_TG_MS_CLPH_TIME;            //4490
     CAM_REG_TG_MS_OPPH_TIME         CAM_TG_MS_OPPH_TIME;            //4494
     UINT32                          rsv_4498[22];                   //4498...44EC
     CAM_REG_HBN_SIZE                CAM_HBN_SIZE;                   //44F0
     CAM_REG_HBN_MODE                CAM_HBN_MODE;                   //44F4
     UINT32                          rsv_44F8[2];                    //44F8...44FC
     CAM_REG_OBC_OFFST0              CAM_OBC_OFFST0;                 //4500
     CAM_REG_OBC_OFFST1              CAM_OBC_OFFST1;                 //4504
     CAM_REG_OBC_OFFST2              CAM_OBC_OFFST2;                 //4508
     CAM_REG_OBC_OFFST3              CAM_OBC_OFFST3;                 //450C
     CAM_REG_OBC_GAIN0               CAM_OBC_GAIN0;                  //4510
     CAM_REG_OBC_GAIN1               CAM_OBC_GAIN1;                  //4514
     CAM_REG_OBC_GAIN2               CAM_OBC_GAIN2;                  //4518
     CAM_REG_OBC_GAIN3               CAM_OBC_GAIN3;                  //451C
     UINT32                          rsv_4520[4];                    //4520...452C
     CAM_REG_LSC_CTL1                CAM_LSC_CTL1;                   //4530
     CAM_REG_LSC_CTL2                CAM_LSC_CTL2;                   //4534
     CAM_REG_LSC_CTL3                CAM_LSC_CTL3;                   //4538
     CAM_REG_LSC_LBLOCK              CAM_LSC_LBLOCK;                 //453C
     CAM_REG_LSC_RATIO               CAM_LSC_RATIO;                  //4540
     CAM_REG_LSC_TPIPE_OFST          CAM_LSC_TPIPE_OFST;             //4544
     CAM_REG_LSC_TPIPE_SIZE          CAM_LSC_TPIPE_SIZE;             //4548
     CAM_REG_LSC_GAIN_TH             CAM_LSC_GAIN_TH;                //454C
     CAM_REG_RPG_SATU_1              CAM_RPG_SATU_1;                 //4550
     CAM_REG_RPG_SATU_2              CAM_RPG_SATU_2;                 //4554
     CAM_REG_RPG_GAIN_1              CAM_RPG_GAIN_1;                 //4558
     CAM_REG_RPG_GAIN_2              CAM_RPG_GAIN_2;                 //455C
     CAM_REG_RPG_OFST_1              CAM_RPG_OFST_1;                 //4560
     CAM_REG_RPG_OFST_2              CAM_RPG_OFST_2;                 //4564
     UINT32                          rsv_4568[6];                    //4568...457C
     CAM_REG_SGG2_PGN                CAM_SGG2_PGN;                   //4580
     CAM_REG_SGG2_GMRC_1             CAM_SGG2_GMRC_1;                //4584
     CAM_REG_SGG2_GMRC_2             CAM_SGG2_GMRC_2;                //4588
     UINT32                          rsv_458C[9];                    //458C...45AC
     CAM_REG_AWB_WIN_ORG             CAM_AWB_WIN_ORG;                //45B0
     CAM_REG_AWB_WIN_SIZE            CAM_AWB_WIN_SIZE;               //45B4
     CAM_REG_AWB_WIN_PIT             CAM_AWB_WIN_PIT;                //45B8
     CAM_REG_AWB_WIN_NUM             CAM_AWB_WIN_NUM;                //45BC
     CAM_REG_AWB_GAIN1_0             CAM_AWB_GAIN1_0;                //45C0
     CAM_REG_AWB_GAIN1_1             CAM_AWB_GAIN1_1;                //45C4
     CAM_REG_AWB_LMT1_0              CAM_AWB_LMT1_0;                 //45C8
     CAM_REG_AWB_LMT1_1              CAM_AWB_LMT1_1;                 //45CC
     CAM_REG_AWB_LOW_THR             CAM_AWB_LOW_THR;                //45D0
     CAM_REG_AWB_HI_THR              CAM_AWB_HI_THR;                 //45D4
     CAM_REG_AWB_PIXEL_CNT0          CAM_AWB_PIXEL_CNT0;             //45D8
     CAM_REG_AWB_PIXEL_CNT1          CAM_AWB_PIXEL_CNT1;             //45DC
     CAM_REG_AWB_PIXEL_CNT2          CAM_AWB_PIXEL_CNT2;             //45E0
     CAM_REG_AWB_ERR_THR             CAM_AWB_ERR_THR;                //45E4
     CAM_REG_AWB_ROT                 CAM_AWB_ROT;                    //45E8
     CAM_REG_AWB_L0_X                CAM_AWB_L0_X;                   //45EC
     CAM_REG_AWB_L0_Y                CAM_AWB_L0_Y;                   //45F0
     CAM_REG_AWB_L1_X                CAM_AWB_L1_X;                   //45F4
     CAM_REG_AWB_L1_Y                CAM_AWB_L1_Y;                   //45F8
     CAM_REG_AWB_L2_X                CAM_AWB_L2_X;                   //45FC
     CAM_REG_AWB_L2_Y                CAM_AWB_L2_Y;                   //4600
     CAM_REG_AWB_L3_X                CAM_AWB_L3_X;                   //4604
     CAM_REG_AWB_L3_Y                CAM_AWB_L3_Y;                   //4608
     CAM_REG_AWB_L4_X                CAM_AWB_L4_X;                   //460C
     CAM_REG_AWB_L4_Y                CAM_AWB_L4_Y;                   //4610
     CAM_REG_AWB_L5_X                CAM_AWB_L5_X;                   //4614
     CAM_REG_AWB_L5_Y                CAM_AWB_L5_Y;                   //4618
     CAM_REG_AWB_L6_X                CAM_AWB_L6_X;                   //461C
     CAM_REG_AWB_L6_Y                CAM_AWB_L6_Y;                   //4620
     CAM_REG_AWB_L7_X                CAM_AWB_L7_X;                   //4624
     CAM_REG_AWB_L7_Y                CAM_AWB_L7_Y;                   //4628
     CAM_REG_AWB_L8_X                CAM_AWB_L8_X;                   //462C
     CAM_REG_AWB_L8_Y                CAM_AWB_L8_Y;                   //4630
     CAM_REG_AWB_L9_X                CAM_AWB_L9_X;                   //4634
     CAM_REG_AWB_L9_Y                CAM_AWB_L9_Y;                   //4638
     CAM_REG_AWB_SPARE               CAM_AWB_SPARE;                  //463C
     UINT32                          rsv_4640[4];                    //4640...464C
     CAM_REG_AE_HST_CTL              CAM_AE_HST_CTL;                 //4650
     CAM_REG_AE_GAIN2_0              CAM_AE_GAIN2_0;                 //4654
     CAM_REG_AE_GAIN2_1              CAM_AE_GAIN2_1;                 //4658
     CAM_REG_AE_LMT2_0               CAM_AE_LMT2_0;                  //465C
     CAM_REG_AE_LMT2_1               CAM_AE_LMT2_1;                  //4660
     CAM_REG_AE_RC_CNV_0             CAM_AE_RC_CNV_0;                //4664
     CAM_REG_AE_RC_CNV_1             CAM_AE_RC_CNV_1;                //4668
     CAM_REG_AE_RC_CNV_2             CAM_AE_RC_CNV_2;                //466C
     CAM_REG_AE_RC_CNV_3             CAM_AE_RC_CNV_3;                //4670
     CAM_REG_AE_RC_CNV_4             CAM_AE_RC_CNV_4;                //4674
     CAM_REG_AE_YGAMMA_0             CAM_AE_YGAMMA_0;                //4678
     CAM_REG_AE_YGAMMA_1             CAM_AE_YGAMMA_1;                //467C
     CAM_REG_AE_HST_SET              CAM_AE_HST_SET;                 //4680
     CAM_REG_AE_HST0_RNG             CAM_AE_HST0_RNG;                //4684
     CAM_REG_AE_HST1_RNG             CAM_AE_HST1_RNG;                //4688
     CAM_REG_AE_HST2_RNG             CAM_AE_HST2_RNG;                //468C
     CAM_REG_AE_HST3_RNG             CAM_AE_HST3_RNG;                //4690
     CAM_REG_AE_SPARE                CAM_AE_SPARE;                   //4694
     UINT32                          rsv_4698[2];                    //4698...469C
     CAM_REG_SGG1_PGN                CAM_SGG1_PGN;                   //46A0
     CAM_REG_SGG1_GMRC_1             CAM_SGG1_GMRC_1;                //46A4
     CAM_REG_SGG1_GMRC_2             CAM_SGG1_GMRC_2;                //46A8
     UINT32                          rsv_46AC;                       //46AC
     CAM_REG_AF_CON                  CAM_AF_CON;                     //46B0
     CAM_REG_AF_WINX_1               CAM_AF_WINX_1;                  //46B4
     CAM_REG_AF_WINX_2               CAM_AF_WINX_2;                  //46B8
     CAM_REG_AF_WINX_3               CAM_AF_WINX_3;                  //46BC
     CAM_REG_AF_WINY_1               CAM_AF_WINY_1;                  //46C0
     CAM_REG_AF_WINY_2               CAM_AF_WINY_2;                  //46C4
     CAM_REG_AF_WINY_3               CAM_AF_WINY_3;                  //46C8
     CAM_REG_AF_SIZE                 CAM_AF_SIZE;                    //46CC
     UINT32                          rsv_46D0;                       //46D0
     CAM_REG_AF_FLT_1                CAM_AF_FLT_1;                   //46D4
     CAM_REG_AF_FLT_2                CAM_AF_FLT_2;                   //46D8
     CAM_REG_AF_FLT_3                CAM_AF_FLT_3;                   //46DC
     CAM_REG_AF_TH                   CAM_AF_TH;                      //46E0
     CAM_REG_AF_FLO_WIN_1            CAM_AF_FLO_WIN_1;               //46E4
     CAM_REG_AF_FLO_SIZE_1           CAM_AF_FLO_SIZE_1;              //46E8
     CAM_REG_AF_FLO_WIN_2            CAM_AF_FLO_WIN_2;               //46EC
     CAM_REG_AF_FLO_SIZE_2           CAM_AF_FLO_SIZE_2;              //46F0
     CAM_REG_AF_FLO_WIN_3            CAM_AF_FLO_WIN_3;               //46F4
     CAM_REG_AF_FLO_SIZE_3           CAM_AF_FLO_SIZE_3;              //46F8
     CAM_REG_AF_FLO_TH               CAM_AF_FLO_TH;                  //46FC
     CAM_REG_AF_IMAGE_SIZE           CAM_AF_IMAGE_SIZE;              //4700
     CAM_REG_AF_FLT_4                CAM_AF_FLT_4;                   //4704
     CAM_REG_AF_FLT_5                CAM_AF_FLT_5;                   //4708
     CAM_REG_AF_STAT_L               CAM_AF_STAT_L;                  //470C
     CAM_REG_AF_STAT_M               CAM_AF_STAT_M;                  //4710
     CAM_REG_AF_FLO_STAT_1L          CAM_AF_FLO_STAT_1L;             //4714
     CAM_REG_AF_FLO_STAT_1M          CAM_AF_FLO_STAT_1M;             //4718
     CAM_REG_AF_FLO_STAT_1V          CAM_AF_FLO_STAT_1V;             //471C
     CAM_REG_AF_FLO_STAT_2L          CAM_AF_FLO_STAT_2L;             //4720
     CAM_REG_AF_FLO_STAT_2M          CAM_AF_FLO_STAT_2M;             //4724
     CAM_REG_AF_FLO_STAT_2V          CAM_AF_FLO_STAT_2V;             //4728
     CAM_REG_AF_FLO_STAT_3L          CAM_AF_FLO_STAT_3L;             //472C
     CAM_REG_AF_FLO_STAT_3M          CAM_AF_FLO_STAT_3M;             //4730
     CAM_REG_AF_FLO_STAT_3V          CAM_AF_FLO_STAT_3V;             //4734
     UINT32                          rsv_4738[10];                   //4738...475C
     CAM_REG_WBN_SIZE                CAM_WBN_SIZE;                   //4760
     CAM_REG_WBN_MODE                CAM_WBN_MODE;                   //4764
     UINT32                          rsv_4768[2];                    //4768...476C
     CAM_REG_FLK_CON                 CAM_FLK_CON;                    //4770
     CAM_REG_FLK_OFST                CAM_FLK_OFST;                   //4774
     CAM_REG_FLK_SIZE                CAM_FLK_SIZE;                   //4778
     CAM_REG_FLK_NUM                 CAM_FLK_NUM;                    //477C
     CAM_REG_LCS_CON                 CAM_LCS_CON;                    //4780
     CAM_REG_LCS_ST                  CAM_LCS_ST;                     //4784
     CAM_REG_LCS_AWS                 CAM_LCS_AWS;                    //4788
     CAM_REG_LCS_FLR                 CAM_LCS_FLR;                    //478C
     CAM_REG_LCS_LRZR_1              CAM_LCS_LRZR_1;                 //4790
     CAM_REG_LCS_LRZR_2              CAM_LCS_LRZR_2;                 //4794
     UINT32                          rsv_4798[2];                    //4798...479C
     CAM_REG_RRZ_CTL                 CAM_RRZ_CTL;                    //47A0
     CAM_REG_RRZ_IN_IMG              CAM_RRZ_IN_IMG;                 //47A4
     CAM_REG_RRZ_OUT_IMG             CAM_RRZ_OUT_IMG;                //47A8
     CAM_REG_RRZ_HORI_STEP           CAM_RRZ_HORI_STEP;              //47AC
     CAM_REG_RRZ_VERT_STEP           CAM_RRZ_VERT_STEP;              //47B0
     CAM_REG_RRZ_HORI_INT_OFST       CAM_RRZ_HORI_INT_OFST;          //47B4
     CAM_REG_RRZ_HORI_SUB_OFST       CAM_RRZ_HORI_SUB_OFST;          //47B8
     CAM_REG_RRZ_VERT_INT_OFST       CAM_RRZ_VERT_INT_OFST;          //47BC
     CAM_REG_RRZ_VERT_SUB_OFST       CAM_RRZ_VERT_SUB_OFST;          //47C0
     CAM_REG_RRZ_MODE_TH             CAM_RRZ_MODE_TH;                //47C4
     CAM_REG_RRZ_MODE_CTL            CAM_RRZ_MODE_CTL;               //47C8
     UINT32                          rsv_47CC[13];                   //47CC...47FC
     CAM_REG_BPC_CON                 CAM_BPC_CON;                    //4800
     CAM_REG_BPC_TH1                 CAM_BPC_TH1;                    //4804
     CAM_REG_BPC_TH2                 CAM_BPC_TH2;                    //4808
     CAM_REG_BPC_TH3                 CAM_BPC_TH3;                    //480C
     CAM_REG_BPC_TH4                 CAM_BPC_TH4;                    //4810
     CAM_REG_BPC_DTC                 CAM_BPC_DTC;                    //4814
     CAM_REG_BPC_COR                 CAM_BPC_COR;                    //4818
     CAM_REG_BPC_TBLI1               CAM_BPC_TBLI1;                  //481C
     CAM_REG_BPC_TBLI2               CAM_BPC_TBLI2;                  //4820
     CAM_REG_BPC_TH1_C               CAM_BPC_TH1_C;                  //4824
     CAM_REG_BPC_TH2_C               CAM_BPC_TH2_C;                  //4828
     CAM_REG_BPC_TH3_C               CAM_BPC_TH3_C;                  //482C
     CAM_REG_BPC_RMM1                CAM_BPC_RMM1;                   //4830
     CAM_REG_BPC_RMM2                CAM_BPC_RMM2;                   //4834
     CAM_REG_BPC_RMM_REVG_1          CAM_BPC_RMM_REVG_1;             //4838
     CAM_REG_BPC_RMM_REVG_2          CAM_BPC_RMM_REVG_2;             //483C
     CAM_REG_BPC_RMM_LEOS            CAM_BPC_RMM_LEOS;               //4840
     CAM_REG_BPC_RMM_GCNT            CAM_BPC_RMM_GCNT;               //4844
     UINT32                          rsv_4848[2];                    //4848...484C
     CAM_REG_NR1_CON                 CAM_NR1_CON;                    //4850
     CAM_REG_NR1_CT_CON              CAM_NR1_CT_CON;                 //4854
     CAM_REG_BNR_RSV1                CAM_BNR_RSV1;                   //4858
     CAM_REG_BNR_RSV2                CAM_BNR_RSV2;                   //485C
     UINT32                          rsv_4860[8];                    //4860...487C
     CAM_REG_PGN_SATU_1              CAM_PGN_SATU_1;                 //4880
     CAM_REG_PGN_SATU_2              CAM_PGN_SATU_2;                 //4884
     CAM_REG_PGN_GAIN_1              CAM_PGN_GAIN_1;                 //4888
     CAM_REG_PGN_GAIN_2              CAM_PGN_GAIN_2;                 //488C
     CAM_REG_PGN_OFST_1              CAM_PGN_OFST_1;                 //4890
     CAM_REG_PGN_OFST_2              CAM_PGN_OFST_2;                 //4894
     UINT32                          rsv_4898[2];                    //4898...489C
     CAM_REG_DM_O_BYP                CAM_DM_O_BYP;                   //48A0
     CAM_REG_DM_O_ED_FLAT            CAM_DM_O_ED_FLAT;               //48A4
     CAM_REG_DM_O_ED_NYQ             CAM_DM_O_ED_NYQ;                //48A8
     CAM_REG_DM_O_ED_STEP            CAM_DM_O_ED_STEP;               //48AC
     CAM_REG_DM_O_RGB_HF             CAM_DM_O_RGB_HF;                //48B0
     CAM_REG_DM_O_DOT                CAM_DM_O_DOT;                   //48B4
     CAM_REG_DM_O_F1_ACT             CAM_DM_O_F1_ACT;                //48B8
     CAM_REG_DM_O_F2_ACT             CAM_DM_O_F2_ACT;                //48BC
     CAM_REG_DM_O_F3_ACT             CAM_DM_O_F3_ACT;                //48C0
     CAM_REG_DM_O_F4_ACT             CAM_DM_O_F4_ACT;                //48C4
     CAM_REG_DM_O_F1_L               CAM_DM_O_F1_L;                  //48C8
     CAM_REG_DM_O_F2_L               CAM_DM_O_F2_L;                  //48CC
     CAM_REG_DM_O_F3_L               CAM_DM_O_F3_L;                  //48D0
     CAM_REG_DM_O_F4_L               CAM_DM_O_F4_L;                  //48D4
     CAM_REG_DM_O_HF_RB              CAM_DM_O_HF_RB;                 //48D8
     CAM_REG_DM_O_HF_GAIN            CAM_DM_O_HF_GAIN;               //48DC
     CAM_REG_DM_O_HF_COMP            CAM_DM_O_HF_COMP;               //48E0
     CAM_REG_DM_O_HF_CORIN_TH        CAM_DM_O_HF_CORIN_TH;           //48E4
     CAM_REG_DM_O_ACT_LUT            CAM_DM_O_ACT_LUT;               //48E8
     UINT32                          rsv_48EC;                       //48EC
     CAM_REG_DM_O_SPARE              CAM_DM_O_SPARE;                 //48F0
     CAM_REG_DM_O_BB                 CAM_DM_O_BB;                    //48F4
     UINT32                          rsv_48F8[6];                    //48F8...490C
     CAM_REG_CCL_GTC                 CAM_CCL_GTC;                    //4910
     CAM_REG_CCL_ADC                 CAM_CCL_ADC;                    //4914
     CAM_REG_CCL_BAC                 CAM_CCL_BAC;                    //4918
     UINT32                          rsv_491C;                       //491C
     CAM_REG_G2G_CNV_1               CAM_G2G_CNV_1;                  //4920
     CAM_REG_G2G_CNV_2               CAM_G2G_CNV_2;                  //4924
     CAM_REG_G2G_CNV_3               CAM_G2G_CNV_3;                  //4928
     CAM_REG_G2G_CNV_4               CAM_G2G_CNV_4;                  //492C
     CAM_REG_G2G_CNV_5               CAM_G2G_CNV_5;                  //4930
     CAM_REG_G2G_CNV_6               CAM_G2G_CNV_6;                  //4934
     CAM_REG_G2G_CTRL                CAM_G2G_CTRL;                   //4938
     UINT32                          rsv_493C[3];                    //493C...4944
     CAM_REG_UNP_OFST                CAM_UNP_OFST;                   //4948
     UINT32                          rsv_494C;                       //494C
     CAM_REG_C02_CON                 CAM_C02_CON;                    //4950
     CAM_REG_C02_CROP_CON1           CAM_C02_CROP_CON1;              //4954
     CAM_REG_C02_CROP_CON2           CAM_C02_CROP_CON2;              //4958
     UINT32                          rsv_495C;                       //495C
     CAM_REG_MFB_CON                 CAM_MFB_CON;                    //4960
     CAM_REG_MFB_LL_CON1             CAM_MFB_LL_CON1;                //4964
     CAM_REG_MFB_LL_CON2             CAM_MFB_LL_CON2;                //4968
     CAM_REG_MFB_LL_CON3             CAM_MFB_LL_CON3;                //496C
     CAM_REG_MFB_LL_CON4             CAM_MFB_LL_CON4;                //4970
     CAM_REG_MFB_LL_CON5             CAM_MFB_LL_CON5;                //4974
     CAM_REG_MFB_LL_CON6             CAM_MFB_LL_CON6;                //4978
     UINT32                          rsv_497C[17];                   //497C...49BC
     CAM_REG_LCE_CON                 CAM_LCE_CON;                    //49C0
     CAM_REG_LCE_ZR                  CAM_LCE_ZR;                     //49C4
     CAM_REG_LCE_QUA                 CAM_LCE_QUA;                    //49C8
     CAM_REG_LCE_DGC_1               CAM_LCE_DGC_1;                  //49CC
     CAM_REG_LCE_DGC_2               CAM_LCE_DGC_2;                  //49D0
     CAM_REG_LCE_GM                  CAM_LCE_GM;                     //49D4
     UINT32                          rsv_49D8[2];                    //49D8...49DC
     CAM_REG_LCE_SLM_LB              CAM_LCE_SLM_LB;                 //49E0
     CAM_REG_LCE_SLM_SIZE            CAM_LCE_SLM_SIZE;               //49E4
     CAM_REG_LCE_OFST                CAM_LCE_OFST;                   //49E8
     CAM_REG_LCE_BIAS                CAM_LCE_BIAS;                   //49EC
     CAM_REG_LCE_IMAGE_SIZE          CAM_LCE_IMAGE_SIZE;             //49F0
     UINT32                          rsv_49F4[3];                   //49F4...4A18
     CAM_REG_CPG_SATU_1              CAM_CPG_SATU_1;                 //4A00
     CAM_REG_CPG_SATU_2              CAM_CPG_SATU_2;                 //4A04
     CAM_REG_CPG_GAIN_1              CAM_CPG_GAIN_1;                 //4A08
     CAM_REG_CPG_GAIN_2              CAM_CPG_GAIN_2;                 //4A0C
     CAM_REG_CPG_OFST_1              CAM_CPG_OFST_1;                 //4A10
     CAM_REG_CPG_OFST_2              CAM_CPG_OFST_2;                 //4A14
     UINT32                          rsv_4A18[1];                    //4A18
     CAM_REG_C42_CON                 CAM_C42_CON;                    //4A1C
     CAM_REG_ANR_CON1                CAM_ANR_CON1;                   //4A20
     CAM_REG_ANR_CON2                CAM_ANR_CON2;                   //4A24
     CAM_REG_ANR_CON3                CAM_ANR_CON3;                   //4A28
     CAM_REG_ANR_YAD1                CAM_ANR_YAD1;                   //4A2C
     CAM_REG_ANR_YAD2                CAM_ANR_YAD2;                   //4A30
     CAM_REG_ANR_4LUT1               CAM_ANR_4LUT1;                  //4A34
     CAM_REG_ANR_4LUT2               CAM_ANR_4LUT2;                  //4A38
     CAM_REG_ANR_4LUT3               CAM_ANR_4LUT3;                  //4A3C
     CAM_REG_ANR_PTY                 CAM_ANR_PTY;                    //4A40
     CAM_REG_ANR_CAD                 CAM_ANR_CAD;                    //4A44
     CAM_REG_ANR_PTC                 CAM_ANR_PTC;                    //4A48
     CAM_REG_ANR_LCE1                CAM_ANR_LCE1;                   //4A4C
     CAM_REG_ANR_LCE2                CAM_ANR_LCE2;                   //4A50
     CAM_REG_ANR_HP1                 CAM_ANR_HP1;                    //4A54
     CAM_REG_ANR_HP2                 CAM_ANR_HP2;                    //4A58
     CAM_REG_ANR_HP3                 CAM_ANR_HP3;                    //4A5C
     CAM_REG_ANR_ACTY                CAM_ANR_ACTY;                   //4A60
     CAM_REG_ANR_ACTC                CAM_ANR_ACTC;                   //4A64
     CAM_REG_ANR_RSV1                CAM_ANR_RSV1;                   //4A68
     CAM_REG_ANR_RSV2                CAM_ANR_RSV2;                   //4A6C
     UINT32                          rsv_4A70[4];                    //4A70...4A7C
     CAM_REG_CCR_CON                 CAM_CCR_CON;                    //4A80
     CAM_REG_CCR_YLUT                CAM_CCR_YLUT;                   //4A84
     CAM_REG_CCR_UVLUT               CAM_CCR_UVLUT;                  //4A88
     CAM_REG_CCR_YLUT2               CAM_CCR_YLUT2;                  //4A8C
     CAM_REG_CCR_SAT_CTRL            CAM_CCR_SAT_CTRL;               //4A90
     CAM_REG_CCR_UVLUT_SP            CAM_CCR_UVLUT_SP;               //4A94
     UINT32                          rsv_4A98[2];                    //4A98...4A9C
     CAM_REG_SEEE_SRK_CTRL           CAM_SEEE_SRK_CTRL;              //4AA0
     CAM_REG_SEEE_CLIP_CTRL          CAM_SEEE_CLIP_CTRL;             //4AA4
     CAM_REG_SEEE_FLT_CTRL_1         CAM_SEEE_FLT_CTRL_1;            //4AA8
     CAM_REG_SEEE_FLT_CTRL_2         CAM_SEEE_FLT_CTRL_2;            //4AAC
     CAM_REG_SEEE_GLUT_CTRL_01       CAM_SEEE_GLUT_CTRL_01;          //4AB0
     CAM_REG_SEEE_GLUT_CTRL_02       CAM_SEEE_GLUT_CTRL_02;          //4AB4
     CAM_REG_SEEE_GLUT_CTRL_03       CAM_SEEE_GLUT_CTRL_03;          //4AB8
     CAM_REG_SEEE_GLUT_CTRL_04       CAM_SEEE_GLUT_CTRL_04;          //4ABC
     CAM_REG_SEEE_GLUT_CTRL_05       CAM_SEEE_GLUT_CTRL_05;          //4AC0
     CAM_REG_SEEE_GLUT_CTRL_06       CAM_SEEE_GLUT_CTRL_06;          //4AC4
     CAM_REG_SEEE_EDTR_CTRL          CAM_SEEE_EDTR_CTRL;             //4AC8
     CAM_REG_SEEE_OUT_EDGE_CTRL      CAM_SEEE_OUT_EDGE_CTRL;         //4ACC
     CAM_REG_SEEE_SE_Y_CTRL          CAM_SEEE_SE_Y_CTRL;             //4AD0
     CAM_REG_SEEE_SE_EDGE_CTRL_1     CAM_SEEE_SE_EDGE_CTRL_1;        //4AD4
     CAM_REG_SEEE_SE_EDGE_CTRL_2     CAM_SEEE_SE_EDGE_CTRL_2;        //4AD8
     CAM_REG_SEEE_SE_EDGE_CTRL_3     CAM_SEEE_SE_EDGE_CTRL_3;        //4ADC
     CAM_REG_SEEE_SE_SPECL_CTRL      CAM_SEEE_SE_SPECL_CTRL;         //4AE0
     CAM_REG_SEEE_SE_CORE_CTRL_1     CAM_SEEE_SE_CORE_CTRL_1;        //4AE4
     CAM_REG_SEEE_SE_CORE_CTRL_2     CAM_SEEE_SE_CORE_CTRL_2;        //4AE8
     CAM_REG_SEEE_GLUT_CTRL_07       CAM_SEEE_GLUT_CTRL_07;          //4AEC
     CAM_REG_SEEE_GLUT_CTRL_08       CAM_SEEE_GLUT_CTRL_08;          //4AF0
     CAM_REG_SEEE_GLUT_CTRL_09       CAM_SEEE_GLUT_CTRL_09;          //4AF4
     CAM_REG_SEEE_GLUT_CTRL_10       CAM_SEEE_GLUT_CTRL_10;          //4AF8
     CAM_REG_SEEE_GLUT_CTRL_11       CAM_SEEE_GLUT_CTRL_11;          //4AFC
     CAM_REG_CRZ_CONTROL             CAM_CRZ_CONTROL;                //4B00
     CAM_REG_CRZ_IN_IMG              CAM_CRZ_IN_IMG;                 //4B04
     CAM_REG_CRZ_OUT_IMG             CAM_CRZ_OUT_IMG;                //4B08
     CAM_REG_CRZ_HORI_STEP           CAM_CRZ_HORI_STEP;              //4B0C
     CAM_REG_CRZ_VERT_STEP           CAM_CRZ_VERT_STEP;              //4B10
     CAM_REG_CRZ_LUMA_HORI_INT_OFST  CAM_CRZ_LUMA_HORI_INT_OFST;     //4B14
     CAM_REG_CRZ_LUMA_HORI_SUB_OFST  CAM_CRZ_LUMA_HORI_SUB_OFST;     //4B18
     CAM_REG_CRZ_LUMA_VERT_INT_OFST  CAM_CRZ_LUMA_VERT_INT_OFST;     //4B1C
     CAM_REG_CRZ_LUMA_VERT_SUB_OFST  CAM_CRZ_LUMA_VERT_SUB_OFST;     //4B20
     CAM_REG_CRZ_CHRO_HORI_INT_OFST  CAM_CRZ_CHRO_HORI_INT_OFST;     //4B24
     CAM_REG_CRZ_CHRO_HORI_SUB_OFST  CAM_CRZ_CHRO_HORI_SUB_OFST;     //4B28
     CAM_REG_CRZ_CHRO_VERT_INT_OFST  CAM_CRZ_CHRO_VERT_INT_OFST;     //4B2C
     CAM_REG_CRZ_CHRO_VERT_SUB_OFST  CAM_CRZ_CHRO_VERT_SUB_OFST;     //4B30
     CAM_REG_CRZ_DER_1               CAM_CRZ_DER_1;                  //4B34
     CAM_REG_CRZ_DER_2               CAM_CRZ_DER_2;                  //4B38
     UINT32                          rsv_4B3C[25];                   //4B3C...4B9C
     CAM_REG_G2C_CONV_0A             CAM_G2C_CONV_0A;                //4BA0
     CAM_REG_G2C_CONV_0B             CAM_G2C_CONV_0B;                //4BA4
     CAM_REG_G2C_CONV_1A             CAM_G2C_CONV_1A;                //4BA8
     CAM_REG_G2C_CONV_1B             CAM_G2C_CONV_1B;                //4BAC
     CAM_REG_G2C_CONV_2A             CAM_G2C_CONV_2A;                //4BB0
     CAM_REG_G2C_CONV_2B             CAM_G2C_CONV_2B;                //4BB4
     CAM_REG_G2C_SHADE_CON_1         CAM_G2C_SHADE_CON_1;            //4BB8
     CAM_REG_G2C_SHADE_CON_2         CAM_G2C_SHADE_CON_2;            //4BBC
     CAM_REG_G2C_SHADE_CON_3         CAM_G2C_SHADE_CON_3;            //4BC0
     CAM_REG_G2C_SHADE_TAR           CAM_G2C_SHADE_TAR;              //4BC4
     CAM_REG_G2C_SHADE_SP            CAM_G2C_SHADE_SP;               //4BC8
     UINT32                          rsv_4BCC[25];                   //4BCC...4C2C
     CAM_REG_SRZ1_CONTROL            CAM_SRZ1_CONTROL;               //4C30
     CAM_REG_SRZ1_IN_IMG             CAM_SRZ1_IN_IMG;                //4C34
     CAM_REG_SRZ1_OUT_IMG            CAM_SRZ1_OUT_IMG;               //4C38
     CAM_REG_SRZ1_HORI_STEP          CAM_SRZ1_HORI_STEP;             //4C3C
     CAM_REG_SRZ1_VERT_STEP          CAM_SRZ1_VERT_STEP;             //4C40
     CAM_REG_SRZ1_HORI_INT_OFST      CAM_SRZ1_HORI_INT_OFST;         //4C44
     CAM_REG_SRZ1_HORI_SUB_OFST      CAM_SRZ1_HORI_SUB_OFST;         //4C48
     CAM_REG_SRZ1_VERT_INT_OFST      CAM_SRZ1_VERT_INT_OFST;         //4C4C
     CAM_REG_SRZ1_VERT_SUB_OFST      CAM_SRZ1_VERT_SUB_OFST;         //4C50
     UINT32                          rsv_4C54[3];                    //4C54...4C5C
     CAM_REG_SRZ2_CONTROL            CAM_SRZ2_CONTROL;               //4C60
     CAM_REG_SRZ2_IN_IMG             CAM_SRZ2_IN_IMG;                //4C64
     CAM_REG_SRZ2_OUT_IMG            CAM_SRZ2_OUT_IMG;               //4C68
     CAM_REG_SRZ2_HORI_STEP          CAM_SRZ2_HORI_STEP;             //4C6C
     CAM_REG_SRZ2_VERT_STEP          CAM_SRZ2_VERT_STEP;             //4C70
     CAM_REG_SRZ2_HORI_INT_OFST      CAM_SRZ2_HORI_INT_OFST;         //4C74
     CAM_REG_SRZ2_HORI_SUB_OFST      CAM_SRZ2_HORI_SUB_OFST;         //4C78
     CAM_REG_SRZ2_VERT_INT_OFST      CAM_SRZ2_VERT_INT_OFST;         //4C7C
     CAM_REG_SRZ2_VERT_SUB_OFST      CAM_SRZ2_VERT_SUB_OFST;         //4C80
     UINT32                          rsv_4C84[3];                    //4C84...4C8C
     CAM_REG_MIX1_CTRL_0             CAM_MIX1_CTRL_0;                //4C90
     CAM_REG_MIX1_CTRL_1             CAM_MIX1_CTRL_1;                //4C94
     CAM_REG_MIX1_SPARE              CAM_MIX1_SPARE;                 //4C98
     UINT32                          rsv_4C9C;                       //4C9C
     CAM_REG_MIX2_CTRL_0             CAM_MIX2_CTRL_0;                //4CA0
     CAM_REG_MIX2_CTRL_1             CAM_MIX2_CTRL_1;                //4CA4
     CAM_REG_MIX2_SPARE              CAM_MIX2_SPARE;                 //4CA8
     UINT32                          rsv_4CAC;                       //4CAC
     CAM_REG_MIX3_CTRL_0             CAM_MIX3_CTRL_0;                //4CB0
     CAM_REG_MIX3_CTRL_1             CAM_MIX3_CTRL_1;                //4CB4
     CAM_REG_MIX3_SPARE              CAM_MIX3_SPARE;                 //4CB8
     UINT32                          rsv_4CBC;                       //4CBC
     CAM_REG_NR3D_BLEND              CAM_NR3D_BLEND;                 //4CC0
     CAM_REG_NR3D_FBCNT_OFF          CAM_NR3D_FBCNT_OFF;             //4CC4
     CAM_REG_NR3D_FBCNT_SIZ          CAM_NR3D_FBCNT_SIZ;             //4CC8
     CAM_REG_NR3D_FB_COUNT           CAM_NR3D_FB_COUNT;              //4CCC
     CAM_REG_NR3D_LMT_CPX            CAM_NR3D_LMT_CPX;               //4CD0
     CAM_REG_NR3D_LMT_Y_CON1         CAM_NR3D_LMT_Y_CON1;            //4CD4
     CAM_REG_NR3D_LMT_Y_CON2         CAM_NR3D_LMT_Y_CON2;            //4CD8
     CAM_REG_NR3D_LMT_Y_CON3         CAM_NR3D_LMT_Y_CON3;            //4CDC
     CAM_REG_NR3D_LMT_U_CON1         CAM_NR3D_LMT_U_CON1;            //4CE0
     CAM_REG_NR3D_LMT_U_CON2         CAM_NR3D_LMT_U_CON2;            //4CE4
     CAM_REG_NR3D_LMT_U_CON3         CAM_NR3D_LMT_U_CON3;            //4CE8
     CAM_REG_NR3D_LMT_V_CON1         CAM_NR3D_LMT_V_CON1;            //4CEC
     CAM_REG_NR3D_LMT_V_CON2         CAM_NR3D_LMT_V_CON2;            //4CF0
     CAM_REG_NR3D_LMT_V_CON3         CAM_NR3D_LMT_V_CON3;            //4CF4
     CAM_REG_NR3D_CTRL               CAM_NR3D_CTRL;                  //4CF8
     CAM_REG_NR3D_ON_OFF             CAM_NR3D_ON_OFF;                //4CFC
     CAM_REG_NR3D_ON_SIZ             CAM_NR3D_ON_SIZ;                //4D00
     CAM_REG_NR3D_SPARE0             CAM_NR3D_SPARE0;                //4D04
     UINT32                          rsv_4D08[2];                    //4D08...4D0C
     CAM_REG_MDP_CROP_X              CAM_MDP_CROP_X;                 //4D10
     UINT32                          rsv_4D14[3];                    //4D14...4D1C
     CAM_REG_MDP_CROP_Y              CAM_MDP_CROP_Y;                 //4D20
     UINT32                          rsv_4D24[39];                   //4D24...4DBC
     CAM_REG_EIS_PREP_ME_CTRL1       CAM_EIS_PREP_ME_CTRL1;          //4DC0
     CAM_REG_EIS_PREP_ME_CTRL2       CAM_EIS_PREP_ME_CTRL2;          //4DC4
     CAM_REG_EIS_LMV_TH              CAM_EIS_LMV_TH;                 //4DC8
     CAM_REG_EIS_FL_OFFSET           CAM_EIS_FL_OFFSET;              //4DCC
     CAM_REG_EIS_MB_OFFSET           CAM_EIS_MB_OFFSET;              //4DD0
     CAM_REG_EIS_MB_INTERVAL         CAM_EIS_MB_INTERVAL;            //4DD4
     CAM_REG_EIS_GMV                 CAM_EIS_GMV;                    //4DD8
     CAM_REG_EIS_ERR_CTRL            CAM_EIS_ERR_CTRL;               //4DDC
     CAM_REG_EIS_IMAGE_CTRL          CAM_EIS_IMAGE_CTRL;             //4DE0
     UINT32                          rsv_4DE4[7];                    //4DE4...4DFC
     CAM_REG_DMX_CTL                 CAM_DMX_CTL;                    //4E00
     CAM_REG_DMX_CROP                CAM_DMX_CROP;                   //4E04
     CAM_REG_DMX_VSIZE               CAM_DMX_VSIZE;                  //4E08
     UINT32                          rsv_4E0C;                       //4E0C
     CAM_REG_BMX_CTL                 CAM_BMX_CTL;                    //4E10
     CAM_REG_BMX_CROP                CAM_BMX_CROP;                   //4E14
     CAM_REG_BMX_VSIZE               CAM_BMX_VSIZE;                  //4E18
     UINT32                          rsv_4E1C;                       //4E1C
     CAM_REG_RMX_CTL                 CAM_RMX_CTL;                    //4E20
     CAM_REG_RMX_CROP                CAM_RMX_CROP;                   //4E24
     CAM_REG_RMX_VSIZE               CAM_RMX_VSIZE;                  //4E28
     UINT32                          rsv_4E2C[9];                    //4E2C...4E4C
     CAM_REG_UFE_CON                 CAM_UFE_CON;                    //4E50
     UINT32                          rsv_4E54[3];                    //4E54...4E5C
     CAM_REG_UFD_CON                 CAM_UFD_CON;                    //4E60
     CAM_REG_UFD_SIZE_CON            CAM_UFD_SIZE_CON;               //4E64
     CAM_REG_UFD_AU_CON              CAM_UFD_AU_CON;                 //4E68
     CAM_REG_UFD_CROP_CON1           CAM_UFD_CROP_CON1;              //4E6C
     CAM_REG_UFD_CROP_CON2           CAM_UFD_CROP_CON2;              //4E70
     UINT32                          rsv_4E74[51];                   //4E74...4F3C
     CAM_REG_SL2_CEN                 CAM_SL2_CEN;                    //4F40
     CAM_REG_SL2_MAX0_RR             CAM_SL2_MAX0_RR;                //4F44
     CAM_REG_SL2_MAX1_RR             CAM_SL2_MAX1_RR;                //4F48
     CAM_REG_SL2_MAX2_RR             CAM_SL2_MAX2_RR;                //4F4C
     CAM_REG_SL2_HRZ                 CAM_SL2_HRZ;                    //4F50
     CAM_REG_SL2_XOFF                CAM_SL2_XOFF;                   //4F54
     CAM_REG_SL2_YOFF                CAM_SL2_YOFF;                   //4F58
     UINT32                          rsv_4F5C;                       //4F5C
     CAM_REG_SL2B_CEN                CAM_SL2B_CEN;                   //4F60
     CAM_REG_SL2B_MAX0_RR            CAM_SL2B_MAX0_RR;               //4F64
     CAM_REG_SL2B_MAX1_RR            CAM_SL2B_MAX1_RR;               //4F68
     CAM_REG_SL2B_MAX2_RR            CAM_SL2B_MAX2_RR;               //4F6C
     CAM_REG_SL2B_HRZ                CAM_SL2B_HRZ;                   //4F70
     CAM_REG_SL2B_XOFF               CAM_SL2B_XOFF;                  //4F74
     CAM_REG_SL2B_YOFF               CAM_SL2B_YOFF;                  //4F78
     UINT32                          rsv_4F7C[9];                    //4F7C...4F9C
     CAM_REG_CRSP_CTRL               CAM_CRSP_CTRL;                  //4FA0
     UINT32                          rsv_4FA4;                       //4FA4
     CAM_REG_CRSP_OUT_IMG            CAM_CRSP_OUT_IMG;               //4FA8
     CAM_REG_CRSP_STEP_OFST          CAM_CRSP_STEP_OFST;             //4FAC
     CAM_REG_CRSP_CROP_X             CAM_CRSP_CROP_X;                //4FB0
     CAM_REG_CRSP_CROP_Y             CAM_CRSP_CROP_Y;                //4FB4
     UINT32                          rsv_4FB8[2];                    //4FB8...4FBC
     CAM_REG_SL2C_CEN                CAM_SL2C_CEN;                   //4FC0
     CAM_REG_SL2C_MAX0_RR            CAM_SL2C_MAX0_RR;               //4FC4
     CAM_REG_SL2C_MAX1_RR            CAM_SL2C_MAX1_RR;               //4FC8
     CAM_REG_SL2C_MAX2_RR            CAM_SL2C_MAX2_RR;               //4FCC
     CAM_REG_SL2C_HRZ                CAM_SL2C_HRZ;                   //4FD0
     CAM_REG_SL2C_XOFF               CAM_SL2C_XOFF;                  //4FD4
     CAM_REG_SL2C_YOFF               CAM_SL2C_YOFF;                  //4FD8
     UINT32                          rsv_4FDC[9];                    //4FDC...4FFC
     CAM_REG_GGM_LUT_RB              CAM_GGM_LUT_RB[144];            //5000
     CAM_REG_GGM_LUT_G               CAM_GGM_LUT_G[144];             //5240
     CAM_REG_GGM_CTRL                CAM_GGM_CTRL;                   //5480
     UINT32                          rsv_5484[223];                  //5484...57FC
     CAM_REG_PCA_TBL                 CAM_PCA_TBL[360];               //5800
     UINT32                          rsv_5DA0[24];                   //5DA0...5DFC
     CAM_REG_PCA_CON1                CAM_PCA_CON1;                   //5E00
     CAM_REG_PCA_CON2                CAM_PCA_CON2;                   //5E04
     UINT32                          rsv_5E08[122];                  //5E08...5FEC
     CAM_REG_TILE_RING_CON1          CAM_TILE_RING_CON1;             //5FF0
     CAM_REG_CTL_IMGI_SIZE           CAM_CTL_IMGI_SIZE;              //5FF4
     UINT32                          rsv_5FF8[18];                   //5FF8...603C
     CAM_REG_CTL_SEL_GLOBAL_SET      CAM_CTL_SEL_GLOBAL_SET;         //6040 SET for 4040
     UINT32                          rsv_6044[243];                     //6044...640C
     CAM_REG_TG2_SEN_MODE            CAM_TG2_SEN_MODE;               //6410
     CAM_REG_TG2_VF_CON              CAM_TG2_VF_CON;                 //6414
     CAM_REG_TG2_SEN_GRAB_PXL        CAM_TG2_SEN_GRAB_PXL;           //6418
     CAM_REG_TG2_SEN_GRAB_LIN        CAM_TG2_SEN_GRAB_LIN;           //641C
     CAM_REG_TG2_PATH_CFG            CAM_TG2_PATH_CFG;               //6420
     CAM_REG_TG2_MEMIN_CTL           CAM_TG2_MEMIN_CTL;              //6424
     CAM_REG_TG2_INT1                CAM_TG2_INT1;                   //6428
     CAM_REG_TG2_INT2                CAM_TG2_INT2;                   //642C
     CAM_REG_TG2_SOF_CNT             CAM_TG2_SOF_CNT;                //6430
     CAM_REG_TG2_SOT_CNT             CAM_TG2_SOT_CNT;                //6434
     CAM_REG_TG2_EOT_CNT             CAM_TG2_EOT_CNT;                //6438
     CAM_REG_TG2_ERR_CTL             CAM_TG2_ERR_CTL;                //643C
     CAM_REG_TG2_DAT_NO              CAM_TG2_DAT_NO;                 //6440
     CAM_REG_TG2_FRM_CNT_ST          CAM_TG2_FRM_CNT_ST;             //6444
     CAM_REG_TG2_FRMSIZE_ST          CAM_TG2_FRMSIZE_ST;             //6448
     CAM_REG_TG2_INTER_ST            CAM_TG2_INTER_ST;               //644C
     UINT32                          rsv_6450[44];                   //6450...64FC
     CAM_REG_OBC_D_OFFST0            CAM_OBC_D_OFFST0;               //6500
     CAM_REG_OBC_D_OFFST1            CAM_OBC_D_OFFST1;               //6504
     CAM_REG_OBC_D_OFFST2            CAM_OBC_D_OFFST2;               //6508
     CAM_REG_OBC_D_OFFST3            CAM_OBC_D_OFFST3;               //650C
     CAM_REG_OBC_D_GAIN0             CAM_OBC_D_GAIN0;                //6510
     CAM_REG_OBC_D_GAIN1             CAM_OBC_D_GAIN1;                //6514
     CAM_REG_OBC_D_GAIN2             CAM_OBC_D_GAIN2;                //6518
     CAM_REG_OBC_D_GAIN3             CAM_OBC_D_GAIN3;                //651C
     UINT32                          rsv_6520[4];                    //6520...652C
     CAM_REG_LSC_D_CTL1              CAM_LSC_D_CTL1;                 //6530
     CAM_REG_LSC_D_CTL2              CAM_LSC_D_CTL2;                 //6534
     CAM_REG_LSC_D_CTL3              CAM_LSC_D_CTL3;                 //6538
     CAM_REG_LSC_D_LBLOCK            CAM_LSC_D_LBLOCK;               //653C
     CAM_REG_LSC_D_RATIO             CAM_LSC_D_RATIO;                //6540
     CAM_REG_LSC_D_TPIPE_OFST        CAM_LSC_D_TPIPE_OFST;           //6544
     CAM_REG_LSC_D_TPIPE_SIZE        CAM_LSC_D_TPIPE_SIZE;           //6548
     CAM_REG_LSC_D_GAIN_TH           CAM_LSC_D_GAIN_TH;              //654C
     CAM_REG_RPG_D_SATU_1            CAM_RPG_D_SATU_1;               //6550
     CAM_REG_RPG_D_SATU_2            CAM_RPG_D_SATU_2;               //6554
     CAM_REG_RPG_D_GAIN_1            CAM_RPG_D_GAIN_1;               //6558
     CAM_REG_RPG_D_GAIN_2            CAM_RPG_D_GAIN_2;               //655C
     CAM_REG_RPG_D_OFST_1            CAM_RPG_D_OFST_1;               //6560
     CAM_REG_RPG_D_OFST_2            CAM_RPG_D_OFST_2;               //6564
     UINT32                          rsv_6568[18];                   //6568...65AC
     CAM_REG_AWB_D_WIN_ORG           CAM_AWB_D_WIN_ORG;              //65B0
     CAM_REG_AWB_D_WIN_SIZE          CAM_AWB_D_WIN_SIZE;             //65B4
     CAM_REG_AWB_D_WIN_PIT           CAM_AWB_D_WIN_PIT;              //65B8
     CAM_REG_AWB_D_WIN_NUM           CAM_AWB_D_WIN_NUM;              //65BC
     CAM_REG_AWB_D_GAIN1_0           CAM_AWB_D_GAIN1_0;              //65C0
     CAM_REG_AWB_D_GAIN1_1           CAM_AWB_D_GAIN1_1;              //65C4
     CAM_REG_AWB_D_LMT1_0            CAM_AWB_D_LMT1_0;               //65C8
     CAM_REG_AWB_D_LMT1_1            CAM_AWB_D_LMT1_1;               //65CC
     CAM_REG_AWB_D_LOW_THR           CAM_AWB_D_LOW_THR;              //65D0
     CAM_REG_AWB_D_HI_THR            CAM_AWB_D_HI_THR;               //65D4
     CAM_REG_AWB_D_PIXEL_CNT0        CAM_AWB_D_PIXEL_CNT0;           //65D8
     CAM_REG_AWB_D_PIXEL_CNT1        CAM_AWB_D_PIXEL_CNT1;           //65DC
     CAM_REG_AWB_D_PIXEL_CNT2        CAM_AWB_D_PIXEL_CNT2;           //65E0
     CAM_REG_AWB_D_ERR_THR           CAM_AWB_D_ERR_THR;              //65E4
     CAM_REG_AWB_D_ROT               CAM_AWB_D_ROT;                  //65E8
     CAM_REG_AWB_D_L0_X              CAM_AWB_D_L0_X;                 //65EC
     CAM_REG_AWB_D_L0_Y              CAM_AWB_D_L0_Y;                 //65F0
     CAM_REG_AWB_D_L1_X              CAM_AWB_D_L1_X;                 //65F4
     CAM_REG_AWB_D_L1_Y              CAM_AWB_D_L1_Y;                 //65F8
     CAM_REG_AWB_D_L2_X              CAM_AWB_D_L2_X;                 //65FC
     CAM_REG_AWB_D_L2_Y              CAM_AWB_D_L2_Y;                 //6600
     CAM_REG_AWB_D_L3_X              CAM_AWB_D_L3_X;                 //6604
     CAM_REG_AWB_D_L3_Y              CAM_AWB_D_L3_Y;                 //6608
     CAM_REG_AWB_D_L4_X              CAM_AWB_D_L4_X;                 //660C
     CAM_REG_AWB_D_L4_Y              CAM_AWB_D_L4_Y;                 //6610
     CAM_REG_AWB_D_L5_X              CAM_AWB_D_L5_X;                 //6614
     CAM_REG_AWB_D_L5_Y              CAM_AWB_D_L5_Y;                 //6618
     CAM_REG_AWB_D_L6_X              CAM_AWB_D_L6_X;                 //661C
     CAM_REG_AWB_D_L6_Y              CAM_AWB_D_L6_Y;                 //6620
     CAM_REG_AWB_D_L7_X              CAM_AWB_D_L7_X;                 //6624
     CAM_REG_AWB_D_L7_Y              CAM_AWB_D_L7_Y;                 //6628
     CAM_REG_AWB_D_L8_X              CAM_AWB_D_L8_X;                 //662C
     CAM_REG_AWB_D_L8_Y              CAM_AWB_D_L8_Y;                 //6630
     CAM_REG_AWB_D_L9_X              CAM_AWB_D_L9_X;                 //6634
     CAM_REG_AWB_D_L9_Y              CAM_AWB_D_L9_Y;                 //6638
     CAM_REG_AWB_D_SPARE             CAM_AWB_D_SPARE;                //663C
     UINT32                          rsv_6640[4];                    //6640...664C
     CAM_REG_AE_D_HST_CTL            CAM_AE_D_HST_CTL;               //6650
     CAM_REG_AE_D_GAIN2_0            CAM_AE_D_GAIN2_0;               //6654
     CAM_REG_AE_D_GAIN2_1            CAM_AE_D_GAIN2_1;               //6658
     CAM_REG_AE_D_LMT2_0             CAM_AE_D_LMT2_0;                //665C
     CAM_REG_AE_D_LMT2_1             CAM_AE_D_LMT2_1;                //6660
     CAM_REG_AE_D_RC_CNV_0           CAM_AE_D_RC_CNV_0;              //6664
     CAM_REG_AE_D_RC_CNV_1           CAM_AE_D_RC_CNV_1;              //6668
     CAM_REG_AE_D_RC_CNV_2           CAM_AE_D_RC_CNV_2;              //666C
     CAM_REG_AE_D_RC_CNV_3           CAM_AE_D_RC_CNV_3;              //6670
     CAM_REG_AE_D_RC_CNV_4           CAM_AE_D_RC_CNV_4;              //6674
     CAM_REG_AE_D_YGAMMA_0           CAM_AE_D_YGAMMA_0;              //6678
     CAM_REG_AE_D_YGAMMA_1           CAM_AE_D_YGAMMA_1;              //667C
     CAM_REG_AE_D_HST_SET            CAM_AE_D_HST_SET;               //6680
     CAM_REG_AE_D_HST0_RNG           CAM_AE_D_HST0_RNG;              //6684
     CAM_REG_AE_D_HST1_RNG           CAM_AE_D_HST1_RNG;              //6688
     CAM_REG_AE_D_HST2_RNG           CAM_AE_D_HST2_RNG;              //668C
     CAM_REG_AE_D_HST3_RNG           CAM_AE_D_HST3_RNG;              //6690
     CAM_REG_AE_D_SPARE              CAM_AE_D_SPARE;                 //6694
     UINT32                          rsv_6698[2];                    //6698...669C
     CAM_REG_SGG1_D_PGN              CAM_SGG1_D_PGN;                 //66A0
     CAM_REG_SGG1_D_GMRC_1           CAM_SGG1_D_GMRC_1;              //66A4
     CAM_REG_SGG1_D_GMRC_2           CAM_SGG1_D_GMRC_2;              //66A8
     UINT32                          rsv_66AC;                       //66AC
     CAM_REG_AF_D_CON                CAM_AF_D_CON;                   //66B0
     CAM_REG_AF_D_WINX_1             CAM_AF_D_WINX_1;                //66B4
     CAM_REG_AF_D_WINX_2             CAM_AF_D_WINX_2;                //66B8
     CAM_REG_AF_D_WINX_3             CAM_AF_D_WINX_3;                //66BC
     CAM_REG_AF_D_WINY_1             CAM_AF_D_WINY_1;                //66C0
     CAM_REG_AF_D_WINY_2             CAM_AF_D_WINY_2;                //66C4
     CAM_REG_AF_D_WINY_3             CAM_AF_D_WINY_3;                //66C8
     CAM_REG_AF_D_SIZE               CAM_AF_D_SIZE;                  //66CC
     UINT32                          rsv_66D0;                       //66D0
     CAM_REG_AF_D_FLT_1              CAM_AF_D_FLT_1;                 //66D4
     CAM_REG_AF_D_FLT_2              CAM_AF_D_FLT_2;                 //66D8
     CAM_REG_AF_D_FLT_3              CAM_AF_D_FLT_3;                 //66DC
     CAM_REG_AF_D_TH                 CAM_AF_D_TH;                    //66E0
     CAM_REG_AF_D_FLO_WIN_1          CAM_AF_D_FLO_WIN_1;             //66E4
     CAM_REG_AF_D_FLO_SIZE_1         CAM_AF_D_FLO_SIZE_1;            //66E8
     CAM_REG_AF_D_FLO_WIN_2          CAM_AF_D_FLO_WIN_2;             //66EC
     CAM_REG_AF_D_FLO_SIZE_2         CAM_AF_D_FLO_SIZE_2;            //66F0
     CAM_REG_AF_D_FLO_WIN_3          CAM_AF_D_FLO_WIN_3;             //66F4
     CAM_REG_AF_D_FLO_SIZE_3         CAM_AF_D_FLO_SIZE_3;            //66F8
     CAM_REG_AF_D_FLO_TH             CAM_AF_D_FLO_TH;                //66FC
     CAM_REG_AF_D_IMAGE_SIZE         CAM_AF_D_IMAGE_SIZE;            //6700
     CAM_REG_AF_D_FLT_4              CAM_AF_D_FLT_4;                 //6704
     CAM_REG_AF_D_FLT_5              CAM_AF_D_FLT_5;                 //6708
     CAM_REG_AF_D_STAT_L             CAM_AF_D_STAT_L;                //670C
     CAM_REG_AF_D_STAT_M             CAM_AF_D_STAT_M;                //6710
     CAM_REG_AF_D_FLO_STAT_1L        CAM_AF_D_FLO_STAT_1L;           //6714
     CAM_REG_AF_D_FLO_STAT_1M        CAM_AF_D_FLO_STAT_1M;           //6718
     CAM_REG_AF_D_FLO_STAT_1V        CAM_AF_D_FLO_STAT_1V;           //671C
     CAM_REG_AF_D_FLO_STAT_2L        CAM_AF_D_FLO_STAT_2L;           //6720
     CAM_REG_AF_D_FLO_STAT_2M        CAM_AF_D_FLO_STAT_2M;           //6724
     CAM_REG_AF_D_FLO_STAT_2V        CAM_AF_D_FLO_STAT_2V;           //6728
     CAM_REG_AF_D_FLO_STAT_3L        CAM_AF_D_FLO_STAT_3L;           //672C
     CAM_REG_AF_D_FLO_STAT_3M        CAM_AF_D_FLO_STAT_3M;           //6730
     CAM_REG_AF_D_FLO_STAT_3V        CAM_AF_D_FLO_STAT_3V;           //6734
     UINT32                          rsv_6738[2];                    //6738...673C
     CAM_REG_W2G_D_BLD               CAM_W2G_D_BLD;                  //6740
     CAM_REG_W2G_D_TH_1              CAM_W2G_D_TH_1;                 //6744
     CAM_REG_W2G_D_TH_2              CAM_W2G_D_TH_2;                 //6748
     CAM_REG_W2G_D_CTL_OFT           CAM_W2G_D_CTL_OFT;              //674C
     UINT32                          rsv_6750[4];                    //6750...675C
     CAM_REG_WBN_D_SIZE              CAM_WBN_D_SIZE;                 //6760
     CAM_REG_WBN_D_MODE              CAM_WBN_D_MODE;                 //6764
     UINT32                          rsv_6768[6];                    //6768...677C
     CAM_REG_LCS_D_CON               CAM_LCS_D_CON;                  //6780
     CAM_REG_LCS_D_ST                CAM_LCS_D_ST;                   //6784
     CAM_REG_LCS_D_AWS               CAM_LCS_D_AWS;                  //6788
     CAM_REG_LCS_D_FLR               CAM_LCS_D_FLR;                  //678C
     CAM_REG_LCS_D_LRZR_1            CAM_LCS_D_LRZR_1;               //6790
     CAM_REG_LCS_D_LRZR_2            CAM_LCS_D_LRZR_2;               //6794
     UINT32                          rsv_6798[2];                    //6798...679C
     CAM_REG_RRZ_D_CTL               CAM_RRZ_D_CTL;                  //67A0
     CAM_REG_RRZ_D_IN_IMG            CAM_RRZ_D_IN_IMG;               //67A4
     CAM_REG_RRZ_D_OUT_IMG           CAM_RRZ_D_OUT_IMG;              //67A8
     CAM_REG_RRZ_D_HORI_STEP         CAM_RRZ_D_HORI_STEP;            //67AC
     CAM_REG_RRZ_D_VERT_STEP         CAM_RRZ_D_VERT_STEP;            //67B0
     CAM_REG_RRZ_D_HORI_INT_OFST     CAM_RRZ_D_HORI_INT_OFST;        //67B4
     CAM_REG_RRZ_D_HORI_SUB_OFST     CAM_RRZ_D_HORI_SUB_OFST;        //67B8
     CAM_REG_RRZ_D_VERT_INT_OFST     CAM_RRZ_D_VERT_INT_OFST;        //67BC
     CAM_REG_RRZ_D_VERT_SUB_OFST     CAM_RRZ_D_VERT_SUB_OFST;        //67C0
     CAM_REG_RRZ_D_MODE_TH           CAM_RRZ_D_MODE_TH;              //67C4
     CAM_REG_RRZ_D_MODE_CTL          CAM_RRZ_D_MODE_CTL;             //67C8
     UINT32                          rsv_67CC[13];                   //67CC...67FC
     CAM_REG_BPC_D_CON               CAM_BPC_D_CON;                  //6800
     CAM_REG_BPC_D_TH1               CAM_BPC_D_TH1;                  //6804
     CAM_REG_BPC_D_TH2               CAM_BPC_D_TH2;                  //6808
     CAM_REG_BPC_D_TH3               CAM_BPC_D_TH3;                  //680C
     CAM_REG_BPC_D_TH4               CAM_BPC_D_TH4;                  //6810
     CAM_REG_BPC_D_DTC               CAM_BPC_D_DTC;                  //6814
     CAM_REG_BPC_D_COR               CAM_BPC_D_COR;                  //6818
     CAM_REG_BPC_D_TBLI1             CAM_BPC_D_TBLI1;                //681C
     CAM_REG_BPC_D_TBLI2             CAM_BPC_D_TBLI2;                //6820
     CAM_REG_BPC_D_TH1_C             CAM_BPC_D_TH1_C;                //6824
     CAM_REG_BPC_D_TH2_C             CAM_BPC_D_TH2_C;                //6828
     CAM_REG_BPC_D_TH3_C             CAM_BPC_D_TH3_C;                //682C
     CAM_REG_BPC_D_RMM1              CAM_BPC_D_RMM1;                 //6830
     CAM_REG_BPC_D_RMM2              CAM_BPC_D_RMM2;                 //6834
     CAM_REG_BPC_D_RMM_REVG_1        CAM_BPC_D_RMM_REVG_1;           //6838
     CAM_REG_BPC_D_RMM_REVG_2        CAM_BPC_D_RMM_REVG_2;           //683C
     CAM_REG_BPC_D_RMM_LEOS          CAM_BPC_D_RMM_LEOS;             //6840
     CAM_REG_BPC_D_RMM_GCNT          CAM_BPC_D_RMM_GCNT;             //6844
     UINT32                          rsv_6848[2];                    //6848...684C
     CAM_REG_NR1_D_CON               CAM_NR1_D_CON;                  //6850
     CAM_REG_NR1_D_CT_CON            CAM_NR1_D_CT_CON;               //6854
     CAM_REG_BNR_D_RSV1              CAM_BNR_D_RSV1;                 //6858
     CAM_REG_BNR_D_RSV2              CAM_BNR_D_RSV2;                 //685C
     UINT32                          rsv_6860[104];                  //6860...6AFC
     CAM_REG_CPG_D_SATU_1            CAM_CPG_D_SATU_1;               //6A00
     CAM_REG_CPG_D_SATU_2            CAM_CPG_D_SATU_2;               //6A04
     CAM_REG_CPG_D_GAIN_1            CAM_CPG_D_GAIN_1;               //6A08
     CAM_REG_CPG_D_GAIN_2            CAM_CPG_D_GAIN_2;               //6A0C
     CAM_REG_CPG_D_OFST_1            CAM_CPG_D_OFST_1;               //6A10
     CAM_REG_CPG_D_OFST_2            CAM_CPG_D_OFST_2;               //6A14
     UINT32                          rsv_6A18[250];                  //6A18...6DFC
     CAM_REG_DMX_D_CTL               CAM_DMX_D_CTL;                  //6E00
     CAM_REG_DMX_D_CROP              CAM_DMX_D_CROP;                 //6E04
     CAM_REG_DMX_D_VSIZE             CAM_DMX_D_VSIZE;                //6E08
     UINT32                          rsv_6E0C;                       //6E0C
     CAM_REG_BMX_D_CTL               CAM_BMX_D_CTL;                  //6E10
     CAM_REG_BMX_D_CROP              CAM_BMX_D_CROP;                 //6E14
     CAM_REG_BMX_D_VSIZE             CAM_BMX_D_VSIZE;                //6E18
     UINT32                          rsv_6E1C;                       //6E1C
     CAM_REG_RMX_D_CTL               CAM_RMX_D_CTL;                  //6E20
     CAM_REG_RMX_D_CROP              CAM_RMX_D_CROP;                 //6E24
     CAM_REG_RMX_D_VSIZE             CAM_RMX_D_VSIZE;                //6E28
     UINT32                          rsv_6E2C[133];                  //6E2C...703C
     CAM_REG_CTL_SEL_GLOBAL_CLR      CAM_CTL_SEL_GLOBAL_CLR;         //7040 CLR for 4040
     UINT32                          rsv_7044[111];                     //7044...71FC
     CAM_REG_DMA_SOFT_RSTSTAT        CAM_DMA_SOFT_RSTSTAT;           //7200
     CAM_REG_TDRI_BASE_ADDR          CAM_TDRI_BASE_ADDR;             //7204
     CAM_REG_TDRI_OFST_ADDR          CAM_TDRI_OFST_ADDR;             //7208
     CAM_REG_TDRI_XSIZE              CAM_TDRI_XSIZE;                 //720C
     CAM_REG_CQ0I_BASE_ADDR          CAM_CQ0I_BASE_ADDR;             //7210
     CAM_REG_CQ0I_XSIZE              CAM_CQ0I_XSIZE;                 //7214
     CAM_REG_CQ0I_D_BASE_ADDR        CAM_CQ0I_D_BASE_ADDR;           //7218
     CAM_REG_CQ0I_D_XSIZE            CAM_CQ0I_D_XSIZE;               //721C
     CAM_REG_VERTICAL_FLIP_EN        CAM_VERTICAL_FLIP_EN;           //7220
     CAM_REG_DMA_SOFT_RESET          CAM_DMA_SOFT_RESET;             //7224
     CAM_REG_LAST_ULTRA_EN           CAM_LAST_ULTRA_EN;              //7228
     CAM_REG_IMGI_SLOW_DOWN          CAM_IMGI_SLOW_DOWN;             //722C
     CAM_REG_IMGI_BASE_ADDR          CAM_IMGI_BASE_ADDR;             //7230
     CAM_REG_IMGI_OFST_ADDR          CAM_IMGI_OFST_ADDR;             //7234
     CAM_REG_IMGI_XSIZE              CAM_IMGI_XSIZE;                 //7238
     CAM_REG_IMGI_YSIZE              CAM_IMGI_YSIZE;                 //723C
     CAM_REG_IMGI_STRIDE             CAM_IMGI_STRIDE;                //7240
     UINT32                          rsv_7244;                       //7244
     CAM_REG_IMGI_CON                CAM_IMGI_CON;                   //7248
     CAM_REG_IMGI_CON2               CAM_IMGI_CON2;                  //724C
     CAM_REG_BPCI_BASE_ADDR          CAM_BPCI_BASE_ADDR;             //7250
     CAM_REG_BPCI_OFST_ADDR          CAM_BPCI_OFST_ADDR;             //7254
     CAM_REG_BPCI_XSIZE              CAM_BPCI_XSIZE;                 //7258
     CAM_REG_BPCI_YSIZE              CAM_BPCI_YSIZE;                 //725C
     CAM_REG_BPCI_STRIDE             CAM_BPCI_STRIDE;                //7260
     CAM_REG_BPCI_CON                CAM_BPCI_CON;                   //7264
     CAM_REG_BPCI_CON2               CAM_BPCI_CON2;                  //7268
     CAM_REG_LSCI_BASE_ADDR          CAM_LSCI_BASE_ADDR;             //726C
     CAM_REG_LSCI_OFST_ADDR          CAM_LSCI_OFST_ADDR;             //7270
     CAM_REG_LSCI_XSIZE              CAM_LSCI_XSIZE;                 //7274
     CAM_REG_LSCI_YSIZE              CAM_LSCI_YSIZE;                 //7278
     CAM_REG_LSCI_STRIDE             CAM_LSCI_STRIDE;                //727C
     CAM_REG_LSCI_CON                CAM_LSCI_CON;                   //7280
     CAM_REG_LSCI_CON2               CAM_LSCI_CON2;                  //7284
     CAM_REG_UFDI_BASE_ADDR          CAM_UFDI_BASE_ADDR;             //7288
     CAM_REG_UFDI_OFST_ADDR          CAM_UFDI_OFST_ADDR;             //728C
     CAM_REG_UFDI_XSIZE              CAM_UFDI_XSIZE;                 //7290
     CAM_REG_UFDI_YSIZE              CAM_UFDI_YSIZE;                 //7294
     CAM_REG_UFDI_STRIDE             CAM_UFDI_STRIDE;                //7298
     CAM_REG_UFDI_CON                CAM_UFDI_CON;                   //729C
     CAM_REG_UFDI_CON2               CAM_UFDI_CON2;                  //72A0
     CAM_REG_LCEI_BASE_ADDR          CAM_LCEI_BASE_ADDR;             //72A4
     CAM_REG_LCEI_OFST_ADDR          CAM_LCEI_OFST_ADDR;             //72A8
     CAM_REG_LCEI_XSIZE              CAM_LCEI_XSIZE;                 //72AC
     CAM_REG_LCEI_YSIZE              CAM_LCEI_YSIZE;                 //72B0
     CAM_REG_LCEI_STRIDE             CAM_LCEI_STRIDE;                //72B4
     CAM_REG_LCEI_CON                CAM_LCEI_CON;                   //72B8
     CAM_REG_LCEI_CON2               CAM_LCEI_CON2;                  //72BC
     CAM_REG_VIPI_BASE_ADDR          CAM_VIPI_BASE_ADDR;             //72C0
     CAM_REG_VIPI_OFST_ADDR          CAM_VIPI_OFST_ADDR;             //72C4
     CAM_REG_VIPI_XSIZE              CAM_VIPI_XSIZE;                 //72C8
     CAM_REG_VIPI_YSIZE              CAM_VIPI_YSIZE;                 //72CC
     CAM_REG_VIPI_STRIDE             CAM_VIPI_STRIDE;                //72D0
     UINT32                          rsv_72D4;                       //72D4
     CAM_REG_VIPI_CON                CAM_VIPI_CON;                   //72D8
     CAM_REG_VIPI_CON2               CAM_VIPI_CON2;                  //72DC
     CAM_REG_VIP2I_BASE_ADDR         CAM_VIP2I_BASE_ADDR;            //72E0
     CAM_REG_VIP2I_OFST_ADDR         CAM_VIP2I_OFST_ADDR;            //72E4
     CAM_REG_VIP2I_XSIZE             CAM_VIP2I_XSIZE;                //72E8
     CAM_REG_VIP2I_YSIZE             CAM_VIP2I_YSIZE;                //72EC
     CAM_REG_VIP2I_STRIDE            CAM_VIP2I_STRIDE;               //72F0
     UINT32                          rsv_72F4;                       //72F4
     CAM_REG_VIP2I_CON               CAM_VIP2I_CON;                  //72F8
     CAM_REG_VIP2I_CON2              CAM_VIP2I_CON2;                 //72FC
     CAM_REG_IMGO_BASE_ADDR          CAM_IMGO_BASE_ADDR;             //7300
     CAM_REG_IMGO_OFST_ADDR          CAM_IMGO_OFST_ADDR;             //7304
     CAM_REG_IMGO_XSIZE              CAM_IMGO_XSIZE;                 //7308
     CAM_REG_IMGO_YSIZE              CAM_IMGO_YSIZE;                 //730C
     CAM_REG_IMGO_STRIDE             CAM_IMGO_STRIDE;                //7310
     CAM_REG_IMGO_CON                CAM_IMGO_CON;                   //7314
     CAM_REG_IMGO_CON2               CAM_IMGO_CON2;                  //7318
     CAM_REG_IMGO_CROP               CAM_IMGO_CROP;                  //731C
     CAM_REG_RRZO_BASE_ADDR          CAM_RRZO_BASE_ADDR;             //7320
     CAM_REG_RRZO_OFST_ADDR          CAM_RRZO_OFST_ADDR;             //7324
     CAM_REG_RRZO_XSIZE              CAM_RRZO_XSIZE;                 //7328
     CAM_REG_RRZO_YSIZE              CAM_RRZO_YSIZE;                 //732C
     CAM_REG_RRZO_STRIDE             CAM_RRZO_STRIDE;                //7330
     CAM_REG_RRZO_CON                CAM_RRZO_CON;                   //7334
     CAM_REG_RRZO_CON2               CAM_RRZO_CON2;                  //7338
     CAM_REG_RRZO_CROP               CAM_RRZO_CROP;                  //733C
     CAM_REG_LCSO_BASE_ADDR          CAM_LCSO_BASE_ADDR;             //7340
     CAM_REG_LCSO_OFST_ADDR          CAM_LCSO_OFST_ADDR;             //7344
     CAM_REG_LCSO_XSIZE              CAM_LCSO_XSIZE;                 //7348
     CAM_REG_LCSO_YSIZE              CAM_LCSO_YSIZE;                 //734C
     CAM_REG_LCSO_STRIDE             CAM_LCSO_STRIDE;                //7350
     CAM_REG_LCSO_CON                CAM_LCSO_CON;                   //7354
     CAM_REG_LCSO_CON2               CAM_LCSO_CON2;                  //7358
     CAM_REG_EISO_BASE_ADDR          CAM_EISO_BASE_ADDR;             //735C
     CAM_REG_EISO_XSIZE              CAM_EISO_XSIZE;                 //7360
     CAM_REG_AFO_BASE_ADDR           CAM_AFO_BASE_ADDR;              //7364
     CAM_REG_AFO_XSIZE               CAM_AFO_XSIZE;                  //7368
     CAM_REG_ESFKO_BASE_ADDR         CAM_ESFKO_BASE_ADDR;            //736C
     CAM_REG_ESFKO_XSIZE             CAM_ESFKO_XSIZE;                //7370
     CAM_REG_ESFKO_OFST_ADDR         CAM_ESFKO_OFST_ADDR;            //7374
     CAM_REG_ESFKO_YSIZE             CAM_ESFKO_YSIZE;                //7378
     CAM_REG_ESFKO_STRIDE            CAM_ESFKO_STRIDE;               //737C
     CAM_REG_ESFKO_CON               CAM_ESFKO_CON;                  //7380
     CAM_REG_ESFKO_CON2              CAM_ESFKO_CON2;                 //7384
     CAM_REG_AAO_BASE_ADDR           CAM_AAO_BASE_ADDR;              //7388
     CAM_REG_AAO_OFST_ADDR           CAM_AAO_OFST_ADDR;              //738C
     CAM_REG_AAO_XSIZE               CAM_AAO_XSIZE;                  //7390
     CAM_REG_AAO_YSIZE               CAM_AAO_YSIZE;                  //7394
     CAM_REG_AAO_STRIDE              CAM_AAO_STRIDE;                 //7398
     CAM_REG_AAO_CON                 CAM_AAO_CON;                    //739C
     CAM_REG_AAO_CON2                CAM_AAO_CON2;                   //73A0
     CAM_REG_VIP3I_BASE_ADDR         CAM_VIP3I_BASE_ADDR;            //73A4
     CAM_REG_VIP3I_OFST_ADDR         CAM_VIP3I_OFST_ADDR;            //73A8
     CAM_REG_VIP3I_XSIZE             CAM_VIP3I_XSIZE;                //73AC
     CAM_REG_VIP3I_YSIZE             CAM_VIP3I_YSIZE;                //73B0
     CAM_REG_VIP3I_STRIDE            CAM_VIP3I_STRIDE;               //73B4
     UINT32                          rsv_73B8;                       //73B8
     CAM_REG_VIP3I_CON               CAM_VIP3I_CON;                  //73BC
     CAM_REG_VIP3I_CON2              CAM_VIP3I_CON2;                 //73C0
     CAM_REG_UFEO_BASE_ADDR          CAM_UFEO_BASE_ADDR;             //73C4
     CAM_REG_UFEO_OFST_ADDR          CAM_UFEO_OFST_ADDR;             //73C8
     CAM_REG_UFEO_XSIZE              CAM_UFEO_XSIZE;                 //73CC
     CAM_REG_UFEO_YSIZE              CAM_UFEO_YSIZE;                 //73D0
     CAM_REG_UFEO_STRIDE             CAM_UFEO_STRIDE;                //73D4
     CAM_REG_UFEO_CON                CAM_UFEO_CON;                   //73D8
     CAM_REG_UFEO_CON2               CAM_UFEO_CON2;                  //73DC
     CAM_REG_MFBO_BASE_ADDR          CAM_MFBO_BASE_ADDR;             //73E0
     CAM_REG_MFBO_OFST_ADDR          CAM_MFBO_OFST_ADDR;             //73E4
     CAM_REG_MFBO_XSIZE              CAM_MFBO_XSIZE;                 //73E8
     CAM_REG_MFBO_YSIZE              CAM_MFBO_YSIZE;                 //73EC
     CAM_REG_MFBO_STRIDE             CAM_MFBO_STRIDE;                //73F0
     CAM_REG_MFBO_CON                CAM_MFBO_CON;                   //73F4
     CAM_REG_MFBO_CON2               CAM_MFBO_CON2;                  //73F8
     CAM_REG_MFBO_CROP               CAM_MFBO_CROP;                  //73FC
     CAM_REG_IMG3BO_BASE_ADDR        CAM_IMG3BO_BASE_ADDR;           //7400
     CAM_REG_IMG3BO_OFST_ADDR        CAM_IMG3BO_OFST_ADDR;           //7404
     CAM_REG_IMG3BO_XSIZE            CAM_IMG3BO_XSIZE;               //7408
     CAM_REG_IMG3BO_YSIZE            CAM_IMG3BO_YSIZE;               //740C
     CAM_REG_IMG3BO_STRIDE           CAM_IMG3BO_STRIDE;              //7410
     CAM_REG_IMG3BO_CON              CAM_IMG3BO_CON;                 //7414
     CAM_REG_IMG3BO_CON2             CAM_IMG3BO_CON2;                //7418
     CAM_REG_IMG3BO_CROP             CAM_IMG3BO_CROP;                //741C
     CAM_REG_IMG3CO_BASE_ADDR        CAM_IMG3CO_BASE_ADDR;           //7420
     CAM_REG_IMG3CO_OFST_ADDR        CAM_IMG3CO_OFST_ADDR;           //7424
     CAM_REG_IMG3CO_XSIZE            CAM_IMG3CO_XSIZE;               //7428
     CAM_REG_IMG3CO_YSIZE            CAM_IMG3CO_YSIZE;               //742C
     CAM_REG_IMG3CO_STRIDE           CAM_IMG3CO_STRIDE;              //7430
     CAM_REG_IMG3CO_CON              CAM_IMG3CO_CON;                 //7434
     CAM_REG_IMG3CO_CON2             CAM_IMG3CO_CON2;                //7438
     CAM_REG_IMG3CO_CROP             CAM_IMG3CO_CROP;                //743C
     CAM_REG_IMG2O_BASE_ADDR         CAM_IMG2O_BASE_ADDR;            //7440
     CAM_REG_IMG2O_OFST_ADDR         CAM_IMG2O_OFST_ADDR;            //7444
     CAM_REG_IMG2O_XSIZE             CAM_IMG2O_XSIZE;                //7448
     CAM_REG_IMG2O_YSIZE             CAM_IMG2O_YSIZE;                //744C
     CAM_REG_IMG2O_STRIDE            CAM_IMG2O_STRIDE;               //7450
     CAM_REG_IMG2O_CON               CAM_IMG2O_CON;                  //7454
     CAM_REG_IMG2O_CON2              CAM_IMG2O_CON2;                 //7458
     CAM_REG_IMG2O_CROP              CAM_IMG2O_CROP;                 //745C
     CAM_REG_IMG3O_BASE_ADDR         CAM_IMG3O_BASE_ADDR;            //7460
     CAM_REG_IMG3O_OFST_ADDR         CAM_IMG3O_OFST_ADDR;            //7464
     CAM_REG_IMG3O_XSIZE             CAM_IMG3O_XSIZE;                //7468
     CAM_REG_IMG3O_YSIZE             CAM_IMG3O_YSIZE;                //746C
     CAM_REG_IMG3O_STRIDE            CAM_IMG3O_STRIDE;               //7470
     CAM_REG_IMG3O_CON               CAM_IMG3O_CON;                  //7474
     CAM_REG_IMG3O_CON2              CAM_IMG3O_CON2;                 //7478
     CAM_REG_IMG3O_CROP              CAM_IMG3O_CROP;                 //747C
     CAM_REG_FEO_BASE_ADDR           CAM_FEO_BASE_ADDR;              //7480
     CAM_REG_FEO_OFST_ADDR           CAM_FEO_OFST_ADDR;              //7484
     CAM_REG_FEO_XSIZE               CAM_FEO_XSIZE;                  //7488
     CAM_REG_FEO_YSIZE               CAM_FEO_YSIZE;                  //748C
     CAM_REG_FEO_STRIDE              CAM_FEO_STRIDE;                 //7490
     CAM_REG_FEO_CON                 CAM_FEO_CON;                    //7494
     CAM_REG_FEO_CON2                CAM_FEO_CON2;                   //7498
     CAM_REG_BPCI_D_BASE_ADDR        CAM_BPCI_D_BASE_ADDR;           //749C
     CAM_REG_BPCI_D_OFST_ADDR        CAM_BPCI_D_OFST_ADDR;           //74A0
     CAM_REG_BPCI_D_XSIZE            CAM_BPCI_D_XSIZE;               //74A4
     CAM_REG_BPCI_D_YSIZE            CAM_BPCI_D_YSIZE;               //74A8
     CAM_REG_BPCI_D_STRIDE           CAM_BPCI_D_STRIDE;              //74AC
     CAM_REG_BPCI_D_CON              CAM_BPCI_D_CON;                 //74B0
     CAM_REG_BPCI_D_CON2             CAM_BPCI_D_CON2;                //74B4
     CAM_REG_LSCI_D_BASE_ADDR        CAM_LSCI_D_BASE_ADDR;           //74B8
     CAM_REG_LSCI_D_OFST_ADDR        CAM_LSCI_D_OFST_ADDR;           //74BC
     CAM_REG_LSCI_D_XSIZE            CAM_LSCI_D_XSIZE;               //74C0
     CAM_REG_LSCI_D_YSIZE            CAM_LSCI_D_YSIZE;               //74C4
     CAM_REG_LSCI_D_STRIDE           CAM_LSCI_D_STRIDE;              //74C8
     CAM_REG_LSCI_D_CON              CAM_LSCI_D_CON;                 //74CC
     CAM_REG_LSCI_D_CON2             CAM_LSCI_D_CON2;                //74D0
     CAM_REG_IMGO_D_BASE_ADDR        CAM_IMGO_D_BASE_ADDR;           //74D4
     CAM_REG_IMGO_D_OFST_ADDR        CAM_IMGO_D_OFST_ADDR;           //74D8
     CAM_REG_IMGO_D_XSIZE            CAM_IMGO_D_XSIZE;               //74DC
     CAM_REG_IMGO_D_YSIZE            CAM_IMGO_D_YSIZE;               //74E0
     CAM_REG_IMGO_D_STRIDE           CAM_IMGO_D_STRIDE;              //74E4
     CAM_REG_IMGO_D_CON              CAM_IMGO_D_CON;                 //74E8
     CAM_REG_IMGO_D_CON2             CAM_IMGO_D_CON2;                //74EC
     CAM_REG_IMGO_D_CROP             CAM_IMGO_D_CROP;                //74F0
     CAM_REG_RRZO_D_BASE_ADDR        CAM_RRZO_D_BASE_ADDR;           //74F4
     CAM_REG_RRZO_D_OFST_ADDR        CAM_RRZO_D_OFST_ADDR;           //74F8
     CAM_REG_RRZO_D_XSIZE            CAM_RRZO_D_XSIZE;               //74FC
     CAM_REG_RRZO_D_YSIZE            CAM_RRZO_D_YSIZE;               //7500
     CAM_REG_RRZO_D_STRIDE           CAM_RRZO_D_STRIDE;              //7504
     CAM_REG_RRZO_D_CON              CAM_RRZO_D_CON;                 //7508
     CAM_REG_RRZO_D_CON2             CAM_RRZO_D_CON2;                //750C
     CAM_REG_RRZO_D_CROP             CAM_RRZO_D_CROP;                //7510
     CAM_REG_LCSO_D_BASE_ADDR        CAM_LCSO_D_BASE_ADDR;           //7514
     CAM_REG_LCSO_D_OFST_ADDR        CAM_LCSO_D_OFST_ADDR;           //7518
     CAM_REG_LCSO_D_XSIZE            CAM_LCSO_D_XSIZE;               //751C
     CAM_REG_LCSO_D_YSIZE            CAM_LCSO_D_YSIZE;               //7520
     CAM_REG_LCSO_D_STRIDE           CAM_LCSO_D_STRIDE;              //7524
     CAM_REG_LCSO_D_CON              CAM_LCSO_D_CON;                 //7528
     CAM_REG_LCSO_D_CON2             CAM_LCSO_D_CON2;                //752C
     CAM_REG_AFO_D_BASE_ADDR         CAM_AFO_D_BASE_ADDR;            //7530
     CAM_REG_AFO_D_XSIZE             CAM_AFO_D_XSIZE;                //7534
     CAM_REG_AFO_D_OFST_ADDR         CAM_AFO_D_OFST_ADDR;            //7538
     CAM_REG_AFO_D_YSIZE             CAM_AFO_D_YSIZE;                //753C
     CAM_REG_AFO_D_STRIDE            CAM_AFO_D_STRIDE;               //7540
     CAM_REG_AFO_D_CON               CAM_AFO_D_CON;                  //7544
     CAM_REG_AFO_D_CON2              CAM_AFO_D_CON2;                 //7548
     CAM_REG_AAO_D_BASE_ADDR         CAM_AAO_D_BASE_ADDR;            //754C
     CAM_REG_AAO_D_OFST_ADDR         CAM_AAO_D_OFST_ADDR;            //7550
     CAM_REG_AAO_D_XSIZE             CAM_AAO_D_XSIZE;                //7554
     CAM_REG_AAO_D_YSIZE             CAM_AAO_D_YSIZE;                //7558
     CAM_REG_AAO_D_STRIDE            CAM_AAO_D_STRIDE;               //755C
     CAM_REG_AAO_D_CON               CAM_AAO_D_CON;                  //7560
     CAM_REG_AAO_D_CON2              CAM_AAO_D_CON2;                 //7564
     CAM_REG_DMA_ERR_CTRL            CAM_DMA_ERR_CTRL;               //7568
     CAM_REG_IMGI_ERR_STAT           CAM_IMGI_ERR_STAT;              //756C
     CAM_REG_BPCI_ERR_STAT           CAM_BPCI_ERR_STAT;              //7570
     CAM_REG_LSCI_ERR_STAT           CAM_LSCI_ERR_STAT;              //7574
     CAM_REG_UFDI_ERR_STAT           CAM_UFDI_ERR_STAT;              //7578
     CAM_REG_LCEI_ERR_STAT           CAM_LCEI_ERR_STAT;              //757C
     CAM_REG_VIPI_ERR_STAT           CAM_VIPI_ERR_STAT;              //7580
     CAM_REG_VIP2I_ERR_STAT          CAM_VIP2I_ERR_STAT;             //7584
     CAM_REG_VIP3I_ERR_STAT          CAM_VIP3I_ERR_STAT;             //7588
     CAM_REG_IMGO_ERR_STAT           CAM_IMGO_ERR_STAT;              //758C
     CAM_REG_RRZO_ERR_STAT           CAM_RRZO_ERR_STAT;              //7590
     CAM_REG_LCSO_ERR_STAT           CAM_LCSO_ERR_STAT;              //7594
     CAM_REG_ESFKO_ERR_STAT          CAM_ESFKO_ERR_STAT;             //7598
     CAM_REG_AAO_ERR_STAT            CAM_AAO_ERR_STAT;               //759C
     CAM_REG_UFEO_ERR_STAT           CAM_UFEO_ERR_STAT;              //75A0
     CAM_REG_MFBO_ERR_STAT           CAM_MFBO_ERR_STAT;              //75A4
     CAM_REG_IMG3BO_ERR_STAT         CAM_IMG3BO_ERR_STAT;            //75A8
     CAM_REG_IMG3CO_ERR_STAT         CAM_IMG3CO_ERR_STAT;            //75AC
     CAM_REG_IMG2O_ERR_STAT          CAM_IMG2O_ERR_STAT;             //75B0
     CAM_REG_IMG3O_ERR_STAT          CAM_IMG3O_ERR_STAT;             //75B4
     CAM_REG_FEO_ERR_STAT            CAM_FEO_ERR_STAT;               //75B8
     CAM_REG_BPCI_D_ERR_STAT         CAM_BPCI_D_ERR_STAT;            //75BC
     CAM_REG_LSCI_D_ERR_STAT         CAM_LSCI_D_ERR_STAT;            //75C0
     CAM_REG_IMGO_D_ERR_STAT         CAM_IMGO_D_ERR_STAT;            //75C4
     CAM_REG_RRZO_D_ERR_STAT         CAM_RRZO_D_ERR_STAT;            //75C8
     CAM_REG_LCSO_D_ERR_STAT         CAM_LCSO_D_ERR_STAT;            //75CC
     CAM_REG_AFO_D_ERR_STAT          CAM_AFO_D_ERR_STAT;             //75D0
     CAM_REG_AAO_D_ERR_STAT          CAM_AAO_D_ERR_STAT;             //75D4
     CAM_REG_DMA_DEBUG_ADDR          CAM_DMA_DEBUG_ADDR;             //75D8
     CAM_REG_DMA_RSV1                CAM_RAW_MAGIC_NUM0;             //75DC
     CAM_REG_DMA_RSV2                CAM_RRZ_CROP_IN;//CAM_RAW_MAGIC_NUM1;             //75E0
     CAM_REG_DMA_RSV3                CAM_RAW_D_MAGIC_NUM0;           //75E4
     CAM_REG_DMA_RSV4                CAM_RRZ_CROP_IN_D;//CAM_RAW_D_MAGIC_NUM1;           //75E8
     CAM_REG_DMA_RSV5                CAM_DMA_RSV5;                   //75EC
     CAM_REG_DMA_RSV6                CAM_DMA_RSV6;                   //75F0
     CAM_REG_DMA_DEBUG_SEL           CAM_DMA_DEBUG_SEL;              //75F4
     CAM_REG_DMA_BW_SELF_TEST        CAM_DMA_BW_SELF_TEST;           //75F8
     UINT32                          rsv_75FC[1665];                 //75FC...8FFC
     CAMSV_REG_CAMSV_MODULE_EN       CAMSV_CAMSV_MODULE_EN;          //9000
     CAMSV_REG_CAMSV_FMT_SEL         CAMSV_CAMSV_FMT_SEL;            //9004
     CAMSV_REG_CAMSV_INT_EN          CAMSV_CAMSV_INT_EN;             //9008
     CAMSV_REG_CAMSV_INT_STATUS      CAMSV_CAMSV_INT_STATUS;         //900C
     CAMSV_REG_CAMSV_SW_CTL          CAMSV_CAMSV_SW_CTL;             //9010
     CAMSV_REG_CAMSV_SPARE0          CAMSV_CAMSV_SPARE0;             //9014
     CAMSV_REG_CAMSV_SPARE1          CAMSV_CAMSV_SPARE1;             //9018
     CAMSV_REG_CAMSV_IMGO_FBC        CAMSV_CAMSV_IMGO_FBC;           //901C
     CAMSV_REG_CAMSV_CLK_EN          CAMSV_CAMSV_CLK_EN;             //9020
     CAMSV_REG_CAMSV_DBG_SET         CAMSV_CAMSV_DBG_SET;            //9024
     CAMSV_REG_CAMSV_DBG_PORT        CAMSV_CAMSV_DBG_PORT;           //9028
     CAMSV_REG_CAMSV_DATE_CODE       CAMSV_CAMSV_DATE_CODE;          //902C
     CAMSV_REG_CAMSV_PROJ_CODE       CAMSV_CAMSV_PROJ_CODE;          //9030
     CAMSV_REG_CAMSV_DCM_DIS         CAMSV_CAMSV_DCM_DIS;            //9034
     CAMSV_REG_CAMSV_DCM_STATUS      CAMSV_CAMSV_DCM_STATUS;         //9038
     CAMSV_REG_CAMSV_PAK             CAMSV_CAMSV_PAK;                //903C
     UINT32                          rsv_9040[112];                  //9040...91FC
     CAMSV_REG_DMA_SOFT_RSTSTAT_SV   CAMSV_DMA_SOFT_RSTSTAT_SV;      //9200
     CAMSV_REG_LAST_ULTRA_EN_SV      CAMSV_LAST_ULTRA_EN_SV;         //9204
     CAMSV_REG_IMGO_SV_BASE_ADDR     CAMSV_IMGO_SV_BASE_ADDR;        //9208
     CAMSV_REG_IMGO_SV_OFST_ADDR     CAMSV_IMGO_SV_OFST_ADDR;        //920C
     CAMSV_REG_IMGO_SV_XSIZE         CAMSV_IMGO_SV_XSIZE;            //9210
     CAMSV_REG_IMGO_SV_YSIZE         CAMSV_IMGO_SV_YSIZE;            //9214
     CAMSV_REG_IMGO_SV_STRIDE        CAMSV_IMGO_SV_STRIDE;           //9218
     CAMSV_REG_IMGO_SV_CON           CAMSV_IMGO_SV_CON;              //921C
     CAMSV_REG_IMGO_SV_CON2          CAMSV_IMGO_SV_CON2;             //9220
     CAMSV_REG_IMGO_SV_CROP          CAMSV_IMGO_SV_CROP;             //9224
     CAMSV_REG_IMGO_SV_D_BASE_ADDR   CAMSV_IMGO_SV_D_BASE_ADDR;      //9228
     CAMSV_REG_IMGO_SV_D_OFST_ADDR   CAMSV_IMGO_SV_D_OFST_ADDR;      //922C
     CAMSV_REG_IMGO_SV_D_XSIZE       CAMSV_IMGO_SV_D_XSIZE;          //9230
     CAMSV_REG_IMGO_SV_D_YSIZE       CAMSV_IMGO_SV_D_YSIZE;          //9234
     CAMSV_REG_IMGO_SV_D_STRIDE      CAMSV_IMGO_SV_D_STRIDE;         //9238
     CAMSV_REG_IMGO_SV_D_CON         CAMSV_IMGO_SV_D_CON;            //923C
     CAMSV_REG_IMGO_SV_D_CON2        CAMSV_IMGO_SV_D_CON2;           //9240
     CAMSV_REG_IMGO_SV_D_CROP        CAMSV_IMGO_SV_D_CROP;           //9244
     CAMSV_REG_DMA_ERR_CTRL_SV       CAMSV_DMA_ERR_CTRL_SV;          //9248
     CAMSV_REG_IMGO_SV_ERR_STAT      CAMSV_IMGO_SV_ERR_STAT;         //924C
     CAMSV_REG_IMGO_SV_D_ERR_STAT    CAMSV_IMGO_SV_D_ERR_STAT;       //9250
     CAMSV_REG_DMA_DEBUG_ADDR_SV     CAMSV_DMA_DEBUG_ADDR_SV;        //9254
     CAMSV_REG_DMA_RSV1_SV           CAMSV_DMA_RSV1_SV;              //9258
     CAMSV_REG_DMA_RSV2_SV           CAMSV_DMA_RSV2_SV;              //925C
     CAMSV_REG_DMA_RSV3_SV           CAMSV_DMA_RSV3_SV;              //9260
     CAMSV_REG_DMA_RSV4_SV           CAMSV_DMA_RSV4_SV;              //9264
     CAMSV_REG_DMA_RSV5_SV           CAMSV_DMA_RSV5_SV;              //9268
     CAMSV_REG_DMA_RSV6_SV           CAMSV_DMA_RSV6_SV;              //926C
     UINT32                          rsv_9270[104];                  //9270...940C
     CAMSV_REG_TG_SEN_MODE           CAMSV_TG_SEN_MODE;              //9410
     CAMSV_REG_TG_VF_CON             CAMSV_TG_VF_CON;                //9414
     CAMSV_REG_TG_SEN_GRAB_PXL       CAMSV_TG_SEN_GRAB_PXL;          //9418
     CAMSV_REG_TG_SEN_GRAB_LIN       CAMSV_TG_SEN_GRAB_LIN;          //941C
     CAMSV_REG_TG_PATH_CFG           CAMSV_TG_PATH_CFG;              //9420
     CAMSV_REG_TG_MEMIN_CTL          CAMSV_TG_MEMIN_CTL;             //9424
     CAMSV_REG_TG_INT1               CAMSV_TG_INT1;                  //9428
     CAMSV_REG_TG_INT2               CAMSV_TG_INT2;                  //942C
     CAMSV_REG_TG_SOF_CNT            CAMSV_TG_SOF_CNT;               //9430
     CAMSV_REG_TG_SOT_CNT            CAMSV_TG_SOT_CNT;               //9434
     CAMSV_REG_TG_EOT_CNT            CAMSV_TG_EOT_CNT;               //9438
     CAMSV_REG_TG_ERR_CTL            CAMSV_TG_ERR_CTL;               //943C
     CAMSV_REG_TG_DAT_NO             CAMSV_TG_DAT_NO;                //9440
     CAMSV_REG_TG_FRM_CNT_ST         CAMSV_TG_FRM_CNT_ST;            //9444
     CAMSV_REG_TG_FRMSIZE_ST         CAMSV_TG_FRMSIZE_ST;            //9448
     CAMSV_REG_TG_INTER_ST           CAMSV_TG_INTER_ST;              //944C
     UINT32                          rsv_9450[4];                    //9450...945C
     CAMSV_REG_TG_FLASHA_CTL         CAMSV_TG_FLASHA_CTL;            //9460
     CAMSV_REG_TG_FLASHA_LINE_CNT    CAMSV_TG_FLASHA_LINE_CNT;       //9464
     CAMSV_REG_TG_FLASHA_POS         CAMSV_TG_FLASHA_POS;            //9468
     CAMSV_REG_TG_FLASHB_CTL         CAMSV_TG_FLASHB_CTL;            //946C
     CAMSV_REG_TG_FLASHB_LINE_CNT    CAMSV_TG_FLASHB_LINE_CNT;       //9470
     CAMSV_REG_TG_FLASHB_POS         CAMSV_TG_FLASHB_POS;            //9474
     CAMSV_REG_TG_FLASHB_POS1        CAMSV_TG_FLASHB_POS1;           //9478
     CAMSV_REG_TG_GSCTRL_CTL         CAMSV_TG_GSCTRL_CTL;            //947C
     CAMSV_REG_TG_GSCTRL_TIME        CAMSV_TG_GSCTRL_TIME;           //9480
     CAMSV_REG_TG_MS_PHASE           CAMSV_TG_MS_PHASE;              //9484
     CAMSV_REG_TG_MS_CL_TIME         CAMSV_TG_MS_CL_TIME;            //9488
     CAMSV_REG_TG_MS_OP_TIME         CAMSV_TG_MS_OP_TIME;            //948C
     CAMSV_REG_TG_MS_CLPH_TIME       CAMSV_TG_MS_CLPH_TIME;          //9490
     CAMSV_REG_TG_MS_OPPH_TIME       CAMSV_TG_MS_OPPH_TIME;          //9494
     UINT32                          rsv_9498[29];                   //9498...9508
     CAMSV_TOP_REG_CAMSV_TOP_DEBUG   CAMSV_TOP_CAMSV_TOP_DEBUG;      //950C
     UINT32                          rsv_9510[188];                  //9510...97FC
     CAMSV_REG_CAMSV2_MODULE_EN      CAMSV_CAMSV2_MODULE_EN;         //9800
     CAMSV_REG_CAMSV2_FMT_SEL        CAMSV_CAMSV2_FMT_SEL;           //9804
     CAMSV_REG_CAMSV2_INT_EN         CAMSV_CAMSV2_INT_EN;            //9808
     CAMSV_REG_CAMSV2_INT_STATUS     CAMSV_CAMSV2_INT_STATUS;        //980C
     CAMSV_REG_CAMSV2_SW_CTL         CAMSV_CAMSV2_SW_CTL;            //9810
     CAMSV_REG_CAMSV2_SPARE0         CAMSV_CAMSV2_SPARE0;            //9814
     CAMSV_REG_CAMSV2_SPARE1         CAMSV_CAMSV2_SPARE1;            //9818
     CAMSV_REG_CAMSV2_IMGO_FBC       CAMSV_CAMSV2_IMGO_FBC;          //981C
     CAMSV_REG_CAMSV2_CLK_EN         CAMSV_CAMSV2_CLK_EN;            //9820
     CAMSV_REG_CAMSV2_DBG_SET        CAMSV_CAMSV2_DBG_SET;           //9824
     CAMSV_REG_CAMSV2_DBG_PORT       CAMSV_CAMSV2_DBG_PORT;          //9828
     CAMSV_REG_CAMSV2_DATE_CODE      CAMSV_CAMSV2_DATE_CODE;         //982C
     CAMSV_REG_CAMSV2_PROJ_CODE      CAMSV_CAMSV2_PROJ_CODE;         //9830
     CAMSV_REG_CAMSV2_DCM_DIS        CAMSV_CAMSV2_DCM_DIS;           //9834
     CAMSV_REG_CAMSV2_DCM_STATUS     CAMSV_CAMSV2_DCM_STATUS;        //9838
     CAMSV_REG_CAMSV2_PAK            CAMSV_CAMSV2_PAK;               //983C
     UINT32                          rsv_9840[244];                  //9840...9C0C
     CAMSV_REG_TG2_SEN_MODE          CAMSV_TG2_SEN_MODE;             //9C10
     CAMSV_REG_TG2_VF_CON            CAMSV_TG2_VF_CON;               //9C14
     CAMSV_REG_TG2_SEN_GRAB_PXL      CAMSV_TG2_SEN_GRAB_PXL;         //9C18
     CAMSV_REG_TG2_SEN_GRAB_LIN      CAMSV_TG2_SEN_GRAB_LIN;         //9C1C
     CAMSV_REG_TG2_PATH_CFG          CAMSV_TG2_PATH_CFG;             //9C20
     CAMSV_REG_TG2_MEMIN_CTL         CAMSV_TG2_MEMIN_CTL;            //9C24
     CAMSV_REG_TG2_INT1              CAMSV_TG2_INT1;                 //9C28
     CAMSV_REG_TG2_INT2              CAMSV_TG2_INT2;                 //9C2C
     CAMSV_REG_TG2_SOF_CNT           CAMSV_TG2_SOF_CNT;              //9C30
     CAMSV_REG_TG2_SOT_CNT           CAMSV_TG2_SOT_CNT;              //9C34
     CAMSV_REG_TG2_EOT_CNT           CAMSV_TG2_EOT_CNT;              //9C38
     CAMSV_REG_TG2_ERR_CTL           CAMSV_TG2_ERR_CTL;              //9C3C
     CAMSV_REG_TG2_DAT_NO            CAMSV_TG2_DAT_NO;               //9C40
     CAMSV_REG_TG2_FRM_CNT_ST        CAMSV_TG2_FRM_CNT_ST;           //9C44
     CAMSV_REG_TG2_FRMSIZE_ST        CAMSV_TG2_FRMSIZE_ST;           //9C48
     CAMSV_REG_TG2_INTER_ST          CAMSV_TG2_INTER_ST;             //9C4C
}isp_reg_t;

#endif // _ISP_REG_H_
