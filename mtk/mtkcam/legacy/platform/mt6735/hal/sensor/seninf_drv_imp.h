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
#ifndef _SENINF_DRV_IMP_H_
#define _SENINF_DRV_IMP_H_


#include <utils/Errors.h>
#include <cutils/log.h>
#include "seninf_drv.h"

#define USE_ISP_OPEN_FD    (1)
#if USE_ISP_OPEN_FD
    #include <mtkcam/drv/isp_drv.h>
#endif

//-----------------------------------------------------------------------------

using namespace android;
//-----------------------------------------------------------------------------


#ifndef USING_MTK_LDVT
#define LOG_MSG(fmt, arg...)    ALOGD("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    ALOGD("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    ALOGE("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#include "uvvf.h"

#if 1
#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]" fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define LOG_MSG(fmt, arg...)
#define LOG_WRN(fmt, arg...)
#define LOG_ERR(fmt, arg...)
#endif
#endif


/*******************************************************************************
*
********************************************************************************/
class SeninfDrvImp : public SeninfDrv {
public:
    static SeninfDrv* getInstance();
    virtual void destroyInstance();
//
private:
    SeninfDrvImp();
    virtual ~SeninfDrvImp();
//
public:
    virtual int init();
    virtual int uninit();
    //
    virtual int setMclk1(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv);
    //
    virtual int setMclk2(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv);
    //
    virtual int setMclk3(unsigned long pcEn, unsigned long mclkSel,
        unsigned long clkCnt, unsigned long clkPol,
        unsigned long clkFallEdge, unsigned long clkRiseEdge, unsigned long padPclkInv);
    //
    virtual int setTg1GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd);
    //
    virtual int setTg2GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd);
    //
    virtual int setSV1GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd);
    //
    virtual int setSV2GrabRange(unsigned long pixelStart, unsigned long pixelEnd,
        unsigned long lineStart, unsigned long lineEnd);
    //
    virtual int setTg1ViewFinderMode(unsigned long spMode);
    //
    virtual int setTg2ViewFinderMode(unsigned long spMode);
    //
    virtual int setSV1ViewFinderMode(unsigned long spMode);
    //
    virtual int setSV2ViewFinderMode(unsigned long spMode);
    //
    virtual int sendCommand(int cmd, unsigned long arg1 = 0, unsigned long arg2 = 0, unsigned long arg3 = 0);
    //
    virtual int setTg1Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode);
    //
    virtual int setTg2Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode);
    //
    virtual int setSV1Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode);
    //
    virtual int setSV2Cfg(TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb, unsigned int twoPxlMode);
    //
    virtual int setSeninfTopMuxCtrl(unsigned int seninfTopIdx, unsigned int seninfSrc);
    //
    virtual int setSeninf1VC(unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id,unsigned int vcNum );
    //
    virtual int setSeninf2VC(unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id,unsigned int vcNum );
    //
    virtual int setSeninf3VC(unsigned int vc0Id, unsigned int vc1Id, unsigned int vc2Id, unsigned int vc3Id,unsigned int vcNum );
    //

    virtual int setSeninf1MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode);
    //
    virtual int setSeninf2MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode);
    //
    virtual int setSeninf3MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode);
    //
    virtual int setSeninf4MuxCtrl(unsigned long hsPol, unsigned long vsPol, SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType, unsigned int twoPxlMode);
    //
    virtual int setSeninf1Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
    //
    virtual int setSeninf2Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
    //
    virtual int setSeninf3Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
    //
    virtual int setSeninf4Ctrl(PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel);
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
                        unsigned long dpcm);
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
                        unsigned long dpcm);

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
                        unsigned long dpcm);
    //
    virtual int setSeninf4Scam(unsigned int scamEn,
                        unsigned int clkInv,
                        unsigned int width,
                        unsigned int height,
                        unsigned int contiMode,
                        unsigned int csdNum,
                        unsigned int DDR_EN);
    //
    virtual int setSeninf4Parallel(unsigned int parallelEn,  unsigned int inDataType);
    //
    virtual int setMclk1IODrivingCurrent(unsigned long ioDrivingCurrent);
    //
    virtual int setMclk2IODrivingCurrent(unsigned long ioDrivingCurrent);
    //
    virtual int setMclk3IODrivingCurrent(unsigned long ioDrivingCurrent);

    virtual int setTG1_TM_Ctl(unsigned int seninfSrc, unsigned int TM_En, unsigned int dummypxl,unsigned int vsync,
                                    unsigned int line,unsigned int pxl);

    virtual int setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel, unsigned long  flashPol);
    //
    virtual int setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel);
    //
    virtual int setFlashEn(bool flashEn) ;
    //
    virtual int setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd);
    //
    virtual int setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode, unsigned long diffCntEn, unsigned long diffCntThr);
    //
    virtual int setN3DI2CPos(unsigned long n3dPos);
    //
    virtual int setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn);
    //
    virtual int getN3DDiffCnt(MUINT32 *pCnt);
    //
    virtual int checkSeninf1Input();
    //
    virtual int checkSeninf2Input();
    //
    virtual int autoDeskewCalibrationSeninf1();
    //
    virtual int autoDeskewCalibrationSeninf2();
    //
    virtual int autoDeskewCalibrationSeninf3();
    //ToDo :remove
    //virtual int setPdnRst(int camera, bool on);

private:
    //
    //IspDrv *m_pIspDrv;//6593
    volatile int mUsers;
    mutable Mutex mLock;
    int mfd;
    int m_fdSensor;
    unsigned int *mpIspHwRegAddr;
    unsigned int *mpSeninfHwRegAddr;
    unsigned int *mpCAMIODrvRegAddr;
    unsigned int *mpCAMMMSYSRegAddr;
    unsigned int *mpCSI2RxAnalogRegStartAddr;
    unsigned int *mpCSI2RxAnalogRegStartAddrAlign;
    unsigned int *mpCSI2RxAnalog0RegAddr;
    unsigned int *mpCSI2RxAnalog1RegAddr;
    unsigned int *mpCSI2RxAnalog2RegAddr;
    unsigned int *mpGpioHwRegAddr;
    unsigned int *mpPLLHwRegAddr;        //ToDo:remove
    unsigned int *mpIPllCon0RegAddr;     //ToDo:remove
    unsigned long mDevice;
    int tg1GrabWidth;
    int tg1GrabHeight;
    int tg2GrabWidth;
    int tg2GrabHeight;
    unsigned int mCSI;
    //
    IspDrv* m_pIspDrv;
};

#endif // _ISP_DRV_H_

