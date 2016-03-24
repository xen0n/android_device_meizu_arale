#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <string.h>
#include <ctype.h>

//#include "MfllCommon.h"

#include <mtkcam/algorithm/libmfbll/MTKMfbll.h>

typedef struct
{
    MUINT32 input_width;
    MUINT32 input_height;
    //
    MUINT32 iso_value;
    MUINT32 iso_high_th;
    MUINT32 iso_low_th;
    //
    MUINT32 max_frame_number;
    MUINT32 bss_clip_th;            //@todo
    MUINT32 memc_bad_mv_range;
    MUINT32 memc_bad_mv_rate_th;    //typo of memc_bad_mb_rate_th
} MFBLL_PIPE_INIT_INFO, *P_MFBLL_PIPE_INIT_INFO;


class MfbllHal
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static MfbllHal* getInstance(char const *userName);
    virtual void destroyInstance(char const *userName);

public: //@todo private:
    MfbllHal();
    virtual ~MfbllHal();
    virtual MBOOL init(void *pInitInData);
    virtual MBOOL uninit();

public:
    virtual MBOOL queryWorkingBufferSize(MUINT32 &workingBufferSize);
    virtual MBOOL queryMemcBufferSize(MUINT32 &imageBufferSize);
    virtual MBOOL querySwMixerBufferSize(MUINT32 &imageBufferSize);
    virtual MBOOL setWorkingBuffer(MUINT8 *workingBuffer);
    //virtual MUINT32 getFrameNumber(MUINT32 iso);

public:
    virtual MBOOL doMemc(MINT32 mvX, MINT32 mvY, MUINT8* baseBufAddr, MUINT8* refBufAddr, MUINT32 *badMvCount);
#if TEST_SWMIXER
    virtual MBOOL doSwMixer(MUINT8 *inputBuffer1, MUINT8 *inputBuffer2, MUINT8 *weightingBuffer);
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  private data
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    // Mfbll Parameters (MEMC + Mixer)
    MFBLL_INIT_PARAM_STRUCT MfbllInitPara;

    // Memc Paramters declaration
    MUINT32 inMEWidth, inMEHeight, MemcImgsize, MemcImgBufsize; //, MemcMvBufsize;
    MINT32 WorkBufSize;
    MFBLL_GET_PROC_INFO_STRUCT Proc1_get_proc_info;
    MFBLL_SET_PROC1_INFO_STRUCT Proc1_set_proc_info;
    MFBLL_PROC1_OUT_STRUCT MfbllMemcResult;

    // Mixer Parameters declaration
    MUINT32 imgW;
    MUINT32 imgH;
    MUINT32 ImgSize; // 32byte alignment
#if TEST_SWMIXER
    MFBLL_SET_PROC2_INFO_STRUCT Proc2_set_proc_info;
    MFBLL_PROC2_OUT_STRUCT Mfbll_result;
#endif

    // working buffer for MEMC & SwMixer
    MUINT8 *WorkingBuffer;

    //
    MTKMfbll* m_pMfbllDrv;

    //
    MFBLL_PIPE_INIT_INFO mTuning;

    // debug
    MUINT32 mDump;
};
