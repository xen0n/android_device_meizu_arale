#ifndef OEM_SUPPORT_H
#define OEM_SUPPORT_H

#include <telephony/ril.h>
#include <telephony/oem-ril.h>
#include <telephony/via-ril.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * All the definitions of oem support domain, can support 32 domains at most.
 */
#define OEM_SUPPORT_EN_MASK             0x8000  /* oem support enable    */
#define OEM_VCALL_IN_SUPPORT_MASK       0x8001  /* voice call in enable  */
#define OEM_VCALL_OUT_SUPPORT_MASK      0x8002  /* voice call out enable */
#define OEM_SMS_IN_SUPPORT_MASK         0x8004  /* SMS in enable         */
#define OEM_SMS_OUT_SUPPORT_MASK        0x8008  /* SMS out enable        */
#define OEM_DATA_CALL_SUPPORT_MASK      0x8010  /* data call enable      */
#define OEM_APN_SUPPORT_MASK            0x8020  /* apn support for data  */
#define OEM_OPT_NAME_SUPPORT_MASK       0x8040  /* operator name */
#define OEM_CARD_STATUS_SUPPORT_MASK    0x8080      /*support oem card status */

#define OEM_SUPPORT_FLAG_PROP_NAME      "ro.oem.customer.support"
#define OEM_RIL_VERSION_PROP_NAME       "ro.oem.ril.version"

#define DEFAULT_OEM_SUPPORT_FLAG        0x0000
#define DEFAULT_OEM_RIL_VERSION         RIL_VERSION

extern int g_oem_support_flag;

int get_oem_ril_version(void);
int  get_oem_support_flag();
int  oem_support_bit_verify(int oem_support_flag, int oem_support_mask);
int  dispatch_oem_rgistration_state(RIL_Rgistration_state *p_input, OEM_RIL_Rgistration_state *p_output);

//temp v6, need check
int  dispatch_oem_card_status(VIA_RIL_CardStatus * p_card_status, RIL_CardStatus_v6 ** pp_oem_card_status);

#define OEM_DEFAULT_RETRY_TIMES 10
#define OEM_MAX_RETRY_TIMES 15

#define PPP0_IFF_UP 0x1 /* interface is up */

/*
data connect state
should as same as the flow enum in viatelecom-withuim-ril.c
enum {
    DATA_STATE_DISCONNECTED,
    DATA_STATE_CONNECTED
}
*/

#define oemSupportEnable(a)         (((a) & OEM_SUPPORT_EN_MASK) == OEM_SUPPORT_EN_MASK)
#define oemVcallInSupportEnable(a)  (((a) & OEM_VCALL_IN_SUPPORT_MASK) == OEM_VCALL_IN_SUPPORT_MASK)
#define oemVcallOutSupportEnable(a) (((a) & OEM_VCALL_OUT_SUPPORT_MASK) == OEM_VCALL_OUT_SUPPORT_MASK)
#define oemSmsInSupportEnable(a)    (((a) & OEM_SMS_IN_SUPPORT_MASK) == OEM_SMS_IN_SUPPORT_MASK)
#define oemSmsOutSupportEnable(a)   (((a) & OEM_SMS_OUT_SUPPORT_MASK) == OEM_SMS_OUT_SUPPORT_MASK)
#define oemDataCallSupportEnable(a) (((a) & OEM_DATA_CALL_SUPPORT_MASK) == OEM_DATA_CALL_SUPPORT_MASK)
#define oemApnSupportEnable(a)      (((a) & OEM_APN_SUPPORT_MASK) == OEM_APN_SUPPORT_MASK)
#define oemOptNameSupportEnable(a)  (((a) & OEM_OPT_NAME_SUPPORT_MASK) == OEM_OPT_NAME_SUPPORT_MASK)
#define oemCardStatusSupportEnalbe(a)  (((a) & OEM_CARD_STATUS_SUPPORT_MASK) == OEM_CARD_STATUS_SUPPORT_MASK)

#ifdef __cplusplus
}
#endif

#endif
