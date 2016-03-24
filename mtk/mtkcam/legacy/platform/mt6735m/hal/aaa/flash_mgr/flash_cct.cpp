#define LOG_TAG "flash_cct.cpp"
#ifdef WIN32
#include "stdafx.h"
#include "flash_cct.h"
#include <vector>
#include <deque>

#else
#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
//#include <mtkcam/hal/aaa/aaa_hal_if.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <flash_awb_param.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <af_param.h>
#include <nvram_drv_mgr.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/common.h>
#include <kd_camera_feature.h>
#include <isp_tuning.h>
#include <mtkcam/featureio/tuning_mgr.h>
#include <camera_custom_cam_cal.h>
#include <cam_cal_drv.h>
#include <flash_feature.h>
#include <mtkcam/hal/IHalSensor.h>
#include <ctype.h>
#include <sys/stat.h>
#include <camera_custom_AEPlinetable.h>
#include <ae_mgr.h>
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include <FlashAlg.h>
#include "strobe_drv.h"
#include <time.h>
#include <nvram_drv.h>
#include "flash_util.h"
#include <vector>
#include <mtkcam/acdk/cct_feature.h>
#include <flash_tuning_custom.h>
#include "flash_mgr_m.h"
#include "flash_pline_tool.h"
#include "nvbuf_util.h"
#include <flash_awb_param.h>
#include <flash_awb_tuning_custom.h>
#include <algorithm>
#include <vector>
#include <deque>
//name space
using namespace NS3Av3;
using namespace NSIspTuning;
using namespace std;
#endif

#ifdef WIN32
#define logI(fmt, ...)
#define logE(fmt, ...)
#define logW(fmt, ...)
#else
#define logI(fmt, arg...) ALOGD(fmt, ##arg)
#define logE(fmt, arg...) ALOGD("FlashError: func=%s line=%d: " fmt, __FUNCTION__, __LINE__, ##arg)
#define logW(fmt, arg...) ALOGD("FlashWarning: %5d: " fmt, __LINE__, ##arg)
#endif

enum
{
    e_CaFrm = 10,
};
//==============================
// functions
//==============================
int FlashMgrM::cctFlashEnable(int en)
{
    logI("cctFlashEnable(en=%d)+ line=%d",en,__LINE__);
    int aeMode;
    int flashMode;

    if(en==1)
    {
        aeMode = MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH;
        flashMode = MTK_FLASH_MODE_SINGLE;
    }
    else
    {
        aeMode = MTK_CONTROL_AE_MODE_ON;
        flashMode = MTK_FLASH_MODE_OFF;
    }
    setAeFlashMode(aeMode, flashMode);
    logI("cctFlashEnable()-");
    return 0;
}
//==============================
// ae functions
//==============================
enum
{
    e_AeTooDark=-100,
    e_SensorSetFail=-99,
    e_AeIsTooLong=-98,
    e_ObjIsFar=-97,
    e_LightSourceNotConstant=-96,
};
struct AeOut
{
    int exp;
    int afe;
    int isp;
    int isEnd;
};

struct AeIn
{
    int sensorDev;
    double y;
    int tar;
    int tarMax;
    int tarMin;
};

int adjExp(int& exp, int& afe, int& isp, double m=1);

int g_bAeEpoch=0;
void doAeInit()
{
    g_bAeEpoch=0;
}


#ifdef WIN32
int setexp2(int exp, int afe, int isp);
int setexp(int dev, int exp, int afe, int isp)
{
    return setexp2(exp, afe, isp);
}
#else
int setexp(int dev, int exp, int afe, int isp)
{
    int err;
    err = AAASensorMgr::getInstance().setSensorExpTime(dev, exp);
    if (FAILED(err))
        return e_SensorSetFail;
    err = AAASensorMgr::getInstance().setSensorGain(dev, afe);
    if (FAILED(err))
        return e_SensorSetFail;
    AE_INFO_T rAEInfo2ISP;
    rAEInfo2ISP.u4Eposuretime = exp;
    rAEInfo2ISP.u4AfeGain = afe;
    rAEInfo2ISP.u4IspGain = isp;
    rAEInfo2ISP.u4RealISOValue = afe;
    IspTuningMgr::getInstance().setAEInfo(dev, rAEInfo2ISP);
    IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)dev, MFALSE, isp>>1);
    return 0;
}
#endif

int doAe(AeIn* in, AeOut* out)
{
    int err=0;
    static int exp;
    static int afe;
    static int isp;
    static double tar;
    static double tarMax;
    static double tarMin;
    out->isEnd=0;
    if(g_bAeEpoch==0)
        {
int IniExp=30000;
int IniAfe=1024;
int IniIsp=1024;
            exp=IniExp;
            isp=IniIsp;
            afe=IniAfe;
            tar=in->tar;
            tarMax=in->tarMax;
            tarMin=in->tarMin;
        }
        else
        {
        double ym;
        ym = in->y;
        logI("doAe exp=%d afe=%d isp=%d y=%5.3lf",exp,afe,isp, ym);
        if(ym>tarMin && ym<tarMax)
            {
            logI("doAe done! ln=%d",__LINE__);
                out->isEnd=1;
            }
        else if(ym>250)
            {
            logI("doAe line=%d",__LINE__);
            adjExp(exp, afe, isp, 1/3.0);
            }
        else if(ym<8)
            {
            logI("doAe line=%d",__LINE__);
            adjExp(exp, afe, isp, 1/8.0);
            }
            else
            {
            logI("doAe line=%d",__LINE__);
            adjExp(exp, afe, isp, (double)tar/ym);
            }
        logI("doAe next exp=%d afe=%d isp=%d",exp,afe,isp, ym);
        }
        if(out->isEnd==0)
        {
        setexp(in->sensorDev, exp, afe, isp);
            out->exp = exp;
            out->afe = afe;
            out->isp = isp;
        }
        else
        {
            out->exp = exp;
            out->afe = afe;
            out->isp = isp;
        logI("doAe done frm=%d",g_bAeEpoch);
        }
    g_bAeEpoch++;
    if(g_bAeEpoch>20)
        return e_AeIsTooLong;
    return err;
}

