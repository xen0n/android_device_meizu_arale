//#######################################################
// how to add code, modify code?
// for almost all cases, it is only change the number (sensor, nvram type) and related code.
// please modify the code before the multi line "//####" (about L124).
//#######################################################
#ifdef WIN32
  #include "win32_test.h"
#else
  #include <utils/Errors.h>
  #include <utils/Log.h>
  #include <fcntl.h>
  #include "nvram_drv.h"
  #include "nvram_drv_imp.h"
  #include "libnvram.h"
  #include "CFG_file_lid.h"
  #include "camera_custom_AEPlinetable.h"
  #include <aaa_types.h>
  #include "flash_param.h"
  #include "flash_tuning_custom.h"
  #include "camera_custom_msdk.h"
  #include <mtkcam/hal/sensor_hal.h>
  //#include <mtkcam/hal/IHalSensor.h>
#endif
#include "nvbuf_util.h"
#include "oem_cam_drv.h"
#include <ctype.h>
#include <sys/stat.h>

//using namespace NSCamCustom;
//using namespace NSCam;

DEF_CriticalSection_Win(m_cs);

#ifdef WIN32
    #define logI(fmt, ...)
    #define logE(fmt, ...)
#else
    #ifdef LOG_TAG
        #undef LOG_TAG
    #endif
    #define LOG_TAG "nvbuf_util.cpp"

    #define logI(fmt, arg...)     ALOGD(LOG_TAG " " fmt, ##arg);
    //#define logI(fmt, arg...)    {usleep(10000); ALOGD(LOG_TAG " " fmt, ##arg);}
    #define logE(fmt, arg...)    ALOGE(LOG_TAG "Error: ln=%d: " fmt, __LINE__, ##arg)
#endif

//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################



enum
{
    e_NvramTypeNum = CAMERA_DATA_TYPE_NUM,
    e_SensorTypeNum = 3,
};

static int g_nvramArrInd[e_NvramTypeNum]=
{
    (int)CAMERA_NVRAM_DATA_ISP,
    (int)CAMERA_NVRAM_DATA_3A,
    (int)CAMERA_NVRAM_DATA_SHADING,
    (int)CAMERA_NVRAM_DATA_LENS,
    (int)CAMERA_DATA_AE_PLINETABLE,
    (int)CAMERA_NVRAM_DATA_STROBE,
    (int)CAMERA_DATA_TSF_TABLE,
    (int)CAMERA_NVRAM_DATA_GEOMETRY,
    (int)CAMERA_NVRAM_DATA_FEATURE,
    (int)CAMERA_NVRAM_VERSION,
    (int)CAMERA_OEM_COMMON,
    (int)CAMERA_OEM_SENSOR_LAYOUT,
    (int)CAMERA_OEM_AE,
    (int)CAMERA_OEM_AWB,
    (int)CAMERA_OEM_AF,
    (int)CAMERA_OEM_PARTS_LIST,
    (int)CAMERA_OEM_FEATURE,
};



static int g_nvramSize[e_NvramTypeNum]=
{
    (int)sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
    (int)sizeof(NVRAM_CAMERA_3A_STRUCT),
    (int)sizeof(NVRAM_CAMERA_SHADING_STRUCT),
    (int)sizeof(NVRAM_LENS_PARA_STRUCT),
    (int)sizeof(AE_PLINETABLE_T),
    (int)sizeof(NVRAM_CAMERA_STROBE_STRUCT),
    (int)sizeof(CAMERA_TSF_TBL_STRUCT),
    (int)sizeof(NVRAM_CAMERA_GEOMETRY_STRUCT),
    (int)sizeof(NVRAM_CAMERA_FEATURE_STRUCT),
    (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT),
    (int)sizeof(OEM_CAMERA_COMMON          ),
    (int)sizeof(OEM_CAMERA_SENSOR_LAYOUT          ),
    (int)sizeof(OEM_CAMERA_AE),
    (int)sizeof(OEM_CAMERA_AWB),
    (int)sizeof(OEM_CAMERA_AF),
    (int)sizeof(OEM_CAMERA_PARTS_LIST),
    (int)sizeof(OEM_CAMERA_FEATURE),
};






