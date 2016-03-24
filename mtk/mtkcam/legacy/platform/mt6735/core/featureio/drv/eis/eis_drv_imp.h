/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file eis_drv_imp.h
*
* EIS Driver Implementation Header File
*
*/


#ifndef _EIS_DRV_IMP_H_
#define _EIS_DRV_IMP_H_

#include "eis_drv.h"

/**
  *@brief EIS HW register
*/
struct EIS_REG_INFO
{
    MUINT32 reg_eis_prep_me_ctrl1;  // CAM_EIS_PREP_ME_CTRL1
    MUINT32 reg_eis_prep_me_ctrl2;  // CAM_EIS_PREP_ME_CTRL2
    MUINT32 reg_eis_lmv_th;         // CAM_EIS_LMV_TH
    MUINT32 reg_eis_fl_offset;      // CAM_EIS_FL_OFFSET
    MUINT32 reg_eis_mb_offset;      // CAM_EIS_MB_OFFSET
    MUINT32 reg_eis_mb_interval;    // CAM_EIS_MB_INTERVAL
    MUINT32 reg_eis_gmv;            // CAM_EIS_GMV, not use
    MUINT32 reg_eis_err_ctrl;       // CAM_EIS_ERR_CTRL, not use
    MUINT32 reg_eis_image_ctrl;     // CAM_EIS_IMAGE_CTRL
public :
    EIS_REG_INFO(MUINT32 a_eisPrepMeCtrl1 = 0,
                       MUINT32 a_eisPrepMeCtrl2 = 0,
                       MUINT32 a_eisLmvTh = 0,
                       MUINT32 a_eisFlOffset = 0,
                       MUINT32 a_eisMbOffset = 0,
                       MUINT32 a_eisMbInterval = 0,
                       MUINT32 a_eisGmv = 0,
                       MUINT32 a_eisErrCtrl = 0,
                       MUINT32 a_eisImageCtrl = 0)
                          : reg_eis_prep_me_ctrl1(a_eisPrepMeCtrl1),
                            reg_eis_prep_me_ctrl2(a_eisPrepMeCtrl2),
                            reg_eis_lmv_th(a_eisLmvTh),
                            reg_eis_fl_offset(a_eisFlOffset),
                            reg_eis_mb_offset(a_eisMbOffset),
                            reg_eis_mb_interval(a_eisMbInterval),
                            reg_eis_gmv(a_eisGmv),
                            reg_eis_err_ctrl(a_eisErrCtrl),
                            reg_eis_image_ctrl(a_eisImageCtrl)

    {}
};

/**
  *@brief EISO DMA register
*/
struct EISO_DMA_INFO
{
    MUINT32 reg_eiso_base_addr; // CAM_EISO_BASE_ADDR
    MUINT32 reg_eiso_xsize;     // CAM_EISO_XSIZE
public :
    EISO_DMA_INFO(MUINT32 a_eisoBaseAddr = 0,
                         MUINT32 a_eisoXsize = 0)
                           : reg_eiso_base_addr(a_eisoBaseAddr),
                             reg_eiso_xsize(a_eisoXsize)
    {}
};

/**
  *@brief SGG2 HW register
*/
struct SGG2_REG_INFO
{
    MUINT32 reg_sgg2_pgn;       // CAM_SGG2_PGN
    MUINT32 reg_sgg2_gmrc_1;    // CAM_SGG2_GMRC_1
    MUINT32 reg_sgg2_gmrc_2;    // CAM_SGG2_GMRC_2
public :
    SGG2_REG_INFO(MUINT32 a_sgg2Pgn = 0,
                          MUINT32 a_sgg2Gmrc1 = 0,
                          MUINT32 a_sgg2Gmrc2 = 0)
                             : reg_sgg2_pgn(a_sgg2Pgn),
                               reg_sgg2_gmrc_1(a_sgg2Gmrc1),
                               reg_sgg2_gmrc_2(a_sgg2Gmrc2)
    {}
};

/**
  * @brief EISO Data
  *
*/
typedef struct _EISO_DATA_
{
    MUINTPTR va;
    MUINTPTR pa;
    MINT64 timeStamp;
}EISO_DATA;

/**
*@brief register name enum
*/
typedef enum
{
    REG_EIS,
    REG_EISO,
    REG_SGG2
}REG_NAME_ENUM;

/**
  *@brief EIS Pass1 HW Setting Callback
*/
class EisP1Cb : public P1_TUNING_NOTIFY
{
     public:
         EisP1Cb(MVOID *arg);
       ~EisP1Cb();

