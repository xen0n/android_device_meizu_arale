/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _HWSYNC_DRV_IMP_H_
#define _HWSYNC_DRV_IMP_H_
//-----------------------------------------------------------------------------
//------------Thread-------------
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <utils/threads.h>
#include <list>
#include <vector>
//
#include "hwsync_drv.h"
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/drv/isp_drv.h>
//-----------------------------------------------------------------------------
using namespace std;
using namespace android;
using namespace NSCam;
//-----------------------------------------------------------------------------


#define SMI_CLOCK    (400000000)    ///<smi clock, the reference clock of n3d related count,268.6666667
#define REF_CLOCK    (SMI_CLOCK/100000)
#define DIFF_THRECNT (SMI_CLOCK/1000)   //1000us -> 1000*smi/1000000
#define CVGTHRE_VDDIFF  (500)   //us
#define CVGTHRE_FPSDIFF (500)   //us


/******************************************************************************
 *
 * @enum HW_SYNC_STATE_ENUM
 * @brief cmd enum for sendCommand.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_STATE_NONE             = 0X00,
    HW_SYNC_STATE_READY2RUN        = 0X01,
    HW_SYNC_STATE_SYNCING         = 0x02,
    HW_SYNC_STATE_READY2LEAVE     = 0x03
}HW_SYNC_STATE;

/******************************************************************************
 *
 * @enum ExecCmd
 * @brief enum for execution stage.
 * @details
 *
 ******************************************************************************/
enum ExecCmd
{
    ExecCmd_UNKNOWN = 0,
    ExecCmd_CHECKDIFF,
    ExecCmd_DOSYNC,
    ExecCmd_LEAVE
};

/******************************************************************************
 *
 * @struct ADJSENSOR_INFO
 * @brief parameter for the sensor we want to adjust.
 * @details
 *
 ******************************************************************************/
typedef struct ADJSENSOR_INFO
{
    MINT32     pclk;                    ///<pixel clock of the sensor we want to adjust
    MUINT32 period[2];                ///<period information without dummy at the current scenario{line pixel, frame length}
    MUINT32 dummyInfo[2];           ///<dummy information(dummy pixel, dummy line)
    MUINT32 dummyLNeeded;            ///<the dummy line we need to set at the current frame
    MUINT32 dummyLFinal;            ///<the dummy line we finally set to sensor at the current frame
    MUINT32 dummyLUB;
    MUINT32 dummyLLB;
    MUINT32 sw_curRegCntTus;        ///<current frame period between VDs of adjusted sensor
    //
    ADJSENSOR_INFO()
    : pclk(0)
    , period({0x0,0x0})
    , dummyInfo({0x0,0x0})
    , dummyLNeeded(0)
    , dummyLFinal(0)
    , dummyLUB(0)
    , dummyLLB(0)
    , sw_curRegCntTus(0)
    {}
};

/******************************************************************************
 *
 * @struct N3D_INFO
 * @brief parameter for n3d registers.
 * @details
 *
 ******************************************************************************/
struct N3D_INFO
{
    MUINT32 order;                  ///<the leading situation at the current frame(1 for vs1 leads vs2, and 0 for vs2 leads vs1)
    MUINT32 vs_regCnt[2];           ///<the time period count of sensor1 (main sensor) and sensor2 (main2 sensor)
                                    ///<vs_reg Cnt(n) actually is vs count(n-1) due to double buffer is adopted.
    MUINT32 vs_regTus[2];
    //trick point
    //the different value calculated from n3d DiffCNT_reg is the value of pre frame,
    //but the order[bit 31] we get is the situation of current frame
    //we could use dbgPort and cnt2 to estimate the different value of current frame
    MUINT32 vdDiffCnt;
    MUINT32 vdDiffTus;
    MUINT32 vs2_vs1Cnt;             ///<debug port is the count value of sensor2 sampled by sensor1 (unit:count)
    MUINT32 vs2_vs1Tus;             ///<debug port is the count value of sensor2 sampled by sensor1 (unit:us)
    //
    N3D_INFO()
    : order(0x0)
    , vdDiffCnt(0x0)
    , vdDiffTus(0x0)
    , vs_regCnt({0x0,0x0})
    , vs_regTus({0x0,0x0})
    , vs2_vs1Cnt(0x0)
    , vs2_vs1Tus(0x0)
    {}
};



/******************************************************************************
 *
 * @class HWSyncDrv Derived Class
 * @brief Driver to sync VDs.
 * @details
 *
 ******************************************************************************/
class HWSyncDrvImp : public HWSyncDrv
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    HWSyncDrvImp();
    virtual ~HWSyncDrvImp();
