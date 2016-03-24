#include <mtkcam/common.h>
#include <camera_custom_nvram.h>
#include <flash_hal.h>
#include <isp_mgr.h>
#include <flash_mgr.h>
#include <flash_param.h>
#include <flash_tuning_custom.h>
#include "strobe_drv.h"

using namespace NSCam;


IHalFlash*
IHalFlash::
getInstance()
{
    return FlashHal::createInstance();
}

FlashHal::
FlashHal()
    :m_pHalSensorList(getPHalSensorList())
    ,m_status(0)
    ,m_prevSensorDev(0)
{
}

FlashHal*
FlashHal::
createInstance()
{
    static FlashHal obj;
    return &obj;
}

MINT32
FlashHal::
getTorchStatus(MINT32 i4SensorOpenIndex)
{
/*  StrobeDrv* pStrobe = StrobeDrv::createInstance();
    m_prevSensorDev = pStrobe->getSensorDev();
*/
    return m_status;
}


MINT32
FlashHal::
setTorchOnOff(MINT32 i4SensorOpenIndex, MBOOL en)
{
    MINT32 i4SensorDevId = (!m_pHalSensorList)? 0: m_pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);
    if(i4SensorDevId==2 && cust_isSubFlashSupport()==0)
          return 1;
    StrobeDrv* pStrobe = StrobeDrv::getInstance(i4SensorDevId);
    if(en==1){
        pStrobe->init();
        pStrobe->setDuty(0);
        pStrobe->setTimeOutTime(0);
        pStrobe->setOnOff(1);
        m_status = 1;
    }else{
        pStrobe->setOnOff(0);
        pStrobe->uninit();
        m_status = 0;
    }
	return 0;
}


IHalSensorList*
FlashHal::
getPHalSensorList()
{
    return IHalSensorList::get();
}


