// FlashAlg.h: interface for the FlashAlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLASHALG_H__B5C93207_0BD8_4488_85C9_47A3A86829E5__INCLUDED_)
#define AFX_FLASHALG_H__B5C93207_0BD8_4488_85C9_47A3A86829E5__INCLUDED_
/*
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/
#include "aaa_algo_option.h"
#include "flash_awb_param.h"

namespace NS3A
{

struct FlashAlgStrobeProfile
{
public:
    float iso;
    int exp;
    float distance;
    int dutyNum;
    int stepNum;
    int dutyTickNum;
    int stepTickNum;
    int* dutyTick;
    int* stepTick;
    float* engTab;
    float pfEng;

    //int dutyNumL;
    //int* dutyTickL;

       AWB_GAIN_T* pFlWbCalTbl;
//    char* RTab;
//    char* GTab;
//    char* BTab;
};

struct FlashAlgStrobeLim
{
    int minStep;
    int maxStep;
    int minDuty;
    int maxDuty;
};

struct FlashAlgExpPara
{
    int exp;
    int iso;
    int isFlash;
    int step;
    int duty;

    //int dutyL;  // modify for dual-flash
};

struct FlashAlgStaData
{
    int row;
    int col;
    int dig_row;
    int dig_col;
    short* data;
    double normalizeFactor;
    int bit;
};

struct FlashAlgPLineNode
{
    int iso;
    int exp;
};


typedef struct    evSetting
{
    MUINT32 u4Eposuretime;   //!<: Exposure time in ms
    MUINT32 u4AfeGain;       //!<: raw gain
    MUINT32 u4IspGain;       //!<: sensor gain
    MUINT8  uIris;           //!<: Iris
    MUINT8  uSensorMode;     //!<: sensor mode
    MUINT8  uFlag;           //!<: flag to indicate hysteresis ...
//    MUINT8  uLV;                        //!<: LV avlue , in ISO 100 condition  LV=TV+AV
}evSetting;


typedef struct    PLine
{
    MUINT32       u4TotalIndex;      //preview table Tatal index
    MINT32        i4StrobeTrigerBV;  // Strobe triger point in strobe auto mode
    MINT32        i4MaxBV;
    MINT32        i4MinBV;
    evSetting *pCurrentTable;   //point to current table
}PLine;


enum
{
    FlashAlg_Err_Ok=0,
    FlashAlg_Err_NoMem=-100,
    FlashAlg_Err_Div0=-101,
    FlashAlg_Err_Para=-102,
    FlashAlg_Err_Other=-103,
    FlashAlg_Err_CaliDataNotSet=-104,
    FlashAlg_Stage_Af=1,
    FlashAlg_Stage_Capture=0,
};

typedef enum
{
       eFlashMainSingle=0,
       eFlashSubSingle,
       eFlashMainDual,
       eFlashSubDual,
       eFlashMain2Single,
} EFLASH_ID_TYPE;


typedef struct
{
        AWB_GAIN_T NonPreferencePrvAWBGain;
        AWB_GAIN_T PreferencePrvAWBGain;
        MINT32 i4AWBCCT;
        AWB_ROTATION_MATRIX_T AwbRotationMatrix;
        MINT32 i4RotationMatrixUnit; // Rotation matrix unit
}FlashAlgInputInfoStruct;

typedef struct
{
    double LumaY;    // outpur for flash AE on NVRAM
    double R;
    double G;
    double B;
    int Rgain;
    int Ggain;
    int Bgain;
    int duty;    // X
    int dutyLT; // Y, low color temporature
}CalData;

typedef struct
{
    int duty;
    int dutyLT; // Y, low color temporature
}ChooseResult;

class FlashAlg
{
public:
    virtual ~FlashAlg();
    static FlashAlg* getInstance(MINT32 const eFlashDev);
    static FlashAlg* getInstanceLite(MINT32 eFlashDev);

    //procedure (must call)
    virtual void Estimate(FlashAlgExpPara* exp)=0;
    virtual int Reset()=0;
    virtual int ResetIntermediate()=0;
    virtual int ResetReport()=0;
    virtual int AddStaData10(FlashAlgStaData* data, FlashAlgExpPara* exp, int* isNeedNext, FlashAlgExpPara* expNext)=0;
    virtual int CalFirstEquAEPara(FlashAlgExpPara* exp, FlashAlgExpPara* EquExp)=0;
    virtual int setFlashProfile(FlashAlgStrobeProfile* pr)=0;
    virtual int setCapturePLine(int num, FlashAlgPLineNode* nodes)=0;
    virtual int setPreflashPLine(int num, FlashAlgPLineNode* nodes)=0;

    virtual int setCapturePLine(PLine* p, int isoAtGain1x)=0;
    virtual int setPreflashPLine(PLine* p, int isoAtGain1x)=0;


    virtual int setEVComp(float ev_comp)=0;
    virtual int setEVCompEx(float ev_comp, float tar, float evLevel)=0;
    virtual float calFlashEng(int duty, int rStep)=0;
    //checkInputParaError
    //setDebugDataSize


    //strobe cali
    virtual int setCaliData(int caliNum, int* caliStep, int* caliVBat_mV, int* caliIFlash_mA, int refIRange, int extrapIRange)=0;
    virtual int calStepDuty(int Bat_mV, int peak_mA, int ave_mA, int* step, int* duty)=0;



    //preference (optional setting)
    virtual int setWTable256(int w, int h, short* tab)=0;

    virtual int setDefaultPreferences()=0;
    virtual int setTuningPreferences(FLASH_TUNING_PARA *tuning_p, int lv)=0;

    //debug
    virtual int setIsSaveSimBinFile(int isSaveBin)=0;
    virtual int setDebugDir(const char* DirName, const char* PrjName)=0;
    virtual void getLastErr(int* time, int* type, int* reserve)=0; //max 3
    virtual void getLastWarning(int* time, int* type, int* reserve)=0; //max 3
    virtual void fillDebugData2(void* data)=0; //500 bytes
    virtual int checkInputParaError(int* num, int* errBuf)=0; //num: input and output
    virtual void setDebugDataSize(int sz)=0; //should be set initially for check the size in the alg.

       virtual MVOID Flash_Awb_Init(FLASH_AWB_INIT_T &FlashAwbInit) = 0;
       virtual MVOID Flash_Awb_Algo(MUINT32 *FlashResultWeight) = 0;
       virtual MVOID getReflectionTblIdx(REFLECTION_TBL_IDX *pReflectionTblIdx)= 0;
    virtual MVOID  getDistanceInfo() = 0;
       virtual MUINT32 calLocationWeight(MUINT32 x, MUINT32 y) = 0;
       virtual MVOID getAvgFlashContribute() = 0;
       virtual MVOID ParameterRangeCheck(FLASH_AWB_TUNING_PARAM_T *pFlashTuningRange) = 0;
       virtual MVOID CalFgWeight() = 0;
       virtual MUINT32 GetYPrimeWeight(MUINT32 YPrimeValue) = 0;

       virtual MVOID setFlashInfo(FlashAlgInputInfoStruct *pFlashInfo) = 0;

    virtual MRESULT DecideCalFlashComb(int CalNum, short *yTab, int totalStep, int totalStepLT, ChooseResult *pChoose) = 0;
        virtual MRESULT InterpolateCalData(int CalNum, short *dutyI, short *dutyLtI, CalData *pCalData, short *yTab, AWB_GAIN_T *pGoldenWBGain, int totalStep, int totalStepLT, AWB_GAIN_T *outWB, short *outYTab) = 0;
};
};

#endif // !defined(AFX_FLASHALG_H__B5C93207_0BD8_4488_85C9_47A3A86829E5__INCLUDED_)
