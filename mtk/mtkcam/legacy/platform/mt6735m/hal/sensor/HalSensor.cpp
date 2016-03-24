/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/HalSensor"
//
#include "MyUtils.h"
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif
// For property_get().
#include <cutils/properties.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//
//
/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#endif

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//multiopen
MUINT32 meSensorDev;
MINT32 mPowerRefCount;
SENSOR_DRIVER_SCENARIO_T g_ScenarioconfMain;
SENSOR_DRIVER_SCENARIO_T g_ScenarioconfSub;
SENSOR_DRIVER_SCENARIO_T g_ScenarioconfMain2;
extern SensorDynamicInfo sensorDynamicInfo[3];
extern MUINT32 mCreateSensorCount;
extern SENSORDRV_INFO_STRUCT sensorDrvInfo[3];

//hwsync drv
//#include "hwsync_drv.h"
//HWSyncDrv* mpHwSyncDrv=NULL;

//judge boot mode
static MUINT32 get_boot_mode(void)
{
  MINT32 fd;
  size_t s;
  char boot_mode[4] = {'0'};
  //MT_NORMAL_BOOT 0 , MT_META_BOOT 1, MT_RECOVERY_BOOT 2, MT_SW_REBOOT 3
  //MT_FACTORY_BOOT 4, MT_ADVMETA_BOOT 5
  fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
  if (fd < 0)
  {
    MY_LOGE("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");
    return 0;
  }

  s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
  close(fd);

  if(s <= 0)
  {
    MY_LOGE("could not read boot mode sys file\n");
    return 0;
  }

  boot_mode[s] = '\0';
  MY_LOGD("Boot Mode %d\n",atoi(boot_mode));
  return atoi(boot_mode);
}

/******************************************************************************
 *
 ******************************************************************************/
HalSensor::PerDataPtr
HalSensor::
kpDefaultPerData = new PerData();


/******************************************************************************
 *
 ******************************************************************************/
HalSensor::
PerData::
PerData()
{
}


/******************************************************************************
 *
 ******************************************************************************/
HalSensor::
~HalSensor()
{
}


/******************************************************************************
 *
 ******************************************************************************/
