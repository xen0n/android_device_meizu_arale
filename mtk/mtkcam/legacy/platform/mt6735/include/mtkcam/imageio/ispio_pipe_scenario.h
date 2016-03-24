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
#ifndef _ISPIO_PIPE_SCENARIO_H_
#define _ISPIO_PIPE_SCENARIO_H_


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Driver Scenario
********************************************************************************/
enum EDrvScenario
{
    //
    eDrvScenario_CC_vFB_Normal = 0,      //  concurrency vFB (normal, P2A path)
    //
    eDrvScenario_CC_vFB_FB = 1,          //  concurrency vFB (FB, P2B path)
    //
    eDrvScenario_CC_CShot = 2 ,          //  concurrency Continuous Shot
    //
    eDrvScenario_CC_MFB_Blending = 3,    //  concurrency MFB blending
    //
    eDrvScenario_IP = 4,                 //  Image Playback (hardware scenario)
    //
    eDrvScenario_CC_MFB_Mixing = 5,      //  concurrency MFB mixing
    //
    eDrvScenario_CC = 6,                 //  concurrency (P2A path) (hardware scenario)
    //
    eDrvScenario_VSS = 7,                //  concurrency (P2A path)
    //
    eDrvScenario_CC_SShot = 8,           //  concurrency (P2A path), img3o is after crz
    //
    eDrvScenario_CC_RAW = 9,             //  IP RAW path with CC scenario
    //
    eDrvScenario_VSS_MFB_Blending = 10,    //  concurrency MFB blending via vss
     //
    eDrvScenario_VSS_MFB_Mixing = 11,      //  concurrency MFB mixing via vss
    //
    //
    eScenarioID_CONFIG_FMT               //  FMT (for MDP dpstream)
};


/*******************************************************************************
* Scenario Format
********************************************************************************/
enum EScenarioFmt
{
    //
    eScenarioFmt_RAW,       //
    //
    eScenarioFmt_YUV,       //
    //
    eScenarioFmt_RGB,       //
    //
    eScenarioFmt_FG,        //
    //
    eScenarioFmt_UFO,       //
    //
    eScenarioFmt_JPG,       //
    //
    eScenarioFmt_MFB,       //
    //
    eScenarioFmt_RGB_LOAD,  //
    //
    eScenarioFmt_UNKNOWN    = 0xFFFFFFFF,   //
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_SCENARIO_H_

