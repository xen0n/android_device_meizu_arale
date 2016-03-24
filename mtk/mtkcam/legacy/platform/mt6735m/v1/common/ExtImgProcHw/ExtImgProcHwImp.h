#ifndef EXT_IMG_PROC_HW_IMP_H
#define EXT_IMG_PROC_HW_IMP_H
//-----------------------------------------------------------------------------
class ExtImgProcHwImp : public ExtImgProcHw
{
    protected:
        ExtImgProcHwImp();
        virtual ~ExtImgProcHwImp();
    //
    public:
        static ExtImgProcHw*    getInstance(void);
        virtual void            destroyInstance(void);
        //
        virtual MBOOL       init(void);
        virtual MBOOL       uninit(void);
        virtual MUINT32     getImgMask(void);
        virtual MBOOL       doImgProc(ImgInfo& img);
   //
   private:
        mutable Mutex   mLock;
        volatile MINT32 mUser;
        MUINT32         mImgMask;
};
//-----------------------------------------------------------------------------
#endif