//
public:
    /**
     * @brief get the singleton instance
     *
     * @details
     *
     * @note
     *
     * @return
     * A singleton instance to this class.
     *
     */
    static HWSyncDrv*      getInstance(void);
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    virtual void        destroyInstance(void);
    /**
     * @brief init the hwsync driver
     *
     * @details
     * @param[in] sensorIdx: sensor index (0,1,2..).
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32       init(MUINT32 sensorIdx);
    /**
     * @brief uninit the hwsync driver
     *
     * @details
     * @param[in] sensorIdx: sensor index(0,1,2..).
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32       uninit(MUINT32 sensorIdx);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @brief sendCommand to change setting
     *
     * @details
     * @param[in] cmd: command.
     * @param[in] senDev: sensor dev.
     * @param[in] senScen: sensor scenario.
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32         sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 senDev,MUINT32 senScen, MUINT32 currfps);


private:
    /**
     * @brief mode changed
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      changeMode();
    /**
     * @brief sync via batch i2c
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      doI2CSync(void);
    /**
     * @brief sync via set dummy line
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      doOneTimeSync(void);
    /**
     * @brief sync via set dummy line
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32      doGeneralSync(void);
    /**
     * @brief check difference of VDs to judge that we need to do sync or not
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates the difference is larger than threshold;
     * - MFALSE indicates the difference is smaller than threshold.
     */
    virtual MBOOL          checkNeedSync(void);
    /**
     * @brief get information of the sensor we want to adjust
     *
     * @details
     * @param[in] senDev: sensor device id.
     * @param[in] pclk: the returned pclk of sensor we want to adjust.
     * @param[in] periodInfo: the returned period information of sensor we want to adjust.
     * @param[in] dummyInfo: the returned dummy information of sensor we want to adjust.
     * @param[in] curfps: current fps.
     *
     * @note
     *
     * @return
     */
    virtual MINT32      getSensorInfo(MUINT32 senDev,MINT32* pclk,MINT32* periodInfo,MINT32* dummyInfo,MINT32 curfps);
    /**
     * @brief check we need to change the sensor which is going to be adjusted or not
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates yes;
     * - MFALSE indicates no.
     */
    virtual MBOOL          checkNeedChangeSr(void);
    /**
     * @brief revert sensor fps whenever hwsync driver are disabled
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates yes;
     * - MFALSE indicates no.
     */
    virtual MINT32         revertSensorFps(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief Create a thread dedicated for sync.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID createThread();
    /**
     * @brief Destroy the thread dedicated for sync.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID destroyThread();
    /**
     * @brief add command to execute
     *
     * @details
     * @param[in] cmd: command to be executed.
     *
     * @note
     *
     * @return
     * - MTRUE indicates the difference is larger than threshold;
     * - MFALSE indicates the difference is smaller than threshold.
     */
    virtual MVOID addExecCmd(ExecCmd const &cmd);
    /**
     * @brief clear all the command
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MVOID clearExecCmds();
     /**
     * @brief get the first command in cmd list.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    virtual MBOOL get1stCmd(ExecCmd &cmd);
      /**
      * @brief Thread loop for dequeuing buffer.
      *
      * @details
      *
      * @note
      *
      * @return
      *
      */
     static  MVOID*  onThreadLoop(MVOID*);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    sem_t      mSemHWSyncLv;
private:
    //[general]
    mutable  Mutex      mLock;
    volatile MINT32     mUser;
    IHalSensor*         mHalSensor;
    IHalSensorList*     mHalSensorList;
    vector<MUINT32>     mSensorIdxList; //
    MUINT32             mSensorDevArray[2]; //sorted by TG(0: TG1 -> dev1, 1: TG2 -> dev2)
    MUINT32             mSensorTgArray[2];  //sorted by the same order with mSensorIdxList (0:mSensorDevArray[0], 1:mSensorDevArray[1], maxNum=2)
    MINT32              mSensorFpsArray[2]; //osrted by the same orfer with  mSensorIdxList (0:mSensorDevArray[0], 1:mSensorDevArray[1], maxNum=2), UNIT: 10 base
    MUINT32             mSensorScenArray[2];//osrted by the same orfer with  mSensorIdxList (0:mSensorDevArray[0], 1:mSensorDevArray[1], maxNum=2)

    HW_SYNC_STATE       mState;
    MINT32              mCheckCnt;  //10 for check diff once
    ISP_DRV_WAIT_IRQ_STRUCT mWaitIrq;
    MINT32              mAdjIdx;
    N3D_INFO            mN3DInfo;
    ADJSENSOR_INFO      mAdjSrInfo;
    MUINT32             mInitPclk[2];       ///<initial sensor pixel clock info, we get this at change mode
    MUINT32             mInitperiodSr1[2];  ///<initial sensor period info, we get this at change mode, cuz some sensor driver would
    MUINT32             mInitperiodSr2[2];
    MBOOL               m1stSync;          //1 st sync after change mode or not
    MINT32              mCurDftFps;     //default fps in current mode
    MINT32              mfpsUB;
    MINT32              mfpsLB;
    //[thread]
    mutable  Mutex      mThreadMtx;
    pthread_t           mThread;
    list<ExecCmd>       mExecCmdList;
    Condition           mExecCmdListCond;
};
//-----------------------------------------------------------------------------
#endif