int adjExp(int exp, int afe, int isp, double m, int* exp2, int* afe2, int* isp2)
{
    double expLev;
    expLev = (double)exp*afe*isp/1000/1024/1024*m;
    int err=0;
    if(expLev>=30*4*4)
    {
        exp=30000;
        afe=1024*4;
        isp=1024*4;
        err = e_AeTooDark;
    }
    else if(expLev>30*4)
    {
        exp=30000;
        afe=1024*4;
        isp=(expLev/30/4)*1024;
}
    else if(expLev>30)
{
        exp=30000;
        afe=(expLev/30)*1024;
        isp=1024;
    }
    else
{
        exp=expLev*1000;
        afe=1024;
        isp=1024;
    }
    if(exp2!=0)
        *exp2=exp;
    if(afe2!=0)
        *afe2=afe;
    if(isp2!=0)
        *isp2=isp;
    return err;
    }
int adjExp(int& exp, int& afe, int& isp, double m)
{
    return adjExp(exp, afe, isp, m, &exp, &afe, &isp);
}


void writeCode(const char* f, short* y, int len, int div=40)
{
    FILE* fp;
    fp = fopen(f, "wt");
    if(fp==0)
        return;
    int i;
    fprintf(fp, "    static short engTab[]=\n    {\n");
    for(i=0; i<len; i++)
    {
        if(i%div==0)
            fprintf(fp,"        ");
        fprintf(fp, "%4d,",y[i]);
        if(i%div==(div-1))
            fprintf(fp,"\n");
    }
    fprintf(fp, "    };\n");
    fclose(fp);
}


int my_ind2duty(FlashIndConvPara& para)
{
    if(para.ind>=1600 || para.ind<0)
        goto ERR_RET;
    if(para.isDual==1)
    {
        if(para.ind>= (para.dutyNum+1)*(para.dutyNumLt+1))
            goto ERR_RET;
        para.duty = para.ind%(para.dutyNum+1)-1;
        para.dutyLt = para.ind/(para.dutyNum+1)-1;
    }
    else
    {
        if(para.ind>=para.dutyNum)
            goto ERR_RET;
        para.duty = para.ind;
        para.dutyLt = -1;
    }
    return 0;
//return with err
ERR_RET:
    return -1;
}

void writeCodeGain(const char* f, AWB_GAIN_T* gain, int len, int isDual, int dutyNum, int dutyNumLt, int div=40)
{
    FILE* fp;
    int duty;
    int dutyLt;

    fp = fopen(f, "wt");
    if(fp==0)
        return;
    int i;
    fprintf(fp, "{{\n");
    for(i=0; i<len-1; i++)
    {

        if(isDual == 1)
        {
        FlashIndConvPara conv;
              conv.ind = i;
              conv.isDual=isDual;
              conv.dutyNum = dutyNum;
        conv.dutyNumLt = dutyNumLt;

              int err;
              err = my_ind2duty(conv);
              duty = conv.duty;
              dutyLt = conv.dutyLt;

        fprintf(fp, "   {%4d,%4d,%4d},  //duty=%d, dutyLt=%d  \n",gain[i].i4R,gain[i].i4G,gain[i].i4B, duty, dutyLt);
        }
        else
        {
        fprintf(fp, "   {%4d,%4d,%4d},  \n",gain[i].i4R,gain[i].i4G,gain[i].i4B);
        }
    }
    fprintf(fp, "   {%4d,%4d,%4d}\n",gain[i].i4R,gain[i].i4G,gain[i].i4B);
    fprintf(fp, "}}\n");
    fclose(fp);
}
int FlashMgrM::cctCaliFixAwb2()
{
#ifdef WIN32
    return 0;
#else
    MUINTPTR handle;
    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(mSensorId, "cctSetAaoInitPara");
    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AWB, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_awb_stat_tg1"))))
    {
        //Error Handling
        logE("cctSetAaoInitPara EPIPECmd_GET_MODULE_HANDLE fail!");
        goto lbExit;
    }

    CAM_REG_AE_GAIN2_0 reg_CAM_REG_AE_GAIN2_0;
    reg_CAM_REG_AE_GAIN2_0.Bits.AE_GAIN2_R = 0x200;
    reg_CAM_REG_AE_GAIN2_0.Bits.AE_GAIN2_G = 0x200;
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_GAIN2_0, reg_CAM_REG_AE_GAIN2_0.Raw);

    CAM_REG_AE_GAIN2_1 reg_CAM_REG_AE_GAIN2_1;
    reg_CAM_REG_AE_GAIN2_1.Bits.AE_GAIN2_B = 0x200;
    IOPIPE_SET_MODUL_REG(handle, CAM_AE_GAIN2_1, reg_CAM_REG_AE_GAIN2_1.Raw);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
    {
        //Error Handling
        logE("cctSetAaoInitPara EPIPECmd_SET_MODULE_CFG_DONE fail!");
        goto lbExit;
    }

