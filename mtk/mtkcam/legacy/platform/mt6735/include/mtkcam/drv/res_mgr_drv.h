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
#ifndef RES_MGR_DRV_H
#define RES_MGR_DRV_H
//-----------------------------------------------------------------------------
class ResMgrDrv
{
    protected:
        virtual ~ResMgrDrv() {};
    //
    public:
        typedef enum
        {
            SCEN_SW_NONE,
            SCEN_SW_CAM_IDLE,
            SCEN_SW_CAM_PRV,
            SCEN_SW_CAM_CAP,
            SCEN_SW_CAM_CSHOT,
            SCEN_SW_VIDEO_PRV,
            SCEN_SW_VIDEO_REC,
            SCEN_SW_VIDEO_REC_HS,
            SCEN_SW_VIDEO_VSS,
            SCEN_SW_ZSD
        }SCEN_SW_ENUM;
        //
        typedef enum
        {
            DEV_CAM,
            DEV_VT
        }DEV_ENUM;
        //
        typedef struct
        {
            SCEN_SW_ENUM    scenSw;
            DEV_ENUM        dev;
        }MODE_STRUCT;
        //
        static ResMgrDrv* createInstance(MUINT32 sensorIdx);
        virtual MVOID   destroyInstance() = 0;
        virtual MBOOL   init() = 0;
        virtual MBOOL   uninit() = 0;
        virtual MBOOL   getMode(MODE_STRUCT* pMode) = 0;
        virtual MBOOL   setMode(MODE_STRUCT* pMode) = 0;
};
//-----------------------------------------------------------------------------
#endif


