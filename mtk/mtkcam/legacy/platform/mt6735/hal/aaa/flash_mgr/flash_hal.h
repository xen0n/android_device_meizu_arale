#ifndef __FLASH_HAL_H__
#define __FLASH_HAL_H__

#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/hal/IHalFlash.h>

using namespace NSCam;

class FlashHal : public IHalFlash
{
protected:
    FlashHal();
    ~FlashHal(){};
public:
    static FlashHal* createInstance();

    /* functions */
    MINT32 getTorchStatus(MINT32 i4SensorOpenIndex);
    MINT32 setTorchOnOff(MINT32 i4SensorOpenIndex, MBOOL en);
private:
    IHalSensorList *const m_pHalSensorList;
    IHalSensorList* getPHalSensorList();
    MBOOL m_status;
    MINT32 m_prevSensorDev;
};

#endif