lbExit:
    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("cctSetAaoInitPara")), MNULL))
    {
        // Error Handling
        logE("EPIPECmd_RELEASE_MODULE_HANDLE fail");
    }
    pPipe->destroyInstance("isp_mgr_awb_stat_tg1");
    return MTRUE;
#endif
}

int FlashMgrM::cctSetAaoInitPara()
{
#ifdef WIN32
    return 0;
#else
    MUINTPTR handle;
    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(mSensorId, "cctSetAaoInitPara");
    // get module handle
    if (MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                           NSImageio::NSIspio::EModule_AWB, (MINTPTR)&handle, (MINTPTR)(&("isp_mgr_awb_stat_tg1"))))
    {
        //Error Handling
        logE("cctSetAaoInitPara EPIPECmd_GET_MODULE_HANDLE fail!");
        goto lbExit;
    }

    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_ORG, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_ORG].val);
    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_SIZE, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_SIZE].val);
    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_PIT, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_PIT].val);
    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_WIN_NUM, m_rIspRegInfo[ERegInfo_CAM_AWB_WIN_NUM].val);

    //CAM_REG_AWB_GAIN1_0 reg_CAM_AWB_GAIN1_0;
    //reg_CAM_AWB_GAIN1_0.Bits.AWB_GAIN1_R = 0x200;
    //reg_CAM_AWB_GAIN1_0.Bits.AWB_GAIN1_G = 0x200;
    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_GAIN1_0, reg_CAM_AWB_GAIN1_0.Raw);

    //CAM_REG_AWB_GAIN1_1 reg_CAM_AWB_GAIN1_1;
    //reg_CAM_AWB_GAIN1_1.Bits.AWB_GAIN1_B = 0x200;
    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_GAIN1_1, reg_CAM_AWB_GAIN1_1.Raw);

    CAM_REG_AWB_LMT1_0 reg_CAM_AWB_LMT1_0;
    reg_CAM_AWB_LMT1_0.Bits.AWB_LMT1_R = 0xfff;
    reg_CAM_AWB_LMT1_0.Bits.AWB_LMT1_G = 0xfff;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LMT1_0, reg_CAM_AWB_LMT1_0.Raw);

    CAM_REG_AWB_LMT1_1 reg_CAM_AWB_LMT1_1;
    reg_CAM_AWB_LMT1_1.Bits.AWB_LMT1_B = 0xfff;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LMT1_1, reg_CAM_AWB_LMT1_1.Raw);

    CAM_REG_AWB_LOW_THR reg_CAM_AWB_LOW_THR;
    reg_CAM_AWB_LOW_THR.Bits.AWB_LOW_THR0 = 0;
    reg_CAM_AWB_LOW_THR.Bits.AWB_LOW_THR1 = 0;
    reg_CAM_AWB_LOW_THR.Bits.AWB_LOW_THR2 = 0;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_LOW_THR, reg_CAM_AWB_LOW_THR.Raw);

    CAM_REG_AWB_HI_THR reg_CAM_AWB_HI_THR;
    reg_CAM_AWB_HI_THR.Bits.AWB_HI_THR0 = 255;
    reg_CAM_AWB_HI_THR.Bits.AWB_HI_THR1 = 255;
    reg_CAM_AWB_HI_THR.Bits.AWB_HI_THR2 = 255;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_HI_THR, reg_CAM_AWB_HI_THR.Raw);

    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT0, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT0].val);
    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT1, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT1].val);
    //IOPIPE_SET_MODUL_REG(handle, CAM_AWB_PIXEL_CNT2, m_rIspRegInfo[ERegInfo_CAM_AWB_PIXEL_CNT2].val);

    CAM_REG_AWB_ERR_THR reg_CAM_AWB_ERR_THR;
    reg_CAM_AWB_ERR_THR.Bits.AWB_ERR_THR = 0;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_ERR_THR, reg_CAM_AWB_ERR_THR.Raw);

    CAM_REG_AWB_ROT reg_CAM_AWB_ROT;
    reg_CAM_AWB_ROT.Bits.AWB_C = 0x100;
    reg_CAM_AWB_ROT.Bits.AWB_S = 0x0;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_ROT, reg_CAM_AWB_ROT.Raw);

    CAM_REG_AWB_L0_X reg_CAM_AWB_L0_X;
    reg_CAM_AWB_L0_X.Bits.AWB_L0_X_LOW = 0x3388;
    reg_CAM_AWB_L0_X.Bits.AWB_L0_X_UP = 0x1388;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L0_X, reg_CAM_AWB_L0_X.Raw);

    CAM_REG_AWB_L0_Y reg_CAM_AWB_L0_Y;
    reg_CAM_AWB_L0_Y.Bits.AWB_L0_Y_LOW = 0x3388;
    reg_CAM_AWB_L0_Y.Bits.AWB_L0_Y_UP = 0x1388;
    IOPIPE_SET_MODUL_REG(handle, CAM_AWB_L0_Y, reg_CAM_AWB_L0_Y.Raw);

    // set module config done
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
    {
        //Error Handling
        logE("cctSetAaoInitPara EPIPECmd_SET_MODULE_CFG_DONE fail!");
        goto lbExit;
    }

lbExit:
    // release handle
    if (MFALSE==pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("cctSetAaoInitPara")), MNULL))
    {
        // Error Handling
        logE("EPIPECmd_RELEASE_MODULE_HANDLE fail");
    }
    pPipe->destroyInstance("isp_mgr_awb_stat_tg1");
    return MTRUE;
