/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _SENINF_DRV_BASE_H_
#define _SENINF_DRV_BASE_H_

/*******************************************************************************
*
********************************************************************************/
enum{
    SENINF_TOP_TG1 = 0x0,
    SENINF_TOP_TG2 = 0x1,
    SENINF_TOP_SV1 = 0x2,
    SENINF_TOP_SV2 = 0x3,
};

enum{
    SENINF_1 = 0x0,
    SENINF_2 = 0x1,
    SENINF_3 = 0x2,
    SENINF_4 = 0x3,
};

typedef enum {
    PAD_10BIT       = 0x0,
    PAD_8BIT_7_0    = 0x3,
    PAD_8BIT_9_2    = 0x4,
}PAD2CAM_DATA_ENUM;


typedef enum { //0:CSI2, 3: parallel, 8:nCSI2
    CSI2            = 0x0,
    TEST_MODEL      = 0x1,
    CCIR656         = 0x2,
    PARALLEL_SENSOR = 0x3,
    SERIAL_SENSOR   = 0x4,
    HD_TV           = 0x5,
    EXT_CSI2_OUT1   = 0x6,
    EXT_CSI2_OUT2   = 0x7,
    MIPI_SENSOR     = 0x8,
    VIRTUAL_CHANNEL_1 = 0x9,
    VIRTUAL_CHANNEL_2 = 0xA,
    VIRTUAL_CHANNEL_3 = 0xB,
}SENINF_SOURCE_ENUM;


typedef enum {
    TG_12BIT    = 0x0,
    TG_10BIT    = 0x1,
    TG_8BIT     = 0x2
}SENSOR_DATA_BITS_ENUM;

typedef enum {
    RAW_8BIT_FMT        = 0x0,
    RAW_10BIT_FMT       = 0x1,
    RAW_12BIT_FMT       = 0x2,
    YUV422_FMT          = 0x3,
    RAW_14BIT_FMT       = 0x4,
    RGB565_MIPI_FMT     = 0x5,
    RGB888_MIPI_FMT     = 0x6,
    JPEG_FMT            = 0x7
}TG_FORMAT_ENUM;

typedef enum {
    ACTIVE_HIGH     = 0x0,
    ACTIVE_LOW      = 0x1,
}CCIR656_OUTPUT_POLARITY_ENUM;

typedef enum {
    IMMIDIANT_TRIGGER   = 0x0,
    REFERENCE_VS1       = 0x1,
    I2C1_BEFORE_I2C2    = 0x2,
    I2C2_BEFORE_I2C1    = 0x3
}N3D_I2C_TRIGGER_MODE_ENUM;

typedef enum drvSeninfCmd_s {
    CMD_SET_DEVICE              = 0x1000,
    CMD_GET_SENINF_ADDR         = 0x2001,
    CMD_DRV_SENINF_MAX             = 0xFFFF
} drvSeninfCmd_e;


#define CAM_PLL_48_GROUP        (1)
#define CAM_PLL_52_GROUP        (2)

/*******************************************************************************
*
********************************************************************************/
class SeninfDrv {
public:
    //
    static SeninfDrv* createInstance();
    virtual void   destroyInstance() = 0;

protected:
    virtual ~SeninfDrv() {};

public:
    virtual int init() = 0;
    //
    virtual int uninit() = 0;
    //
    typedef struct reg_s {
        unsigned long addr;
        unsigned long val;
    } reg_t;
    //
    virtual int setMclk1(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv) = 0;
    //
    virtual int setMclk2(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv) = 0;
    //
    virtual int setMclk3(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv) = 0;

