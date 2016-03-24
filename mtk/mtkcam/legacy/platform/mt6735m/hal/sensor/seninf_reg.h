#ifndef _SENINF_REG_H_
#define _SENINF_REG_H_

// ----------------- seninf_top Bit Field Definitions -------------------




#define SENINF_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.Bits.FieldName)
#define SENINF_READ_REG(RegBase, RegName) (RegBase->RegName.Raw)

//#define SENINF_WRITE_BITS(RegBase, RegName, FieldName, Value)
//    do {
//        (RegBase->RegName.Bits.FieldName) = (Value);
//    } while (0)

#define SENINF_WRITE_REG(RegBase, RegName, Value)                          \
    do {                                                                \
        (RegBase->RegName.Raw) = (Value);                               \
    } while (0)

#define mt65xx_reg_writel(v, a) \
        do {    \
            *(volatile unsigned int *)(a) = (v);    \
        } while (0)



//#define SENINF_BASE_HW     0x15008000
#define SENINF_BASE_HW     0x15008000
#define SENINF_BASE_RANGE  0x4000
typedef unsigned int FIELD;
typedef unsigned int UINT32;
typedef unsigned int u32;

/* start MT6593_SENINF_TOP_CODA.xml*/
typedef volatile union _REG_SENINF_TOP_CTRL_
{
    volatile struct
    {
        FIELD rsv_0                     : 8;
        FIELD SENINF1_PCLK_SEL          : 1;
        FIELD SENINF2_PCLK_SEL          : 1;
        FIELD SENINF1_PCLK_EN           : 1;
        FIELD SENINF2_PCLK_EN           : 1;
        FIELD rsv_12                    : 4;
        FIELD SENINF_TOP_N3D_SW_RST     : 1;
        FIELD rsv_17                    : 14;
        FIELD SENINF_TOP_DBG_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_CTRL;

typedef volatile union _REG_SENINF_TOP_CMODEL_PAR_
{
    volatile struct
    {
        FIELD SENINF1_EN                : 1;
        FIELD SENINF2_EN                : 1;
        FIELD SENINF3_EN                : 1;
        FIELD SENINF4_EN                : 1;
        FIELD SENINF1_FORMAT            : 4;
        FIELD SENINF2_FORMAT            : 4;
        FIELD SENINF3_FORMAT            : 4;
        FIELD SENINF4_FORMAT            : 4;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_CMODEL_PAR;

typedef volatile union _REG_SENINF_TOP_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF1_MUX_SRC_SEL       : 4;
        FIELD SENINF2_MUX_SRC_SEL       : 4;
        FIELD SENINF3_MUX_SRC_SEL       : 4;
        FIELD SENINF4_MUX_SRC_SEL       : 4;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF_TOP_MUX_CTRL;

/* end MT6593_SENINF_TOP_CODA.xml*/

/* start MT6593_seninf_n3d.xml*/
typedef volatile union _REG_N3D_CTL_
{
    volatile struct
    {
        FIELD MODE                      : 2;
        FIELD I2C1_EN                   : 1;
        FIELD I2C2_EN                   : 1;
        FIELD I2C1_INT_EN               : 1;
        FIELD I2C2_INT_EN               : 1;
        FIELD N3D_EN                    : 1;
        FIELD W1CLR                     : 1;
        FIELD DIFF_EN                   : 1;
        FIELD DDBG_SEL                  : 3;
        FIELD MODE1_DBG                 : 1;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_N3D_CTL;

typedef volatile union _REG_N3D_POS_
{
    volatile struct
    {
        FIELD N3D_POS                   : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_POS;

typedef volatile union _REG_N3D_TRIG_
{
    volatile struct
    {
        FIELD I2CA_TRIG                 : 1;
        FIELD I2CB_TRIG                 : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_N3D_TRIG;

typedef volatile union _REG_N3D_INT_
{
    volatile struct
    {
        FIELD I2C1_INT                  : 1;
        FIELD I2C2_INT                  : 1;
        FIELD DIFF_INT                  : 1;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_N3D_INT;

typedef volatile union _REG_N3D_CNT0_
{
    volatile struct
    {
        FIELD N3D_CNT0                  : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_CNT0;

typedef volatile union _REG_N3D_CNT1_
{
    volatile struct
    {
        FIELD N3D_CNT1                  : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_CNT1;

typedef volatile union _REG_N3D_DBG_
{
    volatile struct
    {
        FIELD N3D_DBG                   : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_DBG;

typedef volatile union _REG_N3D_DIFF_THR_
{
    volatile struct
    {
        FIELD N3D_DIFF_THR              : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_DIFF_THR;

typedef volatile union _REG_N3D_DIFF_CNT_
{
    volatile struct
    {
        FIELD N3D_DIFF_CNT              : 32;
    } Bits;
    UINT32 Raw;
} REG_N3D_DIFF_CNT;

/* end MT6593_seninf_n3d.xml*/

/* start MT6593_SENINF_CODA.xml*/
typedef volatile union _REG_SENINF1_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CTRL;

typedef volatile union _REG_SENINF2_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CTRL;

typedef volatile union _REG_SENINF3_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CTRL;

typedef volatile union _REG_SENINF4_CTRL_
{
    volatile struct
    {
        FIELD SENINF_EN                 : 1;
        FIELD NCSI2_SW_RST              : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CCIR_SW_RST               : 1;
        FIELD CKGEN_SW_RST              : 1;
        FIELD TEST_MODEL_SW_RST         : 1;
        FIELD SCAM_SW_RST               : 1;
        FIELD rsv_7                     : 1;
        FIELD SENINF_DEBUG_SEL          : 4;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD rsv_16                    : 12;
        FIELD PAD2CAM_DATA_SEL          : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_CTRL;

/* end MT6593_SENINF_CODA.xml*/

/* start MT6593_SENINF_MUX_CODA.xml*/
typedef volatile union _REG_SENINF1_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_CTRL;

typedef volatile union _REG_SENINF1_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_INTEN;

typedef volatile union _REG_SENINF1_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_INTSTA;

typedef volatile union _REG_SENINF1_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_SIZE;

typedef volatile union _REG_SENINF1_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_1;

typedef volatile union _REG_SENINF1_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_2;

typedef volatile union _REG_SENINF1_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_3;

typedef volatile union _REG_SENINF1_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_4;

typedef volatile union _REG_SENINF1_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_5;

typedef volatile union _REG_SENINF1_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_6;

typedef volatile union _REG_SENINF1_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DEBUG_7;

typedef volatile union _REG_SENINF1_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_SPARE;

typedef volatile union _REG_SENINF1_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DATA;

typedef volatile union _REG_SENINF1_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_DATA_CNT;

typedef volatile union _REG_SENINF1_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_MUX_CROP;

typedef volatile union _REG_SENINF2_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_CTRL;

typedef volatile union _REG_SENINF2_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_INTEN;

typedef volatile union _REG_SENINF2_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_INTSTA;

typedef volatile union _REG_SENINF2_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_SIZE;

typedef volatile union _REG_SENINF2_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_1;

typedef volatile union _REG_SENINF2_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_2;

typedef volatile union _REG_SENINF2_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_3;

typedef volatile union _REG_SENINF2_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_4;

typedef volatile union _REG_SENINF2_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_5;

typedef volatile union _REG_SENINF2_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_6;

typedef volatile union _REG_SENINF2_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DEBUG_7;

typedef volatile union _REG_SENINF2_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_SPARE;

typedef volatile union _REG_SENINF2_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DATA;

typedef volatile union _REG_SENINF2_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_DATA_CNT;

typedef volatile union _REG_SENINF2_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_MUX_CROP;

typedef volatile union _REG_SENINF3_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_CTRL;

typedef volatile union _REG_SENINF3_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_INTEN;

typedef volatile union _REG_SENINF3_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_INTSTA;

typedef volatile union _REG_SENINF3_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_SIZE;

typedef volatile union _REG_SENINF3_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_1;

typedef volatile union _REG_SENINF3_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_2;

typedef volatile union _REG_SENINF3_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_3;

typedef volatile union _REG_SENINF3_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_4;

typedef volatile union _REG_SENINF3_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_5;

typedef volatile union _REG_SENINF3_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_6;

typedef volatile union _REG_SENINF3_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DEBUG_7;

typedef volatile union _REG_SENINF3_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_SPARE;

typedef volatile union _REG_SENINF3_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DATA;

typedef volatile union _REG_SENINF3_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_DATA_CNT;

typedef volatile union _REG_SENINF3_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_MUX_CROP;

typedef volatile union _REG_SENINF4_MUX_CTRL_
{
    volatile struct
    {
        FIELD SENINF_MUX_SW_RST         : 1;
        FIELD SENINF_IRQ_SW_RST         : 1;
        FIELD rsv_2                     : 5;
        FIELD SENINF_HSYNC_MASK         : 1;
        FIELD SENINF_PIX_SEL            : 1;
        FIELD SENINF_VSYNC_POL          : 1;
        FIELD SENINF_HSYNC_POL          : 1;
        FIELD OVERRUN_RST_EN            : 1;
        FIELD SENINF_SRC_SEL            : 4;
        FIELD FIFO_PUSH_EN              : 6;
        FIELD FIFO_FLUSH_EN             : 6;
        FIELD FIFO_FULL_WR_EN           : 2;
        FIELD CROP_EN                   : 1;
        FIELD SENINF_MUX_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_CTRL;

typedef volatile union _REG_SENINF4_MUX_INTEN_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_EN     : 1;
        FIELD SENINF_CRCERR_IRQ_EN      : 1;
        FIELD SENINF_FSMERR_IRQ_EN      : 1;
        FIELD SENINF_VSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_HSIZEERR_IRQ_EN    : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_EN : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_EN : 1;
        FIELD rsv_7                     : 24;
        FIELD SENINF_IRQ_CLR_SEL        : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_INTEN;

typedef volatile union _REG_SENINF4_MUX_INTSTA_
{
    volatile struct
    {
        FIELD SENINF_OVERRUN_IRQ_STA    : 1;
        FIELD SENINF_CRCERR_IRQ_STA     : 1;
        FIELD SENINF_FSMERR_IRQ_STA     : 1;
        FIELD SENINF_VSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_HSIZEERR_IRQ_STA   : 1;
        FIELD SENINF_SENSOR_VSIZEERR_IRQ_STA : 1;
        FIELD SENINF_SENSOR_HSIZEERR_IRQ_STA : 1;
        FIELD rsv_7                     : 25;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_INTSTA;

typedef volatile union _REG_SENINF4_MUX_SIZE_
{
    volatile struct
    {
        FIELD SENINF_VSIZE              : 16;
        FIELD SENINF_HSIZE              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_SIZE;

typedef volatile union _REG_SENINF4_MUX_DEBUG_1_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_1;

typedef volatile union _REG_SENINF4_MUX_DEBUG_2_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_2;

typedef volatile union _REG_SENINF4_MUX_DEBUG_3_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_3;

typedef volatile union _REG_SENINF4_MUX_DEBUG_4_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_4;

typedef volatile union _REG_SENINF4_MUX_DEBUG_5_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_5;

typedef volatile union _REG_SENINF4_MUX_DEBUG_6_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_6;

typedef volatile union _REG_SENINF4_MUX_DEBUG_7_
{
    volatile struct
    {
        FIELD DEBUG_INFO                : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DEBUG_7;

typedef volatile union _REG_SENINF4_MUX_SPARE_
{
    volatile struct
    {
        FIELD rsv_0                     : 9;
        FIELD SENINF_CRC_SEL            : 2;
        FIELD SENINF_VCNT_SEL           : 2;
        FIELD SENINF_FIFO_FULL_SEL      : 1;
        FIELD SENINF_SPARE              : 6;
        FIELD rsv_20                    : 12;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_SPARE;

typedef volatile union _REG_SENINF4_MUX_DATA_
{
    volatile struct
    {
        FIELD SENINF_DATA0              : 16;
        FIELD SENINF_DATA1              : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DATA;

typedef volatile union _REG_SENINF4_MUX_DATA_CNT_
{
    volatile struct
    {
        FIELD SENINF_DATA_CNT           : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_DATA_CNT;

typedef volatile union _REG_SENINF4_MUX_CROP_
{
    volatile struct
    {
        FIELD SENINF_CROP_X1            : 16;
        FIELD SENINF_CROP_X2            : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF4_MUX_CROP;

/* end MT6593_SENINF_MUX_CODA.xml*/

/* start MT6593_SENINF_TG_CODA.xml*/
typedef volatile union _SENINF_REG_TG1_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_PH_CNT;

typedef volatile union _SENINF_REG_TG1_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_SEN_CK;

typedef volatile union _SENINF_REG_TG1_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_CTL;

typedef volatile union _SENINF_REG_TG1_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_SIZE;

typedef volatile union _SENINF_REG_TG1_TM_CLK_
{
    volatile struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG1_TM_CLK;

typedef volatile union _SENINF_REG_TG2_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_PH_CNT;

typedef volatile union _SENINF_REG_TG2_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_SEN_CK;

typedef volatile union _SENINF_REG_TG2_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_CTL;

typedef volatile union _SENINF_REG_TG2_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_SIZE;

typedef volatile union _SENINF_REG_TG2_TM_CLK_
{
    volatile struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG2_TM_CLK;

typedef volatile union _SENINF_REG_TG3_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_PH_CNT;

typedef volatile union _SENINF_REG_TG3_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_SEN_CK;

typedef volatile union _SENINF_REG_TG3_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_TM_CTL;

typedef volatile union _SENINF_REG_TG3_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_TM_SIZE;

typedef volatile union _SENINF_REG_TG3_TM_CLK_
{
    volatile struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG3_TM_CLK;

typedef volatile union _SENINF_REG_TG4_PH_CNT_
{
    volatile struct
    {
        FIELD TGCLK_SEL                 : 2;
        FIELD CLKFL_POL                 : 1;
        FIELD rsv_3                     : 1;
        FIELD EXT_RST                   : 1;
        FIELD EXT_PWRDN                 : 1;
        FIELD PAD_PCLK_INV              : 1;
        FIELD CAM_PCLK_INV              : 1;
        FIELD rsv_8                     : 20;
        FIELD CLKPOL                    : 1;
        FIELD ADCLK_EN                  : 1;
        FIELD rsv_30                    : 1;
        FIELD PCEN                      : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_PH_CNT;

typedef volatile union _SENINF_REG_TG4_SEN_CK_
{
    volatile struct
    {
        FIELD CLKFL                     : 6;
        FIELD rsv_6                     : 2;
        FIELD CLKRS                     : 6;
        FIELD rsv_14                    : 2;
        FIELD CLKCNT                    : 6;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_SEN_CK;

typedef volatile union _SENINF_REG_TG4_TM_CTL_
{
    volatile struct
    {
        FIELD TM_EN                     : 1;
        FIELD TM_RST                    : 1;
        FIELD TM_FMT                    : 1;
        FIELD rsv_3                     : 1;
        FIELD TM_PAT                    : 4;
        FIELD TM_VSYNC                  : 8;
        FIELD TM_DUMMYPXL               : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_TM_CTL;

typedef volatile union _SENINF_REG_TG4_TM_SIZE_
{
    volatile struct
    {
        FIELD TM_PXL                    : 13;
        FIELD rsv_13                    : 3;
        FIELD TM_LINE                   : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_TM_SIZE;

typedef volatile union _SENINF_REG_TG4_TM_CLK_
{
    volatile struct
    {
        FIELD TM_CLK_CNT                : 4;
        FIELD rsv_4                     : 12;
        FIELD TM_CLRBAR_OFT             : 10;
        FIELD rsv_26                    : 2;
        FIELD TM_CLRBAR_IDX             : 3;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} SENINF_REG_TG4_TM_CLK;

/* end MT6593_SENINF_TG_CODA.xml*/

/* start MT6593_SENINF_CCIR656_CODA.xml*/
typedef volatile union _REG_CCIR656_CTL_
{
    volatile struct
    {
        FIELD CCIR656_REV_0             : 1;
        FIELD CCIR656_REV_1             : 1;
        FIELD CCIR656_HS_POL            : 1;
        FIELD CCIR656_VS_POL            : 1;
        FIELD CCIR656_PT_EN             : 1;
        FIELD CCIR656_EN                : 1;
        FIELD rsv_6                     : 2;
        FIELD CCIR656_DBG_SEL           : 4;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_CTL;

typedef volatile union _REG_CCIR656_H_
{
    volatile struct
    {
        FIELD CCIR656_HS_START          : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_HS_END            : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_H;

typedef volatile union _REG_CCIR656_PTGEN_H_1_
{
    volatile struct
    {
        FIELD CCIR656_PT_HTOTAL         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HACTIVE        : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_1;

typedef volatile union _REG_CCIR656_PTGEN_H_2_
{
    volatile struct
    {
        FIELD CCIR656_PT_HWIDTH         : 13;
        FIELD rsv_13                    : 3;
        FIELD CCIR656_PT_HSTART         : 13;
        FIELD rsv_29                    : 3;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_H_2;

typedef volatile union _REG_CCIR656_PTGEN_V_1_
{
    volatile struct
    {
        FIELD CCIR656_PT_VTOTAL         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VACTIVE        : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_1;

typedef volatile union _REG_CCIR656_PTGEN_V_2_
{
    volatile struct
    {
        FIELD CCIR656_PT_VWIDTH         : 12;
        FIELD rsv_12                    : 4;
        FIELD CCIR656_PT_VSTART         : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_V_2;

typedef volatile union _REG_CCIR656_PTGEN_CTL1_
{
    volatile struct
    {
        FIELD CCIR656_PT_TYPE           : 8;
        FIELD rsv_8                     : 8;
        FIELD CCIR656_PT_COLOR_BAR_TH   : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL1;

typedef volatile union _REG_CCIR656_PTGEN_CTL2_
{
    volatile struct
    {
        FIELD CCIR656_PT_Y              : 8;
        FIELD CCIR656_PT_CB             : 8;
        FIELD CCIR656_PT_CR             : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL2;

typedef volatile union _REG_CCIR656_PTGEN_CTL3_
{
    volatile struct
    {
        FIELD CCIR656_PT_BD_Y           : 8;
        FIELD CCIR656_PT_BD_CB          : 8;
        FIELD CCIR656_PT_BD_CR          : 8;
        FIELD rsv_24                    : 8;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_PTGEN_CTL3;

typedef volatile union _REG_CCIR656_STATUS_
{
    volatile struct
    {
        FIELD CCIR656_IN_FIELD          : 1;
        FIELD CCIR656_IN_VS             : 1;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_CCIR656_STATUS;

/* end MT6593_SENINF_CCIR656_CODA.xml*/

/* start MT6593_MIPI_RX_CONFIG_CODA.xml*/
typedef volatile union _REG_MIPI_RX_CON00_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD RG_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRC_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON00_CSI0;

typedef volatile union _REG_MIPI_RX_CON04_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD0_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON04_CSI0;

typedef volatile union _REG_MIPI_RX_CON08_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD1_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON08_CSI0;

typedef volatile union _REG_MIPI_RX_CON0C_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD2_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON0C_CSI0;

typedef volatile union _REG_MIPI_RX_CON10_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD3_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON10_CSI0;

typedef volatile union _REG_MIPI_RX_CON24_CSI0_
{
    volatile struct
    {
        FIELD CSI0_BIST_NUM             : 2;
        FIELD CSI0_BIST_EN              : 1;
        FIELD rsv_3                     : 2;
        FIELD CSI0_BIST_FIX_PAT         : 1;
        FIELD CSI0_BIST_CLK_SEL         : 1;
        FIELD CSI0_BIST_CLK4X_SEL       : 1;
        FIELD CSI0_BIST_TERM_DELAY      : 8;
        FIELD CSI0_BIST_SETTLE_DELAY    : 8;
        FIELD CSI0_BIST_LN0_MUX         : 2;
        FIELD CSI0_BIST_LN1_MUX         : 2;
        FIELD CSI0_BIST_LN2_MUX         : 2;
        FIELD CSI0_BIST_LN3_MUX         : 2;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON24_CSI0;

typedef volatile union _REG_MIPI_RX_CON28_CSI0_
{
    volatile struct
    {
        FIELD CSI0_BIST_START           : 1;
        FIELD CSI0_BIST_DATA_OK         : 1;
        FIELD CSI0_BIST_HS_FSM_OK       : 1;
        FIELD CSI0_BIST_LANE_FSM_OK     : 1;
        FIELD CSI0_BIST_CSI2_DATA_OK    : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON28_CSI0;

typedef volatile union _REG_MIPI_RX_CON34_CSI0_
{
    volatile struct
    {
        FIELD BIST_MODE                 : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON34_CSI0;

typedef volatile union _REG_MIPI_RX_CON38_CSI0_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_MODE      : 1;
        FIELD MIPI_RX_SW_CAL_MODE       : 1;
        FIELD MIPI_RX_HW_CAL_START      : 1;
        FIELD MIPI_RX_HW_CAL_OPTION     : 1;
        FIELD MIPI_RX_HW_CAL_SW_RST     : 1;
        FIELD MIPI_RX_MACRO_SRC_SEL     : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON38_CSI0;

typedef volatile union _REG_MIPI_RX_CON3C_CSI0_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_          : 32;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON3C_CSI0;

typedef volatile union _REG_MIPI_RX_CON40_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_LPRX_SWAP    : 1;
        FIELD RG_CSI0_LNRC_HSRX_INVERT  : 1;
        FIELD rsv_2                     : 2;
        FIELD RG_CSI0_LNRD0_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD0_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD1_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD1_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD2_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD2_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD3_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD3_HSRX_INVERT : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON40_CSI0;

typedef volatile union _REG_MIPI_RX_CON44_CSI0_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD DA_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON44_CSI0;

typedef volatile union _REG_MIPI_RX_CON48_CSI0_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON48_CSI0;

typedef volatile union _REG_MIPI_RX_CON50_CSI0_
{
    volatile struct
    {
        FIELD RG_CSI0_BCLK_INV          : 1;
        FIELD rsv_1                     : 3;
        FIELD RG_CSI0_BCLK_MON          : 1;
        FIELD RG_CSI0_4XCLK_MON         : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON50_CSI0;

typedef volatile union _REG_MIPI_RX_CON00_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD RG_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRC_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON00_CSI1;

typedef volatile union _REG_MIPI_RX_CON04_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD0_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON04_CSI1;

typedef volatile union _REG_MIPI_RX_CON08_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD1_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON08_CSI1;

typedef volatile union _REG_MIPI_RX_CON0C_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD2_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON0C_CSI1;

typedef volatile union _REG_MIPI_RX_CON10_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD3_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON10_CSI1;

typedef volatile union _REG_MIPI_RX_CON24_CSI1_
{
    volatile struct
    {
        FIELD CSI0_BIST_NUM             : 2;
        FIELD CSI0_BIST_EN              : 1;
        FIELD rsv_3                     : 2;
        FIELD CSI0_BIST_FIX_PAT         : 1;
        FIELD CSI0_BIST_CLK_SEL         : 1;
        FIELD CSI0_BIST_CLK4X_SEL       : 1;
        FIELD CSI0_BIST_TERM_DELAY      : 8;
        FIELD CSI0_BIST_SETTLE_DELAY    : 8;
        FIELD CSI0_BIST_LN0_MUX         : 2;
        FIELD CSI0_BIST_LN1_MUX         : 2;
        FIELD CSI0_BIST_LN2_MUX         : 2;
        FIELD CSI0_BIST_LN3_MUX         : 2;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON24_CSI1;

typedef volatile union _REG_MIPI_RX_CON28_CSI1_
{
    volatile struct
    {
        FIELD CSI0_BIST_START           : 1;
        FIELD CSI0_BIST_DATA_OK         : 1;
        FIELD CSI0_BIST_HS_FSM_OK       : 1;
        FIELD CSI0_BIST_LANE_FSM_OK     : 1;
        FIELD CSI0_BIST_CSI2_DATA_OK    : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON28_CSI1;

typedef volatile union _REG_MIPI_RX_CON34_CSI1_
{
    volatile struct
    {
        FIELD BIST_MODE                 : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON34_CSI1;

typedef volatile union _REG_MIPI_RX_CON38_CSI1_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_MODE      : 1;
        FIELD MIPI_RX_SW_CAL_MODE       : 1;
        FIELD MIPI_RX_HW_CAL_START      : 1;
        FIELD MIPI_RX_HW_CAL_OPTION     : 1;
        FIELD MIPI_RX_HW_CAL_SW_RST     : 1;
        FIELD MIPI_RX_MACRO_SRC_SEL     : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON38_CSI1;

typedef volatile union _REG_MIPI_RX_CON3C_CSI1_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_          : 32;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON3C_CSI1;

typedef volatile union _REG_MIPI_RX_CON40_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_LPRX_SWAP    : 1;
        FIELD RG_CSI0_LNRC_HSRX_INVERT  : 1;
        FIELD rsv_2                     : 2;
        FIELD RG_CSI0_LNRD0_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD0_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD1_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD1_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD2_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD2_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD3_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD3_HSRX_INVERT : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON40_CSI1;

typedef volatile union _REG_MIPI_RX_CON44_CSI1_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD DA_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON44_CSI1;

typedef volatile union _REG_MIPI_RX_CON48_CSI1_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON48_CSI1;

typedef volatile union _REG_MIPI_RX_CON50_CSI1_
{
    volatile struct
    {
        FIELD RG_CSI0_BCLK_INV          : 1;
        FIELD rsv_1                     : 3;
        FIELD RG_CSI0_BCLK_MON          : 1;
        FIELD RG_CSI0_4XCLK_MON         : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON50_CSI1;

typedef volatile union _REG_MIPI_RX_CON00_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD RG_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRC_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON00_CSI2;

typedef volatile union _REG_MIPI_RX_CON04_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD0_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON04_CSI2;

typedef volatile union _REG_MIPI_RX_CON08_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD1_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON08_CSI2;

typedef volatile union _REG_MIPI_RX_CON0C_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD2_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON0C_CSI2;

typedef volatile union _REG_MIPI_RX_CON10_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD RG_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD RG_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD AD_CSI0_LNRD3_HSRX_OFFSET_OUT : 1;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON10_CSI2;

typedef volatile union _REG_MIPI_RX_CON24_CSI2_
{
    volatile struct
    {
        FIELD CSI0_BIST_NUM             : 2;
        FIELD CSI0_BIST_EN              : 1;
        FIELD rsv_3                     : 2;
        FIELD CSI0_BIST_FIX_PAT         : 1;
        FIELD CSI0_BIST_CLK_SEL         : 1;
        FIELD CSI0_BIST_CLK4X_SEL       : 1;
        FIELD CSI0_BIST_TERM_DELAY      : 8;
        FIELD CSI0_BIST_SETTLE_DELAY    : 8;
        FIELD CSI0_BIST_LN0_MUX         : 2;
        FIELD CSI0_BIST_LN1_MUX         : 2;
        FIELD CSI0_BIST_LN2_MUX         : 2;
        FIELD CSI0_BIST_LN3_MUX         : 2;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON24_CSI2;

typedef volatile union _REG_MIPI_RX_CON28_CSI2_
{
    volatile struct
    {
        FIELD CSI0_BIST_START           : 1;
        FIELD CSI0_BIST_DATA_OK         : 1;
        FIELD CSI0_BIST_HS_FSM_OK       : 1;
        FIELD CSI0_BIST_LANE_FSM_OK     : 1;
        FIELD CSI0_BIST_CSI2_DATA_OK    : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON28_CSI2;

typedef volatile union _REG_MIPI_RX_CON34_CSI2_
{
    volatile struct
    {
        FIELD BIST_MODE                 : 1;
        FIELD rsv_1                     : 31;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON34_CSI2;

typedef volatile union _REG_MIPI_RX_CON38_CSI2_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_MODE      : 1;
        FIELD MIPI_RX_SW_CAL_MODE       : 1;
        FIELD MIPI_RX_HW_CAL_START      : 1;
        FIELD MIPI_RX_HW_CAL_OPTION     : 1;
        FIELD MIPI_RX_HW_CAL_SW_RST     : 1;
        FIELD MIPI_RX_MACRO_SRC_SEL     : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON38_CSI2;

typedef volatile union _REG_MIPI_RX_CON3C_CSI2_
{
    volatile struct
    {
        FIELD MIPI_RX_SW_CTRL_          : 32;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON3C_CSI2;

typedef volatile union _REG_MIPI_RX_CON40_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_LNRC_LPRX_SWAP    : 1;
        FIELD RG_CSI0_LNRC_HSRX_INVERT  : 1;
        FIELD rsv_2                     : 2;
        FIELD RG_CSI0_LNRD0_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD0_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD1_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD1_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD2_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD2_HSRX_INVERT : 1;
        FIELD RG_CSI0_LNRD3_LPRX_SWAP   : 1;
        FIELD RG_CSI0_LNRD3_HSRX_INVERT : 1;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON40_CSI2;

typedef volatile union _REG_MIPI_RX_CON44_CSI2_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD0_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD0_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD0_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRC_HSRX_CAL_EN  : 1;
        FIELD DA_CSI0_LNRC_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD1_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD1_HSRX_OFFSET_CODE : 5;
        FIELD rsv_23                    : 9;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON44_CSI2;

typedef volatile union _REG_MIPI_RX_CON48_CSI2_
{
    volatile struct
    {
        FIELD DA_CSI0_LNRD2_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD2_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD2_HSRX_OFFSET_CODE : 5;
        FIELD rsv_7                     : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_APPLY : 1;
        FIELD DA_CSI0_LNRD3_HSRX_CAL_EN : 1;
        FIELD DA_CSI0_LNRD3_HSRX_OFFSET_CODE : 5;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON48_CSI2;

typedef volatile union _REG_MIPI_RX_CON50_CSI2_
{
    volatile struct
    {
        FIELD RG_CSI0_BCLK_INV          : 1;
        FIELD rsv_1                     : 3;
        FIELD RG_CSI0_BCLK_MON          : 1;
        FIELD RG_CSI0_4XCLK_MON         : 1;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_MIPI_RX_CON50_CSI2;

/* end MT6593_MIPI_RX_CONFIG_CODA.xml*/

/* start MT6593_SENINF_CSI2_CODA.xml*/
typedef volatile union _REG_SENINF1_CSI2_CTRL_
{
    volatile struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CTRL;

typedef volatile union _REG_SENINF1_CSI2_DELAY_
{
    volatile struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DELAY;

typedef volatile union _REG_SENINF1_CSI2_INTEN_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTEN;

typedef volatile union _REG_SENINF1_CSI2_INTSTA_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_INTSTA;

typedef volatile union _REG_SENINF1_CSI2_ECCDBG_
{
    volatile struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_ECCDBG;

typedef volatile union _REG_SENINF1_CSI2_CRCDBG_
{
    volatile struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CRCDBG;

typedef volatile union _REG_SENINF1_CSI2_DBG_
{
    volatile struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DBG;

typedef volatile union _REG_SENINF1_CSI2_VER_
{
    volatile struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VER;

typedef volatile union _REG_SENINF1_CSI2_SHORT_INFO_
{
    volatile struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_SHORT_INFO;

typedef volatile union _REG_SENINF1_CSI2_LNFSM_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNFSM;

typedef volatile union _REG_SENINF1_CSI2_LNMUX_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_LNMUX;

typedef volatile union _REG_SENINF1_CSI2_HSYNC_CNT_
{
    volatile struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_HSYNC_CNT;

typedef volatile union _REG_SENINF1_CSI2_CAL_
{
    volatile struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_CAL;

typedef volatile union _REG_SENINF1_CSI2_DS_
{
    volatile struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_DS;

typedef volatile union _REG_SENINF1_CSI2_VS_
{
    volatile struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_VS;

typedef volatile union _REG_SENINF1_CSI2_BIST_
{
    volatile struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_CSI2_BIST;

typedef volatile union _REG_SENINF2_CSI2_CTRL_
{
    volatile struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CTRL;

typedef volatile union _REG_SENINF2_CSI2_DELAY_
{
    volatile struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DELAY;

typedef volatile union _REG_SENINF2_CSI2_INTEN_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_INTEN;

typedef volatile union _REG_SENINF2_CSI2_INTSTA_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_INTSTA;

typedef volatile union _REG_SENINF2_CSI2_ECCDBG_
{
    volatile struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_ECCDBG;

typedef volatile union _REG_SENINF2_CSI2_CRCDBG_
{
    volatile struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CRCDBG;

typedef volatile union _REG_SENINF2_CSI2_DBG_
{
    volatile struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DBG;

typedef volatile union _REG_SENINF2_CSI2_VER_
{
    volatile struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_VER;

typedef volatile union _REG_SENINF2_CSI2_SHORT_INFO_
{
    volatile struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_SHORT_INFO;

typedef volatile union _REG_SENINF2_CSI2_LNFSM_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_LNFSM;

typedef volatile union _REG_SENINF2_CSI2_LNMUX_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_LNMUX;

typedef volatile union _REG_SENINF2_CSI2_HSYNC_CNT_
{
    volatile struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_HSYNC_CNT;

typedef volatile union _REG_SENINF2_CSI2_CAL_
{
    volatile struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_CAL;

typedef volatile union _REG_SENINF2_CSI2_DS_
{
    volatile struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_DS;

typedef volatile union _REG_SENINF2_CSI2_VS_
{
    volatile struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_VS;

typedef volatile union _REG_SENINF2_CSI2_BIST_
{
    volatile struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_CSI2_BIST;

typedef volatile union _REG_SENINF3_CSI2_CTRL_
{
    volatile struct
    {
        FIELD CSI2_EN                   : 1;
        FIELD DLANE1_EN                 : 1;
        FIELD DLANE2_EN                 : 1;
        FIELD DLANE3_EN                 : 1;
        FIELD CSI2_ECC_EN               : 1;
        FIELD CSI2_ED_SEL               : 1;
        FIELD CSI2_CLK_MISS_EN          : 1;
        FIELD CSI2_LP11_RST_EN          : 1;
        FIELD CSI2_SYNC_RST_EN          : 1;
        FIELD CSI2_ESC_EN               : 1;
        FIELD CSI2_SCLK_SEL             : 1;
        FIELD CSI2_SCLK4X_SEL           : 1;
        FIELD CSI2_SW_RST               : 1;
        FIELD CSI2_VSYNC_TYPE           : 1;
        FIELD CSI2_HSRXEN_PFOOT_CLR     : 1;
        FIELD CSI2_SYNC_CLR_EXTEND      : 1;
        FIELD CSI2_ASYNC_OPTION         : 1;
        FIELD CSI2_DATA_FLOW            : 2;
        FIELD CSI2_BIST_ERROR_COUNT     : 8;
        FIELD CSI2_BIST_START           : 1;
        FIELD CSI2_BIST_DATA_OK         : 1;
        FIELD CSI2_HS_FSM_OK            : 1;
        FIELD CSI2_LANE_FSM_OK          : 1;
        FIELD CSI2_BIST_CSI2_DATA_OK    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_CTRL;

typedef volatile union _REG_SENINF3_CSI2_DELAY_
{
    volatile struct
    {
        FIELD LP2HS_CLK_TERM_DELAY      : 8;
        FIELD rsv_8                     : 8;
        FIELD LP2HS_DATA_SETTLE_DELAY   : 8;
        FIELD LP2HS_DATA_TERM_DELAY     : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_DELAY;

typedef volatile union _REG_SENINF3_CSI2_INTEN_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ_EN            : 1;
        FIELD ECC_ERR_IRQ_EN            : 1;
        FIELD ECC_CORRECT_IRQ_EN        : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ_EN   : 1;
        FIELD rsv_4                     : 4;
        FIELD CSI2_WC_NUMBER            : 16;
        FIELD CSI2_DATA_TYPE            : 6;
        FIELD VCHANNEL_ID               : 2;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_INTEN;

typedef volatile union _REG_SENINF3_CSI2_INTSTA_
{
    volatile struct
    {
        FIELD CRC_ERR_IRQ               : 1;
        FIELD ECC_ERR_IRQ               : 1;
        FIELD ECC_CORRECT_IRQ           : 1;
        FIELD CSI2SYNC_NONSYNC_IRQ      : 1;
        FIELD CSI2_IRQ_CLR_SEL          : 1;
        FIELD CSI2_SPARE                : 3;
        FIELD rsv_8                     : 12;
        FIELD CSI2OUT_HSYNC             : 1;
        FIELD CSI2OUT_VSYNC             : 1;
        FIELD rsv_22                    : 10;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_INTSTA;

typedef volatile union _REG_SENINF3_CSI2_ECCDBG_
{
    volatile struct
    {
        FIELD CSI2_ECCDB_EN             : 1;
        FIELD rsv_1                     : 7;
        FIELD CSI2_ECCDB_BSEL           : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_ECCDBG;

typedef volatile union _REG_SENINF3_CSI2_CRCDBG_
{
    volatile struct
    {
        FIELD CSI2_CRCDB_EN             : 1;
        FIELD CSI2_SPARE                : 7;
        FIELD CSI2_CRCDB_WSEL           : 16;
        FIELD CSI2_CRCDB_BSEL           : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_CRCDBG;

typedef volatile union _REG_SENINF3_CSI2_DBG_
{
    volatile struct
    {
        FIELD CSI2_DEBUG_ON             : 1;
        FIELD CSI2_DBG_SRC_SEL          : 4;
        FIELD CSI2_DATA_HS_CS           : 6;
        FIELD CSI2_CLK_LANE_CS          : 5;
        FIELD VCHANNEL0_ID              : 2;
        FIELD VCHANNEL1_ID              : 2;
        FIELD VCHANNEL_ID_EN            : 1;
        FIELD rsv_21                    : 1;
        FIELD LNC_LPRXDB_EN             : 1;
        FIELD LN0_LPRXDB_EN             : 1;
        FIELD LN1_LPRXDB_EN             : 1;
        FIELD LN2_LPRXDB_EN             : 1;
        FIELD LN3_LPRXDB_EN             : 1;
        FIELD LNC_HSRXDB_EN             : 1;
        FIELD LN0_HSRXDB_EN             : 1;
        FIELD LN1_HSRXDB_EN             : 1;
        FIELD LN2_HSRXDB_EN             : 1;
        FIELD LN3_HSRXDB_EN             : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_DBG;

typedef volatile union _REG_SENINF3_CSI2_VER_
{
    volatile struct
    {
        FIELD DATE                      : 8;
        FIELD MONTH                     : 8;
        FIELD YEAR                      : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_VER;

typedef volatile union _REG_SENINF3_CSI2_SHORT_INFO_
{
    volatile struct
    {
        FIELD CSI2_LINE_NO              : 16;
        FIELD CSI2_FRAME_NO             : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_SHORT_INFO;

typedef volatile union _REG_SENINF3_CSI2_LNFSM_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_CS          : 7;
        FIELD rsv_7                     : 1;
        FIELD CSI2_DATA_LN1_CS          : 7;
        FIELD rsv_15                    : 1;
        FIELD CSI2_DATA_LN2_CS          : 7;
        FIELD rsv_23                    : 1;
        FIELD CSI2_DATA_LN3_CS          : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_LNFSM;

typedef volatile union _REG_SENINF3_CSI2_LNMUX_
{
    volatile struct
    {
        FIELD CSI2_DATA_LN0_MUX         : 2;
        FIELD CSI2_DATA_LN1_MUX         : 2;
        FIELD CSI2_DATA_LN2_MUX         : 2;
        FIELD CSI2_DATA_LN3_MUX         : 2;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_LNMUX;

typedef volatile union _REG_SENINF3_CSI2_HSYNC_CNT_
{
    volatile struct
    {
        FIELD CSI2_HSYNC_CNT            : 13;
        FIELD rsv_13                    : 19;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_HSYNC_CNT;

typedef volatile union _REG_SENINF3_CSI2_CAL_
{
    volatile struct
    {
        FIELD CSI2_CAL_EN               : 1;
        FIELD rsv_1                     : 3;
        FIELD CSI2_CAL_STATE            : 3;
        FIELD rsv_7                     : 9;
        FIELD CSI2_CAL_CNT_1            : 8;
        FIELD CSI2_CAL_CNT_2            : 8;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_CAL;

typedef volatile union _REG_SENINF3_CSI2_DS_
{
    volatile struct
    {
        FIELD CSI2_DS_EN                : 1;
        FIELD CSI2_DS_CTRL              : 2;
        FIELD rsv_3                     : 29;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_DS;

typedef volatile union _REG_SENINF3_CSI2_VS_
{
    volatile struct
    {
        FIELD CSI2_VS_CTRL              : 2;
        FIELD rsv_2                     : 30;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_VS;

typedef volatile union _REG_SENINF3_CSI2_BIST_
{
    volatile struct
    {
        FIELD CSI2_BIST_LNR0_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR1_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR2_DATA_OK    : 1;
        FIELD CSI2_BIST_LNR3_DATA_OK    : 1;
        FIELD rsv_4                     : 28;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_CSI2_BIST;

/* end MT6593_SENINF_CSI2_CODA.xml*/

/* start MT6593_ncsi2.xml*/
typedef volatile union _REG_SENINF1_NCSI2_CTL_
{
    volatile struct /* 0x150083A0 */
    {
        FIELD  DATA_LANE0_EN            :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DATA_LANE1_EN            :  1;      /*  1.. 1, 0x00000002 */
        FIELD  DATA_LANE2_EN            :  1;      /*  2.. 2, 0x00000004 */
        FIELD  DATA_LANE3_EN            :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CLOCK_LANE_EN            :  1;      /*  4.. 4, 0x00000010 */
        FIELD  ECC_EN                   :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CRC_EN                   :  1;      /*  6.. 6, 0x00000040 */
        FIELD  HSRX_DET_EN              :  1;      /*  7.. 7, 0x00000080 */
        FIELD  HS_PRPR_EN               :  1;      /*  8.. 8, 0x00000100 */
        FIELD  HS_END_EN                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  rsv_10                   :  2;      /* 10..11, 0x00000C00 */
        FIELD  GENERIC_LONG_PACKET_EN   :  1;      /* 12..12, 0x00001000 */
        FIELD  IMAGE_PACKET_EN          :  1;      /* 13..13, 0x00002000 */
        FIELD  BYTE2PIXEL_EN            :  1;      /* 14..14, 0x00004000 */
        FIELD  VS_TYPE                  :  1;      /* 15..15, 0x00008000 */
        FIELD  ED_SEL                   :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                   :  1;      /* 17..17, 0x00020000 */
        FIELD  FLUSH_MODE               :  2;      /* 18..19, 0x000C0000 */
        FIELD  SYNC_DET_SCHEME          :  1;      /* 20..20, 0x00100000 */
        FIELD  SYNC_DET_EN              :  1;      /* 21..21, 0x00200000 */
        FIELD  SYNC_DET_BITSWAP_EN      :  1;      /* 22..22, 0x00400000 */
        FIELD  ASYNC_FIFO_RST_SCH       :  2;      /* 23..24, 0x01800000 */
        FIELD  HS_TRAIL_EN              :  1;      /* 25..25, 0x02000000 */
        FIELD  REF_SYNC_DET_EN          :  1;      /* 26..26, 0x04000000 */
        FIELD  CLOCK_HS_OPTION          :  1;      /* 27..27, 0x08000000 */
        FIELD  VS_OUT_CYCLE_NUMBER      :  2;      /* 28..29, 0x30000000 */
        FIELD  rsv_30                   :  2;      /* 30..31, 0xC0000000 */
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_CTL;

typedef volatile union _REG_SENINF1_NCSI2_LNRC_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_TIMING;

typedef volatile union _REG_SENINF1_NCSI2_LNRD_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRD_TIMING;

typedef volatile union _REG_SENINF1_NCSI2_DPCM_
{
    volatile struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 3;
        FIELD DI_30_DPCM_EN             : 1;
        FIELD DI_31_DPCM_EN             : 1;
        FIELD DI_32_DPCM_EN             : 1;
        FIELD DI_33_DPCM_EN             : 1;
        FIELD DI_34_DPCM_EN             : 1;
        FIELD DI_35_DPCM_EN             : 1;
        FIELD DI_36_DPCM_EN             : 1;
        FIELD DI_37_DPCM_EN             : 1;
        FIELD DI_2A_DPCM_EN             : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DPCM;

typedef volatile union _REG_SENINF1_NCSI2_INT_EN_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 16;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_INT_EN;

typedef volatile union _REG_SENINF1_NCSI2_INT_STATUS_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_INT_STATUS;

typedef volatile union _REG_SENINF1_NCSI2_DGB_SEL_
{
    volatile struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DGB_SEL;

typedef volatile union _REG_SENINF1_NCSI2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DBG_PORT;

typedef volatile union _REG_SENINF1_NCSI2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE0;

typedef volatile union _REG_SENINF1_NCSI2_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_SPARE1;

typedef volatile union _REG_SENINF1_NCSI2_LNRC_FSM_
{
    volatile struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRC_FSM;

typedef volatile union _REG_SENINF1_NCSI2_LNRD_FSM_
{
    volatile struct
    {
        FIELD LNRD0_RX_FSM              : 7;
        FIELD rsv_7                     : 1;
        FIELD LNRD1_RX_FSM              : 7;
        FIELD rsv_15                    : 1;
        FIELD LNRD2_RX_FSM              : 7;
        FIELD rsv_23                    : 1;
        FIELD LNRD3_RX_FSM              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_LNRD_FSM;

typedef volatile union _REG_SENINF1_NCSI2_FRAME_LINE_NUM_
{
    volatile struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_FRAME_LINE_NUM;

typedef volatile union _REG_SENINF1_NCSI2_GENERIC_SHORT_
{
    volatile struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_GENERIC_SHORT;

typedef volatile union _REG_SENINF1_NCSI2_HSRX_DBG_
{
    volatile struct
    {
        FIELD DATA_LANE0_HSRX_EN        : 1;
        FIELD DATA_LANE1_HSRX_EN        : 1;
        FIELD DATA_LANE2_HSRX_EN        : 1;
        FIELD DATA_LANE3_HSRX_EN        : 1;
        FIELD CLOCK_LANE_HSRX_EN        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_HSRX_DBG;

typedef volatile union _REG_SENINF1_NCSI2_DI_
{
    volatile struct
    {
        FIELD VC0                       : 2;
        FIELD DT0                       : 6;
        FIELD VC1                       : 2;
        FIELD DT1                       : 6;
        FIELD VC2                       : 2;
        FIELD DT2                       : 6;
        FIELD VC3                       : 2;
        FIELD DT3                       : 6;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DI;

typedef volatile union _REG_SENINF1_NCSI2_HS_TRAIL_
{
    volatile struct
    {
        FIELD HS_TRAIL_PARAMETER        : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_HS_TRAIL;

typedef volatile union _REG_SENINF1_NCSI2_DI_CTRL_
{
    volatile struct
    {
        FIELD VC0_INTERLEAVING          : 1;
        FIELD DT0_INTERLEAVING          : 2;
        FIELD rsv_3                     : 5;
        FIELD VC1_INTERLEAVING          : 1;
        FIELD DT1_INTERLEAVING          : 2;
        FIELD rsv_11                    : 5;
        FIELD VC2_INTERLEAVING          : 1;
        FIELD DT2_INTERLEAVING          : 2;
        FIELD rsv_19                    : 5;
        FIELD VC3_INTERLEAVING          : 1;
        FIELD DT3_INTERLEAVING          : 2;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_SENINF1_NCSI2_DI_CTRL;

typedef volatile union _REG_SENINF2_NCSI2_CTL_
{
    volatile struct/* 0x150087A0 */
    {
        FIELD  DATA_LANE0_EN            :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DATA_LANE1_EN            :  1;      /*  1.. 1, 0x00000002 */
        FIELD  DATA_LANE2_EN            :  1;      /*  2.. 2, 0x00000004 */
        FIELD  DATA_LANE3_EN            :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CLOCK_LANE_EN            :  1;      /*  4.. 4, 0x00000010 */
        FIELD  ECC_EN                   :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CRC_EN                   :  1;      /*  6.. 6, 0x00000040 */
        FIELD  HSRX_DET_EN              :  1;      /*  7.. 7, 0x00000080 */
        FIELD  HS_PRPR_EN               :  1;      /*  8.. 8, 0x00000100 */
        FIELD  HS_END_EN                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  rsv_10                   :  2;      /* 10..11, 0x00000C00 */
        FIELD  GENERIC_LONG_PACKET_EN   :  1;      /* 12..12, 0x00001000 */
        FIELD  IMAGE_PACKET_EN          :  1;      /* 13..13, 0x00002000 */
        FIELD  BYTE2PIXEL_EN            :  1;      /* 14..14, 0x00004000 */
        FIELD  VS_TYPE                  :  1;      /* 15..15, 0x00008000 */
        FIELD  ED_SEL                   :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                   :  1;      /* 17..17, 0x00020000 */
        FIELD  FLUSH_MODE               :  2;      /* 18..19, 0x000C0000 */
        FIELD  SYNC_DET_SCHEME          :  1;      /* 20..20, 0x00100000 */
        FIELD  SYNC_DET_EN              :  1;      /* 21..21, 0x00200000 */
        FIELD  SYNC_DET_BITSWAP_EN      :  1;      /* 22..22, 0x00400000 */
        FIELD  ASYNC_FIFO_RST_SCH       :  2;      /* 23..24, 0x01800000 */
        FIELD  HS_TRAIL_EN              :  1;      /* 25..25, 0x02000000 */
        FIELD  REF_SYNC_DET_EN          :  1;      /* 26..26, 0x04000000 */
        FIELD  CLOCK_HS_OPTION          :  1;      /* 27..27, 0x08000000 */
        FIELD  VS_OUT_CYCLE_NUMBER      :  2;      /* 28..29, 0x30000000 */
        FIELD  rsv_30                   :  2;      /* 30..31, 0xC0000000 */
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_CTL;

typedef volatile union _REG_SENINF2_NCSI2_LNRC_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRC_TIMING;

typedef volatile union _REG_SENINF2_NCSI2_LNRD_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRD_TIMING;

typedef volatile union _REG_SENINF2_NCSI2_DPCM_
{
    volatile struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 3;
        FIELD DI_30_DPCM_EN             : 1;
        FIELD DI_31_DPCM_EN             : 1;
        FIELD DI_32_DPCM_EN             : 1;
        FIELD DI_33_DPCM_EN             : 1;
        FIELD DI_34_DPCM_EN             : 1;
        FIELD DI_35_DPCM_EN             : 1;
        FIELD DI_36_DPCM_EN             : 1;
        FIELD DI_37_DPCM_EN             : 1;
        FIELD DI_2A_DPCM_EN             : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DPCM;

typedef volatile union _REG_SENINF2_NCSI2_INT_EN_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 16;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_INT_EN;

typedef volatile union _REG_SENINF2_NCSI2_INT_STATUS_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_INT_STATUS;

typedef volatile union _REG_SENINF2_NCSI2_DGB_SEL_
{
    volatile struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DGB_SEL;

typedef volatile union _REG_SENINF2_NCSI2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DBG_PORT;

typedef volatile union _REG_SENINF2_NCSI2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_SPARE0;

typedef volatile union _REG_SENINF2_NCSI2_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_SPARE1;

typedef volatile union _REG_SENINF2_NCSI2_LNRC_FSM_
{
    volatile struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRC_FSM;

typedef volatile union _REG_SENINF2_NCSI2_LNRD_FSM_
{
    volatile struct
    {
        FIELD LNRD0_RX_FSM              : 7;
        FIELD rsv_7                     : 1;
        FIELD LNRD1_RX_FSM              : 7;
        FIELD rsv_15                    : 1;
        FIELD LNRD2_RX_FSM              : 7;
        FIELD rsv_23                    : 1;
        FIELD LNRD3_RX_FSM              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_LNRD_FSM;

typedef volatile union _REG_SENINF2_NCSI2_FRAME_LINE_NUM_
{
    volatile struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_FRAME_LINE_NUM;

typedef volatile union _REG_SENINF2_NCSI2_GENERIC_SHORT_
{
    volatile struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_GENERIC_SHORT;

typedef volatile union _REG_SENINF2_NCSI2_HSRX_DBG_
{
    volatile struct
    {
        FIELD DATA_LANE0_HSRX_EN        : 1;
        FIELD DATA_LANE1_HSRX_EN        : 1;
        FIELD DATA_LANE2_HSRX_EN        : 1;
        FIELD DATA_LANE3_HSRX_EN        : 1;
        FIELD CLOCK_LANE_HSRX_EN        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_HSRX_DBG;

typedef volatile union _REG_SENINF2_NCSI2_DI_
{
    volatile struct
    {
        FIELD VC0                       : 2;
        FIELD DT0                       : 6;
        FIELD VC1                       : 2;
        FIELD DT1                       : 6;
        FIELD VC2                       : 2;
        FIELD DT2                       : 6;
        FIELD VC3                       : 2;
        FIELD DT3                       : 6;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DI;

typedef volatile union _REG_SENINF2_NCSI2_HS_TRAIL_
{
    volatile struct
    {
        FIELD HS_TRAIL_PARAMETER        : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_HS_TRAIL;

typedef volatile union _REG_SENINF2_NCSI2_DI_CTRL_
{
    volatile struct
    {
        FIELD VC0_INTERLEAVING          : 1;
        FIELD DT0_INTERLEAVING          : 2;
        FIELD rsv_3                     : 5;
        FIELD VC1_INTERLEAVING          : 1;
        FIELD DT1_INTERLEAVING          : 2;
        FIELD rsv_11                    : 5;
        FIELD VC2_INTERLEAVING          : 1;
        FIELD DT2_INTERLEAVING          : 2;
        FIELD rsv_19                    : 5;
        FIELD VC3_INTERLEAVING          : 1;
        FIELD DT3_INTERLEAVING          : 2;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_SENINF2_NCSI2_DI_CTRL;

typedef volatile union _REG_SENINF3_NCSI2_CTL_
{
    volatile struct
    {
        FIELD  DATA_LANE0_EN            :  1;      /*  0.. 0, 0x00000001 */
        FIELD  DATA_LANE1_EN            :  1;      /*  1.. 1, 0x00000002 */
        FIELD  DATA_LANE2_EN            :  1;      /*  2.. 2, 0x00000004 */
        FIELD  DATA_LANE3_EN            :  1;      /*  3.. 3, 0x00000008 */
        FIELD  CLOCK_LANE_EN            :  1;      /*  4.. 4, 0x00000010 */
        FIELD  ECC_EN                   :  1;      /*  5.. 5, 0x00000020 */
        FIELD  CRC_EN                   :  1;      /*  6.. 6, 0x00000040 */
        FIELD  HSRX_DET_EN              :  1;      /*  7.. 7, 0x00000080 */
        FIELD  HS_PRPR_EN               :  1;      /*  8.. 8, 0x00000100 */
        FIELD  HS_END_EN                :  1;      /*  9.. 9, 0x00000200 */
        FIELD  rsv_10                   :  2;      /* 10..11, 0x00000C00 */
        FIELD  GENERIC_LONG_PACKET_EN   :  1;      /* 12..12, 0x00001000 */
        FIELD  IMAGE_PACKET_EN          :  1;      /* 13..13, 0x00002000 */
        FIELD  BYTE2PIXEL_EN            :  1;      /* 14..14, 0x00004000 */
        FIELD  VS_TYPE                  :  1;      /* 15..15, 0x00008000 */
        FIELD  ED_SEL                   :  1;      /* 16..16, 0x00010000 */
        FIELD  rsv_17                   :  1;      /* 17..17, 0x00020000 */
        FIELD  FLUSH_MODE               :  2;      /* 18..19, 0x000C0000 */
        FIELD  SYNC_DET_SCHEME          :  1;      /* 20..20, 0x00100000 */
        FIELD  SYNC_DET_EN              :  1;      /* 21..21, 0x00200000 */
        FIELD  SYNC_DET_BITSWAP_EN      :  1;      /* 22..22, 0x00400000 */
        FIELD  ASYNC_FIFO_RST_SCH       :  2;      /* 23..24, 0x01800000 */
        FIELD  HS_TRAIL_EN              :  1;      /* 25..25, 0x02000000 */
        FIELD  REF_SYNC_DET_EN          :  1;      /* 26..26, 0x04000000 */
        FIELD  CLOCK_HS_OPTION          :  1;      /* 27..27, 0x08000000 */
        FIELD  VS_OUT_CYCLE_NUMBER      :  2;      /* 28..29, 0x30000000 */
        FIELD  rsv_30                   :  2;      /* 30..31, 0xC0000000 */
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_CTL;

typedef volatile union _REG_SENINF3_NCSI2_LNRC_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRC_TIMING;

typedef volatile union _REG_SENINF3_NCSI2_LNRD_TIMING_
{
    volatile struct
    {
        FIELD TERM_PARAMETER            : 8;
        FIELD SETTLE_PARAMETER          : 8;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRD_TIMING;

typedef volatile union _REG_SENINF3_NCSI2_DPCM_
{
    volatile struct
    {
        FIELD DPCM_MODE                 : 4;
        FIELD rsv_4                     : 3;
        FIELD DI_30_DPCM_EN             : 1;
        FIELD DI_31_DPCM_EN             : 1;
        FIELD DI_32_DPCM_EN             : 1;
        FIELD DI_33_DPCM_EN             : 1;
        FIELD DI_34_DPCM_EN             : 1;
        FIELD DI_35_DPCM_EN             : 1;
        FIELD DI_36_DPCM_EN             : 1;
        FIELD DI_37_DPCM_EN             : 1;
        FIELD DI_2A_DPCM_EN             : 1;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DPCM;

typedef volatile union _REG_SENINF3_NCSI2_INT_EN_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 16;
        FIELD INT_WCLR_EN               : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_INT_EN;

typedef volatile union _REG_SENINF3_NCSI2_INT_STATUS_
{
    volatile struct
    {
        FIELD ERR_FRAME_SYNC            : 1;
        FIELD ERR_ID                    : 1;
        FIELD ERR_ECC_NO_ERROR          : 1;
        FIELD ERR_ECC_CORRECTED         : 1;
        FIELD ERR_ECC_DOUBLE            : 1;
        FIELD ERR_CRC                   : 1;
        FIELD ERR_AFIFO                 : 1;
        FIELD ERR_MULTI_LANE_SYNC       : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD0     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD1     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD2     : 1;
        FIELD ERR_SOT_SYNC_HS_LNRD3     : 1;
        FIELD FS                        : 1;
        FIELD LS                        : 1;
        FIELD GS                        : 1;
        FIELD rsv_15                    : 17;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_INT_STATUS;

typedef volatile union _REG_SENINF3_NCSI2_DGB_SEL_
{
    volatile struct
    {
        FIELD DEBUG_SEL                 : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DGB_SEL;

typedef volatile union _REG_SENINF3_NCSI2_DBG_PORT_
{
    volatile struct
    {
        FIELD CTL_DBG_PORT              : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DBG_PORT;

typedef volatile union _REG_SENINF3_NCSI2_SPARE0_
{
    volatile struct
    {
        FIELD SPARE0                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_SPARE0;

typedef volatile union _REG_SENINF3_NCSI2_SPARE1_
{
    volatile struct
    {
        FIELD SPARE1                    : 32;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_SPARE1;

typedef volatile union _REG_SENINF3_NCSI2_LNRC_FSM_
{
    volatile struct
    {
        FIELD LNRC_RX_FSM               : 6;
        FIELD rsv_6                     : 26;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRC_FSM;

typedef volatile union _REG_SENINF3_NCSI2_LNRD_FSM_
{
    volatile struct
    {
        FIELD LNRD0_RX_FSM              : 7;
        FIELD rsv_7                     : 1;
        FIELD LNRD1_RX_FSM              : 7;
        FIELD rsv_15                    : 1;
        FIELD LNRD2_RX_FSM              : 7;
        FIELD rsv_23                    : 1;
        FIELD LNRD3_RX_FSM              : 7;
        FIELD rsv_31                    : 1;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_LNRD_FSM;

typedef volatile union _REG_SENINF3_NCSI2_FRAME_LINE_NUM_
{
    volatile struct
    {
        FIELD FRAME_NUM                 : 16;
        FIELD LINE_NUM                  : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_FRAME_LINE_NUM;

typedef volatile union _REG_SENINF3_NCSI2_GENERIC_SHORT_
{
    volatile struct
    {
        FIELD GENERIC_SHORT_PACKET_DT   : 6;
        FIELD rsv_6                     : 10;
        FIELD GENERIC_SHORT_PACKET_DATA : 16;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_GENERIC_SHORT;

typedef volatile union _REG_SENINF3_NCSI2_HSRX_DBG_
{
    volatile struct
    {
        FIELD DATA_LANE0_HSRX_EN        : 1;
        FIELD DATA_LANE1_HSRX_EN        : 1;
        FIELD DATA_LANE2_HSRX_EN        : 1;
        FIELD DATA_LANE3_HSRX_EN        : 1;
        FIELD CLOCK_LANE_HSRX_EN        : 1;
        FIELD rsv_5                     : 27;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_HSRX_DBG;

typedef volatile union _REG_SENINF3_NCSI2_DI_
{
    volatile struct
    {
        FIELD VC0                       : 2;
        FIELD DT0                       : 6;
        FIELD VC1                       : 2;
        FIELD DT1                       : 6;
        FIELD VC2                       : 2;
        FIELD DT2                       : 6;
        FIELD VC3                       : 2;
        FIELD DT3                       : 6;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DI;

typedef volatile union _REG_SENINF3_NCSI2_HS_TRAIL_
{
    volatile struct
    {
        FIELD HS_TRAIL_PARAMETER        : 8;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_HS_TRAIL;

typedef volatile union _REG_SENINF3_NCSI2_DI_CTRL_
{
    volatile struct
    {
        FIELD VC0_INTERLEAVING          : 1;
        FIELD DT0_INTERLEAVING          : 2;
        FIELD rsv_3                     : 5;
        FIELD VC1_INTERLEAVING          : 1;
        FIELD DT1_INTERLEAVING          : 2;
        FIELD rsv_11                    : 5;
        FIELD VC2_INTERLEAVING          : 1;
        FIELD DT2_INTERLEAVING          : 2;
        FIELD rsv_19                    : 5;
        FIELD VC3_INTERLEAVING          : 1;
        FIELD DT3_INTERLEAVING          : 2;
        FIELD rsv_27                    : 5;
    } Bits;
    UINT32 Raw;
} REG_SENINF3_NCSI2_DI_CTRL;

typedef volatile union _SCAM_REG1_CFG_
{
    volatile struct /* 0x1500_8e20*/
    {
        FIELD  INTEN0                                :  1;      /*  0.. 0, 0x00000001 */
        FIELD  INTEN1                                :  1;      /*  1.. 1, 0x00000002 */
        FIELD  INTEN2                                :  1;      /*  2.. 2, 0x00000004 */
        FIELD  INTEN3                                :  1;      /*  3.. 3, 0x00000008 */
        FIELD  INTEN4                                :  1;      /*  4.. 4, 0x00000010 */
        FIELD  INTEN5                                :  1;      /*  5.. 5, 0x00000020 */
        FIELD  INTEN6                                :  1;      /*  6.. 6, 0x00000040 */
        FIELD  INTEN7                                :  1;      /*  7.. 7, 0x00000080 */
        FIELD  rsv_8                                 :  3;      /*  8..10, 0x00000700 */
        FIELD  SPCLK_INV                             :  1;      /* 11..11, 0x00000800 */
        FIELD  Clock_inverse                         :  1;      /* 12..12, 0x00001000 */
        FIELD  INTEN8                                :  1;      /* 13..13, 0x00002000 */
        FIELD  TIME_OUT_CNT_EN                       :  1;      /* 14..14, 0x00004000 */
        FIELD  rsv_15                                :  2;      /* 15..16, 0x00018000 */
        FIELD  Continuous_mode                       :  1;      /* 17..17, 0x00020000 */
        FIELD  LANECON_EN                            :  1;      /* 18..18, 0x00040000 */
        FIELD  rsv_19                                :  1;      /* 19..19, 0x00080000 */
        FIELD  Debug_mode                            :  1;      /* 20..20, 0x00100000 */
        FIELD  CAL_MD                                :  1;      /* 21..21, 0x00200000 */
        FIELD  CAL_CRC_ON                            :  1;      /* 22..22, 0x00400000 */
        FIELD  INTEN9                                :  1;      /* 23..23, 0x00800000 */
        FIELD  CSD_NUM                               :  3;      /* 24..26, 0x07000000 */
        FIELD  rsv_27                                :  1;      /* 27..27, 0x08000000 */
        FIELD  Warning_mask                          :  1;      /* 28..28, 0x10000000 */
        FIELD  C_DATA_PACKET_MD                      :  2;      /* 29..30, 0x60000000 */
        FIELD  rsv_31                                :  1;      /* 31..31, 0x80000000 */
    } Bits;
    UINT32 Raw;
}SCAM_REG1_CFG; /* SCAM_CFG */


typedef volatile union _SCAM_REG1_CON_
{
    volatile struct
    {
        FIELD Enable                    : 1;
        FIELD rsv_1                     : 15;
        FIELD Reset                     : 1;
        FIELD rsv_17                    : 15;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_CON;

typedef volatile union _SCAM_REG1_INT_
{
    volatile struct
    {
        FIELD INT0                      : 1;
        FIELD INT1                      : 1;
        FIELD INT2                      : 1;
        FIELD INT3                      : 1;
        FIELD INT4                      : 1;
        FIELD INT5                      : 1;
        FIELD INT6                      : 1;
        FIELD INT7                      : 1;
        FIELD INT8                      : 1;
        FIELD INT9                      : 1;
        FIELD rsv_10                    : 22;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INT;

typedef volatile union _SCAM_REG1_SIZE_
{
    volatile struct
    {
        FIELD WIDTH                     : 12;
        FIELD rsv_12                    : 4;
        FIELD HEIGHT                    : 12;
        FIELD rsv_28                    : 4;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_SIZE;

typedef volatile union _SCAM_REG1_CFG2_
{
    volatile struct
    {
        FIELD DIS_GATED_CLK             : 1;
        FIELD Reserved                  : 31;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_CFG2;

typedef volatile union _SCAM_REG1_INFO0_
{
    volatile struct
    {
        FIELD LINE_ID                   : 16;
        FIELD PACKET_SIZE               : 16;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO0;

typedef volatile union _SCAM_REG1_INFO1_
{
    volatile struct
    {
        FIELD Reserved                  : 8;
        FIELD DATA_ID                   : 6;
        FIELD CRC_ON                    : 1;
        FIELD ACTIVE                    : 1;
        FIELD DATA_CNT                  : 16;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO1;

typedef volatile union _SCAM_REG1_INFO2_
{
    volatile struct
    {
        FIELD INFO2                     : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO2;

typedef volatile union _SCAM_REG1_INFO3_
{
    volatile struct
    {
        FIELD INFO3                     : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO3;

typedef volatile union _SCAM_REG1_INFO4_
{
    volatile struct
    {
        FIELD FEND_CNT                  : 4;
        FIELD W_CRC_CNT                 : 4;
        FIELD W_SYNC_CNT                : 4;
        FIELD W_PID_CNT                 : 4;
        FIELD W_LID_CNT                 : 4;
        FIELD W_DID_CNT                 : 4;
        FIELD W_SIZE_CNT                : 4;
        FIELD C_CRC_CNT                 : 4;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO4;

typedef volatile union _SCAM_REG1_INFO5_
{
    volatile struct
    {
        FIELD INFO5                     : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO5;

typedef volatile union _SCAM_REG1_INFO6_
{
    volatile struct
    {
        FIELD W_TIME_OUT_CNT            : 4;
        FIELD C_DATA_PACKET_CNT         : 4;
        FIELD rsv_8                     : 24;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_INFO6;

typedef volatile union _SCAM_REG1_DDR_CTRL_
{
    volatile struct
    {
        FIELD CSK_SEL                   : 1;
        FIELD CSK_0_INV                 : 1;
        FIELD CSK_1_INV                 : 1;
        FIELD CSD_0_SWAP                : 1;
        FIELD CSD_1_SWAP                : 1;
        FIELD CSD_2_SWAP                : 1;
        FIELD CSD_3_SWAP                : 1;
        FIELD DDR_MODE                  : 1;
        FIELD CSD_0_DLY                 : 5;
        FIELD CSD_1_DLY                 : 5;
        FIELD CSD_2_DLY                 : 5;
        FIELD CSD_3_DLY                 : 5;
        FIELD PAD_CSD_NUM               : 2;
        FIELD rsv_30                    : 2;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_DDR_CTRL;

typedef volatile union _SCAM_REG1_TIME_OUT_
{
    volatile struct
    {
        FIELD TIME_OUT_VALUE            : 32;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_TIME_OUT;

typedef volatile union _SCAM_REG1_LINE_ID_START_
{
    volatile struct
    {
        FIELD LINE_ID_START             : 12;
        FIELD rsv_12                    : 20;
    } Bits;
    UINT32 Raw;
} SCAM_REG1_LINE_ID_START;

/* end MT6593_ncsi2.xml*/

// ----------------- seninf_top  Grouping Definitions -------------------
// ----------------- seninf_top Register Definition -------------------
typedef volatile struct _seninf_reg_t_
{
     REG_SENINF_TOP_CTRL             SENINF_TOP_CTRL;                //8000
     REG_SENINF_TOP_CMODEL_PAR       SENINF_TOP_CMODEL_PAR;          //8004
     REG_SENINF_TOP_MUX_CTRL         SENINF_TOP_MUX_CTRL;            //8008
     UINT32                          rsv_800C[45];                   //800C...80BC
     REG_N3D_CTL                     N3D_CTL;                        //80C0
     REG_N3D_POS                     N3D_POS;                        //80C4
     REG_N3D_TRIG                    N3D_TRIG;                       //80C8
     REG_N3D_INT                     N3D_INT;                        //80CC
     REG_N3D_CNT0                    N3D_CNT0;                       //80D0
     REG_N3D_CNT1                    N3D_CNT1;                       //80D4
     REG_N3D_DBG                     N3D_DBG;                        //80D8
     REG_N3D_DIFF_THR                N3D_DIFF_THR;                   //80DC
     REG_N3D_DIFF_CNT                N3D_DIFF_CNT;                   //80E0
     UINT32                          rsv_80E4[7];                    //80E4...80FC
     REG_SENINF1_CTRL                SENINF1_CTRL;                   //8100
     UINT32                          rsv_8104[7];                    //8104...811C
     REG_SENINF1_MUX_CTRL            SENINF1_MUX_CTRL;               //8120
     REG_SENINF1_MUX_INTEN           SENINF1_MUX_INTEN;              //8124
     REG_SENINF1_MUX_INTSTA          SENINF1_MUX_INTSTA;             //8128
     REG_SENINF1_MUX_SIZE            SENINF1_MUX_SIZE;               //812C
     REG_SENINF1_MUX_DEBUG_1         SENINF1_MUX_DEBUG_1;            //8130
     REG_SENINF1_MUX_DEBUG_2         SENINF1_MUX_DEBUG_2;            //8134
     REG_SENINF1_MUX_DEBUG_3         SENINF1_MUX_DEBUG_3;            //8138
     REG_SENINF1_MUX_DEBUG_4         SENINF1_MUX_DEBUG_4;            //813C
     REG_SENINF1_MUX_DEBUG_5         SENINF1_MUX_DEBUG_5;            //8140
     REG_SENINF1_MUX_DEBUG_6         SENINF1_MUX_DEBUG_6;            //8144
     REG_SENINF1_MUX_DEBUG_7         SENINF1_MUX_DEBUG_7;            //8148
     REG_SENINF1_MUX_SPARE           SENINF1_MUX_SPARE;              //814C
     REG_SENINF1_MUX_DATA            SENINF1_MUX_DATA;               //8150
     REG_SENINF1_MUX_DATA_CNT        SENINF1_MUX_DATA_CNT;           //8154
     REG_SENINF1_MUX_CROP            SENINF1_MUX_CROP;               //8158
     UINT32                          rsv_815C[41];                   //815C...81FC
     SENINF_REG_TG1_PH_CNT           SENINF_TG1_PH_CNT;              //8200
     SENINF_REG_TG1_SEN_CK           SENINF_TG1_SEN_CK;              //8204
     SENINF_REG_TG1_TM_CTL           SENINF_TG1_TM_CTL;              //8208
     SENINF_REG_TG1_TM_SIZE          SENINF_TG1_TM_SIZE;             //820C
     SENINF_REG_TG1_TM_CLK           SENINF_TG1_TM_CLK;              //8210
     UINT32                          rsv_8214[59];                   //8214...82FC
     REG_MIPI_RX_CON00_CSI0          MIPI_RX_CON00_CSI0;             //8300
     REG_MIPI_RX_CON04_CSI0          MIPI_RX_CON04_CSI0;             //8304
     REG_MIPI_RX_CON08_CSI0          MIPI_RX_CON08_CSI0;             //8308
     REG_MIPI_RX_CON0C_CSI0          MIPI_RX_CON0C_CSI0;             //830C
     REG_MIPI_RX_CON10_CSI0          MIPI_RX_CON10_CSI0;             //8310
     UINT32                          rsv_8314[4];                    //8314...8320
     REG_MIPI_RX_CON24_CSI0          MIPI_RX_CON24_CSI0;             //8324
     REG_MIPI_RX_CON28_CSI0          MIPI_RX_CON28_CSI0;             //8328
     UINT32                          rsv_832C[2];                    //832C...8330
     REG_MIPI_RX_CON34_CSI0          MIPI_RX_CON34_CSI0;             //8334
     REG_MIPI_RX_CON38_CSI0          MIPI_RX_CON38_CSI0;             //8338
     REG_MIPI_RX_CON3C_CSI0          MIPI_RX_CON3C_CSI0;             //833C
     REG_MIPI_RX_CON40_CSI0          MIPI_RX_CON40_CSI0;             //8340
     REG_MIPI_RX_CON44_CSI0          MIPI_RX_CON44_CSI0;             //8344
     REG_MIPI_RX_CON48_CSI0          MIPI_RX_CON48_CSI0;             //8348
     UINT32                          rsv_834C;                       //834C
     REG_MIPI_RX_CON50_CSI0          MIPI_RX_CON50_CSI0;             //8350
     UINT32                          rsv_8354[3];                    //8354...835C
     REG_SENINF1_CSI2_CTRL           SENINF1_CSI2_CTRL;              //8360
     REG_SENINF1_CSI2_DELAY          SENINF1_CSI2_DELAY;             //8364
     REG_SENINF1_CSI2_INTEN          SENINF1_CSI2_INTEN;             //8368
     REG_SENINF1_CSI2_INTSTA         SENINF1_CSI2_INTSTA;            //836C
     REG_SENINF1_CSI2_ECCDBG         SENINF1_CSI2_ECCDBG;            //8370
     REG_SENINF1_CSI2_CRCDBG         SENINF1_CSI2_CRCDBG;            //8374
     REG_SENINF1_CSI2_DBG            SENINF1_CSI2_DBG;               //8378
     REG_SENINF1_CSI2_VER            SENINF1_CSI2_VER;               //837C
     REG_SENINF1_CSI2_SHORT_INFO     SENINF1_CSI2_SHORT_INFO;        //8380
     REG_SENINF1_CSI2_LNFSM          SENINF1_CSI2_LNFSM;             //8384
     REG_SENINF1_CSI2_LNMUX          SENINF1_CSI2_LNMUX;             //8388
     REG_SENINF1_CSI2_HSYNC_CNT      SENINF1_CSI2_HSYNC_CNT;         //838C
     REG_SENINF1_CSI2_CAL            SENINF1_CSI2_CAL;               //8390
     REG_SENINF1_CSI2_DS             SENINF1_CSI2_DS;                //8394
     REG_SENINF1_CSI2_VS             SENINF1_CSI2_VS;                //8398
     REG_SENINF1_CSI2_BIST           SENINF1_CSI2_BIST;              //839C
     REG_SENINF1_NCSI2_CTL           SENINF1_NCSI2_CTL;              //83A0
     REG_SENINF1_NCSI2_LNRC_TIMING   SENINF1_NCSI2_LNRC_TIMING;      //83A4
     REG_SENINF1_NCSI2_LNRD_TIMING   SENINF1_NCSI2_LNRD_TIMING;      //83A8
     REG_SENINF1_NCSI2_DPCM          SENINF1_NCSI2_DPCM;             //83AC
     REG_SENINF1_NCSI2_INT_EN        SENINF1_NCSI2_INT_EN;           //83B0
     REG_SENINF1_NCSI2_INT_STATUS    SENINF1_NCSI2_INT_STATUS;       //83B4
     REG_SENINF1_NCSI2_DGB_SEL       SENINF1_NCSI2_DGB_SEL;          //83B8
     REG_SENINF1_NCSI2_DBG_PORT      SENINF1_NCSI2_DBG_PORT;         //83BC
     REG_SENINF1_NCSI2_SPARE0        SENINF1_NCSI2_SPARE0;           //83C0
     REG_SENINF1_NCSI2_SPARE1        SENINF1_NCSI2_SPARE1;           //83C4
     REG_SENINF1_NCSI2_LNRC_FSM      SENINF1_NCSI2_LNRC_FSM;         //83C8
     REG_SENINF1_NCSI2_LNRD_FSM      SENINF1_NCSI2_LNRD_FSM;         //83CC
     REG_SENINF1_NCSI2_FRAME_LINE_NUM SENINF1_NCSI2_FRAME_LINE_NUM;   //83D0
     REG_SENINF1_NCSI2_GENERIC_SHORT SENINF1_NCSI2_GENERIC_SHORT;    //83D4
     REG_SENINF1_NCSI2_HSRX_DBG      SENINF1_NCSI2_HSRX_DBG;         //83D8
     REG_SENINF1_NCSI2_DI            SENINF1_NCSI2_DI;               //83DC
     REG_SENINF1_NCSI2_HS_TRAIL      SENINF1_NCSI2_HS_TRAIL;         //83E0
     REG_SENINF1_NCSI2_DI_CTRL       SENINF1_NCSI2_DI_CTRL;          //83E4
     UINT32                          rsv_83E8[70];                   //83E8...84FC
     REG_SENINF2_CTRL                SENINF2_CTRL;                   //8500
     UINT32                          rsv_8504[7];                    //8504...851C
     REG_SENINF2_MUX_CTRL            SENINF2_MUX_CTRL;               //8520
     REG_SENINF2_MUX_INTEN           SENINF2_MUX_INTEN;              //8524
     REG_SENINF2_MUX_INTSTA          SENINF2_MUX_INTSTA;             //8528
     REG_SENINF2_MUX_SIZE            SENINF2_MUX_SIZE;               //852C
     REG_SENINF2_MUX_DEBUG_1         SENINF2_MUX_DEBUG_1;            //8530
     REG_SENINF2_MUX_DEBUG_2         SENINF2_MUX_DEBUG_2;            //8534
     REG_SENINF2_MUX_DEBUG_3         SENINF2_MUX_DEBUG_3;            //8538
     REG_SENINF2_MUX_DEBUG_4         SENINF2_MUX_DEBUG_4;            //853C
     REG_SENINF2_MUX_DEBUG_5         SENINF2_MUX_DEBUG_5;            //8540
     REG_SENINF2_MUX_DEBUG_6         SENINF2_MUX_DEBUG_6;            //8544
     REG_SENINF2_MUX_DEBUG_7         SENINF2_MUX_DEBUG_7;            //8548
     REG_SENINF2_MUX_SPARE           SENINF2_MUX_SPARE;              //854C
     REG_SENINF2_MUX_DATA            SENINF2_MUX_DATA;               //8550
     REG_SENINF2_MUX_DATA_CNT        SENINF2_MUX_DATA_CNT;           //8554
     REG_SENINF2_MUX_CROP            SENINF2_MUX_CROP;               //8558
     UINT32                          rsv_855C[41];                   //855C...85FC
     SENINF_REG_TG2_PH_CNT           SENINF_TG2_PH_CNT;              //8600
     SENINF_REG_TG2_SEN_CK           SENINF_TG2_SEN_CK;              //8604
     SENINF_REG_TG2_TM_CTL           SENINF_TG2_TM_CTL;              //8608
     SENINF_REG_TG2_TM_SIZE          SENINF_TG2_TM_SIZE;             //860C
     SENINF_REG_TG2_TM_CLK           SENINF_TG2_TM_CLK;              //8610
     UINT32                          rsv_8614[59];                   //8614...86FC
     REG_MIPI_RX_CON00_CSI1          MIPI_RX_CON00_CSI1;             //8700
     REG_MIPI_RX_CON04_CSI1          MIPI_RX_CON04_CSI1;             //8704
     REG_MIPI_RX_CON08_CSI1          MIPI_RX_CON08_CSI1;             //8708
     REG_MIPI_RX_CON0C_CSI1          MIPI_RX_CON0C_CSI1;             //870C
     REG_MIPI_RX_CON10_CSI1          MIPI_RX_CON10_CSI1;             //8710
     UINT32                          rsv_8714[4];                    //8714...8720
     REG_MIPI_RX_CON24_CSI1          MIPI_RX_CON24_CSI1;             //8724
     REG_MIPI_RX_CON28_CSI1          MIPI_RX_CON28_CSI1;             //8728
     UINT32                          rsv_872C[2];                    //872C...8730
     REG_MIPI_RX_CON34_CSI1          MIPI_RX_CON34_CSI1;             //8734
     REG_MIPI_RX_CON38_CSI1          MIPI_RX_CON38_CSI1;             //8738
     REG_MIPI_RX_CON3C_CSI1          MIPI_RX_CON3C_CSI1;             //873C
     REG_MIPI_RX_CON40_CSI1          MIPI_RX_CON40_CSI1;             //8740
     REG_MIPI_RX_CON44_CSI1          MIPI_RX_CON44_CSI1;             //8744
     REG_MIPI_RX_CON48_CSI1          MIPI_RX_CON48_CSI1;             //8748
     UINT32                          rsv_874C;                       //874C
     REG_MIPI_RX_CON50_CSI1          MIPI_RX_CON50_CSI1;             //8750
     UINT32                          rsv_8754[3];                    //8754...875C
     REG_SENINF2_CSI2_CTRL           SENINF2_CSI2_CTRL;              //8760
     REG_SENINF2_CSI2_DELAY          SENINF2_CSI2_DELAY;             //8764
     REG_SENINF2_CSI2_INTEN          SENINF2_CSI2_INTEN;             //8768
     REG_SENINF2_CSI2_INTSTA         SENINF2_CSI2_INTSTA;            //876C
     REG_SENINF2_CSI2_ECCDBG         SENINF2_CSI2_ECCDBG;            //8770
     REG_SENINF2_CSI2_CRCDBG         SENINF2_CSI2_CRCDBG;            //8774
     REG_SENINF2_CSI2_DBG            SENINF2_CSI2_DBG;               //8778
     REG_SENINF2_CSI2_VER            SENINF2_CSI2_VER;               //877C
     REG_SENINF2_CSI2_SHORT_INFO     SENINF2_CSI2_SHORT_INFO;        //8780
     REG_SENINF2_CSI2_LNFSM          SENINF2_CSI2_LNFSM;             //8784
     REG_SENINF2_CSI2_LNMUX          SENINF2_CSI2_LNMUX;             //8788
     REG_SENINF2_CSI2_HSYNC_CNT      SENINF2_CSI2_HSYNC_CNT;         //878C
     REG_SENINF2_CSI2_CAL            SENINF2_CSI2_CAL;               //8790
     REG_SENINF2_CSI2_DS             SENINF2_CSI2_DS;                //8794
     REG_SENINF2_CSI2_VS             SENINF2_CSI2_VS;                //8798
     REG_SENINF2_CSI2_BIST           SENINF2_CSI2_BIST;              //879C
     REG_SENINF2_NCSI2_CTL           SENINF2_NCSI2_CTL;              //87A0
     REG_SENINF2_NCSI2_LNRC_TIMING   SENINF2_NCSI2_LNRC_TIMING;      //87A4
     REG_SENINF2_NCSI2_LNRD_TIMING   SENINF2_NCSI2_LNRD_TIMING;      //87A8
     REG_SENINF2_NCSI2_DPCM          SENINF2_NCSI2_DPCM;             //87AC
     REG_SENINF2_NCSI2_INT_EN        SENINF2_NCSI2_INT_EN;           //87B0
     REG_SENINF2_NCSI2_INT_STATUS    SENINF2_NCSI2_INT_STATUS;       //87B4
     REG_SENINF2_NCSI2_DGB_SEL       SENINF2_NCSI2_DGB_SEL;          //87B8
     REG_SENINF2_NCSI2_DBG_PORT      SENINF2_NCSI2_DBG_PORT;         //87BC
     REG_SENINF2_NCSI2_SPARE0        SENINF2_NCSI2_SPARE0;           //87C0
     REG_SENINF2_NCSI2_SPARE1        SENINF2_NCSI2_SPARE1;           //87C4
     REG_SENINF2_NCSI2_LNRC_FSM      SENINF2_NCSI2_LNRC_FSM;         //87C8
     REG_SENINF2_NCSI2_LNRD_FSM      SENINF2_NCSI2_LNRD_FSM;         //87CC
     REG_SENINF2_NCSI2_FRAME_LINE_NUM SENINF2_NCSI2_FRAME_LINE_NUM;   //87D0
     REG_SENINF2_NCSI2_GENERIC_SHORT SENINF2_NCSI2_GENERIC_SHORT;    //87D4
     REG_SENINF2_NCSI2_HSRX_DBG      SENINF2_NCSI2_HSRX_DBG;         //87D8
     REG_SENINF2_NCSI2_DI            SENINF2_NCSI2_DI;               //87DC
     REG_SENINF2_NCSI2_HS_TRAIL      SENINF2_NCSI2_HS_TRAIL;         //87E0
     REG_SENINF2_NCSI2_DI_CTRL       SENINF2_NCSI2_DI_CTRL;          //87E4
     UINT32                          rsv_87E8[70];                   //87E8...88FC
     REG_SENINF3_CTRL                SENINF3_CTRL;                   //8900
     UINT32                          rsv_8904[7];                    //8904...891C
     REG_SENINF3_MUX_CTRL            SENINF3_MUX_CTRL;               //8920
     REG_SENINF3_MUX_INTEN           SENINF3_MUX_INTEN;              //8924
     REG_SENINF3_MUX_INTSTA          SENINF3_MUX_INTSTA;             //8928
     REG_SENINF3_MUX_SIZE            SENINF3_MUX_SIZE;               //892C
     REG_SENINF3_MUX_DEBUG_1         SENINF3_MUX_DEBUG_1;            //8930
     REG_SENINF3_MUX_DEBUG_2         SENINF3_MUX_DEBUG_2;            //8934
     REG_SENINF3_MUX_DEBUG_3         SENINF3_MUX_DEBUG_3;            //8938
     REG_SENINF3_MUX_DEBUG_4         SENINF3_MUX_DEBUG_4;            //893C
     REG_SENINF3_MUX_DEBUG_5         SENINF3_MUX_DEBUG_5;            //8940
     REG_SENINF3_MUX_DEBUG_6         SENINF3_MUX_DEBUG_6;            //8944
     REG_SENINF3_MUX_DEBUG_7         SENINF3_MUX_DEBUG_7;            //8948
     REG_SENINF3_MUX_SPARE           SENINF3_MUX_SPARE;              //894C
     REG_SENINF3_MUX_DATA            SENINF3_MUX_DATA;               //8950
     REG_SENINF3_MUX_DATA_CNT        SENINF3_MUX_DATA_CNT;           //8954
     REG_SENINF3_MUX_CROP            SENINF3_MUX_CROP;               //8958
     UINT32                          rsv_895C[41];                   //895C...89FC
     SENINF_REG_TG3_PH_CNT           SENINF_TG3_PH_CNT;              //8A00
     SENINF_REG_TG3_SEN_CK           SENINF_TG3_SEN_CK;              //8A04
     SENINF_REG_TG3_TM_CTL           SENINF_TG3_TM_CTL;              //8A08
     SENINF_REG_TG3_TM_SIZE          SENINF_TG3_TM_SIZE;             //8A0C
     SENINF_REG_TG3_TM_CLK           SENINF_TG3_TM_CLK;              //8A10
     UINT32                          rsv_8A14[59];                   //8A14...8AFC
     REG_MIPI_RX_CON00_CSI2          MIPI_RX_CON00_CSI2;             //8B00
     REG_MIPI_RX_CON04_CSI2          MIPI_RX_CON04_CSI2;             //8B04
     REG_MIPI_RX_CON08_CSI2          MIPI_RX_CON08_CSI2;             //8B08
     REG_MIPI_RX_CON0C_CSI2          MIPI_RX_CON0C_CSI2;             //8B0C
     REG_MIPI_RX_CON10_CSI2          MIPI_RX_CON10_CSI2;             //8B10
     UINT32                          rsv_8B14[4];                    //8B14...8B20
     REG_MIPI_RX_CON24_CSI2          MIPI_RX_CON24_CSI2;             //8B24
     REG_MIPI_RX_CON28_CSI2          MIPI_RX_CON28_CSI2;             //8B28
     UINT32                          rsv_8B2C[2];                    //8B2C...8B30
     REG_MIPI_RX_CON34_CSI2          MIPI_RX_CON34_CSI2;             //8B34
     REG_MIPI_RX_CON38_CSI2          MIPI_RX_CON38_CSI2;             //8B38
     REG_MIPI_RX_CON3C_CSI2          MIPI_RX_CON3C_CSI2;             //8B3C
     REG_MIPI_RX_CON40_CSI2          MIPI_RX_CON40_CSI2;             //8B40
     REG_MIPI_RX_CON44_CSI2          MIPI_RX_CON44_CSI2;             //8B44
     REG_MIPI_RX_CON48_CSI2          MIPI_RX_CON48_CSI2;             //8B48
     UINT32                          rsv_8B4C;                       //8B4C
     REG_MIPI_RX_CON50_CSI2          MIPI_RX_CON50_CSI2;             //8B50
     UINT32                          rsv_8B54[3];                    //8B54...8B5C
     REG_SENINF3_CSI2_CTRL           SENINF3_CSI2_CTRL;              //8B60
     REG_SENINF3_CSI2_DELAY          SENINF3_CSI2_DELAY;             //8B64
     REG_SENINF3_CSI2_INTEN          SENINF3_CSI2_INTEN;             //8B68
     REG_SENINF3_CSI2_INTSTA         SENINF3_CSI2_INTSTA;            //8B6C
     REG_SENINF3_CSI2_ECCDBG         SENINF3_CSI2_ECCDBG;            //8B70
     REG_SENINF3_CSI2_CRCDBG         SENINF3_CSI2_CRCDBG;            //8B74
     REG_SENINF3_CSI2_DBG            SENINF3_CSI2_DBG;               //8B78
     REG_SENINF3_CSI2_VER            SENINF3_CSI2_VER;               //8B7C
     REG_SENINF3_CSI2_SHORT_INFO     SENINF3_CSI2_SHORT_INFO;        //8B80
     REG_SENINF3_CSI2_LNFSM          SENINF3_CSI2_LNFSM;             //8B84
     REG_SENINF3_CSI2_LNMUX          SENINF3_CSI2_LNMUX;             //8B88
     REG_SENINF3_CSI2_HSYNC_CNT      SENINF3_CSI2_HSYNC_CNT;         //8B8C
     REG_SENINF3_CSI2_CAL            SENINF3_CSI2_CAL;               //8B90
     REG_SENINF3_CSI2_DS             SENINF3_CSI2_DS;                //8B94
     REG_SENINF3_CSI2_VS             SENINF3_CSI2_VS;                //8B98
     REG_SENINF3_CSI2_BIST           SENINF3_CSI2_BIST;              //8B9C
     REG_SENINF3_NCSI2_CTL           SENINF3_NCSI2_CTL;              //8BA0
     REG_SENINF3_NCSI2_LNRC_TIMING   SENINF3_NCSI2_LNRC_TIMING;      //8BA4
     REG_SENINF3_NCSI2_LNRD_TIMING   SENINF3_NCSI2_LNRD_TIMING;      //8BA8
     REG_SENINF3_NCSI2_DPCM          SENINF3_NCSI2_DPCM;             //8BAC
     REG_SENINF3_NCSI2_INT_EN        SENINF3_NCSI2_INT_EN;           //8BB0
     REG_SENINF3_NCSI2_INT_STATUS    SENINF3_NCSI2_INT_STATUS;       //8BB4
     REG_SENINF3_NCSI2_DGB_SEL       SENINF3_NCSI2_DGB_SEL;          //8BB8
     REG_SENINF3_NCSI2_DBG_PORT      SENINF3_NCSI2_DBG_PORT;         //8BBC
     REG_SENINF3_NCSI2_SPARE0        SENINF3_NCSI2_SPARE0;           //8BC0
     REG_SENINF3_NCSI2_SPARE1        SENINF3_NCSI2_SPARE1;           //8BC4
     REG_SENINF3_NCSI2_LNRC_FSM      SENINF3_NCSI2_LNRC_FSM;         //8BC8
     REG_SENINF3_NCSI2_LNRD_FSM      SENINF3_NCSI2_LNRD_FSM;         //8BCC
     REG_SENINF3_NCSI2_FRAME_LINE_NUM SENINF3_NCSI2_FRAME_LINE_NUM;   //8BD0
     REG_SENINF3_NCSI2_GENERIC_SHORT SENINF3_NCSI2_GENERIC_SHORT;    //8BD4
     REG_SENINF3_NCSI2_HSRX_DBG      SENINF3_NCSI2_HSRX_DBG;         //8BD8
     REG_SENINF3_NCSI2_DI            SENINF3_NCSI2_DI;               //8BDC
     REG_SENINF3_NCSI2_HS_TRAIL      SENINF3_NCSI2_HS_TRAIL;         //8BE0
     REG_SENINF3_NCSI2_DI_CTRL       SENINF3_NCSI2_DI_CTRL;          //8BE4
     UINT32                          rsv_8BE8[70];                   //8BE8...8CFC
     REG_SENINF4_CTRL                SENINF4_CTRL;                   //8D00
     UINT32                          rsv_8D04[7];                    //8D04...8D1C
     REG_SENINF4_MUX_CTRL            SENINF4_MUX_CTRL;               //8D20
     REG_SENINF4_MUX_INTEN           SENINF4_MUX_INTEN;              //8D24
     REG_SENINF4_MUX_INTSTA          SENINF4_MUX_INTSTA;             //8D28
     REG_SENINF4_MUX_SIZE            SENINF4_MUX_SIZE;               //8D2C
     REG_SENINF4_MUX_DEBUG_1         SENINF4_MUX_DEBUG_1;            //8D30
     REG_SENINF4_MUX_DEBUG_2         SENINF4_MUX_DEBUG_2;            //8D34
     REG_SENINF4_MUX_DEBUG_3         SENINF4_MUX_DEBUG_3;            //8D38
     REG_SENINF4_MUX_DEBUG_4         SENINF4_MUX_DEBUG_4;            //8D3C
     REG_SENINF4_MUX_DEBUG_5         SENINF4_MUX_DEBUG_5;            //8D40
     REG_SENINF4_MUX_DEBUG_6         SENINF4_MUX_DEBUG_6;            //8D44
     REG_SENINF4_MUX_DEBUG_7         SENINF4_MUX_DEBUG_7;            //8D48
     REG_SENINF4_MUX_SPARE           SENINF4_MUX_SPARE;              //8D4C
     REG_SENINF4_MUX_DATA            SENINF4_MUX_DATA;               //8D50
     REG_SENINF4_MUX_DATA_CNT        SENINF4_MUX_DATA_CNT;           //8D54
     REG_SENINF4_MUX_CROP            SENINF4_MUX_CROP;               //8D58
     UINT32                          rsv_8D5C[41];                   //8D5C...8DFC
     SENINF_REG_TG4_PH_CNT           SENINF_TG4_PH_CNT;              //8E00
     SENINF_REG_TG4_SEN_CK           SENINF_TG4_SEN_CK;              //8E04
     SENINF_REG_TG4_TM_CTL           SENINF_TG4_TM_CTL;              //8E08
     SENINF_REG_TG4_TM_SIZE          SENINF_TG4_TM_SIZE;             //8E0C
     SENINF_REG_TG4_TM_CLK           SENINF_TG4_TM_CLK;              //8E10
     UINT32                          rsv_8E14[3];                    //8E14...8E1C
     SCAM_REG1_CFG                   SCAM1_CFG;                      // 8E20
     SCAM_REG1_CON                   SCAM1_CON;                      // 8E24
     SCAM_REG1_INT                   SCAM1_INT;                      // 8E28
     SCAM_REG1_SIZE                  SCAM1_SIZE;                     // 8E2C
     SCAM_REG1_CFG2                  SCAM1_CFG2;                     // 8E30
     SCAM_REG1_INFO0                 SCAM1_INFO0;                    // 8E34
     SCAM_REG1_INFO1                 SCAM1_INFO1;                    // 8E38
     SCAM_REG1_INFO2                 SCAM1_INFO2;                    // 8E3C
     SCAM_REG1_INFO3                 SCAM1_INFO3;                    // 8E40
     SCAM_REG1_INFO4                 SCAM1_INFO4;                    // 8E44
     SCAM_REG1_INFO5                 SCAM1_INFO5;                    // 8E48
     SCAM_REG1_INFO6                 SCAM1_INFO6;                    // 8E4C
     SCAM_REG1_DDR_CTRL              SCAM1_DDR_CTRL;                 // 8E50
     SCAM_REG1_TIME_OUT              SCAM1_TIME_OUT;                 // 8E54
     SCAM_REG1_LINE_ID_START         SCAM1_LINE_ID_START;            // 8E58
     UINT32                          rsv_8E5C[25];                   //8E5C...8EBC
     REG_CCIR656_CTL                 CCIR656_CTL;                    //8EC0
     REG_CCIR656_H                   CCIR656_H;                      //8EC4
     REG_CCIR656_PTGEN_H_1           CCIR656_PTGEN_H_1;              //8EC8
     REG_CCIR656_PTGEN_H_2           CCIR656_PTGEN_H_2;              //8ECC
     REG_CCIR656_PTGEN_V_1           CCIR656_PTGEN_V_1;              //8ED0
     REG_CCIR656_PTGEN_V_2           CCIR656_PTGEN_V_2;              //8ED4
     REG_CCIR656_PTGEN_CTL1          CCIR656_PTGEN_CTL1;             //8ED8
     REG_CCIR656_PTGEN_CTL2          CCIR656_PTGEN_CTL2;             //8EDC
     REG_CCIR656_PTGEN_CTL3          CCIR656_PTGEN_CTL3;             //8EE0
     REG_CCIR656_STATUS              CCIR656_STATUS;                 //8EE4
}seninf_reg_t;

#endif // _SENINF_REG_H_