#endif
}

class CaliData
{
public:
    int duty;
    int dutyLT;
    float ref;
    int exp;
    int afe;
    int isp;
    float y[e_CaFrm];
    float r[e_CaFrm];
    float g[e_CaFrm];
    float b[e_CaFrm];
    int t[e_CaFrm];
};

typedef std::deque<CaliData> dqCaliData;
typedef std::vector<int> vectorInt;
CaliData g_CaliDataDark[2];
dqCaliData g_CaliData;
dqCaliData g_CaliData1;
dqCaliData g_CaliData2;
dqCaliData g_CaliData3;
vectorInt g_dutyArr;
vectorInt g_dutyArrLt;

int cmpCaliData(const void* a, const void* b)
{
    const CaliData* pa;
    const CaliData* pb;
    pa = (CaliData*)a;
    pb = (CaliData*)b;
    if((pa->ref)> (pb->ref))
        return -1;
    else if((pa->ref)< (pb->ref))
        return 1;
    else
        return 0;
}

int FlashMgrM::cctCalibration(FlashExePara* para, FlashExeRep* rep)
{
#define STATE_INIT 0
#define STATE_AE 1
#define STATE_AE_POST 2
#define STATE_RATIO 3
#define STATE_RATIO2 4
#define STATE_END 5
#define STATE_END2 6
    static int state=STATE_INIT;
    rep->isEnd=0;
    int i;
    int j;
    int err=0;
    static DUTY_MASK_FUNC dutyMaskFunc;
    static int dutyNum;
    static int dutyNumLt;
    static int preStateEndCnt=-1;
    static int exp;
    static int isp;
    static int afe;
    static int expS1;
    static int ispS1;
    static int afeS1;
    static int curId=0;
    static int frameTar=0;
    static int frameBase=0;
    static int ratioRound=0;

logI("cctCalibration st=%d ln=%d",state, __LINE__);
    if(m_pfFrameCount==0)
    {
        cctSetAaoInitPara();
        g_CaliData.clear();
        g_CaliData1.clear();
        g_CaliData2.clear();
        g_dutyArr.clear();
        g_dutyArrLt.clear();
        dutyNum =0;
        dutyNumLt =0;
        preStateEndCnt=0;
        exp=0;
        afe=0;
        isp=0;
        curId=0;
        frameTar=0;
        frameBase=0;
        ratioRound=0;
        state=STATE_INIT;
        ratioRound=1;
    }
    if(state==STATE_INIT)
    {
        logI("cctPreflashTest state=STATE_INIT line=%d",__LINE__);
        NVRAM_CAMERA_STROBE_STRUCT* pNv;
        nvGetBuf(pNv);
        FLASH_PROJECT_PARA prjPara;
        prjPara = getAutoProjectPara();
        dutyNum = prjPara.dutyNum;
        dutyNumLt = 1;
        dutyMaskFunc = prjPara.dutyAvailableMaskFunc;

        if(cust_isDualFlashSupport(m_sensorDev)==0)
            {
            g_dutyArr.push_back(-1);
            g_dutyArrLt.push_back(-1);
            for(i=0;i<prjPara.dutyNum;i++)
            {
                g_dutyArr.push_back(i);
                g_dutyArrLt.push_back(-1);
            }
            g_dutyArr.push_back(-1);
            g_dutyArrLt.push_back(-1);
            StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
            pStrobe->initTemp();
        }
        else
        {
            dutyNumLt = prjPara.dutyNumLT;
            for(j=-1;j<prjPara.dutyNumLT;j++)
            for(i=-1;i<prjPara.dutyNum;i++)
            {
                g_dutyArr.push_back(i);
                g_dutyArrLt.push_back(j);
            }
            g_dutyArr.push_back(-1);
            g_dutyArrLt.push_back(-1);

            StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
            StrobeDrv* pStrobe2 = StrobeDrv::getInstance(m_sensorDev, 2);
            pStrobe->initTemp();
            pStrobe2->initTemp();
        }
        state=STATE_AE;
        preStateEndCnt=m_pfFrameCount;
        doAeInit();
    }
    if(state==STATE_AE)
    {
        logI("cctPreflashTest state=STATE_AE line=%d",__LINE__);
        int aeCycle=20;
        if(m_pfFrameCount%aeCycle==0)
        {
            StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
            usleep(15000);
            pStrobe->setDuty(dutyNum-1);
            pStrobe->setTimeOutTime(300);
            pStrobe->setOnOff(0);
            pStrobe->setOnOff(1);
        }
        else if(m_pfFrameCount%aeCycle==3)
        {
            double yrgb[4];
            cal_center_3aSta_yrgb_romeHw(para->staBuf, para->staX, para->staY, yrgb);
            logI("STATE_AE yrgb=%5.3lf %5.3lf %5.3lf %5.3lf",yrgb[0],yrgb[1],yrgb[2],yrgb[3]);
            AeIn in;
            AeOut out;
            in.sensorDev = m_sensorDev;
            in.y = yrgb[2];
            in.tar = 200;
            in.tarMax = 220;
            in.tarMin = 180;
            out.isEnd=0;
            StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
            pStrobe->setOnOff(0);
            err = doAe(&in, &out);
        if(out.isEnd==1)
        {
                logI("cctPreflashTest ae_end line=%d",__LINE__);
                StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
            pStrobe->setOnOff(0);
            state=STATE_AE_POST;
                preStateEndCnt = m_pfFrameCount;
            exp = out.exp;
            isp = out.isp;
            afe = out.afe;
                expS1 = exp;
                ispS1 = isp;
                afeS1 = afe;
            }
        }
    }
    else if(state==STATE_AE_POST)
    {
        logI("cctPreflashTest state=STATE_AE_POST line=%d",__LINE__);
        if(m_pfFrameCount>preStateEndCnt+60)
        {
            state=STATE_RATIO;
            preStateEndCnt=m_pfFrameCount;

        }
    }
    else if(state==STATE_RATIO)
    {
NextId:
        if(m_pfFrameCount==preStateEndCnt+1)
        {
            curId = 0;
            frameBase = m_pfFrameCount;
            frameTar = m_pfFrameCount+40;
        }
        if(m_pfFrameCount==frameTar)
        {
            if(ratioRound==2)
            {
                double maxY=0;
                for(j=0;j<e_CaFrm;j++)
                {
                    if(g_CaliData[curId].g[j]>maxY)
                        maxY = g_CaliData[curId].g[j];
                }
                int ret;
                ret = adjExp(exp, isp, afe, 1);
                if(maxY<50 && ret==0)
                {
                    ret = adjExp(exp, isp, afe, 4);
                    g_CaliData.pop_back();
                }
                else
                {
                    curId ++;
                }
                frameBase = m_pfFrameCount;
                frameTar = m_pfFrameCount+40;
            }
            else
            {
                curId ++;
                frameBase = m_pfFrameCount;
                frameTar = m_pfFrameCount+40;
            }
        }
        int cnt;
        cnt = m_pfFrameCount-frameBase;
        logI("cctPreflashTest state=STATE_RATIO line=%d id=%d cnt=%d",__LINE__,curId,cnt);
        //end or next round
        if(curId>=(int)g_dutyArr.size())
        {
            if(ratioRound==1)
            {
                ratioRound=2;
                g_dutyArr.clear();
                g_dutyArrLt.clear();
                int sz;
                sz = g_CaliData.size();
                g_CaliDataDark[0] = g_CaliData[0];
                g_CaliDataDark[1] = g_CaliData[sz-1];
                g_CaliData.pop_back();
                g_CaliData.pop_front();
                sz = g_CaliData.size();
                for(i=0;i<sz;i++)
                {
                    int j;
                    float maxY=0;
                    for(j=0;j<e_CaFrm;j++)
                    {
                        if(g_CaliData[i].g[j]>maxY)
                            maxY = g_CaliData[i].g[j];
                    }
                    if(maxY<50)
                    {
                        CaliData d;
                        d = g_CaliData[i];
                        d.ref = maxY;
                        g_CaliData2.push_back(d);
                    }
                }
                g_CaliData1 = g_CaliData;
                sz = g_CaliData2.size();
                if(sz==0)
        {
                    preStateEndCnt=m_pfFrameCount;
                    state=STATE_RATIO2;
            goto FUNC_NEXT;
        }
                else
        {
                    CaliData* dd;
                    dd = new CaliData[sz];
                    for(i=0;i<sz;i++)
                        dd[i]=g_CaliData2[i];
                    std::qsort(dd, sz, sizeof(CaliData), cmpCaliData);
                    for(i=0;i<sz;i++)
                        g_CaliData2[i]=dd[i];
                    delete []dd;
                    for(i=0;i<sz;i++)
            {
                        g_dutyArr.push_back(g_CaliData2[i].duty);
                        g_dutyArrLt.push_back(g_CaliData2[i].dutyLT);
                    }
                    g_CaliData2.clear();
                    g_CaliData.clear();
                    adjExp(exp, isp, afe, 4);
                    frameBase = m_pfFrameCount+1;
                    frameTar = m_pfFrameCount+41;
                    curId=0;
                    goto FUNC_NEXT;
                }
                preStateEndCnt = m_pfFrameCount;
            }
            else
        {
                g_CaliData2=g_CaliData;
                preStateEndCnt=m_pfFrameCount;
                state=STATE_RATIO2;
                goto FUNC_NEXT;
        }
    }
        if(cnt==0)
    {
            if(ratioRound==2)
                setexp(m_sensorDev, exp, afe, isp);
        }
        if(cnt==3)
        {
            StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
            StrobeDrv* pStrobe2 = StrobeDrv::getInstance(m_sensorDev, 2);
            int bOnTest=0;
            if(cust_isDualFlashSupport(m_sensorDev)==0)
                bOnTest=1;
            else if(g_dutyArr[curId]==-1 && g_dutyArrLt[curId]==-1)
                bOnTest=1;
            else if(dutyMaskFunc(g_dutyArr[curId], g_dutyArrLt[curId])==1)
                bOnTest=1;
            if(bOnTest==1)
            {
                CaliData d;
                d.duty = g_dutyArr[curId];
                d.dutyLT = g_dutyArrLt[curId];
                d.exp = exp;
                d.afe = afe;
                d.isp = isp;
                if(g_dutyArr[curId]>=0)
            {
                    pStrobe->setTimeOutTime(300);
                    pStrobe->setDuty(g_dutyArr[curId]);
                    pStrobe->setOnOff(0);
                    pStrobe->setOnOff(1);
            }
                else
                    pStrobe->setOnOff(0);
                if(g_dutyArrLt[curId]>=0)
                {
                    pStrobe2->setTimeOutTime(300);
                    pStrobe2->setDuty(g_dutyArrLt[curId]);
                    pStrobe2->setOnOff(0);
                    pStrobe2->setOnOff(1);
        }
                else
                    pStrobe2->setOnOff(0);
                g_CaliData.push_back(d);

                }
        else
        {
                pStrobe->setOnOff(0);
                pStrobe2->setOnOff(0);
                frameTar=m_pfFrameCount;
                goto NextId;
            }

    }
        if(cnt==8)
        {
            StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
            StrobeDrv* pStrobe2 = StrobeDrv::getInstance(m_sensorDev, 2);
            pStrobe->setOnOff(0);
            pStrobe2->setOnOff(0);
}
        if(cnt>=3 && cnt<=12)
{
            double yrgb[4];
            cal_center_3aSta_yrgb_romeHw(para->staBuf, para->staX, para->staY, yrgb);
    /*
            FILE* fp;
            char ss[100];
            sprintf(ss, "/sdcard/fe_%02d%02d.txt",id,cnt);
            fp = fopen(ss, "wb");
            fwrite(para->staBuf, 1, para->staX*para->staY*5, fp);
            fclose(fp);
            */
            int sz;
            sz = g_CaliData.size();
            g_CaliData[sz-1].y[cnt-3]=yrgb[0];
            g_CaliData[sz-1].r[cnt-3]=yrgb[1];
            g_CaliData[sz-1].g[cnt-3]=yrgb[2];
            g_CaliData[sz-1].b[cnt-3]=yrgb[3];
            g_CaliData[sz-1].t[cnt-3]=FlashUtil::getMs();
            logI("cctPreflashTest id=%d, cnt=%d, yrgb=%5.3lf %5.3lf %5.3lf %5.3lf",curId,cnt-3,yrgb[0],yrgb[1],yrgb[2],yrgb[3]);

        }
    }
    else if(state==STATE_RATIO2)
    {
        static int testNum=0;
        static int duty[3]={-1,-1,-1};
        static int dutyLt[3]={-1,-1,-1};
        if(m_pfFrameCount==preStateEndCnt+1)
        {
            frameBase = m_pfFrameCount;
            setexp(m_sensorDev, expS1, afeS1, ispS1);
            int duty1=-1;
            int dutyLt2=-1;
            int duty3=-1;
            int dutyLt3=-1;
            float maxYRef1=-1;
            float maxYRef2=-1;
            float maxYRef3=-1;
    int sz;
            sz = g_CaliData1.size();
            testNum=0;
    for(i=0;i<sz;i++)
    {
                int j;
                float maxY=0;
                for(j=0;j<e_CaFrm;j++)
                {
                    if(g_CaliData1[i].g[j]>maxY)
                        maxY = g_CaliData1[i].g[j];
    }
                if(maxYRef1<maxY && g_CaliData1[i].dutyLT==-1)
                {
                    maxYRef1=maxY;
                    duty1 = g_CaliData1[i].duty;
}
                if(maxYRef2<maxY && g_CaliData1[i].duty==-1)
{
                    maxYRef2=maxY;
                    dutyLt2 = g_CaliData1[i].dutyLT;
}
                if(maxYRef3<maxY)
{
                    maxYRef3=maxY;
                    duty3 = g_CaliData1[i].duty;
                    dutyLt3 = g_CaliData1[i].dutyLT;
}
}
            if(maxYRef1!=-1)
            {
                duty[testNum]=duty1;
                dutyLt[testNum]=-1;
                testNum++;
}
            if(maxYRef2!=-1)
    {
                duty[testNum]=-1;
                dutyLt[testNum]=dutyLt2;
                testNum++;
    }
            if((duty3!=-1 && dutyLt3!=-1)|| testNum==0)
    {
                duty[testNum]=duty3;
                dutyLt[testNum]=dutyLt3;
                testNum++;
    }
            g_CaliData3.clear();
}
        int id;
        int cnt;
        id =  (m_pfFrameCount-frameBase)/80;
        cnt = (m_pfFrameCount-frameBase)%80;
        StrobeDrv* pStrobe = StrobeDrv::getInstance(m_sensorDev);
        StrobeDrv* pStrobe2 = StrobeDrv::getInstance(m_sensorDev, 2);
        if(id>=testNum)
        {
            preStateEndCnt=m_pfFrameCount;
            state=STATE_END;
            goto FUNC_NEXT;
    }
        if(cnt==0)
        {
            if(duty[id]>=0)
    {
                pStrobe->setTimeOutTime(300);
                pStrobe->setDuty(duty[id]);
                pStrobe->setOnOff(0);
                pStrobe->setOnOff(1);
    }
            else
                pStrobe->setOnOff(0);
            if(dutyLt[id]>=0)
            {
                pStrobe2->setTimeOutTime(300);
                pStrobe2->setDuty(dutyLt[id]);
                pStrobe2->setOnOff(0);
                pStrobe2->setOnOff(1);
}
            else
                pStrobe2->setOnOff(0);
            CaliData d;
            d.exp = expS1;
            d.afe = afeS1;
            d.isp = ispS1;
            d.duty = duty[id];
            d.dutyLT = dutyLt[id];
            g_CaliData3.push_back(d);
    }
        if(cnt<10)
{
            double yrgb[4];
            cal_center_3aSta_yrgb_romeHw(para->staBuf, para->staX, para->staY, yrgb);
    int sz;
            CaliData d;
            g_CaliData3[id].y[cnt]=yrgb[0];
            g_CaliData3[id].r[cnt]=yrgb[1];
            g_CaliData3[id].g[cnt]=yrgb[2];
            g_CaliData3[id].b[cnt]=yrgb[3];
            g_CaliData3[id].t[cnt]=FlashUtil::getMs();
    }
}
    else if(state==STATE_END)
    {
        //write text file
        logI("cctPreflashTest state=STATE_END line=%d",__LINE__);
        FILE* fp;
#ifdef WIN32
        fp = fopen("d:\\temp\\eng_all.txt","wt");
#else
        fp = fopen("/sdcard/eng_all.txt","wt");
#endif
        if(fp!=0)
{
            fprintf(fp,"t\texp\tafe\tisp\tduty\tdutyLt\ty\tr\tg\tb\n");
    int sz;
            sz = g_CaliData1.size();
            for(j=0;j<sz;j++)
    {
                for(i=0;i<e_CaFrm;i++)
    {
                    fprintf(fp,"%d\t%d\t%d\t%d\t%d\t%d\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
                        g_CaliData1[j].t[i], g_CaliData1[j].exp, g_CaliData1[j].afe, g_CaliData1[j].isp, g_CaliData1[j].duty, g_CaliData1[j].dutyLT, g_CaliData1[j].y[i], g_CaliData1[j].r[i], g_CaliData1[j].g[i], g_CaliData1[j].b[i]);
    }
}
            fprintf(fp,"============\n");
            sz = g_CaliData2.size();
            for(j=0;j<sz;j++)
{
                for(i=0;i<e_CaFrm;i++)
    {
                    fprintf(fp,"%d\t%d\t%d\t%d\t%d\t%d\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
                        g_CaliData2[j].t[i], g_CaliData2[j].exp, g_CaliData2[j].afe, g_CaliData2[j].isp, g_CaliData2[j].duty, g_CaliData2[j].dutyLT, g_CaliData2[j].y[i], g_CaliData2[j].r[i], g_CaliData2[j].g[i], g_CaliData2[j].b[i]);
                }
    }
            fprintf(fp,"============\n");
            sz = g_CaliData3.size();
            for(j=0;j<sz;j++)
            {
                for(i=0;i<e_CaFrm;i++)
    {
                    fprintf(fp,"%d\t%d\t%d\t%d\t%d\t%d\t%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
                        g_CaliData3[j].t[i], g_CaliData3[j].exp, g_CaliData3[j].afe, g_CaliData3[j].isp, g_CaliData3[j].duty, g_CaliData3[j].dutyLT, g_CaliData3[j].y[i], g_CaliData3[j].r[i], g_CaliData3[j].g[i], g_CaliData3[j].b[i]);
                }
    }
            fclose(fp);
}
        //----------------
        //dark & ob
        double ymDark1=0;
        double ymDark2=0;
        double difY;
        for(i=0;i<e_CaFrm;i++)
        {
            ymDark1+=g_CaliDataDark[0].y[i];
            ymDark2+=g_CaliDataDark[1].y[i];
        }
        ymDark1/=e_CaFrm;
        ymDark2/=e_CaFrm;
        difY = ymDark1-ymDark2;
        if(difY<0)
            difY=-difY;
        if(difY>8)
{
            err=e_LightSourceNotConstant;
            goto FUNC_END;
        }
        double yob=0;
        yob = (ymDark1+ymDark2)/2;
        double ymax=0;
        //
    int sz;
        sz  = g_CaliData1.size();
        for(i=0;i<sz;i++)
    {
            for(j=0;j<e_CaFrm;j++)
    {
                if(g_CaliData1[i].y[j]>ymax)
                    ymax=g_CaliData1[i].y[j];
    }
}
        if(ymax<2*yob)
        {
            err=e_ObjIsFar;
            rep->isEnd=1;
            goto FUNC_END;
        }
        float magRatoi;
        magRatoi = 9999/(ymax-yob);
        for(i=0;i<sz;i++)
{
            for(j=0;j<e_CaFrm;j++)
            {
                g_CaliData1[i].y[j] = (g_CaliData1[i].y[j]-yob)*magRatoi;
                ymax=g_CaliData1[i].y[j];
            }
        }
        NVRAM_CAMERA_STROBE_STRUCT* nv;
        nvGetBuf(nv, 0);
        NVRAM_CAMERA_3A_STRUCT* nv3A;
    #ifdef WIN32
        nv3A = (NVRAM_CAMERA_3A_STRUCT*)getNv3ABuf();
    #else
        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_sensorDev, (void*&)nv3A);
    #endif
        if(err!=0)
            logE("err=%d",err);
        //nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain;

        for(i=0;i<1600;i++)
    {
            nv->engTab.yTab[i]=0;
    }

        int Vr;
        int Vg;
        int Vb;
        sz = g_CaliData1.size();
        for(i=0;i<1600;i++)
        {
            int ind=-1;


            int isDual;
            int duty;
            int dutyLt;


            //duty=(i%(dutyNum+1))-1;
            //dutyLt=(i/(dutyNum+1))-1;
            isDual = cust_isDualFlashSupport(m_sensorDev);
            //---------------

            FlashIndConvPara conv;
            conv.ind = i;
            conv.isDual=isDual;
            conv.dutyNum = dutyNum;
            conv.dutyNumLt = dutyNumLt;
            int err;
            err = ind2duty(conv);
            duty = conv.duty;
            dutyLt = conv.dutyLt;

            //---------------
            if(err==0)
            for(j=0;j<sz;j++)
            {
                if(g_CaliData1[j].duty == duty &&
                    (g_CaliData1[j].dutyLT ==dutyLt || (!isDual))  )
    {
                    ind=j;
                    break;
    }
}
            ymax=-1;
            Vr = -1;
            Vg = -1;
            Vb = -1;
            int bSigSmall=0;
            if(ind!=-1)
{
                for(j=0;j<e_CaFrm;j++)
    {
                    if(ymax<g_CaliData1[ind].y[j])
    {
                        ymax=g_CaliData1[ind].y[j];
                        Vr=g_CaliData1[ind].r[j];
                        Vg=g_CaliData1[ind].g[j];
                        Vb=g_CaliData1[ind].b[j];
    }
}
                if(Vg<=50)
                    bSigSmall=1;
            }

            ind=-1;
            if(bSigSmall==1 )
{
                int sz2;
                 sz2 = g_CaliData2.size();
                for(j=0;j<sz2;j++)
    {
                    if(  g_CaliData2[j].duty == duty &&
                        (g_CaliData2[j].dutyLT ==dutyLt || (!isDual))  )
    {
                        ind=j;
                        break;
    }
}
                if(ind!=-1)
                {
                    double ymax2;
                    ymax2=0;
                    for(j=0;j<e_CaFrm;j++)
                    {
                        if(ymax2<g_CaliData2[ind].y[j])
    {
                            ymax2=g_CaliData2[ind].y[j];
                            Vr=g_CaliData2[ind].r[j];
                            Vg=g_CaliData2[ind].g[j];
                            Vb=g_CaliData2[ind].b[j];
                        }
    }
}
    }
            nv->engTab.yTab[i]=ymax;
            nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain[i].i4R = Vg*512/Vr;
            nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain[i].i4G = 512;
            nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain[i].i4B = Vg*512/Vb;

}
#ifdef WIN32
        if(cust_isDualFlashSupport(m_sensorDev)==1)
{
            writeCode("d:\\temp\\eng_code.txt", nv->engTab.yTab, (dutyNumLt+1)*(dutyNum+1), dutyNum+1);
                     writeCodeGain("d:\\temp\\fwb_code.txt", nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain, 1600, 1, dutyNum, dutyNumLt);
}
        else
{
            writeCode("d:\\temp\\eng_code.txt", nv->engTab.yTab, dutyNum, dutyNum+1);
               writeCodeGain("d:\\temp\\fwb_code.txt", nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain, 1600, 0, dutyNum, dutyNumLt);
}
#else
        if(cust_isDualFlashSupport(m_sensorDev)==1)
{
            writeCode("/sdcard/eng_code.txt", nv->engTab.yTab, (dutyNumLt+1)*(dutyNum+1), dutyNum+1);
               writeCodeGain("/sdcard/fwb_code.txt", nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain, 1600, 1, dutyNum, dutyNumLt);
    }
        else
    {
            writeCode("/sdcard/eng_code.txt", nv->engTab.yTab, dutyNum, dutyNum+1);
               writeCodeGain("/sdcard/fwb_code.txt", nv3A->rFlashAWBNVRAM.rCalibrationData.flashWBGain, 1600, 0, dutyNum, dutyNumLt);
        }
#endif
        //----------------
        g_CaliData.clear();
        g_CaliData1.clear();
        g_CaliData2.clear();
        g_CaliData3.clear();
        g_dutyArr.clear();
        g_dutyArrLt.clear();
        rep->isEnd=1;
        state = STATE_END2;
        goto FUNC_END;
    }
