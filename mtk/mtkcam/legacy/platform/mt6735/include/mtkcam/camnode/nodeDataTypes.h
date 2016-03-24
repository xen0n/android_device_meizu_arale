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

#ifndef _MTK_CAMERA_INCLUDE_CAMNODE_NODEDATATYPES_H_
#define _MTK_CAMERA_INCLUDE_CAMNODE_NODEDATATYPES_H_
//
//


/******************************************************************************
*
*******************************************************************************/


namespace NSCamNode {

/*******************************************************************************
*
********************************************************************************/

//28 bit(notifytype) + 4 bit(category)
// category   (bit 0~3)  : 0~7
// notifytype (bit 4~31) : should be 2^n, n = 0~27
#define NOTIFYCATEGORY(category)       ( (category) & 0xF )
#define NOTIFYTYPE(category, type)     ( ((category) & 0xF) | ((type & 0xFFFFFFF) << 4) )

/*******************************************************************************
*
********************************************************************************/
//4 bits
typedef enum NodeNotifyCategory{
    NODECAT_BASIC_NOTIFY = NOTIFYCATEGORY(0x0),
    NODECAT_FEATUREPIPE_NOTIFY = NOTIFYCATEGORY(0x1),
    PRESERVED_1          = NOTIFYCATEGORY(0x2),
    PRESERVED_2          = NOTIFYCATEGORY(0x3),
    PRESERVED_3          = NOTIFYCATEGORY(0x4),
    PRESERVED_4          = NOTIFYCATEGORY(0x5),
    PRESERVED_5          = NOTIFYCATEGORY(0x6),
    NODECAT_GLOBAL_CMD   = NOTIFYCATEGORY(0x7),
} NodeDataCategory;

typedef enum NodeEndPoint{
    ENDPOINT_SRC       = 0x0,
    ENDPOINT_DST       = 0x1,
} NodeEndPoint;
//28+4 bits
typedef enum NodeDataTypes{
    /* no type */
    NODE_DATATYPE_NONE   = 0,
    /* basic data */
    PASS1_RESIZEDRAW     = 1,
    PASS1_FULLRAW        = 2,
    PASS1_PURERAW        = 3,   // output data type for pass1 node
    CONTROL_RESIZEDRAW   = 11,
    CONTROL_FULLRAW      = 12,
    CONTROL_PURERAW      = 13,  // input data type for default control node
    CONTROL_PRV_SRC      = 21,
    CONTROL_CAP_SRC      = 22,
    CONTROL_DBGINFO      = 23,
    CONTROL_EISDATA      = 24,
    CONTROL_PURERAW_SRC  = 25,  // output data type for default control node
    PASS2_PRV_SRC        = 31,
    PASS2_PRV_DST_0      = 41,
    PASS2_PRV_DST_1      = 43,
    PASS2_PRV_DST_2      = 45,
    PASS2_PRV_DST_3      = 47,
    PASS2_CAP_SRC        = 32,
    PASS2_CAP_DST_0      = 42,
    PASS2_CAP_DST_1      = 44,
    PASS2_CAP_DST_2      = 46,
    JPEG_ENC_SW_SRC      = 100,
    JPEG_ENC_SW_DST      = 101,
    JPEG_ENC_HW_SRC      = 102,
    JPEG_ENC_HW_DST      = 103,
    SHOTCB_RAWDATA_SRC   = 200,
    SHOTCB_YUVDATA_SRC   = 201,
    SHOTCB_POSTVIEW_SRC  = 202,
    SHOTCB_JPEG_SRC      = 203,
    SHOTCB_THUMBNAIL_SRC = 204,
    SHOTCB_DBGINFO_SRC   = 205,
    SHOTCB_EISDATA_SRC   = 206,
    SHOTCB_JPS_SRC       = 207, // JPS
    SHOTCB_DBGINFO_1_SRC = 208, // for main2
    SHOTCB_RAWDATA_DST   = 210,
    SHOTCB_YUVDATA_DST   = 211,
    SHOTCB_POSTVIEW_DST  = 212,
    RECBUFSS_SRC         = 300,
    /* stereo feature */
    SYNC_SRC_0           = 430,
    SYNC_SRC_1           = 431,
    SYNC_DST_0           = 432,
    SYNC_DST_1           = 433,
    STEREO_SRC           = 400,
    STEREO_FEO           = 401,
    STEREO_IMG           = 402,
    STEREO_RGB           = 403,
    STEREO_DST           = 404,
    STEREO_CTRL_FEO_0    = 410,
    STEREO_CTRL_IMG_0    = 411,
    STEREO_CTRL_RGB_0    = 412,
    STEREO_CTRL_FEO_1    = 413,
    STEREO_CTRL_IMG_1    = 414,
    STEREO_CTRL_RGB_1    = 415,
    STEREO_CTRL_MAIN_SRC = 416,
    STEREO_CTRL_DST_M    = 417, // for SBS: main
    STEREO_CTRL_DST_S    = 418, // for SBS: main2(small)
    STEREO_CTRL_MAIN_DST = 419,
    /* image transform */
    TRANSFORM_SRC        = 500,
    TRANSFORM_DST_0      = 501,
    TRANSFORM_DST_1      = 502,
    //
    DUALIT_SRC_0         = 510,
    DUALIT_SRC_1         = 511,
    DUALIT_DST_0         = 512, // (rotate) from SRC_0
    DUALIT_DST_1         = 513, // (rotate) from SRC_1
    DUALIT_DST_2         = 514, // SBS
    /* ExtImgProc */
    EXTIMGPROC_SRC       = 600,
    EXTIMGPROC_DST_0     = 601,
    /* PDAF */
    PDAF_SRC             = 700, // input data type of PDAF node
    /* feature pipe */
    P2A_TO_EISSW         = 1000,
    P2A_TO_VFBSW_DSIMG   = 1002,
    P2A_TO_GPU           = 1003,
    P2A_TO_P2B           = 1004,
    P2A_TO_MDP_QPARAMS   = 1005,
    EISSW_TO_VFBSW       = 1006,
    EISSW_TO_GPU         = 1007,
    VFBSW_TO_GPU         = 1008,
    VFBSW_TO_P2B_DSIMG   = 1009,
    VFBSW_TO_P2B_ALPCL   = 1010,
    VFBSW_TO_P2B_ALPNR   = 1011,
    VFBSW_TO_P2B_PCA     = 1012,
    GPU_TO_MDP           = 1013,
    NR3D_PINGPONG        = 1015,
    P2A_TO_FD_DSIMG      = 1016,
    FD_TO_VFBSW          = 1017,
    P2A_TO_SWITCHER      = 1018,
    MDP_TO_SWITCHER      = 1019,
    SWITCHER_TO_GPU      = 1020,
    SWITCHER_TO_P2B      = 1021,
    SWITCHER_TO_VFBSW    = 1022,
} NodeDataTypes;

typedef enum NodeNotifyTypes{
    NODE_NOTIFY_NONE     = 0x0,
    /********************** basic notify **********************/
    /*
     * Notify the timing before starting pass1
     * @param cmd  : PASS1_START_ISP
     * @param ext1 : magic num
     */
    PASS1_START_ISP      = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x1        ) ,
    /*
     * Notify the timing before stoping pass1
     * @param cmd  : PASS1_STOP_ISP
     */
    PASS1_STOP_ISP       = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x2        ) ,

    /* not used yet
     * Notify the timing after starting pass1
     * @param cmd: PASS1_SOF
     */
    PASS1_SOF            = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x4        ) ,

    /*
     * Notify the timing of end of frame (pass1)
     * @param cmd  : PASS1_EOF
     * @param ext1 : magic num
     *               MAGIC_NUM_INVALID : should not do 3A update
     *               other: do 3A update
     * @param ext2 : MAGIC_NUM_INVALID: no buffer availabe (due to sensor delay)
     *               otherwise, buffer available, ext2 is the dequed magic #
     */
    PASS1_EOF            = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x8        ) , // 4

    /*
     * Notify the timing after configFrame of pass1
     * @param cmd  : PASS1_CONFIG_FRAME
     */
    PASS1_CONFIG_FRAME   = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x10       ) ,

    /* not used yet */
    PASS2_SOF            = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x20       ) ,

    /* not used yet */
    PASS2_EOF            = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x40       ) ,

    /*
     * Notify pass1 to stop.
     * @param cmd  : CONTROL_STOP_PASS1
     */
    CONTROL_STOP_PASS1   = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x80       ) ,

    /*
     * Notify pass1 to stop.
     * @param cmd  : CONTROL_STOP_PASS1
     */
    CONTROL_SHUTTER      = NOTIFYTYPE(NODECAT_BASIC_NOTIFY , 0x100       ) ,

    /********************** feature pipe notify **********************/

    FEATURE_MASK_CHANGE  = NOTIFYTYPE(NODECAT_FEATUREPIPE_NOTIFY , 0x1   ) ,

    /********************** feature pipe notify **********************/

    /*
     * Notify the timing that each node in the graph has stopped.
     * This will be received after onStop.
     */
   LATE_STOP            =  NOTIFYTYPE(NODECAT_GLOBAL_CMD , 0x1   ) ,

} NodeNotifyTypes;

}; // namespace NSCamNode
#endif  //_MTK_CAMERA_INCLUDE_CAMNODE_NODEDATATYPES_H_

