
#define LOG_TAG "flash_util.cpp"

#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
//#include <mtkcam/hal/aaa/aaa_hal_if.h>
//#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <af_param.h>
#include <nvram_drv_mgr.h>
#include <flash_awb_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
#include <mtkcam/common.h>
#include <kd_camera_feature.h>
#include <isp_tuning.h>
#include <ispdrv_mgr.h>
#include <mtkcam/featureio/tuning_mgr.h>
#include <camera_custom_cam_cal.h>
#include <cam_cal_drv.h>
#include <flash_feature.h>
#include <mtkcam/hal/IHalSensor.h>


//ae inc

#include <ctype.h>
#include <sys/stat.h>
#include <camera_custom_AEPlinetable.h>
//#include <CamDefs.h>
#include <ae_mgr.h>
//#include <ae_algo_if.h>
//#include <mtkcam/drv/sensor_hal.h>.
#include <ae_tuning_custom.h>
#include <aaa_sensor_mgr.h>
#include <FlashAlg.h>

#include "strobe_drv.h"
#include <time.h>
#include <kd_camera_feature.h>
#include <isp_mgr.h>

#include <isp_tuning_mgr.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include "flash_util.h"
#include <vector>
#include <mtkcam/acdk/cct_feature.h>
#include <flash_tuning_custom.h>
//#include "flash_mgr_m.h"



enum
{
    FILE_COUNT_FILE_NOT_FOUND = -1,
    FILE_COUNT_FILE_NOT_CONTAIN_INT = -2,
};
FlashUtil* FlashUtil::getInstance()
{
    static FlashUtil obj;
    return &obj;
}
FlashUtil::FlashUtil()
{

}
FlashUtil::~FlashUtil()
{

}


int isInt(const char* s)
{
  bool isInt = true;
  int slen;
  slen = strlen(s);
    for(int i = 0; i < slen; i++)
    {
      if(!isdigit(s[i]))
      {
        isInt = false;
        break;
      }
    }
  return isInt;
}


#include <cutils/properties.h>
int FlashUtil::getPropInt(const char* sId, int DefVal)
{
    int ret;
    char ss[PROPERTY_VALUE_MAX] = {'\0'};
    char sDef[20];
    sprintf(sDef,"%d",DefVal);
    property_get(sId, ss, sDef);
    ret = atoi(ss);
    return ret;
}





int FlashUtil::getMs()
{
    //    max:
    //    2147483648 digit
    //    2147483.648 second
    //    35791.39413 min
    //    596.5232356 hour
    //    24.85513481 day
    //int t;
    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //t = (tv.tv_sec*1000 + (tv.tv_usec+500)/1000);

    int t;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (ts.tv_sec*1000+ts.tv_nsec/1000000);

    return t;
}



void FlashUtil::createDir(const char* dir)
{
#ifdef WIN32
    CreateDirectory(dir,0);
#else
    mkdir(dir,S_IRWXU | S_IRWXG | S_IRWXO);

#endif

}
void FlashUtil::aaSub(void* arr, int* sub)
{

    //@@ rewrite to new ae/awb data
    /*
    int AA_W=120;
    int AA_H=90;
    AWB89AE_STAT_T* p;
    p = (AWB89AE_STAT_T*)arr;
    int xi;
    int yi;
    int yy;

    int i;
    int j;
    for(i=0;i<25;i++)
    {
        sub[i]=0;
    }


    for(j=0;j<AA_H;j++)
    for(i=0;i<AA_W;i++)
    {
        yy=p->LINE[j].AE_WIN[i];
        xi = i/24;
        yi = j/18;
        sub[yi*5+xi]+=yy;
    }

    for(i=0;i<25;i++)
    {
        sub[i]/=12*9;
    }
*/

}
int FlashUtil::aaToBmp(void* arr, const char* aeF, const char* awbF)
{
    //@@rewrite to new ae/awb data
    /*
    AWB89AE_STAT_T* p;
    p = (AWB89AE_STAT_T*)arr;
    int ret=0;
    int i;
    int j;
    short* pR;
    short* pG;
    short* pB;
    short* pY;
    int AA_W=120;
    int AA_H=90;
    pY = new short[AA_H*AA_W];
    pR = new short[AA_H*AA_W];
    pG = new short[AA_H*AA_W];
    pB = new short[AA_H*AA_W];
    int ind=0;
    for(j=0;j<AA_H;j++)
    for(i=0;i<AA_W;i++)
    {
        pY[ind]=p->LINE[j].AE_WIN[i];
        pR[ind]=p->LINE[j].AWB_WIN[i].rMainStat.ucR;
        pG[ind]=p->LINE[j].AWB_WIN[i].rMainStat.ucG;
        pB[ind]=p->LINE[j].AWB_WIN[i].rMainStat.ucB;
        if(pY[ind]>255)
            pY[ind]=255;
        if(pR[ind]>255)
            pR[ind]=255;
        if(pG[ind]>255)
            pB[ind]=255;
        if(pB[ind]>255)
            pB[ind]=255;
        ind++;
    }
    int e=0;

//    if(aeF!=0)
//        e = arrayToGrayBmp(aeF, pY, 2, AA_W, AA_H, 255);
//    if(ret==0)
//        ret=e;
//    if(awbF!=0)
//        e = arrayToColorBmp(awbF, pR, pG, pB, 2, AA_W, AA_H, 255);

    if(ret==0)
        ret=e;
    delete []pY;
    delete []pR;
    delete []pG;
    delete []pB;
    return ret;
    */
    return 0;
}
int FlashUtil::setFileCount(const char* fname, int cnt)
{
    int err=0;
    FILE* fp;
    fp = fopen(fname, "wt");
    if(fp==NULL)
    {
        err = FILE_COUNT_FILE_NOT_FOUND;
    }
    else
    {
        fprintf(fp,"%d",cnt);
        fclose(fp);
    }
    return err;
}


