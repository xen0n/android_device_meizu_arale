/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

/**
 * @file pd_algo_if.h
 * @brief PDAF algorithm interface, for raw sensor.
 */
#ifndef _PD_ALGO_IF_H_
#define _PD_ALGO_IF_H_

namespace NS3A
{

/**
 * @brief PD algorithm interface class
 */
class IPdAlgo {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    IPdAlgo() {}
    virtual ~IPdAlgo() {}

private:
    IPdAlgo(const IPdAlgo&);
    IPdAlgo& operator=(const IPdAlgo&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief create instance
     * @param [in] i4SensorDev Sensor device
     */
    static  IPdAlgo* createInstance(MINT32 const i4SensorDev);

    /**
     * @brief destroy instance
     */
    virtual MVOID   destroyInstance() = 0;

    /**
     * @brief Initial PD algorithm.
     * @param [in] a_sPDInput Input PD algorithm settings from pdaf manager; Please refer PD_INIT_T in pd_param.h
     */
    virtual MRESULT initPD(PD_INIT_T &a_sPDInitParam) = 0;

    /**
     * @brief Extract PD pixel tasks.
     * @param [in] a_sPDInput Input PD extract info from pdaf manager; Please refer PD_EXTRACT_INPUT_T in pd_param.h
     * @param [in] a_sPDOutput Onput PD extracted data to pdaf manager; Please refer PD_EXTRACT_DATA_T in pd_param.h
     */
    virtual MRESULT extractPD(PD_EXTRACT_INPUT_T &a_sPDInput, PD_EXTRACT_DATA_T &a_sPDOutput) = 0;

    /**
     * @brief Handle PD algorithm tasks.
     * @param [in] a_sPDInput Input PD algorithm settings from pdaf manager; Please refer PD_INPUT_T in pd_param.h
     * @param [in] a_sPDOutput Onput PD algorithm settings to pdaf manager; Please refer PD_OUTPUT_T in pd_param.h
     */
    virtual MRESULT handlePD(PD_INPUT_T &a_sPDInput, PD_OUTPUT_T &a_sPDOutput) = 0;

    /**
     * @brief Set PD parameters to PD algorithm.
     * @param [in] a_sPDNvram Input PD algorithm settings from pdaf manager; Please refer PD_NVRAM_T in camera_custom_nvram.h.
     */
    virtual MRESULT updatePDParam(PD_NVRAM_T &a_sPDNvram) = 0;

     /**
     * @brief Set PD block info to PD algorithm.
     * @param [in] a_sPDConfig Input PD algorithm settings from pdaf manager; Please refer PD_CONFIG_T in pd_param.h
     */
    virtual MRESULT setPDBlockInfo(PD_CONFIG_T &a_sPDConfig) = 0;


    /**
     * @brief Send debug information to PDAF manager. For internal debug information.
     * @param [in] a_sPDDebugInfo debug information data pointer.;Please refer PD_DEBUG_INFO_T in dbg_pd_param.h
     */
    virtual MRESULT getDebugInfo(AF_DEBUG_INFO_T &a_sPDDebugInfo) = 0;

    /**
     * @brief Send hybrid AF related information to PDAF manager. For internal information.
     * @param [in] a_arrInfo int array for hybridAF
     */
    virtual MRESULT getInfoForHybridAF(int a_arrInfo[10]) = 0;

};

}; // namespace NS3A

#endif