//for sensor driver
static int g_sensorArrInd[e_SensorTypeNum]=
{
    (int)DUAL_CAMERA_MAIN_SENSOR,
    (int)DUAL_CAMERA_SUB_SENSOR,
    (int)DUAL_CAMERA_MAIN_2_SENSOR
};

static int g_isNvBufRead[e_NvramTypeNum][e_SensorTypeNum];
static int g_isVerNvBufRead;

////////////////////////////////////////////////////


template <class T>
static T* getMemMain()
{
    static T st;
    return &st;
}
template <class T>
static T* getMemSub()
{
    static T st;
    return &st;
}
template <class T>
static T* getMemMain2()
{
    static T st;
    return &st;
}
int getVerMem(void*& buf)
{
    static NVRAM_CAMERA_VERSION_STRUCT obj;
    buf = (void*)&obj;
    return 0;
}

template <class T>
static int getMemDataType(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& m)
{
    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        m = getMemMain<T>();
        return 0;
    }
    else if(sensorDev==DUAL_CAMERA_SUB_SENSOR)
    {
        m= getMemSub<T>();
        return 0;
    }
    else if(sensorDev==DUAL_CAMERA_MAIN_2_SENSOR)
    {
        m= getMemMain2<T>();
        return 0;
    }
    else
    {
        return NvBufUtil::e_NV_SensorDevWrong;
    }
}

static int getMem(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, void*& buf)
{
    if(nvRamId==CAMERA_NVRAM_DATA_ISP)        return getMemDataType<NVRAM_CAMERA_ISP_PARAM_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)        return getMemDataType<NVRAM_CAMERA_3A_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)        return getMemDataType<NVRAM_CAMERA_SHADING_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)        return getMemDataType<NVRAM_LENS_PARA_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE)        return getMemDataType<AE_PLINETABLE_T>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)        return getMemDataType<NVRAM_CAMERA_STROBE_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_DATA_TSF_TABLE)        return getMemDataType<CAMERA_TSF_TBL_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)        return getMemDataType<NVRAM_CAMERA_GEOMETRY_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)        return getMemDataType<NVRAM_CAMERA_FEATURE_STRUCT>(sensorDev, buf);

    else if(nvRamId==CAMERA_NVRAM_VERSION)        return getMemDataType<NVRAM_CAMERA_VERSION_STRUCT>(sensorDev, buf);
    else if(nvRamId==CAMERA_OEM_COMMON)        return getMemDataType<OEM_CAMERA_COMMON>(sensorDev, buf);
    else if(nvRamId==CAMERA_OEM_SENSOR_LAYOUT)        return getMemDataType<OEM_CAMERA_SENSOR_LAYOUT>(sensorDev, buf);
    else if(nvRamId==CAMERA_OEM_AE)        return getMemDataType<OEM_CAMERA_AE>(sensorDev, buf);

    else if(nvRamId==CAMERA_OEM_AWB)        return getMemDataType<OEM_CAMERA_AWB>(sensorDev, buf);
    else if(nvRamId==CAMERA_OEM_AF)        return getMemDataType<OEM_CAMERA_AF>(sensorDev, buf);
    else if(nvRamId==CAMERA_OEM_PARTS_LIST)        return getMemDataType<OEM_CAMERA_PARTS_LIST>(sensorDev, buf);
    else if(nvRamId==CAMERA_OEM_FEATURE)        return getMemDataType<OEM_CAMERA_FEATURE>(sensorDev, buf);

    return NvBufUtil::e_NvramIdWrong;
}




static int readVerNvramNoLock(void*& p);
static int readCommonOemNoLock(void*& p);
int getSenorArrInd(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev);

