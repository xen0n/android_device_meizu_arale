
#include "ts_fake_sensor.h"

#define SENSOR_PREVIEW_MODE     (0)
#define SENSOR_CAPTURE_MODE     (1)
#define SENSOR_MODE             (1)

#if (SENSOR_MODE == SENSOR_PREVIEW_MODE)
#define SENSOR_WIDTH        (1600)
#define SENSOR_HEIGHT       (1200)
#elif (SENSOR_MODE == SENSOR_CAPTURE_MODE)
#define SENSOR_WIDTH        (2088) //(4176) //
#define SENSOR_HEIGHT       (1544) //(3088) //
#else
#define SENSOR_WIDTH        (1600)
#define SENSOR_HEIGHT       (1200)
#endif

TS_FakeSensorList *TS_FakeSensorList::get()
{
    static TS_FakeSensorList single;
    return &single;
}

TS_FakeSensor *TS_FakeSensorList::createSensor (
                char const* szCallerName,
                MUINT const uCountOfIndex,
                MUINT const*pArrayOfIndex)
{
    static TS_FakeSensor single;
    return &single;
}

void TS_FakeSensorList::querySensorStaticInfo(
        MUINT sensorDevIdx,
        SensorStaticInfo *pSensorStaticInfo)
{
#if 1
    pSensorStaticInfo[0].previewWidth = SENSOR_WIDTH;
    pSensorStaticInfo[0].previewHeight = SENSOR_HEIGHT;
    pSensorStaticInfo[0].captureWidth = SENSOR_WIDTH;
    pSensorStaticInfo[0].captureHeight = SENSOR_HEIGHT;
#elif 0
    pSensorStaticInfo[0].previewWidth = 4176;
    pSensorStaticInfo[0].previewHeight = 3088;
    pSensorStaticInfo[0].captureWidth = 4176;
    pSensorStaticInfo[0].captureHeight = 3088;
#elif 0
    pSensorStaticInfo[0].previewWidth = 2088;
    pSensorStaticInfo[0].previewHeight = 1544;
    pSensorStaticInfo[0].captureWidth = 2088;
    pSensorStaticInfo[0].captureHeight = 1544;
#else
    pSensorStaticInfo[0].previewWidth = 1600;
    pSensorStaticInfo[0].previewHeight = 1200;
    pSensorStaticInfo[0].captureWidth = 1600;
    pSensorStaticInfo[0].captureHeight = 1200;
#endif

    pSensorStaticInfo[0].rawSensorBit = 10;
    pSensorStaticInfo[0].sensorFormatOrder = 1;
}