FUNC_NEXT:
FUNC_END:
    if(err!=0)
    {
#ifdef WIN32
#else
        rep->isEnd=1;
         AE_MODE_CFG_T capInfo;
        AeMgr::getInstance(m_sensorDev).getCaptureParams(capInfo);
        capInfo.u4Eposuretime = 1000;
        capInfo.u4AfeGain = 1024;
        capInfo.u4IspGain = 1024;
        m_flashOnPrecapture=0;
        AeMgr::getInstance(m_sensorDev).updateCaptureParams(capInfo);
#endif
    }
    return err;
    }

#ifdef WIN32
#else
//==============================
//==============================
//ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META
int FlashMgrM::cctReadNvramToPcMeta(void* out, MUINT32* realOutSize)
{
    //---------------
    // check input
    logI("cctReadNvramToPcMeta+");
    logI("pOut=%p pRealOutSize=%d", out, *realOutSize);
    //---------------
    NVRAM_CAMERA_STROBE_STRUCT* pNv;
    nvGetBuf(pNv, 1);
    memcpy((void*)out, pNv, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
    logI("realOutSize=%d",*realOutSize);
    logI("cctReadNvramToPcMeta-");
    return 0;
}
//==============================
//ACDK_CCT_OP_STROBE_SET_NVDATA_META
int FlashMgrM::cctSetNvdataMeta(void* in, int inSize)
{
    //---------------
    // check input
    logI("cctSetNvdataMeta+");
    logI("pIn=%p inSz=%d", in, inSize);
    int sz;
    sz = sizeof(ACDK_STROBE_STRUCT);
    if(inSize!=sz)
    {
        logE("inSize is wrong");
        return FL_ERR_CCT_INPUT_SIZE_WRONG;
    }
    //---------------
    NVRAM_CAMERA_STROBE_STRUCT* pNv;
    nvGetBuf(pNv);
    memcpy( pNv, (void*)in, sizeof(NVRAM_CAMERA_STROBE_STRUCT));
    logI("cctSetNvdataMeta-");
    return 0;
}

//==============================
//ACDK_CCT_OP_STROBE_WRITE_NVRAM
int FlashMgrM::cctWriteNvram()
    {
    logI("cctWriteNvram+");
    int err;
    err = nvWrite();
    logI("cctWriteNvram-");
    return err;
    }
#endif
