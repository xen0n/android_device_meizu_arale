#ifndef I_EXT_IMG_PROC_H
#define I_EXT_IMG_PROC_H
//-----------------------------------------------------------------------------
#include <IImageBuffer.h>
using namespace NSCam;
//-----------------------------------------------------------------------------
class IExtImgProc
{
    public:
        #define PLANE_AMOUNT_MAX    3

        struct ImgInfo
        {
            MUINT32         bufType;
            const char*     format;
            MUINT32         width;
            MUINT32         height;
            MUINT32         stride[PLANE_AMOUNT_MAX];
            MUINTPTR        virtAddr;
            MUINT32         bufSize;
            //
            ImgInfo()
                : bufType(0)
                , format(NULL)
                , width(0)
                , height(0)
                , virtAddr(0)
                , bufSize(0)
            {
                for(MUINT32 i=0; i<PLANE_AMOUNT_MAX; i++)
                {
                    stride[i] = 0;
                }
            }
        };
        //
        virtual ~IExtImgProc() {}
    public:
        virtual MBOOL       init(void) = 0;
        virtual MBOOL       uninit(void) = 0;
        virtual MUINT32     getImgMask(void) = 0;
        virtual MBOOL       doImgProc(ImgInfo& img) { return MTRUE; };
        virtual MBOOL       doImgProc(
                                MUINT32             bufType,
                                IImageBuffer* const pImgBuf) { return MTRUE; };
};
//-----------------------------------------------------------------------------
#endif

