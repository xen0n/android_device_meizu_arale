#pragma once

enum
{
    e_OemErr_FileErr=-1000,
};

enum
{
    e_MaxPathSize=50,
    //CAMERA_DATA_TYPE_NUM=5,
};


enum
{
    e_SensorRelation,
    //e_StrobeRelation,  //all depend on sensor id
    //e_LensRelation,
    e_NoneRelation,
};

enum
{
    e_SupportSensorDev,
    e_NotSupportSensorDev,
};

struct OemItemDres
{
    int id;
    int sensorDevRelation;
    int componentRelation;
    char path[e_MaxPathSize];

};

OemItemDres* getOemDres();

class OemCamDrv
{
public:
    static OemCamDrv* getInstance();
    int readOem(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          unsigned long u4SensorID,
                          CAMERA_DATA_TYPE_ENUM nvRamId,
                          void *a_pNvramData,
                          unsigned long a_u4NvramDataSize,
                          int version
                          );

    int isOemAvailable();



};

