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
#ifndef _HWSYNC_DRV_H_
#define _HWSYNC_DRV_H_

/******************************************************************************
 *
 * @enum HW_SYNC_CMD_ENUM
 * @brief cmd enum for sendCommand.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_CMD_SET_MODE         = 0x01,         //sensor mode(by resolution and fps)
}HW_SYNC_CMD_ENUM;

/******************************************************************************
 *
 * @enum HW_SYNC_CMD_ENUM
 * @brief the current mode which matches  sensor mode.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_MODE_NONE         = 0x00,
}HW_SYNC_MODE_ENUM;


/******************************************************************************
 *
 * @class HWSyncDrv BaseClass
 * @brief Driver to sync VDs.
 * @details
 *
 ******************************************************************************/
class HWSyncDrv
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    HWSyncDrv(){};
    virtual ~HWSyncDrv() {};
//
public:
    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static HWSyncDrv*      createInstance();
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    virtual void        destroyInstance(void) = 0;
    virtual MBOOL       init(MUINT32 sensorIdx) = 0;
    virtual MBOOL       uninit(MUINT32 sensorIdx) = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MINT32         sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 senDev,MUINT32 senScen, MUINT32 currfps)=0;
};
//----------------------------------------------------------------------------
#endif  // _HWSYNC_DRV_H_


