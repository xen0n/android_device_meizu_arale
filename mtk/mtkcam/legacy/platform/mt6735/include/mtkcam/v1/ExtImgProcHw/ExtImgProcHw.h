#ifndef EXT_IMG_PROC_HW_H
#define EXT_IMG_PROC_HW_H
//-----------------------------------------------------------------------------
class ExtImgProcHw : public IExtImgProc
{
    public:
        typedef enum
        {
            BufType_ISP_P2_TwoRun_In = 0x00010000,
        }BufTypeEnum;
    //
    protected:
        virtual ~ExtImgProcHw() {};
    //
    public:
        static ExtImgProcHw*    createInstance(void);
        virtual void            destroyInstance(void) = 0;
        //
        #if 0
        virtual MBOOL       init(void) = 0;
        virtual MBOOL       uninit(void) = 0;
        virtual MUINT32     getImgMask(void) = 0;
        virtual MBOOL       doImgProc(
                                MUINT32         bufType,
                                IImageBuffer&   imgBuf) = 0;
        #endif
};
//-----------------------------------------------------------------------------
#endif