static int readRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& version)
{
    if(cust_getCamParaType()==e_CamParaTypeNvram)
    {

        int err;
        void* buf;
        int devArrInd;
        err = readVerNvramNoLock(buf);
        if(err!=0)
        {
            logI("readVerNvramNoLock error ln=%d", __LINE__);
            return err;
        }
        NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;
        devArrInd = getSenorArrInd(sensorDev);


        logI( "readRamVersion nvRamId=%d ver : isp=%d,3a=%d,sh=%d,lens=%d,pl=%d,stb=%d,tsf=%d,geo=%d,feature=%d",nvRamId,
            verStruct->ispVer[devArrInd], verStruct->aaaVer[devArrInd], verStruct->shadingVer[devArrInd],
            verStruct->lensVer[devArrInd], verStruct->aePlineVer[devArrInd], verStruct->strobeVer[devArrInd],
            verStruct->tsfVer[devArrInd], verStruct->geometryVer[devArrInd], verStruct->featureVer[devArrInd]);

        if(nvRamId==CAMERA_NVRAM_DATA_ISP)            {version = verStruct->ispVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_3A)        {version = verStruct->aaaVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)    {version = verStruct->shadingVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_LENS)       {version = verStruct->lensVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_DATA_AE_PLINETABLE) {version = verStruct->aePlineVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {version = verStruct->strobeVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_DATA_TSF_TABLE) {version = verStruct->tsfVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY) {version = verStruct->geometryVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {version = verStruct->featureVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_OEM_FEATURE)        {version = verStruct->featureVer[devArrInd]; return 0;}
    }
    else
    {
        //getMemDataType<NVRAM_CAMERA_ISP_PARAM_STRUCT>(sensorDev, buf);
        int err;
        void* buf;
        OEM_CAMERA_COMMON* oem;
        int devArrInd;
        devArrInd = getSenorArrInd(sensorDev);
        //err = getMem(CAMERA_OEM_COMMON, DUAL_CAMERA_MAIN_SENSOR, buf);
        //OemCamDrv::getInstance()->readOem(DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_OEM_COMMON, bu);
        err = readCommonOemNoLock(buf);
        oem = (OEM_CAMERA_COMMON*)buf;
        if(nvRamId==CAMERA_NVRAM_DATA_ISP)            {version = oem->ispVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_3A)        {version = oem->aaaVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)    {version = oem->shadingVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_LENS)       {version = oem->lensVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_DATA_AE_PLINETABLE) {version = oem->aePlineVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {version = oem->strobeVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_DATA_TSF_TABLE) {version = oem->tsfVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY) {version = oem->geometryVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {version = oem->featureVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_OEM_FEATURE)        {version = oem->featureVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_OEM_PARTS_LIST)        {version = oem->partsListVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_OEM_AE)        {version = oem->oemAeVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_OEM_AWB)        {version = oem->oemAwbVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_OEM_AF)        {version = oem->oemAfVer[devArrInd]; return 0;}
        else if(nvRamId==CAMERA_OEM_SENSOR_LAYOUT)        {version = oem->oemLayoutVer[devArrInd]; return 0;}


    }
    return NvBufUtil::e_NvramIdWrong;
}

int writeVerNvramNoLock();
static int writeRamVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int version)
{
    logI("nvId=%d ver=%d", (int)nvRamId, version);
    int err;
    void* buf;
    int devArrInd;
    err = getVerMem(buf);
    if(err!=0)
        return err;
    devArrInd = getSenorArrInd(sensorDev);
    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;
    if(nvRamId==CAMERA_NVRAM_DATA_ISP)            {verStruct->ispVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)        {verStruct->aaaVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)    {verStruct->shadingVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)       {verStruct->lensVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_AE_PLINETABLE) {verStruct->aePlineVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {verStruct->strobeVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_DATA_TSF_TABLE) {verStruct->tsfVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)   {verStruct->geometryVer[devArrInd]=version;}
    else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {verStruct->featureVer[devArrInd]=version;}

    else return NvBufUtil::e_NvramIdWrong;
    err = writeVerNvramNoLock();
    return err;
}




static int writeRamUpdatedVersion(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
{
    int err;
    void* buf;
    int devArrInd;
    err = getVerMem(buf);
    if(err!=0)
        return err;
    devArrInd = getSenorArrInd(sensorDev);
    NVRAM_CAMERA_VERSION_STRUCT* verStruct = (NVRAM_CAMERA_VERSION_STRUCT*)buf;
    unsigned char* buf2;
    buf2  = (unsigned char*)buf;
    logI("qq writeRamUpdatedVersion %d %d %d %d %d %d %d %d %d %d buf:%d",
        buf2[0], buf2[1], buf2[2], buf2[3], buf2[4],
        buf2[5], buf2[6], buf2[7], buf2[8], buf2[9], (int)buf
        );

    if(nvRamId==CAMERA_NVRAM_DATA_ISP)            {verStruct->ispVer[devArrInd]=NVRAM_CAMERA_PARA_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_3A)        {verStruct->aaaVer[devArrInd]=NVRAM_CAMERA_3A_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_SHADING)    {verStruct->shadingVer[devArrInd]=NVRAM_CAMERA_SHADING_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_LENS)       {verStruct->lensVer[devArrInd]=NVRAM_CAMERA_LENS_FILE_VERSION;}
    else if(nvRamId==CAMERA_NVRAM_DATA_STROBE)  {verStruct->strobeVer[devArrInd]=NVRAM_CAMERA_STROBE_FILE_VERSION;}
//    else if(nvRamId==CAMERA_NVRAM_DATA_GEOMETRY)   {verStruct->geometryVer[devArrInd]=NVRAM_CAMERA_GEOMETRY_FILE_VERSION;}
//  else if(nvRamId==CAMERA_NVRAM_DATA_FEATURE)   {verStruct->featureVer[devArrInd]=NVRAM_CAMERA_FEATURE_FILE_VERSION;}
    else return NvBufUtil::e_NvramIdWrong;

    logI("qq writeRamUpdatedVersion %d %d %d %d %d %d %d %d %d %d buf2:%d",
        buf2[0], buf2[1], buf2[2], buf2[3], buf2[4],
        buf2[5], buf2[6], buf2[7], buf2[8], buf2[9], (int)buf        );

    err = writeVerNvramNoLock();

    logI("writeRamUpdatedVersion- nvRamId=%d", nvRamId);
    return err;
}

static void createDir(const char* dir)
{
    mkdir(dir,S_IRWXU | S_IRWXG);
}

int NvBufUtil::writeAllOemToData()
{

    createDir("/data/oem/");
    createDir("/data/oem/media/");
    createDir("/data/oem/media/camera/");

    int i;
    int j;
    int err=0;
    char fname1[300];
    char fname2[300];
    void* p1;
    void* p2;
    OemItemDres* pOemItemDres;
    pOemItemDres = getOemDres();
    for(i=0;i<CAMERA_DATA_TYPE_NUM;i++)
    {
        sprintf(fname1, "data/");
        sprintf(fname2, "data/");
        strcat(fname1, pOemItemDres[i].path);
        strcat(fname2, pOemItemDres[i].path);

        readDefault((CAMERA_DATA_TYPE_ENUM)i, DUAL_CAMERA_MAIN_SENSOR);
        readDefault((CAMERA_DATA_TYPE_ENUM)i, DUAL_CAMERA_SUB_SENSOR);

        getMem((CAMERA_DATA_TYPE_ENUM)i, DUAL_CAMERA_MAIN_SENSOR, p1);
        getMem((CAMERA_DATA_TYPE_ENUM)i, DUAL_CAMERA_SUB_SENSOR, p2);

        FILE* fp;



        logI("sensorRelation %d",pOemItemDres[i].sensorDevRelation);

        if(pOemItemDres[i].sensorDevRelation == e_SupportSensorDev)
        {
            strcat(fname1, "_main_0");
            strcat(fname2, "_sub_0");
        }

        fp = fopen(fname1, "wb");
        if(fp==0)
        {
            err=e_NV_FileRWWrong;
            logE("fname1(%s) is null", fname1);
        }
        else
        {
            fwrite(p1, 1, g_nvramSize[i], fp);
            fclose(fp);
        }

        logI("fname1 %s",fname1);
        logI("fname2 %s",fname2);

        if(pOemItemDres[i].sensorDevRelation == e_SupportSensorDev)
        {

            fp = fopen(fname2, "wb");
            if(fp==0)
            {
                err=e_NV_FileRWWrong;
                logE("fname2(%s) is null", fname2);
            }
            else
            {
                fwrite(p2, 1, g_nvramSize[i], fp);
                fclose(fp);
            }
        }




    }
    return err;
}

//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
//#######################################################
int NvBufUtil::isOemAvailable()
{
    return OemCamDrv::getInstance()->isOemAvailable();
}

int getSenorArrInd(CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev)
{
    int sensorArrSz;
    sensorArrSz = sizeof(g_sensorArrInd)/sizeof(int);
    int arrInd=-1;
    int i;
    for(i=0;i<sensorArrSz;i++)
    {
        if(sensorDev==g_sensorArrInd[i])
            arrInd=i;
    }
    return arrInd;
}
int getNvramArrInd(CAMERA_DATA_TYPE_ENUM nvEnum)
{
    int nvArrSz;
    nvArrSz = sizeof(g_nvramArrInd)/sizeof(int);
    int arrInd=-1;
    int i;
    for(i=0;i<nvArrSz;i++)
    {
        if(nvEnum==g_nvramArrInd[i])
            arrInd=i;
    }
    return arrInd;
}
NvBufUtil::NvBufUtil()
{
}

void NvBufUtil::setAndroidMode(int isAndroid)
{
    //NvramDrvBase* nvDrv;
    //nvDrv = NvramDrvBase::createInstance();
    //nvDrv->setAndroidMode (isAndroid);
    //nvDrv->destroyInstance();

}

NvBufUtil& NvBufUtil::getInstance()
{
    static int bInit=0;
    if(bInit==0)
    {
        DEF_InitCs(m_cs);
        bInit=1;
        int nvRamArrSz;
        nvRamArrSz = sizeof(g_nvramArrInd)/sizeof(int);
        int sensorArrSz;
        sensorArrSz = sizeof(g_sensorArrInd)/sizeof(int);
        int i;
        int j;
        for(i=0;i<nvRamArrSz;i++)
        for(j=0;j<sensorArrSz;j++)
            g_isNvBufRead[i][j]=0;
        g_isVerNvBufRead=0;
    }
    static NvBufUtil obj;
    return obj;
}


static int getSensorID(CAMERA_DUAL_CAMERA_SENSOR_ENUM i4SensorDev, int& sensorId)
{
    MUINT32 u4SensorID;
    SensorHal*const pSensorHal = SensorHal::createInstance();
    switch  ( i4SensorDev )
    {
    case DUAL_CAMERA_MAIN_SENSOR:
        pSensorHal->sendCommand(SENSOR_DEV_MAIN, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    case DUAL_CAMERA_SUB_SENSOR:
        pSensorHal->sendCommand(SENSOR_DEV_SUB, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    case DUAL_CAMERA_MAIN_2_SENSOR:
        pSensorHal->sendCommand(SENSOR_DEV_MAIN_2, SENSOR_CMD_GET_SENSOR_ID, reinterpret_cast<MINT32>(&u4SensorID), 0, 0);
        break;
    default:    //  Shouldn't happen.
        logE("Invalid sensor device: %d", i4SensorDev);
        break;
    }
    sensorId = u4SensorID;
    return 0;
}




int nvbufUtil_getSensorId(int sensorDev, int& sensorId)
{
    return getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, sensorId);
}


static int getNvSize(CAMERA_DATA_TYPE_ENUM camDataType, int& sz)
{
    int arrInd;
    arrInd = getNvramArrInd(camDataType);
    if(arrInd<0)
        return NvBufUtil::e_NvramIdWrong;
    sz = g_nvramSize[arrInd];
    return 0;
}

static int isBufRead(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& isRead)
{
    /*
    int i;
    for(i=0;i<3;i++)
    {
    logI("isBufRead_%d %d %d %d %d %d %d %d %d %d %d %d",
        i,
        g_isNvBufRead[0][i],
        g_isNvBufRead[1][i],
        g_isNvBufRead[2][i],
        g_isNvBufRead[3][i],
        g_isNvBufRead[4][i],
        g_isNvBufRead[5][i],
        g_isNvBufRead[6][i],
        g_isNvBufRead[7][i],
        g_isNvBufRead[8][i],
        g_isNvBufRead[9][i],
        g_isNvBufRead[10][i]);
    }*/
    int nvArrInd;
    int sensorArrInd;
    nvArrInd = getNvramArrInd(nvRamId);
    if(nvArrInd<0)
        return NvBufUtil::e_NvramIdWrong;

    sensorArrInd = getSenorArrInd(sensorDev);
    if(sensorArrInd<0)
        return NvBufUtil::e_SensorDevWrong;

    isRead = g_isNvBufRead[nvArrInd][sensorArrInd];
    g_isNvBufRead[nvArrInd][sensorArrInd]=1;


    return 0;
}

static int getSensorIdandMem(CAMERA_DATA_TYPE_ENUM nvRamId, CAMERA_DUAL_CAMERA_SENSOR_ENUM sensorDev, int& seonsorId, void*& buf, int& nvSize)
{
    int err;
    err = getSensorID((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, seonsorId);
    if(err!=0)
        return err;
    err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, buf);
    if(err!=0)
        return err;
    err = getNvSize(nvRamId, nvSize);
    if(err!=0)
        return err;
    return 0;
}



int readDefaultNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    int err;
    int u4SensorID;
    void* buf;
    int nvSize;
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, buf, nvSize);


    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->readDefaultData (
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
            buf    );
    nvDrv->destroyInstance();
    return err;
}

//==============================================================
// for camera version use
int readVerNvramNoLock(void*& p)
{
    int err;
    err = getVerMem(p);
    if(err!=0)
        return err;
    if(g_isVerNvBufRead==1)
    {
        return 0;
    }
    int nvSize;
    nvSize = (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT);
    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->readNoDefault(
               (CAMERA_DUAL_CAMERA_SENSOR_ENUM)DUAL_CAMERA_MAIN_SENSOR, CAMERA_NVRAM_VERSION, p, nvSize);
    nvDrv->destroyInstance();
    g_isVerNvBufRead=1;



    return err;
}

static int readCommonOemNoLock(void*& p)
{
    int err;
    err = getMem(CAMERA_OEM_COMMON, DUAL_CAMERA_MAIN_SENSOR, p);
    if(err!=0)
        return err;
    if(g_isVerNvBufRead==1)
    {
        return 0;
    }
    int nvSize;
    nvSize = (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT);
    //----------------------------
    OemCamDrv::getInstance()->readOem(DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_OEM_COMMON, p, nvSize, 0);
    g_isVerNvBufRead=1;
    return err;
}

int writeVerNvramNoLock()
{
    int err;
    void* buf;
    err = getVerMem(buf);
    if(err!=0)
        return err;
    int nvSize;
    nvSize = (int)sizeof(NVRAM_CAMERA_VERSION_STRUCT);
    //----------------------------
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->writeNvram(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)DUAL_CAMERA_MAIN_SENSOR, 0, CAMERA_NVRAM_VERSION,
            buf, nvSize    );
    nvDrv->destroyInstance();
    return err;
}
//==============================================================

int writeNvramNoLock(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    int err;
    void* buf;
    int u4SensorID;
    int nvSize;
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, buf,  nvSize);
    if(err!=0)
        return err;

    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->writeNvram(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
            buf, nvSize    );
    nvDrv->destroyInstance();
    return err;
}
//===========================================================================
//===========================================================================
//===========================================================================
int NvBufUtil::getBuf(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p)
{
    logI("getBuf ln=%d",__LINE__);
    DEF_AutoLock(m_cs);
    int err;
    err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
    return err;
}

int NvBufUtil::getBufAndRead(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev, void*& p, int bForceRead)
{
logI("getBufAndRead+ ln=%d ramId=%d sensorDev=%d",__LINE__,(int)nvRamId,sensorDev);
    DEF_AutoLock(m_cs);
    int err;
    int bRead;
    err = isBufRead(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, bRead);
    if(err!=0)
        return err;
    if(bForceRead==1)
        bRead=0;
//logI("getBufAndRead ln=%d",__LINE__);
    if(bRead==1)
    {

        err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
//logI("getBufAndRead ln=%d p=%d",__LINE__, (int)p);
        if(err!=0)
            return err;
        return 0;
    }


    int nvSize;
    if(nvRamId==CAMERA_OEM_COMMON)
    {
        err = getMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, p);
        nvSize = sizeof(OEM_CAMERA_COMMON);
        if(err!=0)
            return err;
        OemCamDrv::getInstance()->readOem((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, 0, nvRamId, p, nvSize, 0);
        return 0;
    }



    //logI("getBufAndRead ln=%d",__LINE__);
        int ramVer;
        err = readRamVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, ramVer);
        logI("readRamVersion ln=%d ver=%d err=%d",__LINE__, ramVer, err);

        if(err!=0)
            return err;
    //logI("getBufAndRead ln=%d",__LINE__);
        int u4SensorID;

        err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, p,  nvSize);
        if(err!=0)
            return err;
    //logI("getBufAndRead ln=%d p=%d",__LINE__,p);
    if(cust_getCamParaType()==e_CamParaTypeNvram)
    {
        //----------------------------
        NvramDrvBase* nvDrv;
        nvDrv = NvramDrvBase::createInstance();
        err  = nvDrv->readNvrameEx (
                (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
                p, nvSize, ramVer);
        nvDrv->destroyInstance();
    }
    else
    {
        OemCamDrv::getInstance()->readOem((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId, p, nvSize, ramVer);

    }

logI("getBufAndRead- ln=%d",__LINE__);
    return err;
}
int NvBufUtil::write(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    logI("wwrite ln=%d nvRamId=%d sensorDev=%d",__LINE__, (int)nvRamId, sensorDev);
    DEF_AutoLock(m_cs);
logI("write ln=%d",__LINE__);
    int err;
    int err2;
    void* buf;
    int u4SensorID;
    int nvSize;

logI("write ln=%d",__LINE__);
    err = getSensorIdandMem(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, buf,  nvSize);

logI("write ln=%d err=%d",__LINE__,err);
    if(err!=0)
        return err;
    int ver;
    //err = getNvBufVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM) sensorDev, ver);
    //if(err!=0)
        //return err;
    NvramDrvBase* nvDrv;
    nvDrv = NvramDrvBase::createInstance();
    err  = nvDrv->writeNvram(
            (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, u4SensorID, nvRamId,
            buf, nvSize    );
logI("write ln=%d err=%d",__LINE__,err);
    nvDrv->destroyInstance();
    if(err!=0)
        return err;
    err = writeRamUpdatedVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev);
logI("write ln=%d err=%d",__LINE__,err);
    if(err!=0)
        return err;
    int ramVer;
    err = readRamVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, ramVer);
logI("write ln=%d err=%d",__LINE__,err);
    logI("readRamVersion ln=%d ver=%d err=%d",__LINE__, ramVer, err);
    if(err!=0)
        return err;
    //err = setNvBufVersion(nvRamId, (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorDev, ramVer);
    return err;
}
int NvBufUtil::readDefault(CAMERA_DATA_TYPE_ENUM nvRamId, int sensorDev)
{
    logI("readDefault ln=%d",__LINE__);
    DEF_AutoLock(m_cs);
    return readDefaultNoLock(nvRamId, sensorDev);
}
