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
#include "oem_cam_drv.h"
#include "camera_custom_nvram.h"
#include <mtkcam/hal/sensor_hal.h>

#undef LOG_TAG
#define LOG_TAG "oem_cam_drv.cpp"

#define logI(fmt, arg...)    ALOGD(fmt, ##arg)
#define logE(fmt, arg...) ALOGD("MError: func=%s line=%d: " fmt, __FUNCTION__, __LINE__, ##arg)




//==========================================





//#define OEM_COMMON_PATH "oem/media/camera/cam_common"

OemItemDres gOemItemDres[CAMERA_DATA_TYPE_NUM]=
{
    {CAMERA_NVRAM_DATA_ISP        ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_isp"},
    {CAMERA_NVRAM_DATA_3A         ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_3a"},
    {CAMERA_NVRAM_DATA_SHADING    ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_shading"},
    {CAMERA_NVRAM_DATA_LENS       ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_lens"},
    {CAMERA_DATA_AE_PLINETABLE    ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_pline"},
    {CAMERA_NVRAM_DATA_STROBE     ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_strobe"},
    {CAMERA_DATA_TSF_TABLE        ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_tsf"},
    {CAMERA_NVRAM_DATA_GEOMETRY   ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_geometry"},
    {CAMERA_NVRAM_DATA_FEATURE    ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/cam_feature"},
    {CAMERA_NVRAM_VERSION         ,e_NotSupportSensorDev , e_NoneRelation, "oem/system/firmware/camera/cam_ver"},
    {CAMERA_OEM_COMMON            ,e_NotSupportSensorDev , e_NoneRelation, "oem/system/firmware/camera/oem_common"},
    {CAMERA_OEM_SENSOR_LAYOUT     ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/oem_sensor"},
    {CAMERA_OEM_AE                ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/oem_ae"},
    {CAMERA_OEM_AWB               ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/oem_awb"},
    {CAMERA_OEM_AF                ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/oem_af"},
    {CAMERA_OEM_PARTS_LIST        ,e_NotSupportSensorDev , e_NoneRelation,  "oem/system/firmware/camera/oem_part_list"},
    {CAMERA_OEM_FEATURE           ,e_SupportSensorDev , e_SensorRelation, "oem/system/firmware/camera/oem_feature"},
};



OemItemDres* getOemDres()
{
    return gOemItemDres;
}

void sortOemItemDres()
{
    int maxTest=500;
    int whileTestCnt=0;
    int i;
    int j;
    //test
    for(i=0;i<CAMERA_DATA_TYPE_NUM;i++)
    {
        if(gOemItemDres[i].id<0 || gOemItemDres[i].id>=CAMERA_DATA_TYPE_NUM)
            logE("gOemItemDres[i].id is not correct!!! (<0 || > max)");

        for(j=i+1;j<CAMERA_DATA_TYPE_NUM;j++)
        {
            if(gOemItemDres[i].id == gOemItemDres[j].id)
            {
                logE("gOemItemDres[i].id is not correct!!! (duplicated)");
            }
        }
    }
    OemItemDres tmp;
    for(i=0;i<CAMERA_DATA_TYPE_NUM;i++)
    {
        while(gOemItemDres[i].id != i)
        {
            int pos2;
            pos2 = gOemItemDres[i].id;
            tmp = gOemItemDres[pos2];
            gOemItemDres[pos2] = gOemItemDres[i];
            gOemItemDres[i] =tmp;
            whileTestCnt++;
            if(whileTestCnt>maxTest)
                break;
        }
    }
}





static android::Mutex      gLock;
static int gIsOemExist=0;
OEM_CAMERA_COMMON gCamCommon;

//========================================
OemCamDrv* OemCamDrv::getInstance()
{
    static OemCamDrv obj;
    return &obj;
}


static int readCommonOem()
{
    logI("readCommonOem ln=%d",__LINE__);
    static int isRead=0;
    if(isRead==1)
        return 0;
logI("readCommonOem ln=%d",__LINE__);
    sortOemItemDres();
    isRead=1;
    FILE* fp;
    fp = fopen(gOemItemDres[CAMERA_OEM_COMMON].path, "rb");

    if(fp==0)
    {
logI("readCommonOem ln=%d",__LINE__);
        gIsOemExist=0;
        fillDefaultOemComm(&gCamCommon);
        return 0;
    }
    else
        gIsOemExist=1;
logI("readCommonOem ln=%d",__LINE__);
    int r;
    int err=0;
    r = fread(&gCamCommon, 1, sizeof(OEM_CAMERA_COMMON), fp);
    if(r!=sizeof(OEM_CAMERA_COMMON))
        err=e_OemErr_FileErr;
    fclose(fp);
    return err;
}

int OemCamDrv::isOemAvailable()
{
    logI("isOemAvailable ln=%d",__LINE__);
    Mutex::Autolock _l(gLock);
    int type;
    type = cust_getCamParaType();
    if(type == e_CamParaTypeOem)
    {
        int err;
        err = readCommonOem();
        if(gIsOemExist==0 || err!=0)
        {
            logI("isOemAvailable ln=%d isAvailable=%d",__LINE__,0);
            return 0;
        }
        else
        {
            logI("isOemAvailable ln=%d isAvailable=%d",__LINE__,1);
            return 1;
        }
    }
    else
    {
        logI("isOemAvailable ln=%d isAvailable=%d",__LINE__,0);
        return 0;
    }
};
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
int OemCamDrv::readOem(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          unsigned long u4SensorID,
                          CAMERA_DATA_TYPE_ENUM nvRamId,
                          void *a_pNvramData,
                          unsigned long a_u4NvramDataSize,
                          int a_version)
{

    logI("readOem ln=%d sensorDev=%d id=%d",__LINE__, a_eSensorType, nvRamId);
    Mutex::Autolock _l(gLock);
    readCommonOem();
    if(gIsOemExist==0)
    {
        int err;
        NvramDrvBase* nvDrv;
        nvDrv = NvramDrvBase::createInstance();
        err  = nvDrv->readDefaultData (
                (CAMERA_DUAL_CAMERA_SENSOR_ENUM)a_eSensorType, u4SensorID, nvRamId,
                a_pNvramData    );
        nvDrv->destroyInstance();
        return 0;
    }
logI("readOem ln=%d",__LINE__);
    //=====================================
    if(nvRamId == CAMERA_OEM_COMMON)
    {
        memcpy(a_pNvramData, &gCamCommon, sizeof(OEM_CAMERA_COMMON));
        return 0;
    }
logI("readOem ln=%d",__LINE__);

    char fname[e_MaxPathSize+20];

    strcpy(fname, gOemItemDres[nvRamId].path);

    if(gOemItemDres[nvRamId].sensorDevRelation==e_SensorRelation)
    {
        if(a_eSensorType == DUAL_CAMERA_MAIN_SENSOR)
            strcat(fname, "_main");
        else if(a_eSensorType == DUAL_CAMERA_SUB_SENSOR)
            strcat(fname, "_sub");
        else //if(a_eSensorType == DUAL_CAMERA_MAIN2_SENSOR)
            strcat(fname, "_main2");

    }



    int id=0;
     getSensorID(a_eSensorType, id);

logI("readOem getSensorID ln=%d id=%d",__LINE__,id);


    if(gOemItemDres[nvRamId].componentRelation == e_SensorRelation)
    {
        if(a_eSensorType == DUAL_CAMERA_MAIN_SENSOR)
        {
            int i;
            int oemId=0;
            char s[20];
            for(i=0;i<10;i++)
            {
                if( id ==  gCamCommon.OemMainSensorId[i])
                {
                    oemId=i;
                    break;
                }
            }
            sprintf(s,"_%d",oemId);
            strcat(fname, s);
        }
        else if(a_eSensorType == DUAL_CAMERA_SUB_SENSOR)
        {
            int i;
            int oemId=0;
            char s[20];
            for(i=0;i<10;i++)
            {
                if( id ==  gCamCommon.OemSubSensorId[i])
                {
                    oemId=i;
                    break;
                }
            }
            sprintf(s,"_%d",oemId);
            strcat(fname, s);
        }
    }

logI("readOem ln=%d nvRamId=%d fname=%s",__LINE__,nvRamId, fname);

    FILE* fp;
    fp = fopen(fname, "rb");
    if(fp==0)
    {
        int err;
        NvramDrvBase* nvDrv;
        nvDrv = NvramDrvBase::createInstance();
        err  = nvDrv->readDefaultData (
                (CAMERA_DUAL_CAMERA_SENSOR_ENUM)a_eSensorType, u4SensorID, nvRamId,
                a_pNvramData    );
        nvDrv->destroyInstance();
        return 0;
    }
logI("readOem ln=%d",__LINE__);

    fread(a_pNvramData, 1, a_u4NvramDataSize, fp);
    fclose(fp);
logI("readOem ln=%d",__LINE__);
    return 0;
}
