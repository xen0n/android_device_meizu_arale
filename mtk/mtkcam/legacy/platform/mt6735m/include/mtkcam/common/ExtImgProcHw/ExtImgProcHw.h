#ifndef EXT_IMG_PROC_HW_H
#define EXT_IMG_PROC_HW_H
//-----------------------------------------------------------------------------
class ExtImgProcHw : public IExtImgProc
{
    public:
        typedef enum
        {
            BufType_ISP_VSS_P1_Out          = 0x00010000,
            BufType_ISP_VSS_P2_TwoRun_In    = 0x00020000,
            BufType_ISP_ZSD_P1_Out          = 0x00040000
        }BufTypeEnum;
    //
    protected:
        virtual ~ExtImgProcHw() {};
    //
    public:
        static ExtImgProcHw*    createInstance(void);
        virtual void            destroyInstance(void) = 0;
        //
        virtual MBOOL       init(void) = 0;
        virtual MBOOL       uninit(void) = 0;
        virtual MUINT32     getImgMask(void) = 0;
        virtual MBOOL       doImgProc(ImgInfo& img) = 0;
};
//-----------------------------------------------------------------------------
#endif


