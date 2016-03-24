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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
 * @file aao_buf_mgr.h
 * @brief aao buffer manager
 */

#ifndef _AAO_BUF_MGR_H_
#define _AAO_BUF_MGR_H_

#include <utils/threads.h>
#include <list>
#include <mtkcam/drv_common/imem_drv.h>
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>

using namespace std;
using namespace android;


class IMemDrv;
class IspDrv;

namespace NS3Av3
{

/**
 * @brief enum for 3A statistics DMA
 */

typedef IMEM_BUF_INFO BufInfo_T;
typedef list<BufInfo_T> BufInfoList_T;

/*******************************************************************************
*  AAO buffer
*******************************************************************************/
#define AE_STAT_SIZE (((AWB_WINDOW_NUM_X + 3) / 4 ) * 4 * AWB_WINDOW_NUM_Y)
#define AE_HIST_SIZE (4 * 256)
#define AWB_STAT_SIZE (AWB_WINDOW_NUM_X * AWB_WINDOW_NUM_Y * 4)
#define AAO_BUFFER_SIZE (AE_STAT_SIZE + AE_HIST_SIZE + AWB_STAT_SIZE)
#define AAO_OFFSET_ADDR (0)
#define AAO_XSIZE (AAO_BUFFER_SIZE - 1)
#define AAO_YSIZE (0)
#define AAO_STRIDE_BUS_SIZE (3)

#define MAX_AAO_BUFFER_CNT (2)

/**
 * @brief 3A statistics buffer manager
 */
class IAAOBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:  ////
    IAAOBufMgr();
    ~IAAOBufMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief get instance
     */
    static IAAOBufMgr& getInstance()
    {
        static IAAOBufMgr singleton;
        return singleton;
    }

    /**
     * @brief init function
     */
    MRESULT init(MINT32 const i4SensorIdx);

    /**
     * @brief uninit function
     */
    MRESULT uninit();

    /**
     * @brief print the Virual and Physical memory address
     */
    MBOOL debugPrint();

    /**
     * @brief enqueue HW buffer
     * @param [in] rBufInfo buffer information
     */
    MRESULT enqueueHwBuf(BufInfo_T& rBufInfo);

    /**
     * @brief dequeue HW buffer
     * @param [in] rBufInfo buffer information
     */
    MRESULT dequeueHwBuf(BufInfo_T& rBufInfo);

    /**
     * @brief get current HW buffer
     * @return physical memory address of current HW buffer
     */
    MUINT32 getCurrHwBuf();

    /**
     * @brief get next HW buffer
     * @return physical memory address of next HW buffer
     */
    MUINT32 getNextHwBuf();

    /**
     * @brief allocate HW buffer
     * @param [in] rBufInfo buffer information
     * @param [in] u4BufSize buffer size
     */
    MRESULT allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize);

    /**
     * @brief free HW buffer
     * @param [in] rBufInfo buffer information
     */
    MRESULT freeBuf(BufInfo_T &rBufInfo);

    /**
     * @brief update DMA base address
     * @param [in] camdma2type<ECamDMA_AAO> AAO DMA
     * @param [in] u4BaseAddr base address
     */
    MRESULT updateDMABaseAddr(MUINT32 u4BaseAddr);

    /**
     * @brief DMA init function
     * @param [in] camdma2type<ECamDMA_AAO> AAO DMA
     */
    MRESULT DMAInit();

    /**
     * @brief DMA uninit function
     * @param [in] camdma2type<ECamDMA_AAO> AAO DMA
     */
    MRESULT DMAUninit();

    /**
     * @brief AA statistics enable function
     * @param [in] En TRUE: enable, FALSE: disable
     */
    MRESULT AAStatEnable(MBOOL En);

    /**
     * @brief send command to normal pipe
     * @param [in] cmd: ISP command
     * @param [in] arg1: enable module
     * @param [in] arg2: NULL
     * @param [in] arg3: NULL
     */
    MBOOL sendCommandNormalPipe(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IMemDrv *m_pIMemDrv;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL                  m_bDebugEnable;
    MINT32 m_i4SensorIdx;
    BufInfoList_T          m_rHwBufList;
    BufInfo_T m_rAAOBufInfo[MAX_AAO_BUFFER_CNT];

};

};  //  namespace NS3Av3
#endif // _AAO_BUF_MGR_H_

