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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <linux/ioctl.h>
#include <string.h>


#include <cutils/log.h>

#ifdef MTK_RRC_ENABLE
  #define MTK_RRC_INC_DRV
#endif

#ifdef MTK_RRC_INC_DRV
  #include <linux/rrc_drv.h>
#endif

#include <utils/Errors.h>

#include <refresh_rate_control.h>

#include    <utils/Log.h>

#include <cutils/properties.h>



#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RefreshRateControl"



#define RRC_DRV_NAME           "/dev/mtk_rrc"

static int currentID;

static unsigned int allocated;


RefreshRateControl::RefreshRateControl()
{
}


RefreshRateControl::~RefreshRateControl()
{
}


int RefreshRateControl::setScenario(int scenario, bool enable){

#ifdef MTK_RRC_INC_DRV

    int drvID ;

    RRC_DRV_DATA rrc_data ;
    unsigned int result = 0;
    int config_enable = enable ;

    if(!(scenario > RRC_TYPE_NONE && scenario < RRC_TYPE_MAX_SIZE)){
        ALOGW("[RRC] setScenario RRC Driver scenario error range (%d)", scenario);
        return -1;
    }



#ifdef MTK_RRC_ENABLE_PROP

    {
        char value[PROPERTY_VALUE_MAX];
        unsigned long u4PQOpt;

        property_get("persist.rrc", value, "0");
        u4PQOpt = atol(value);
        if(0 != u4PQOpt)
        {

            if(u4PQOpt == 1){
                /* skip all event */
                //ALOGW("[RRC] skip all event!!\n");
                return 0;
            } else if( u4PQOpt == 2 ) {
                /* skip video event */
                if((scenario >= RRC_TYPE_VIDEO_NORMAL && scenario <= RRC_TYPE_VIDEO_WIFI_DISPLAY)){
                    return 0;
                }
            } else if (u4PQOpt == 3) {
                /* always keep HIGH */
                if((scenario >= RRC_TYPE_VIDEO_NORMAL && scenario <= RRC_TYPE_VIDEO_WIFI_DISPLAY)){
                    return 0;
                }
                /* skip touch leave */
                if( scenario == RRC_DRV_TYPE_TOUCH_EVENT && enable == 0){
                    return 0;
                }
            } else if (u4PQOpt == 4) {
                /* always keep LOW */
                if((scenario >= RRC_TYPE_VIDEO_NORMAL && scenario <= RRC_TYPE_VIDEO_WIFI_DISPLAY)){
                    return 0;
                }
                /* skip touch down */
                if( scenario == RRC_DRV_TYPE_TOUCH_EVENT && enable == 1){
                    return 0;
                }
            }
        }
    }
#endif


    ALOGW("[RRC] setScenario %d, config_enable %d!!\n", scenario, config_enable);


    if(scenario == RRC_TYPE_VIDEO_120HZ)
    {
        scenario = RRC_DRV_TYPE_VIDEO_PLAYBACK ;
        if ( config_enable ){
            config_enable = 2 ;
        }
        ALOGW("[RRC] Video120Hz setScenario %d, config_enable %d!!\n", scenario, config_enable);
    }



    drvID = open(RRC_DRV_NAME, O_RDONLY, 0);

    if( drvID == -1 )
    {
        ALOGW("Open RRC Driver Error (%s)", strerror(errno));
        return -1;
    }


    if(ioctl(drvID, RRC_IOCTL_CMD_INIT)<0)
    {
        ALOGW("RefreshRateControl Driver->RRC_IOCTL_CMD_INIT Error (%s)", strerror(errno));
        close(drvID);
        drvID = -1;
        return -1;
    }


    rrc_data.scenario = scenario ;
    rrc_data.enable = config_enable ;

    if(ioctl(drvID, RRC_IOCTL_CMD_SET_SCENARIO_TYPE, &rrc_data) < 0)
    {
        ALOGW("RefreshRateControl Driver->RRC_IOCTL_CMD_SET_SCENARIO_TYPE Error (%s)", strerror(errno));
        close(drvID);
        return -1;
    }

    if(ioctl(drvID, RRC_IOCTL_CMD_DEINIT) < 0)
    {
        return -1;
    }

    close(drvID);

#endif

    return 0;
}