     virtual void p1TuningNotify(MVOID* pInput,MVOID *pOutput);
    virtual const char* TuningName() { return "Update EIS"; };
 };

/**
  *@brief Implementation of EisDrv class
*/
class EisDrvImp : public EisDrv
{
public:

    /**
         *@brief Create EisDrv object
         *@param[in] sensorIdx : sensor index
         *@return
         *-EisDrvImp object
       */
    static EisDrv *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy EisDrv object
       */
    virtual MVOID DestroyInstance();

    /**
         *@brief Initial function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init();

    /**
         *@brief Uninitial function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Configure EIS and related register value
         *@param[in] aSce : EIS configure scenario
         *@param[in] aSensorTg : sensor TG info for debuging usage
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigEisReg(const EIS_SCENARIO_ENUM &aSce,const MUINT32 &aSensorTg);

    /**
         *@brief Configure FEO
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigFeo();

     /**
         *@brief Enable/Disable EIS
         *@param[in] aEn : MTRUE (enable) / MFALSE (disable)
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 EnableEis(const MBOOL &aEn);

     /**
         *@brief Set configure done
         *@param[in] aState : state enum
       */
    virtual MVOID SetEisoThreadState(EIS_SW_STATE_ENUM aState);

    /**
         *@brief To indicate the first frame
         *@param[in] aFirst : 1 (is the first frame) / 0 (is not the first frame)
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 SetFirstFrame(const MUINT32 &aFirst);

    /**
         *@brief To indicate the first frame
         *@param[in] aInputW : FE input width
         *@param[in] aInputH : FE input height
         *@param[in] aBlockNum : FE block number
       */
    virtual MVOID SetFeBlockNum(const MUINT32 &aInputW, const MUINT32 &aInputH,const MUINT32 &aBlockNum);

    /**
         *@brief Get first frame or not
         *@return
         *-0 : not first frame, 1 : first frame
       */
    virtual MUINT32 GetFirstFrameInfo();

    /**
         *@brief Get first frame or not
         *@return
         *-0 : not 2-pixel mode, 1 : 2-pixel mode
       */
    virtual MUINT32 Get2PixelMode();

    /**
         *@brief  Get input width/height of EIS HW
       */
    virtual MVOID GetEisInputSize(MUINT32 *aWidth, MUINT32 *aHeight);

    /**
         *@brief  Return  EIS HW setting of EOS_OP_HORI
         *@return
         *-EOS_OP_HORI
       */
    virtual MUINT32 GetEisDivH();

    /**
         *@brief  Return  EIS HW setting of EIS_OP_VERT
         *@return
         *-EIS_OP_VERT
       */
    virtual MUINT32 GetEisDivV();

    /**
         *@brief  Return  Total MB number
         *@return
         *-MBNum_V * MB_Num_H
       */
    virtual MUINT32 GetEisMbNum();

    /**
         *@brief  Get FEO register setting
         *@param[in,out] aFeoRegInfo : FEO_COFIG_DATA
       */
    virtual MVOID GetFeoRegInfo(FEO_COFIG_DATA *aFeoRegInfo);

    /**
         *@brief Get EIS HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates EIS HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetEisSupportInfo(const MUINT32 &aSensorIdx);

#if EIS_ALGO_READY

    /**
         *@brief  Get statistic of EIS HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(EIS_STATISTIC_STRUCT *apEisStat,const MINT64 &aTimeStamp);

#else

    /**
         *@brief  Get statistic of EIS HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(EIS_STATISTIC_T *apEisStat,const MINT64 &aTimeStamp);

#endif

    /**
         *@brief  Get statistic of FEO
         *@param[in,out] FEO_STAT_DATA : FEO statistic data
       */
    virtual MVOID GetFeoStatistic(FEO_STAT_DATA *aFeoStatData);

    /**
         *@brief  Flush memory
         *@param[in] aDma : DMA port : EISO or FEO
          *@param[in] aFlush : flush type
       */
    virtual MVOID FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush);

    /**
         *@brief  Dump EIS register setting
         *@param[in] aEisPass : EIS pass ENUM
         *@details Debug usage
       */
    virtual MVOID DumpReg(const EIS_PASS_ENUM &aEisPass);

     /**
         *@brief Do boundary check
         *@param[in,out] a_input : input number
         *@param[in] upBound : upper bound
         *@param[in] lowBound : lower bound
       */
    MVOID  BoundaryCheck(MUINT32 &aInput,const MUINT32 &upBound,const MUINT32 &lowBound);

