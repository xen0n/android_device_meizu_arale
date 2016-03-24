#ifndef _LOMO_HAL_JNI_H_
#define _LOMO_HAL_JNI_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

//#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
//SL#include "DpIspStream.h"

// MTK_CONTROL_EFFECT_MODE
typedef mtk_camera_metadata_enum_android_control_effect_mode_t LOMO_HAL_JNI_EFFECT_IDX;
//    #include "DpBlitStream.h"

//#define PLANE_NUM 3
/**
*@enum eMDPMGR_OUTPORT_INDEX
*/
typedef enum eLOMOHALJNI_RETURN_TYPE
{
    LOMOHALJNI_PASS2_NOT_READY    =  1,
    LOMOHALJNI_NO_ERROR         =  0,
    LOMOHALJNI_API_FAIL         = -1,
    LOMOHALJNI_BLITSTREAM_FAIL = -2,
    LOMOHALJNI_NULL_OBJECT      = -3,
    LOMOHALJNI_WRONG_PARAM      = -4,
    LOMOHALJNI_STILL_USERS      = -5,
    LOMOHALJNI_DIRECTLINK_FAIL      = -6,
}LOMOHALJNI_RETURN_TYPE;



/**
*@
*/


/**
 *@class LomoHalJni
 *@brief Lomo hal of JNI I/F for matrix menu layout
*/
class LomoHalJni
{
    public :

        /**
              *@brief LomoHalJni constructor
             */
        LomoHalJni () {};


        /**
              *@brief Create LomoHalJni Object
             */
        static LomoHalJni *createInstance( void );

        /**
               *@brief Destory LomoHalJni Object
             */
        virtual MVOID destroyInstance( void );

        /**
               *@brief Initialize function
               *@note Must call this function after createInstance and before other functions
               *
               *@return
               *-MTURE indicates success, otherwise indicates fail
             */
        virtual MINT32 init () = 0;

        /**
               *@brief Uninitialize function
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 uninit() = 0;

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
                                                                     MUINT32 ppSrcImgHeight ) = 0;

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
                                                                       ) = 0;



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
        virtual MINT32 UploadLomoSrcImage(MUINT8* pvCBVA) = 0;



       /**
               *@brief free Pass2 source image from YV12 to YUY2
               *
               *@param[in] ppSrcImgMVA : post process src MVA addr
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 FreeLomoSrcImage(void) = 0;


        /**
               *@brief post process effect with effectIdx and dst image VA  Ahead(YUY2 to YV12)
               *
               *@param[in] ppEffectIdx : post process effectIdx
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 LomoImageEnque(MUINT8** ppDstImgVA, MINT32 ppEffectIdx)  = 0;

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
        virtual MINT32 LomoImageDeque(MUINT8** ppDstImgMVA, MINT32 ppEffectIdx)  = 0;

        /**
               *@brief free dst image MVA (YUY2 to YV12)
               *
               *@param[in] ppDstImgMVA : post process dst image virtual addr
               *
               *@return
               *-MTRUE indicates success, otherwise indicates fail
             */
        virtual MINT32 FreeLomoDstImage(void) = 0;

    protected:
        /**
              *@brief LomoHalJni destructor
             */
        virtual ~LomoHalJni() {};
};

#include <sys/time.h>
#include <cutils/log.h>
class LomoTimer {
public:
    inline MINT32 getUsTime()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    LomoTimer(const char* info, MINT32 sensorDevId, MBOOL enable)
        : mInfo(info), mIdx(sensorDevId), mEnable(enable)
    {
        if (mEnable) mStartTime = getUsTime();
    }

    MVOID start(const char* info, MINT32 sensorDevId, MBOOL enable) //used by global/static variables
    {
        mInfo = info;
        mIdx = sensorDevId;
        mEnable = enable;
        if (mEnable) mStartTime = getUsTime();
    }

    MVOID printTime()
    {
        if (!mEnable) return;
        MINT32 endTime = getUsTime();
        ALOGD("[lomo: %s,(ms) Id: %d] : %f\n", mInfo, mIdx, ((double)(endTime - mStartTime)) / 1000);
    }
    MVOID fps(MINT32 count)
    {
        if (!mEnable) return;
        MINT32 endTime = getUsTime();
        double lddiff = double(endTime - mStartTime);
        if(lddiff==0)
        {
            lddiff = 0.1;
            ALOGD("[lomo: %s,count-%d (fps) Id: %d] too fast! : %f\n", mInfo, count , mIdx, (double)1000/lddiff*1000);
        }
        else
        {
            ALOGD("[lomo: %s,count-%d (fps) Id: %d] non-zero: %f\n", mInfo, count , mIdx, (double)1000/lddiff*1000);
        }
    }
    ~LomoTimer()
    {
    }

protected:
    const char* mInfo;
    MINT32 mStartTime;
    MINT32 mIdx;
    MBOOL mEnable;
};


#endif

