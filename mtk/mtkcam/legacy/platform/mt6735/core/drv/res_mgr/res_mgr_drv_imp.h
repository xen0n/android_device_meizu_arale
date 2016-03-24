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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef RES_MGR_DRV_IMP_H
#define RES_MGR_DRV_IMP_H
//-----------------------------------------------------------------------------
#include <mtkcam/Log.h>
#include <mtkcam/drv/res_mgr_drv.h>
//-----------------------------------------------------------------------------
using namespace android;
//-----------------------------------------------------------------------------
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]"          fmt, __FUNCTION__,           ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s]WRN(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s]ERR(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
//-----------------------------------------------------------------------------
class ResMgrDrvImp : public ResMgrDrv
{
    protected:
        ResMgrDrvImp(MUINT32 sensorIdx);
        virtual ~ResMgrDrvImp();
    //
    public:
        static ResMgrDrv* getInstance(MUINT32 sensorIdx);
        virtual MVOID   destroyInstance();
        virtual MBOOL   init();
        virtual MBOOL   uninit();
        virtual MBOOL   getMode(MODE_STRUCT* pMode);
        virtual MBOOL   setMode(MODE_STRUCT* pMode);
        virtual MUINT32 getSensorIdx();
    private:
        //
        mutable Mutex       mLock;
        volatile MINT32     mUser;
        MUINT32             mSensorIdx;
        MODE_STRUCT         mMode;
};
//-----------------------------------------------------------------------------
#endif