static MUINT32 mPowerOnTM[] = {
    0x00004410, 0x00000005,

    //0x00004414, 0x00001001, //CAM_TG_VF_CON, VFDATA_EN

    0x00004418, ((((SENSOR_WIDTH+0x0002)&0x7FFF)<<16) | 0x0002),    // TG_SEN_GRAB_PXL
    0x0000441C, ((((SENSOR_HEIGHT+0x0002)&0x1FFF)<<16) | 0x0002),   // TG_SEN_GRAB_LIN
    // (2,2), 2088x1544
    //0x00004418, 0x082A0002, // TG_SEN_GRAB_PXL
    //0x0000441C, 0x060A0002, // TG_SEN_GRAB_LIN

    #if 0 //no need
    0x00004420, 0x00000000,
    0x00004424, 0x00001430,
    0x00004428, 0x00000000,
    0x0000442C, 0x00000000,
    0x00004430, 0x00000000,
    0x00004434, 0x00000010,
    0x00004438, 0x0FFFFFFF,
    0x0000443C, 0x00000000,
    #endif

    #if 0//RO reg
    0x00004440, 0x0031313F,
    0x00004444, 0x00000000,
    0x00004448, 0x13890FA1, //CAM_TG_FRMSIZE_ST
    0x0000444C, 0xA5A50403,
    #endif

    #if 0 //no need
    0x00004460, 0x00000000,
    0x00004464, 0x00000000,
    0x00004468, 0x00000000,
    0x0000446C, 0x00000000,
    0x00004470, 0x00000000,
    0x00004474, 0x00000000,
    0x00004478, 0x00000000,
    0x0000447C, 0x00000000,
    0x00004480, 0x00000000,
    0x00004484, 0x00000000,
    0x00004488, 0x00000000,
    0x0000448C, 0x00000000,
    0x00004490, 0x00000000,
    0x00004494, 0x00000000,
    #endif

    0x00008000, 0x00000C00, //SENINF_TOP_CTRL
    #if 0 //no need
    //0x00008004, 0x00000000, //SENINF_TOP_CMODEL_PAR
    //0x00008008, 0x00003210, //SENINF_TOP_MUX_CTRL
    #endif
    0x00008100, 0x00001001,
    0x00008104, 0x00000000,
    0x00008108, 0x00000000,
    0x0000810C, 0x00000000,
    0x00008110, 0x00000000,
    0x00008114, 0x00000000,
    0x00008118, 0x00000000,
    0x0000811C, 0x00000000,
    0x00008120, 0x96DF1080,
    0x00008124, 0x8000007F,
    #if 0 //RO
    0x00008128, 0x00000078,
    #endif
    0x0000812C, 0x00000000,
    #if 0 //RO
    0x00008130, 0xAE00C200,
    0x00008134, 0x13890FA0,
    0x00008138, 0x13890FA0,
    0x0000813C, 0x13890FA0,
    0x00008140, 0x13890FA0,
    0x00008144, 0xFFFFEE90,
    0x00008148, 0xFFFFEE90,
    #endif
    0x0000814C, 0x000E2000,
    #if 0 //RO
    0x00008150, 0xC0007858,
    0x00008154, 0x01315029,
    #endif
    0x00008158, 0x00000000,

    #if 0 //no need
    //0x00008200, 0xA0000001, // TG1_PH_CNT
    //0x00008204, 0x00010001, // TG1_SEN_CK
    #endif
    0x00008208, 0x00040481, // TG1_TM_CTL
                            // [7:4]: green(3), red(5), blue(6), H_BAR(12), V_BAR(13)
    0x0000820C, 0x0FA01388, // TG1_TM_SIZE
    0x00008210, 0x00000000  // TG1_TM_CLK
};
MBOOL TS_FakeSensor::powerOn(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    MUINT32 nNum = 0, i = 0, ret = 0;

    m_pIspDrv = IspDrv::createInstance();
    if (NULL == m_pIspDrv) {
        TS_LOGD("Error: IspDrv CreateInstace fail");
        return 0;
    }

    ret = m_pIspDrv->init();
    if (ret < 0) {
        TS_LOGD("Error: IspDrv init fail");
        return 0;
    }

    m_pIspReg = (isp_reg_t *)m_pIspDrv->getRegAddr();

    nNum = (sizeof(mPowerOnTM) / sizeof(mPowerOnTM[0])) / 2;
    TS_LOGD(" Total %d registers", nNum);

    for (i = 0; i < nNum; i++) {
        if (0x00008208 == mPowerOnTM[i<<1])
        {
            TS_LOGD("Seninf Test Mode : %d", (atoi(szCallerName) & 0xF));
            mPowerOnTM[(i<<1)+1] = (0x00040401 | ((atoi(szCallerName) & 0xF) << 4));
        }
        m_pIspDrv->writeReg(mPowerOnTM[i<<1], mPowerOnTM[(i<<1)+1]);
    }

    for (i = 0; i < nNum; i++) {
        TS_LOGD(" Reg[x%08x] = x%08x/x%08x", mPowerOnTM[i<<1],
            m_pIspDrv->readReg(mPowerOnTM[i<<1]), mPowerOnTM[(i<<1)+1]);
    }

    return 1;
}
MBOOL TS_FakeSensor::powerOff(
        char const* szCallerName,
        MUINT const uCountOfIndex,
        MUINT const*pArrayOfIndex)
{
    if (m_pIspDrv) {
        m_pIspDrv->uninit();
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

    return 1;
}
MBOOL TS_FakeSensor::configure(
        MUINT const         uCountOfParam,
        IHalSensor::ConfigParam const*  pArrayOfParam)
{
    return 1;
}
MBOOL TS_FakeSensor::querySensorDynamicInfo(
          MUINT32 sensorIdx,
          SensorDynamicInfo *pSensorDynamicInfo)
{
    pSensorDynamicInfo->TgInfo;
    pSensorDynamicInfo->pixelMode;

    return 1;
}
