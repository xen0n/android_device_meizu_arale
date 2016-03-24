//! \file  lomo_hal_jni.h

#ifndef _LOMO_HAL_JNI_IMP_H_
#define _LOMO_HAL_JNI_IMP_H_

#include "lomo_hal_jni.h"

//------------Thread-------------
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------

#define LOMO_HAL_JNI_MAX_PLANE 3

typedef enum LOMO_STATE
{
    LOMO_STATE_NONE   = 0x0000,
    LOMO_STATE_ALIVE  = 0x0001,
    LOMO_STATE_UNINIT = 0x0002
}LOMO_STATE_ENUM;


/**
 *@class LomoHalJniImp
 *@brief Implementation of LomoHalJni
*/
class LomoHalJniImp : public LomoHalJni
{
    public :

        /**
              *@brief LomoHalJniImp constructor
             */
        LomoHalJniImp();

        /**
              *@brief MdpMgrImp destructor
             */
        virtual ~LomoHalJniImp();


        /**
               *@brief Initialize function
               *@note Must call this function after createInstance and before other functions
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 init ();

        /**
               *@brief Uninitialize function
               *@return
               *-MTRUE indicates success, otherwise indicates fail
            */
        virtual MINT32 uninit();



        /**
               *@brief Prepare Pass2 source image from YV12 to YUY2
               *
               *@param[in] pvCBVA : preview call back image virtual addr
               *@param[in] pvCBWidth : preview call back image Width
               *@param[in] pvCBHeight : preview call back image Width
               *@param[in] pvCBformat : post process src image format
               *@param[in] ppSrcImgWidth : post process src image Width
               *@param[in] ppSrcImgHeight : post process src image Width
               *@param[Out] ppSrcImgMVA : post process src MVA addr
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 AllocLomoSrcImage(MUINT32 pvCBWidth, \
                                                                     MUINT32 pvCBHeight, \
                                                                     MUINT32 pvCBformat, \
                                                                     MUINT32 ppSrcImgWidth, \
                                                                     MUINT32 ppSrcImgHeight \
                                                                       ) ;


        /**
               *@brief Prepare Pass2 dst image from YUY2 to YV12
               *
               *@param[in] Number : Number of buffers
               *@param[in] ppDstImgWidth : post process dst image Width
               *@param[in] ppDstImgHeight : post process dst image Width
               *@param[Out] ppDstImgVA : post process dst VA addr
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 AllocLomoDstImage(MUINT32 Number, \
                                                                     MUINT8** ppDstImgVA \
                                                                       ) ;
       /**
               *@brief Upload Pass2 source image from YV12 to YUY2
               *
               *@param[in] pvCBVA : preview call back image virtual addr
               *@param[in] pvCBWidth : preview call back image Width
               *@param[in] pvCBHeight : preview call back image Width
               *@param[in] pvCBformat : post process src image format
               *@param[in] ppSrcImgWidth : post process src image Width
               *@param[in] ppSrcImgHeight : post process src image Width
               *@param[Out] ppSrcImgMVA : post process src MVA addr
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 UploadLomoSrcImage(MUINT8* pvCBVA) ;



       /**
               *@brief free Pass2 source image from YV12 to YUY2
               *
               *@param[in] ppSrcImgMVA : post process src MVA addr
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 FreeLomoSrcImage(void);

        /**
               *@brief post process effect with effectIdx and dst image VA (YUY2 to YV12)
               *
               *@param[in] ppDstImgMVA : post process dst image virtual addr
               *@param[in] ppDstImgWidth : post process dst image Width
               *@param[in] ppDstImgHeight : post process dst image Width
               *@param[in] ppDstImgformat : post process dst image format
               *@param[in] ppEffectIdx : post process effectIdx
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 LomoImageEnque( MUINT8** ppDstImgVA, MINT32 ppEffectIdx)  ;
        /**
               *@brief post process effect with effectIdx and dst image VA (YUY2 to YV12)
               *
               *@param[in] ppDstImgMVA : post process dst image virtual addr
               *@param[in] ppDstImgWidth : post process dst image Width
               *@param[in] ppDstImgHeight : post process dst image Width
               *@param[in] ppDstImgformat : post process dst image format
               *@param[in] ppEffectIdx : post process effectIdx
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 LomoImageDeque(MUINT8** ppDstImgMVA, MINT32 ppEffectIdx)  ;

        /**
               *@brief free dst image MVA (YUY2 to YV12)
               *
               *@param[in] ppDstImgMVA : post process dst image virtual addr
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 FreeLomoDstImage(void) ;

    private :


        /**************************************************************************************/



        MUINT32 gu32pvCBWidth;
        MUINT32 gu32pvCBHeight;
        MUINT32 gu32pvCBformat;
        MUINT8* gu32pvCBVA[LOMO_HAL_JNI_MAX_PLANE];
        MUINT8* gu32pvCBMVA[LOMO_HAL_JNI_MAX_PLANE];

        MUINT32 gu32ppSrcImgWidth;
        MUINT32 gu32ppSrcImgHeight;
        MUINT8* gu32ppSrcImgVA[LOMO_HAL_JNI_MAX_PLANE];
        MUINT8* gu32ppSrcImgMVA[LOMO_HAL_JNI_MAX_PLANE];

        MUINT32 gu32ppDstImgWidth;
        MUINT32 gu32ppDstImgHeight;
        MUINT32 gu32ppDstImgformat;
        MUINT8* gu32ppDstImgVA[LOMO_HAL_JNI_MAX_PLANE];
        MUINT8* gu32ppDstImgMVA[LOMO_HAL_JNI_MAX_PLANE];

        // state
        LOMO_STATE_ENUM mState;

        // thread
        pthread_t mLomoDequeThread;
        sem_t mDequeSem;
        sem_t mDequeDoneSem;
        sem_t mDequeSemEnd;


       MUINT32 ColorEffectSetting(MUINT32 caseNum)  ;
        /**
        *@brief Deque thread loop
        */

        static MVOID *LomoDequeThreadLoop(MVOID *arg);
        /**
        *@brief Set state
        *@param[in] aState : state
        */
        MVOID SetLomoState(const LOMO_STATE_ENUM &aState);

        /**
        *@brief Get state
        *@return
        *-state
        */
        LOMO_STATE_ENUM GetLomoState();

        /**
        *@brief Change thread setting
        */
        MVOID ChangeThreadSetting();


        /**
        *@brief Deque one image
        */
        MVOID LomoDequeTrigger();
        /**
        *@brief Deque one image done
        */
        LOMOHALJNI_RETURN_TYPE LomoDequeBuffer();

};

#endif