#if EIS_WORK_AROUND

    virtual MUINT32 GetTgRrzRatio();

#endif

    //------------------------------------------------------------------------

    // EIS and related register setting
    EIS_REG_INFO mEisRegSetting;
    EISO_DMA_INFO mEisoDmaSetting;
    SGG2_REG_INFO mSgg2RegSetting;


    // member variable
    MUINT32 mIsConfig;
    MUINT32 mIsFirst;
    MUINT32 mIs2Pixel;
    MUINT32 mTotalMBNum;
    MUINT32 mImgWidth;
    MUINT32 mImgHeight;
    MUINT32 mEisDivH;
    MUINT32 mEisDivV;
    EIS_SENSOR_ENUM mSensorType;
    EIS_SCENARIO_ENUM mEisHwCfgSce;

private:

    /**
         *@brief LCSO thread loop
       */
    static MVOID *EisoThreadLoop(MVOID *arg);

    /**
         *@brief  EisDrvImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    EisDrvImp(const MUINT32 &aSensorIdx);

    /**
         *@brief  EisDrvImp destructor
       */
    ~EisDrvImp();

    /**
         *@brief Update EISO base address
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 UpdateEiso();

    /**
         *@brief Update EISO base address index
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 UpdateEisoIdx();

    /**
         *@brief Create memory by using IMem
         *@param[in,out] memSize : input already calculated size and will set to alingSize
         *@param[in] bufCnt : how many memory need to be created
         *@param[in,out] bufInfo : pointer to IMEM_BUF_INFO
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo);

    /**
         *@brief Destroy memory by using IMem
         *@param[in] bufCnt : how many memory need to be destroyed
         *@param[in,out] bufInfo : pointer to IMEM_BUF_INFO
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMemBuf(const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo);

    /**
         *@brief Perform complement2 on value according to digit
         *@param[in] value : value need to do complement2
         *@param[in] digit : indicate how many digits in value are valid
         *@return
         *-value after doing complement2
       */
    MINT32 Complement2(MUINT32 value, MUINT32 digit);

    /**
         *@brief Write register to HW by using INormalPipe
         *@param[in] aRegName : name of register which is going to write
         *@param[in] aSensorType : sensor type
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 WriteReg(const REG_NAME_ENUM &aRegName);

    /**
         *@brief Write register to HW by using INormalPipe
         *@param[in] aEisSel : value of eis mux sel
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 ConfigMuxSel(const MUINT32 &aEisSel);

    /**
         *@brief Set state
         *@param[in] aState : state
       */
    MVOID SetEisState(const EIS_SW_STATE_ENUM &aState);

    /**
         *@brief Get state
         *@return
         *-state
       */
    EIS_SW_STATE_ENUM GetEisState();

    /**
         *@brief Change thread setting
         *@param[in] userName : thread name
       */
    MVOID ChangeThreadSetting(char const *userName);

    /**
         *@brief Get timestamp as ISP driver gave MW
         *@param[in] aSec : second
         *@param[in] aUs : micro second
       */
    MINT64 GetTimeStamp(const MUINT32 &aSec,const MUINT32 &aUs);

    /***********************************************************************************/

    volatile MINT32 mUsers;
    mutable Mutex mLock;
    mutable Mutex mIdxLock;
    mutable Mutex mQueLock;
    mutable Mutex mSateLock;

    // thread
    pthread_t mEisoThread;
    sem_t mEisoSem;
    sem_t mEisoSemEnd;

    // condition
    Condition mEisoCond;

    // IMEM
    IMemDrv *m_pIMemDrv;
    IMEM_BUF_INFO mEisoDmaBuf;
    IMEM_BUF_INFO mFeoDmaBuf;

    // INormalPipe
    INormalPipe *m_pNormalPipe;

    // ISP driver object
    IspDrv *m_pISPDrvObj;
    IspDrv *m_pISPVirtDrv;  // for command queue

    // FEO variable
    MUINT32 mFeoStaticNumX;
    MUINT32 mFeoStaticNumY;

    // member variable
    MUINT32 mSensorIdx;
    MUINT32 mSensorTg;
    MUINT32 mEiso_addrIdx;
    MUINT32 mEiso_virAddr;
    MUINT32 mEiso_phyAddr;
    MBOOL   mEisHwSupport;

    // Pass1 HW setting callback
    EisP1Cb *m_pEisP1Cb;

    // state
    EIS_SW_STATE_ENUM mState;

    //EISO data
    queue<EISO_DATA> mEisoData;
};

#endif // _EIS_DRV_IMP_H_