HalSensor::
HalSensor()
    : IHalSensor()
    //
    , mMutex()
    , mSensorDataMap()
    , mpSensorDrv(NULL)
    , mpSeninfDrv(NULL)
    //
{

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensor::
destroyInstance(
    char const* szCallerName
)
{
    mCreateSensorCount--;
    MY_LOGD("HalSensor destroyInstance count = %d, caller =%s",mCreateSensorCount,szCallerName);
    HalSensorList::singleton()->closeSensor(this, szCallerName);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalSensor::
onDestroy()
{
    MY_LOGD("#Sensor:%d", mSensorDataMap.size());
    //
    Mutex::Autolock _l(mMutex);
    //
    MY_LOGI_IF(0<mPowerRefCount, "Forget to powerOff before destroying. PowerRefCount:%d", mPowerRefCount);//ToDo:remove for multi-open
    if (mPowerRefCount == 0) {
        mpSeninfDrv->destroyInstance();//ToDo: necessary?
        mpSeninfDrv = NULL;
        mpSensorDrv->destroyInstance();
        mpSensorDrv = NULL;
        mSensorDataMap.clear();//ToDo:test
    }

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
onCreate(
    SortedVector<MUINT>const& vSensorIndex
)
{
    MBOOL ret = MFALSE;
    //
    MY_LOGD("+ #Sensor:%d", vSensorIndex.size());
    //
    Mutex::Autolock _l(mMutex);
    //
    //
    mSensorDataMap.clear();
    mSensorDataMap.setCapacity(vSensorIndex.size());
    for (MUINT i = 0; i < vSensorIndex.size(); i++)
    {
        MUINT const uSensorIndex = vSensorIndex[i];
        //
        sp<PerData> pData = new PerData;
        mSensorDataMap.add(uSensorIndex, pData);

        mpSensorDrv = SensorDrv::get();
        if  ( ! mpSensorDrv ) {
            MY_LOGE("SensorDrv::get() return NULL");
            goto lbExit;
        }
        mpSeninfDrv = SeninfDrv::createInstance();
        if  ( ! mpSeninfDrv ) {
            MY_LOGE("SeninfDrv::createInstance() return NULL");
            goto lbExit;
        }
    }
    ret = MTRUE;
    //
lbExit:
    if  ( ! ret )
    {
        mSensorDataMap.clear();
    }
    //
    MY_LOGD("- ret:%d", ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
isMatch(SortedVector<MUINT>const& vSensorIndex) const
{
    if  ( vSensorIndex.size() != mSensorDataMap.size() )
    {
        MY_LOGD("isMatch vSensorIndex:%d, mSensorDataMap:%d\n", vSensorIndex.size(),mSensorDataMap.size());//ToDo: remove
        return  MFALSE;
    }
    //
    for (MUINT i = 0; i < vSensorIndex.size(); i++)
    {
        if  ( vSensorIndex[i] != mSensorDataMap.keyAt(i) )
        {
        MY_LOGD("isMatch vSensorIndex[i]:%d, mSensorDataMap[i]:%d\n", vSensorIndex[i],mSensorDataMap.keyAt(i));//ToDo:remove
            return  MFALSE;
        }
    }
    //
    return  MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
HalSensor::PerDataPtr
HalSensor::
queryPerData(MUINT iSensorIndex, char const* szCaller) const
{
    ssize_t i = mSensorDataMap.indexOfKey(iSensorIndex);
    if  ( i < 0 )
    {
        MY_LOGE("<%s> Bad SensorIndex:%d -> use default data", szCaller, iSensorIndex);
        return  kpDefaultPerData;
    }
    //
    return  mSensorDataMap.valueAt(i);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
powerOn(
    char const* szCallerName,
    MUINT const uCountOfIndex,
    MUINT const*pArrayOfIndex
)
{
    MBOOL ret = MFALSE;
    MUINT sensorDev = 0;
    MUINT pcEn = 1;
    //
    Mutex::Autolock _l(mMutex);

    {
        #define DEV_IOC_MAGIC       'd'
        #define READ_DEV_DATA       _IOR(DEV_IOC_MAGIC,  1, unsigned int)
        unsigned int devinfo_data = 3;
        unsigned int isp_bonding = 0;
        MBOOL ISP_Limit = MFALSE;
        MUINT16 CapWidth = sensorDrvInfo[0].SensorCapWidth;
        int fd = open("/dev/devmap", O_RDONLY, 0);
        if(fd < 0)
        {
            MY_LOGE("/dev/devmap kernel open fail, errno(%d):%s",errno,strerror(errno));
        }
        else
        {
            if(ioctl(fd, READ_DEV_DATA, &devinfo_data) == 0)
            {
                isp_bonding = (devinfo_data >> 29) & 0x3;
                MY_LOGD("devinfo_data(0x%08X) Bound(%d) CapWidth(%d)", devinfo_data, isp_bonding, CapWidth);
                if(isp_bonding != 0x0)
                {
                    switch(isp_bonding){
                        case 1: // 16M
                            if(CapWidth > 4708)
                            {
                                ISP_Limit =MTRUE;
                            }
                            break;
                        case 2: // 13M
                            if(CapWidth > 4324)
                            {
                                ISP_Limit =MTRUE;
                            }
                            break;
                        case 3: // 8M
                            if(CapWidth > 3364)
                            {
                                ISP_Limit =MTRUE;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            else
            {
                MY_LOGE("Get devinfo_data fail");
            }
            close(fd);
        }
        if (ISP_Limit == MTRUE) {
            MY_LOGE("Warning!NOT Full function chip Bound(%d) CapWidth(%d)", isp_bonding, CapWidth);
            //return MFALSE;
        }
    }

    for (MUINT i = 0; i < uCountOfIndex; i++)
    {
        sensorDev |= HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfIndex[i])->meSensorDev;

    }
    MY_LOGD(" sensorDev : %d\n ",sensorDev);


    if ((sensorDev & meSensorDev) != 0) {
         MY_LOGD("sensorDev(0x%x) is already powerOn \n ",sensorDev);
         goto lbExit;
    }

    if (sensorDev == SENSOR_DEV_NONE) {
        MY_LOGD("meSensorDev is NONE ");
        goto lbExit;
    }

    ret = mpSeninfDrv->init();
    if (ret < 0) {
        MY_LOGE("pSeninfDrv->init() fail ");
        return MFALSE;
    }

    ret = mpSensorDrv->init(sensorDev);
    if (ret < 0) {
        MY_LOGE("mpSensorDrv->init() fail ");
        return MFALSE;
    }

    ret = setTgPhase(sensorDev, pcEn);
    if (ret < 0) {
        MY_LOGE("setTgPhase fail n");
        return MFALSE;
    }
    //
    ret = setSensorIODrivingCurrent(sensorDev);
    if (ret < 0) {
        MY_LOGE("initial IO driving current fail ");
        return MFALSE;
    }


    // Open sensor, try to open 3 time
    for (int i =0; i < 3; i++) {
        ret = mpSensorDrv->open(sensorDev);
        if (ret < 0) {
            MY_LOGE("pSensorDrv->open fail, retry = %d ", i);
        }
        else {
            break;
        }
    }
    if (ret < 0) {
        MY_LOGE("pSensorDrv->open fail");
        return MFALSE;
    }

    meSensorDev =  meSensorDev | sensorDev;
    mPowerRefCount++;

 /*   //hwsync driver, create instance and do init
    switch(get_boot_mode())
    {
        case 1:
        case 4:
            break;
        default:
            for (MUINT i = 0; i < uCountOfIndex; i++)
            {
                mpHwSyncDrv=HWSyncDrv::createInstance();
                mpHwSyncDrv->init(pArrayOfIndex[i]);
            }
            break;
    }*/
    ret = MTRUE;
    lbExit:
    MY_LOGD("- <%s> PowerRefCount:%d ret:%d, meSensorDev = 0x%x\n", szCallerName, mPowerRefCount, ret,meSensorDev);

    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalSensor::
powerOff(
    char const* szCallerName,
    MUINT const uCountOfIndex,
    MUINT const*pArrayOfIndex
)
{
    MBOOL ret = MFALSE;
    MUINT sensorDev = 0;    //multiopen
    MUINT pcEn = 0;

    Mutex::Autolock _l(mMutex);

    //hwsync driver, do uninit and destroy instance
    switch(get_boot_mode())
    {
        case 1:
        case 4:
            break;
    /*    default:
            if(mpHwSyncDrv)
            {
                for (MUINT i = 0; i < uCountOfIndex; i++)
                {
                    mpHwSyncDrv->uninit(pArrayOfIndex[i]);
                    mpHwSyncDrv->destroyInstance();
                }
            }
            break;*/
    }
    //

    for (MUINT i = 0; i < uCountOfIndex; i++)
    {
        sensorDev |= HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfIndex[i])->meSensorDev;

    }
    MY_LOGD(" sensorDev : %d\n ",sensorDev);

    if( sensorDev & SENSOR_DEV_MAIN) {
        sensorDynamicInfo[0].TgInfo = CAM_TG_NONE;
        sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
    }
    if( sensorDev & SENSOR_DEV_SUB) {
        sensorDynamicInfo[1].TgInfo = CAM_TG_NONE;
        sensorDynamicInfo[1].pixelMode = ONE_PIXEL_MODE;
    }
    if( sensorDev & SENSOR_DEV_MAIN_2) {
        sensorDynamicInfo[2].TgInfo = CAM_TG_NONE;
        sensorDynamicInfo[2].pixelMode = ONE_PIXEL_MODE;
    }

    if ((sensorDev & meSensorDev) == 0) {
         MY_LOGD("sensorDev(0x%x) is not exist \n ",sensorDev);
         goto lbExit;
    }

    if  ( 0 == mPowerRefCount )
    {
        MY_LOGE("<%s> Cannot powerOff without powerOn", szCallerName);
        goto lbExit;
    }

    if (mpSensorDrv) {
        #ifdef USING_MTK_LDVT
            //mpSeninfDrv->setPdnRst(sensorDev, 0);//ToDo: Remove for test
        #endif

        ret = mpSensorDrv->close(sensorDev);
        if(ret < 0) {
            MY_LOGE("mpSensorDrv->close fail");
            return MFALSE;
        }
    }
    else {
        MY_LOGE("mpSensorDrv is NULL");
    }

    ret = setTgPhase(sensorDev, pcEn);
    if (ret < 0) {
        MY_LOGE("setTgPhase fail n");
        return MFALSE;
    }

    if (mpSensorDrv) {

        ret = mpSensorDrv->uninit();
        if(ret < 0) {
            MY_LOGE("mpSensorDrv->uninit fail");
            return MFALSE;
        }
    }

    if (mpSeninfDrv) {

        ret = mpSeninfDrv->uninit();
        if(ret < 0) {
            MY_LOGE("mpSeninfDrv->uninit fail");
            return MFALSE;
        }
    }


    meSensorDev = meSensorDev & (~sensorDev);
    mPowerRefCount--;


    ret = MTRUE;
lbExit:
    MY_LOGD("- <%s> PowerRefCount:%d ret:%d, meSensorDev = 0x%x\n", szCallerName, mPowerRefCount, ret,meSensorDev);

    return  ret;
}


MINT32 HalSensor::reset()
{
    MINT32 ret = 0, tg1Ret = 0;
    MUINT sensorDev = 0;
    SensorDrv *const pSensorDrv = SensorDrv::get();
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

    MY_LOGE("Start reset meSensorDev %d!\n",meSensorDev);
    //ret = pSensorDrv->close();  LukeHu--
    if (pSensorDrv) {
        ret = pSensorDrv->close(meSensorDev);
        if(ret < 0) {
            MY_LOGE("pSensorDrv->close fail");
            return ret;
        }
		else
			MY_LOGE("pSensorDrv->close Succeed");


        usleep(100);
        pSeninfDrv->resetCSI2();//reset CSI2

		    // Open sensor, try to open 3 time
    	for (int i =0; i < 3; i++) {
        	ret = pSensorDrv->open(meSensorDev);
        	if (ret < 0) {
            	MY_LOGE("pSensorDrv->open fail, retry = %d ", i);
        	}
        	else {
				MY_LOGE("pSensorDrv->open succeed, retry = %d ", i);
            	break;
        	}
    	}

    	if (ret < 0) {
        	MY_LOGE("pSensorDrv->open fail");
        	return MFALSE;
    	}

        usleep(10);

		if(g_ScenarioconfMain.sensorDevId != 0x00){
			ret = pSensorDrv->setScenario(g_ScenarioconfMain);
			MY_LOGE("pSensorDrv->setScenario(g_ScenarioconfMain)");
		}


		if(g_ScenarioconfSub.sensorDevId != 0x00){
			ret = pSensorDrv->setScenario(g_ScenarioconfSub);
			MY_LOGE("pSensorDrv->setScenario(g_ScenarioconfSub)");
		}


		if(g_ScenarioconfMain2.sensorDevId != 0x00){
			ret = pSensorDrv->setScenario(g_ScenarioconfMain2);
			MY_LOGE("pSensorDrv->setScenario(g_ScenarioconfMain2)");
		}

    }
    else {
        MY_LOGE("mpSensorDrv is NULL");
    }
	MY_LOGE("End reset!\n");
    return ret;
}