int FlashUtil::getFileCount(const char* fname, int* fcnt, int defaultValue)
{
    int err=0;
    int v;
    FILE* fp;
    fp = fopen(fname, "rb");
    if(fp==NULL)
    {
        err = FILE_COUNT_FILE_NOT_FOUND;
    }
    else
    {
        char s[101];
        fscanf(fp,"%100s",s);
        fclose(fp);
        int bNum;
        bNum = isInt(s);
        if(bNum==(int)true)
        {
            v=atoi(s);
        }
        else
        {
            err = FILE_COUNT_FILE_NOT_CONTAIN_INT;
        }
    }
    if(err==0)
        *fcnt = v;
    else
        *fcnt = defaultValue;

    return err;
}




void get1_4_range(int v, int& dn, int& up)
{
    int v2;
    v2 = v;
    double v1_4; // 1/4*v
    int n1_4_2; // half of 1/4*v
    if(v%2==1)
    {
        v2--;
    }
    v1_4 = v2/4.0;
    n1_4_2 = (int)(v1_4/2.0+0.5);
    dn = v2/2-n1_4_2;
    up = v2/2+(n1_4_2-1);
    if(v%2==1)
    {
        up++;
    }

}

template <class T>
double cal_1_4Mean(T* v, int w, int h)
{
    int wst;
    int wed;
    int hst;
    int hed;
    get1_4_range(w, wst, wed);
    get1_4_range(h, hst, hed);
    int cnt=0;
    int i;
    int j;
    int ind;
    double m=0;
    for(i=wst;i<=wed;i++)
    for(j=hst;j<=hed;j++)
    {
        ind = (j*w+i);
        m += v[ind];
        cnt++;
    }
    m/=cnt;
    return m;
}
int cal_center_3aSta_yrgb_romeHw(void* buf, int w, int h, double* yrgb)
{
    double* y;
    double* r;
    double* g;
    double* b;
    y = new double[w*h];
    r = new double[w*h];
    g = new double[w*h];
    b = new double[w*h];
    int ret;
    ret = convert3aSta_romeHw_yrgb(buf, w, h, y, r, g, b);

    yrgb[0]=cal_1_4Mean(y, w, h);
    yrgb[1]=cal_1_4Mean(r, w, h);
    yrgb[2]=cal_1_4Mean(g, w, h);
    yrgb[3]=cal_1_4Mean(b, w, h);


    delete []y;
    delete []r;
    delete []g;
    delete []b;
    return 0;
}

int convert3aSta_romeHw_bmp(void* arr, int w, int h, const char* aeF, const char* awbF)
{
    short* y;
    short* r;
    short* g;
    short* b;
    y = new short[w*h];
    r = new short[w*h];
    g = new short[w*h];
    b = new short[w*h];
    convert3aSta_romeHw_yrgb(arr, w, h, y, r, g, b);
    arrayToBmp(aeF, y, y, y, w, h);
    arrayToBmp(awbF, r, g, b, w, h);
    delete []y;
    delete []r;
    delete []g;
    delete []b;
    return 0;
}



void MemBuf::deleteBuf()
{
    if(buf!=0)
    {
        delete []buf;
        buf=0;
    }
}
char* MemBuf::newChar(int sz)
{
    deleteBuf();
    char* p;
    p = new char[sz];
    buf = p;
    return p;
}
short* MemBuf::newShort(int sz)
{
    deleteBuf();
    short* p;
    p = new short[sz];
    buf = p;
    return p;
}
int* MemBuf::newInt(int sz)
{
    deleteBuf();
    deleteBuf();
    int* p;
    p = new int[sz];
    buf = p;
    return p;

}
float* MemBuf::newFloat(int sz)
{
    deleteBuf();
    float* p;
    p = new float [sz];
    buf = p;
    return p;
}
double* MemBuf::newDouble(int sz)
{
    deleteBuf();
    double* p;
    p = new double [sz];
    buf = p;
    return p;
}
MemBuf::MemBuf()
{
    buf=0;
}
MemBuf::~MemBuf()
{
    deleteBuf();
}
