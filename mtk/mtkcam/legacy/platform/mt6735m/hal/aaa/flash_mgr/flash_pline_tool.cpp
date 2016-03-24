
#ifdef WIN32
#include "stdafx.h"
#include "win_test.h"
#include "flashpline.h"
#include "math.h"

#define logI(fmt, ...)
#else
#define LOG_TAG "flash_pline_tool.cpp"
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
//ae inc
#include <ctype.h>
#include <sys/stat.h>
#include <camera_custom_AEPlinetable.h>
#include <ae_mgr.h>
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
#include "strobe_drv.h"
#include <time.h>
#include <nvram_drv.h>
#include "flash_util.h"
#include <vector>
#include <mtkcam/acdk/cct_feature.h>
#include <flash_tuning_custom.h>
#include "flash_mgr_m.h"
#include "flash_pline_tool.h"
#include "math.h"
#include "mtkcam/metadata/client/mtk_metadata_tag.h"


#define logI(fmt, arg...) ALOGD(fmt, ##arg)
#endif
//==============================
//function
//==============================
FlashPline::FlashPline()
{
    pEvSetting=0;
}
FlashPline::~FlashPline()
{
    if(pEvSetting!=0)
        delete []pEvSetting;
}

double log2(double v)
{
    return log(v)/log(2.0);
}

int FlashPline::aePline2FlashAlgPline(PLine* p, const strAETable* pAE, int maxExp)
{
    logI("aePline2FlashAlgPline");
    int i;
    int len;
    len = pAE->u4TotalIndex;
    if(pEvSetting!=0)
        delete []pEvSetting;
    pEvSetting = new evSetting[len];

    /*
    FILE* fp;
    fp = fopen("/sdcard/ffpline.txt","wt");

    for(i=0; i<(int)pAE->u4TotalIndex; i++)
    {
        fprintf(fp,"%d\t%d\t%d\n",
            pAE->pCurrentTable[i].u4Eposuretime,
            pAE->pCurrentTable[i].u4AfeGain,
            pAE->pCurrentTable[i].u4IspGain);
    }

    fclose(fp);
    */

    double evModify=0;
    double maxGainLevel=0;
    int maxAfe=0;
    int maxIsp=0;
    if(maxExp!=0)
    {
        if((int)pAE->pCurrentTable->sPlineTable[len-1].u4Eposuretime>maxExp)
        {
            evModify = log2(pAE->pCurrentTable->sPlineTable[len-1].u4Eposuretime/(double)maxExp);
            evModify*=10;
            maxAfe = pAE->pCurrentTable->sPlineTable[len-1].u4AfeGain;
            maxIsp = pAE->pCurrentTable->sPlineTable[len-1].u4IspGain;
            maxGainLevel = (double)maxAfe*maxIsp;
        }
    }

    p->i4MaxBV=pAE->i4MaxBV;
    p->i4MinBV=pAE->i4MinBV+evModify;
    p->u4TotalIndex=pAE->u4TotalIndex;
    p->i4StrobeTrigerBV=pAE->i4StrobeTrigerBV;

    p->pCurrentTable = pEvSetting;
    for(i=0; i<(int)pAE->u4TotalIndex; i++)
    {
        if(maxExp==0)
        {
            p->pCurrentTable[i].u4Eposuretime = pAE->pCurrentTable->sPlineTable[i].u4Eposuretime;
            p->pCurrentTable[i].u4AfeGain = pAE->pCurrentTable->sPlineTable[i].u4AfeGain;
            p->pCurrentTable[i].u4IspGain = pAE->pCurrentTable->sPlineTable[i].u4IspGain;
        }
        else
        {
            double expLevel;
            double gainLevel;
            int exp;
            int afe;
            int isp;
            exp = pAE->pCurrentTable->sPlineTable[i].u4Eposuretime;
            afe = pAE->pCurrentTable->sPlineTable[i].u4AfeGain;
            isp = pAE->pCurrentTable->sPlineTable[i].u4IspGain;
            expLevel = (double)exp*afe*isp;
            if(exp>maxExp)
            {
                p->pCurrentTable[i].u4Eposuretime = maxExp;
                gainLevel = expLevel/maxExp;
                if(gainLevel>maxGainLevel)
                {
                    p->pCurrentTable[i].u4AfeGain = maxAfe;
                    p->pCurrentTable[i].u4IspGain = maxIsp;
                }
                else if(gainLevel/1024 > maxAfe)
                {
                    p->pCurrentTable[i].u4AfeGain = maxAfe;
                    p->pCurrentTable[i].u4IspGain = expLevel/maxExp/maxAfe;
                }
                else
                {
                    p->pCurrentTable[i].u4AfeGain = gainLevel/1024;
                    p->pCurrentTable[i].u4IspGain = 1024;
                }
            }
            else
            {
                p->pCurrentTable[i].u4Eposuretime = pAE->pCurrentTable->sPlineTable[i].u4Eposuretime;
                p->pCurrentTable[i].u4AfeGain = pAE->pCurrentTable->sPlineTable[i].u4AfeGain;
                p->pCurrentTable[i].u4IspGain = pAE->pCurrentTable->sPlineTable[i].u4IspGain;
            }
        }
        p->pCurrentTable[i].uIris = pAE->pCurrentTable->sPlineTable[i].uIris;
        p->pCurrentTable[i].uSensorMode = pAE->pCurrentTable->sPlineTable[i].uSensorMode;
        p->pCurrentTable[i].uFlag = pAE->pCurrentTable->sPlineTable[i].uFlag;
    }

    /*
    FILE* fp;
    fp = fopen("d:\\temp\\ffpline2.txt","wt");

    for(i=0; i<(int)pAE->u4TotalIndex; i++)
    {
        fprintf(fp,"%d\t%d\t%d\n",
            p->pCurrentTable[i].u4Eposuretime,
            p->pCurrentTable[i].u4AfeGain,
            p->pCurrentTable[i].u4IspGain);
    }
    fclose(fp);
    */

    return 0;

}