    //
    virtual int setTg1GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd) = 0;
    //
    virtual int setTg2GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd) = 0;
    //
    virtual int setSV1GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd) = 0;
    //
    virtual int setSV2GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd) = 0;
    //
    virtual int setTg1ViewFinderMode(unsigned long spMode) = 0;
 //
    virtual int setTg2ViewFinderMode(unsigned long spMode) = 0;
    //
    virtual int setSV1ViewFinderMode(unsigned long spMode) = 0;
 //
    virtual int setSV2ViewFinderMode(unsigned long spMode) = 0;

    //
    virtual int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0) = 0;

    //
    virtual int setTg1Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode) = 0;
    //
    virtual int setTg2Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode) = 0;
    //
    virtual int setSV1Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode) = 0;
    //
    virtual int setSV2Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode) = 0;
    //

    virtual int setSeninfTopMuxCtrl(unsigned int seninfTopIdx, unsigned int seninfSrc) = 0;
    //
    virtual int setSeninf1VC(unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id,unsigned int vcNum )= 0;
    //
    virtual int setSeninf2VC(unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id,unsigned int vcNum )= 0;
    //
    virtual int setSeninf3VC(unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id,unsigned int vcNum )= 0;
    //
    virtual int setSeninf1MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode) = 0;
    //
    virtual int setSeninf2MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode) = 0;
    //
    virtual int setSeninf3MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode) = 0;
    //
    virtual int setSeninf4MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode) = 0;
    //
    virtual int setSeninf1Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel) = 0;
    //
    virtual int setSeninf2Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel) = 0;
    //
    virtual int setSeninf3Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel) = 0;
    //
    virtual int setSeninf4Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel) = 0;
    //
    virtual int setSeninf1NCSI2(unsigned long dataTermDelay,
                        unsigned long dataSettleDelay,
                        unsigned long clkTermDelay,
                        unsigned long vsyncType,
                        unsigned long dlaneNum,
                        unsigned long ncsi2En,
                        unsigned long dataheaderOrder,
                        unsigned long mipi_type,
                        unsigned long HSRXDET,
                        unsigned long dpcm) = 0;
    //
    virtual int setSeninf2NCSI2(unsigned long dataTermDelay,
                        unsigned long dataSettleDelay,
                        unsigned long clkTermDelay,
                        unsigned long vsyncType,
                        unsigned long dlaneNum,
                        unsigned long ncsi2En,
                        unsigned long dataheaderOrder,
                        unsigned long mipi_type,
                        unsigned long HSRXDET,
                        unsigned long dpcm) = 0;

    //
    virtual int setSeninf3NCSI2(unsigned long dataTermDelay,
                        unsigned long dataSettleDelay,
                        unsigned long clkTermDelay,
                        unsigned long vsyncType,
                        unsigned long dlaneNum,
                        unsigned long ncsi2En,
                        unsigned long dataheaderOrder,
                        unsigned long mipi_type,
                        unsigned long HSRXDET,
                        unsigned long dpcm) = 0;
    //
    virtual int setSeninf4Scam(unsigned int scamEn,
                        unsigned int clkInv,
                        unsigned int width,
                        unsigned int height,
                        unsigned int contiMode,
                        unsigned int csdNum,
                        unsigned int DDR_EN) = 0;
    //
    virtual int setSeninf4Parallel(unsigned int parallelEn,  unsigned int inDataType) = 0;
    //
    virtual int setMclk1IODrivingCurrent(unsigned long ioDrivingCurrent) = 0;
    //
    virtual int setMclk2IODrivingCurrent(unsigned long ioDrivingCurrent) = 0;
    //
    virtual int setMclk3IODrivingCurrent(unsigned long ioDrivingCurrent) = 0;
    //
    virtual int setTG1_TM_Ctl(unsigned int seninfSrc, unsigned int TM_En, unsigned int dummypxl,unsigned int vsync,
                              unsigned int line,unsigned int pxl) = 0;
    //
    virtual int setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel, unsigned long  flashPol) = 0;
    //
    virtual int setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel) = 0;
    //
    virtual int setFlashEn(bool flashEn) = 0;
    //
    virtual int setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd) = 0;
    //
    virtual int setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode, unsigned long diffCntEn, unsigned long diffCntThr) = 0;
    //
    virtual int setN3DI2CPos(unsigned long n3dPos) = 0;
    //
    virtual int setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn) = 0;
    //
    virtual int getN3DDiffCnt(MUINT32 *pCnt) = 0;
    //
    virtual int checkSeninf1Input() = 0;
    //
    virtual int checkSeninf2Input() = 0;
    //
    virtual int autoDeskewCalibrationSeninf1() = 0;
    virtual int autoDeskewCalibrationSeninf2() = 0;
    virtual int autoDeskewCalibrationSeninf3() = 0;
    //ToDo :remove
    //virtual int setPdnRst(int camera, bool on) = 0;

};

#endif // _ISP_DRV_H_