int FlashPlineTool::searchAePlineIndex(int* ind, strAETable* pAE, int exp, int afe, int isp)
{
    int i;
    int minErr;
    int sz;
    int bestInd=0;
    double expLevTar;
    expLevTar = (double)exp*afe*isp/(1024*1024);
    minErr=expLevTar;
    sz = (int)pAE->u4TotalIndex;
    for(i=0; i<sz; i++)
    {
        int expLev;
        int exp;
        int afe;
        int isp;
        double dif;
        exp = pAE->pCurrentTable->sPlineTable[i].u4Eposuretime;
        afe = pAE->pCurrentTable->sPlineTable[i].u4AfeGain;
        isp = pAE->pCurrentTable->sPlineTable[i].u4IspGain;
        expLev = (double)exp*afe*isp/(1024*1024);
        dif=expLev-expLevTar;
        if(dif<0)
            dif=-dif;
        if(minErr>dif)
        {
            minErr=dif;
            bestInd=i;
        }
    }
    logI("bestMatchIndex: %d", bestInd);
    *ind = bestInd;
    return 0;
}


int FlashPlineTool::aePlineWrite(const char* fname,  strAETable* pAE)
{
    int i;
    FILE* fp;
    fp = fopen(fname,"wt");
    for(i=0;i<(int)pAE->u4TotalIndex;i++)
    {
            fprintf(fp, "%d\t%d\t%d\n",
            pAE->pCurrentTable->sPlineTable[i].u4Eposuretime,
            pAE->pCurrentTable->sPlineTable[i].u4AfeGain,
            pAE->pCurrentTable->sPlineTable[i].u4IspGain);
    }
    fclose(fp);
    return 0;
}

int FlashPlineTool::flashAlgPlineWrite(const char* fname,  strAETable* pPline)
{
    int i;
    FILE* fp;
    fp = fopen(fname,"wt");
    for(i=0;i<(int)pPline->u4TotalIndex;i++)
    {
            fprintf(fp, "%d\t%d\t%d\n",
            pPline->pCurrentTable->sPlineTable[i].u4Eposuretime,
            pPline->pCurrentTable->sPlineTable[i].u4AfeGain,
            pPline->pCurrentTable->sPlineTable[i].u4IspGain);
    }
    fclose(fp);
    return 0;
}
