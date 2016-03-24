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

#define LOG_TAG "Drv/IMem"
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//
#include "imem_drv_imp.h"
//
#include "camera_isp.h"
#include <utils/Log.h>
//
#define ISP_DRV_DEV_NAME  "/dev/camera-isp"
//
#define _ION_DEVICE_NAME_ "/dev/ion"
//
//#define IMEM_DRV_DELAY usleep(50000);
#define IMEM_DRV_DELAY

#define LITERAL_TO_STRING_INTERNAL(x)    #x
#define LITERAL_TO_STRING(x) LITERAL_TO_STRING_INTERNAL(x)

#define CHECK(condition) \
    LOG_ALWAYS_FATAL_IF( \
            !(condition), \
            "%s", \
            __FILE__ ":" LITERAL_TO_STRING(__LINE__) \
            " CHECK(" #condition ") failed.")

//-----------------------------------------------------------------------------
IMemDrv* IMemDrv::createInstance()
{
    DBG_LOG_CONFIG(drv, imem_drv);
    return IMemDrvImp::getInstance();
}
//-----------------------------------------------------------------------------
IMemDrvImp::IMemDrvImp()
    :mInitCount(0),
     mIspFd(-1),
#if defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
     mpM4UDrv(NULL),
#endif
     mLocal_InitCount(0)
{
    IMEM_DBG("getpid[0x%08x],gettid[0x%08x] ", getpid() ,gettid());
}
//-----------------------------------------------------------------------------
IMemDrvImp::~IMemDrvImp()
{
    IMEM_DBG("");
}
//-----------------------------------------------------------------------------
IMemDrv* IMemDrvImp::getInstance(void)
{
    static IMemDrvImp singleton;
    IMEM_DBG("singleton[0x%08x],getpid[0x%08x],gettid[0x%08x] ",&singleton,getpid() ,gettid());
    IMEM_DRV_DELAY
    return &singleton;
}
//-----------------------------------------------------------------------------
void IMemDrvImp::destroyInstance(void)
{
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
///Flow concept: we do increase global and local count first, then judge we have to initial m4uDrv/ion_dev and m4uPort according
///              local count and global count repectively
MBOOL IMemDrvImp::init(void)
{
    vidopAddr=0;
    MBOOL Result = MTRUE;
    MINT32 ret = 0;
    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    IMEM_DBG("mCount(%d/%d),fd(%d)", mInitCount,mLocal_InitCount,mIspFd);
    //
    Mutex::Autolock lock(mLock);
    //////////////////////////////////////////
    //init. buf_map
    //erase all
    buf_map.clear();
    //

#if defined (__ISP_USE_PMEM__)
    //
#elif defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
    //////////////////////////////////////////////////////
    // we initial m4udrv and open ion device when local count is 0,
    // and config m4v ports when global count is 1
    if(mLocal_InitCount==0)
    {
       mpM4UDrv = new MTKM4UDrv();
       #if defined (__ISP_USE_ION__)
          mIonDrv = mt_ion_open("imem_drv.cpp");
          if (mIonDrv < 0)
          {
             IMEM_ERR("ion device open FAIL ");
             return MFALSE;
          }
            IMEM_INF("open ion id(%d).\n", mIonDrv);
       #endif
    }//match if local count
#endif
    //
#if defined(_use_kernel_ref_cnt_)
    //
    if ( mIspFd < 0 ) {
        mIspFd = open(ISP_DRV_DEV_NAME, O_RDONLY);
        if (mIspFd < 0)    // 1st time open failed.
        {
            IMEM_ERR("ISP kernel open fail, errno(%d):%s.", errno, strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
    }
    //
    IMEM_DBG("use kernel ref. cnt.mIspFd(%d)",mIspFd);
    //
    ///////////////////////////////////////////////
    //increase global and local count first
    ref_cnt.ctrl = ISP_REF_CNT_INC;
    ref_cnt.id = ISP_REF_CNT_ID_IMEM;
    ref_cnt.data_ptr = (MINT32*)&mInitCount;

    ret = ioctl(mIspFd,ISP_REF_CNT_CTRL,&ref_cnt);
    if(ret < 0)
    {
        IMEM_ERR("ISP_REF_CNT_INC fail(%d)[errno(%d):%s],fd(%d) \n",ret, errno, strerror(errno),mIspFd);
        Result = MFALSE;
        *((int *)0) = 0xdead0000;
        goto EXIT;
    }
    android_atomic_inc(&mLocal_InitCount);
#else
    IMEM_DRV_DELAY
    android_atomic_inc(&mInitCount);
    //IMEM_DBG("#flag3# mInitCount(%d),mInitCount>0 and run w\o _use_kernel_ref_cnt_\n",mInitCount);
#endif

#if defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
    if(mLocal_InitCount==1 && mInitCount==1)
    {
        M4U_PORT_STRUCT port;
        port.Virtuality = 1;
        port.Security = 0;
        port.domain = 3;
        port.Distance = 1;
        port.Direction = 0; //M4U_DMA_READ_WRITE
        //
        port.ePortID = M4U_PORT_IMGO;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_RRZO;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_AAO;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        //port.ePortID = M4U_PORT_LCSO;
        //ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_ESFKO;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_IMGO_S;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_LSCI;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_LSCI_D;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_BPCI;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_BPCI_D;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_UFDI;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_IMGI;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_IMG2O;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_IMG3O;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_VIPI;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_VIP2I;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_VIP3I;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_LCEI;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_RB;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_RP;
        ret = mpM4UDrv->m4u_config_port(&port);
        //
        port.ePortID = M4U_PORT_WR;
        ret = mpM4UDrv->m4u_config_port(&port);
    }//match if local & global count
#endif
    //
    EXIT:
    if(!Result)
    {
    }
    return Result;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
///Flow concept: we do decrease global and local count first, then judge we have to uninit m4uDrv/ion_dev and m4uPort according
///              local count and global count repectively
MBOOL IMemDrvImp::uninit(void)
{
    MBOOL Result = MTRUE;
    MINT32 ret = 0;
    ISP_REF_CNT_CTRL_STRUCT ref_cnt;
    //
    IMEM_INF("mCount(%d/%d),fd(%d)", mInitCount,mLocal_InitCount,mIspFd);
    //
    Mutex::Autolock lock(mLock);
    //
#if defined(_use_kernel_ref_cnt_)
    if(mIspFd < 0)
    {
        IMEM_ERR("mIspFd < 0 \n");
        goto EXIT;
    }
    ///////////////////////////////////////////////
    //decrease global and local count first
    // More than one user
    ref_cnt.ctrl = ISP_REF_CNT_DEC;
    ref_cnt.id = ISP_REF_CNT_ID_IMEM;
    ref_cnt.data_ptr = (MINT32*)&mInitCount;
    ret = ioctl(mIspFd,ISP_REF_CNT_CTRL,&ref_cnt);
    if(ret < 0)
    {
        IMEM_ERR("ISP_REF_CNT_DEC fail(%d)[errno(%d):%s] \n",ret, errno, strerror(errno));
        Result = MFALSE;
    }
    android_atomic_dec(&mLocal_InitCount);
    //
    //local ==0, global !=0 del m4u object only
    if ( mLocal_InitCount <= 0 ) {
        if ( mIspFd >= 0 ) {
            close(mIspFd);
            mIspFd = -1;
            IMEM_DBG("mIspFd(%d)",mIspFd);
        }
    }
    if(Result == MFALSE)
        goto EXIT;
#else
    ///IMEM_DBG("mInitCount(%d)",mInitCount);
    // More than one user
    android_atomic_dec(&mInitCount);
#endif

#if defined (__ISP_USE_PMEM__)
    //
#elif defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
    //////////////////////////////////////////////////////
    // we delete m4udrv and close ion device when local count is 1,
    // and unconfig m4v ports when global count is 1
    if ( mLocal_InitCount <= 0 )
    {
       #if defined (__ISP_USE_ION__)
          // we have to handle local ion drv here
          // if process A open ionID, then process B open ionID before process A call ImemDrv_uninit,
          // process A would not do close ionID.
          if (mIonDrv)
          {
            IMEM_INF("close ion id(%d).\n", mIonDrv);
            ion_close(mIonDrv);
          }
       #endif

#if 0 //QQ no need
       if(mInitCount<=0)
       {
           M4U_PORT_STRUCT port;
           port.Virtuality = 0;
           port.Security = 0;
           port.domain = 3;
           port.Distance = 1;
           port.Direction = 0; //M4U_DMA_READ_WRITE
           //
          port.ePortID = M4U_PORT_IMGO;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_RRZO;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_AAO;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          //port.ePortID = M4U_PORT_LCSO;
          //ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_ESFKO;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_IMGO_S;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_LSCI;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_LSCI_D;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_BPCI;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_BPCI_D;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_UFDI;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_IMGI;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_IMG2O;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_IMG3O;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_VIPI;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_VIP2I;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_VIP3I;
          ret = mpM4UDrv->m4u_config_port(&port);
          //
          port.ePortID = M4U_PORT_LCEI;
          ret = mpM4UDrv->m4u_config_port(&port);
       }
#endif
       delete mpM4UDrv;
       mpM4UDrv = NULL;
    }
#endif

    EXIT:

    return Result;
}
//-----------------------------------------------------------------------------
MBOOL IMemDrvImp::reset(void)
{
    IMEM_DBG("");
    //erase all
    buf_map.clear();
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MINT32 IMemDrvImp::allocVirtBuf(
    IMEM_BUF_INFO* pInfo)
{

    if(pInfo->size <= 0)
    {
        LOG_ERR("size 0!");
        return -1;
    }

    total_size += pInfo->size;
    IMEM_INF("Imem usage[0x%x]",total_size);

#if defined (__ISP_USE_PMEM__)
    IMEM_DBG("__ISP_USE_PMEM__");
    //
    pInfo->virtAddr= (MUINTPTR) ::pmem_alloc_sync(pInfo->size, &pInfo->memID);
    //
    IMEM_DBG("memID[0x%x]",pInfo->memID);
#elif defined (__ISP_USE_STD_M4U__)
    IMEM_DBG("__ISP_USE_STD_M4U__");
    //
    pInfo->memID = (MINT32)(IMEM_MIN_ION_FD-1);
    pInfo->virtAddr = ::memalign(L1_CACHE_BYTES, pInfo->size); //32Bytes align //QQ

#elif defined (__ISP_USE_ION__)
    IMEM_DBG("__ISP_USE_ION__");
    //
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;

    struct ion_fd_data fd_data;
    int ion_prot_flags = pInfo->useNoncache ? 0 : (ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC);
    //a. Allocate a buffer
    if(ion_alloc(
        mIonDrv,
        pInfo->size,
        0, //32 //alignment
        ION_HEAP_MULTIMEDIA_MASK,
        ion_prot_flags,
        &pIonHandle))
    {
        IMEM_ERR("ion_alloc fail, size(0x%x)",pInfo->size);
        return -1;
    }
    //b. Map a new fd for client.
    if(ion_share(
        mIonDrv,
        pIonHandle,
        &IonBufFd))
    {
        IMEM_ERR("ion_share fail");
        return -1;
    }
    pInfo->memID = (MINT32)IonBufFd; // Tianshu suggest to keep this fd
    //c. Map FD to a virtual space.
    pInfo->virtAddr = (MUINTPTR)ion_mmap(mIonDrv,NULL, pInfo->size, PROT_READ|PROT_WRITE, MAP_SHARED, IonBufFd, 0);
    if (!pInfo->virtAddr)
    {
        IMEM_ERR("Cannot map ion buffer. memID(0x%x)/size(0x%x)/va(0x%x)",pInfo->memID,pInfo->size,pInfo->virtAddr);
        return -1;
    }
    IMEM_INF("mID[0x%x]/size[0x%x]/VA[0x%x]",pInfo->memID,pInfo->size,pInfo->virtAddr);

    //
    IMEM_DBG("ionFd[0x%x]",pInfo->memID);
#endif

    return 0;
}

//-----------------------------------------------------------------------------
MINT32 IMemDrvImp::freeVirtBuf(
    IMEM_BUF_INFO* pInfo)
{
    IMEM_INF("mID[0x%x]/size[0x%x]/VA[0x%x]/PA[0x%x]",pInfo->memID,pInfo->size,pInfo->virtAddr,pInfo->phyAddr);
    total_size -= pInfo->size;
    IMEM_INF("Imem usage[0x%x]",total_size);

    IMEM_DRV_DELAY

#if defined (__ISP_USE_PMEM__)
    IMEM_DBG("pmem ID[0x%x]",pInfo->memID);
    //
    if( pInfo->virtAddr != 0 )
    {
        ::pmem_free(
            (MUINT8*)(pInfo->virtAddr),
            pInfo->size,
            pInfo->memID);
    }
#else
    if ( IMEM_MIN_ION_FD > pInfo->memID ) {
        if ( pInfo->virtAddr != 0 ) {
            ::free((MUINT8 *)pInfo->virtAddr);
        }

    }
    //
#if defined (__ISP_USE_ION__)
    //
    else{
        ion_user_handle_t pIonHandle;
        MINT32 IonBufFd;
        IMEM_DBG("ionFd[0x%x]",pInfo->memID);
        //a. get handle of ION_IOC_SHARE from fd_data.fd
        IonBufFd = pInfo->memID;
        if(ion_import(
            mIonDrv,
            IonBufFd,
            &pIonHandle))
        {
            IMEM_ERR("ion_import fail, memID(0x%x)",IonBufFd);
            return -1;
        }
        //free for IMPORT ref cnt
        if(ion_free(
            mIonDrv,
            pIonHandle))
        {
            IMEM_ERR("ion_free fail");
            return -1;
        }
        ion_munmap(mIonDrv,(char*)pInfo->virtAddr, pInfo->size);
        ion_share_close(mIonDrv,pInfo->memID);

        //d. pair of ION_IOC_ALLOC
        if(ion_free(
            mIonDrv,
            pIonHandle))
        {
            IMEM_ERR("ion_free fail");
            return -1;
        }
    }
#else
    else{
        IMEM_ERR("ERROR:mem ID(0x%x)",pInfo->memID);
        return -1;
    }
#endif

#endif

    return 0;
}
//-----------------------------------------------------------------------------
MINT32  IMemDrvImp::mapPhyAddr(
    IMEM_BUF_INFO* pInfo)
{
    stIMEM_MAP_INFO map_info;

    //IMEM_DBG("memID[0x%x]/size[0x%x]/vAddr[0x%x],S/C(%d/%d)",pInfo->memID,pInfo->size,pInfo->virtAddr,pInfo->bufSecu,pInfo->bufCohe);
    IMEM_DRV_DELAY

    IMEM_DBG(" mapPhyAddr");

    //check mapping
    if ( 0 ==  buf_map.count(pInfo->virtAddr) ) {
        IMEM_DBG(" NO Mapped");
        //
#if     defined(__ISP_USE_PMEM__)
            pInfo->phyAddr = (MUINTPTR)::pmem_get_phys(pInfo->memID);
#else
        if ( IMEM_MIN_ION_FD > pInfo->memID ) {
            //
            MUINT32 phyAddr = 0;
            this->allocM4UMemory(pInfo->virtAddr,pInfo->size,&phyAddr,pInfo->memID);
            pInfo->phyAddr = (MUINTPTR)phyAddr;
        }
#if defined(__ISP_USE_ION__)
        else /*if ( BUF_TYPE_ION == pInfo->type )*/  {
            //
            ion_user_handle_t pIonHandle;
            MINT32 IonBufFd;
            MINT32 err;
            MINT32 ret = 0;
            //struct ion_handle_data handle_data;
            struct ion_custom_data custom_data;
            struct ion_mm_data mm_data;
            struct ion_sys_data sys_data;
            //struct ion_fd_data fd_data;
            //
            //a. get handle of ION_IOC_SHARE from IonBufFd
            IonBufFd = pInfo->memID;
            if(ion_import(
                mIonDrv,
                IonBufFd,
                &pIonHandle))
            {
                IMEM_ERR("ion_import fail, memID(0x%x)",IonBufFd);
                return -1;
            }
            //
            mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
            mm_data.config_buffer_param.handle = pIonHandle;//allocation_data.handle;
            mm_data.config_buffer_param.eModuleID = M4U_PORT_IMGI;    // 1;
            mm_data.config_buffer_param.security = pInfo->bufSecu;
            mm_data.config_buffer_param.coherent = pInfo->bufCohe;

            err = ion_custom_ioctl(mIonDrv, ION_CMD_MULTIMEDIA, &mm_data);
            if(err == (-ION_ERROR_CONFIG_LOCKED))
            {
                IMEM_DBG("IOCTL[ION_IOC_CUSTOM] Double config after map phy address");
            }
            else if(err != 0)
            {
                IMEM_ERR("IOCTL[ION_IOC_CUSTOM] ION_CMD_MULTIMEDIA Config Buffer failed!");
                return -1;
            }
            //
            sys_data.sys_cmd = ION_SYS_GET_PHYS;
            sys_data.get_phys_param.handle = pIonHandle;  //allocation_data.handle;
            if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM, &sys_data))
            {
                IMEM_ERR("IOCTL[ION_IOC_CUSTOM] Config Buffer failed!");
                return -1;
            }
            //
            pInfo->phyAddr = (MUINTPTR)(sys_data.get_phys_param.phy_addr);

            //free for IMPORT ref cnt
            if(ion_free(
                mIonDrv,
                pIonHandle))
            {
                IMEM_ERR("ion_free fail");
                return -1;
            }
            //
            IMEM_DBG("Physical address=0x%08X len=0x%X", sys_data.get_phys_param.phy_addr, sys_data.get_phys_param.len);

        }
#else
        else {
            IMEM_ERR("ERROR:mem ID(%d)",pInfo->memID);
            return -1;
        }
#endif

#endif
        //
        map_info.pAddr = (MUINT32)(pInfo->phyAddr);
        map_info.size = pInfo->size;
        //ION mapping should be fast enough,so for ION mapping everytime.
        //buf_map.insert(pair<int, stIMEM_MAP_INFO>(pInfo->virtAddr, map_info));


        //buf_map.insert(map<int, int> :: value_type(virtAddr, phyAddr)) ;
        //buf_map[virtAddr] = phyAddr;
    }
    else {
        IMEM_DBG(" Already Mapped");
        map_info = buf_map[pInfo->virtAddr];
        pInfo->phyAddr = (MUINTPTR)(map_info.pAddr);
    }
    //
    IMEM_INF("mID(0x%x),size(0x%x),VA(x%x),PA(0x%x),S/C(%d/%d)",pInfo->memID,pInfo->size,pInfo->virtAddr,pInfo->phyAddr,pInfo->bufSecu,pInfo->bufCohe);

    return 0;
}
//-----------------------------------------------------------------------------
MINT32  IMemDrvImp::unmapPhyAddr(
    IMEM_BUF_INFO* pInfo)
{
    stIMEM_MAP_INFO map_info;

    //mapping
#if 0
    if ( 0 !=  buf_map.count(pInfo->virtAddr) ) {
        //
        map_info = buf_map[pInfo->virtAddr];
#else
    if ( 1 ) {
        //
        map_info.size = pInfo->size;
        map_info.pAddr = (MUINT32)(pInfo->phyAddr);
#endif
        //
        IMEM_DBG("[IMEM_BUFFER] - unmap<memID(0x%x),size(0x%x),virtAddr(0x%x),phyAddr(0x%x)>",pInfo->memID,pInfo->size,pInfo->virtAddr,pInfo->phyAddr);
        IMEM_DRV_DELAY
        //
#if     defined(__ISP_USE_PMEM__)
        //NOT support for PMEM
        IMEM_DBG("BUF_TYPE_PMEM: free by virtMem free");
#else
        //
        if ( IMEM_MIN_ION_FD > pInfo->memID ) {
            IMEM_DBG("BUF_TYPE_STD_M4U");
            IMEM_DRV_DELAY
            pInfo->phyAddr = (MUINTPTR)(map_info.pAddr);
            pInfo->size = map_info.size;
            this->freeM4UMemory(pInfo->virtAddr,map_info.pAddr,pInfo->size,pInfo->memID);
        }

        else
        {//using ion
        }
#endif
        //
        buf_map.erase(pInfo->virtAddr);

    }

    return 0;
}
//-----------------------------------------------------------------------------
MINT32  IMemDrvImp::doIonCacheFlush(MINT32 memID,eIONCacheFlushType flushtype)
{
    #if defined(__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
    ion_user_handle_t pIonHandle;
    MINT32 IonBufFd;
    MINT32 err;
    IMEM_DBG("+");
    //a. get handle of ION_IOC_SHARE from IonBufFd and increase handle ref count
    IonBufFd = memID;
    if(ion_import(mIonDrv,IonBufFd,&pIonHandle))
    {
        IMEM_ERR("ion_import fail,memID(0x%x)",IonBufFd);
        return -1;
    }

    //b. cache sync by range
    struct ion_sys_data sys_data;
    sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
    sys_data.cache_sync_param.handle=pIonHandle;
    sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_BY_RANGE;
    if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM,&sys_data))
    {
        IMEM_ERR("CAN NOT DO SYNC, memID/(0x%x)",memID);
        if(ion_free(mIonDrv,pIonHandle))
        {
            IMEM_ERR("ion_free fail");
            return -1;
        }
        return -1;
    }

    //c. decrease handle ref count
    if(ion_free(mIonDrv,pIonHandle))
    {
        IMEM_ERR("ion_free fail");
        return -1;
    }
    IMEM_DBG("-");
    #endif
    return 0;
}
//-----------------------------------------------------------------------------
MINT32 IMemDrvImp::cacheFlushAll(
    void)
{
    IMEM_ERR("DO NOT SUPPORT cache flush all, plz use cacheSyncbyRange");
    return 0;
}
//-----------------------------------------------------------------------------
MINT32 IMemDrvImp::cacheSyncbyRange(IMEM_CACHECTRL_ENUM ctrl,IMEM_BUF_INFO* pInfo)
{
#if defined(__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)

    //flush all if the buffer size is larger than 50M (suggested by K)
    if((ctrl == IMEM_CACHECTRL_ENUM_FLUSH) && (pInfo->size >= 0x3200000))
    {
        mpM4UDrv->m4u_cache_flush_all(M4U_PORT_IMGI);
        IMEM_DBG("+ Flush all, c/m/va/sz/pa(%d/0x%x/0x%x/0x%x/0x%x)",ctrl,pInfo->memID,pInfo->virtAddr,pInfo->size,pInfo->phyAddr);
        return 0;
    }

    //cache sync by range
    MINT32 err=0;
    MUINT32 phyAddr = (MUINT32)(pInfo->phyAddr);
    IMEM_DBG("+, c/m/va/sz/pa(%d/0x%x/0x%x/0x%x/0x%x)",ctrl,pInfo->memID,pInfo->virtAddr,pInfo->size,pInfo->phyAddr);

    if( IMEM_MIN_ION_FD > pInfo->memID)
    {
            switch(ctrl)
        {
            case IMEM_CACHECTRL_ENUM_FLUSH:
                mpM4UDrv->m4u_cache_sync(M4U_PORT_IMGI,M4U_CACHE_FLUSH_BY_RANGE,pInfo->virtAddr,pInfo->size,phyAddr);
                break;
               case IMEM_CACHECTRL_ENUM_INVALID:
                   mpM4UDrv->m4u_cache_sync(M4U_PORT_IMGI,M4U_CACHE_INVALID_BY_RANGE,pInfo->virtAddr,pInfo->size,phyAddr);
                   break;
               default:
                   LOG_ERR("ERR cmd(%d)",ctrl);
                   break;
          }
    }
    else
    {
        //a. get handle of ION_IOC_SHARE from IonBufFd and increase handle ref count
        ion_user_handle_t pIonHandle;
        MINT32 IonBufFd;
        IonBufFd = pInfo->memID;
        if(ion_import(mIonDrv,IonBufFd,&pIonHandle))
        {
            IMEM_ERR("ion_import fail, memId(0x%x)",IonBufFd);
            return -1;
        }

        //b. cache sync by range
        struct ion_sys_data sys_data;
        sys_data.sys_cmd=ION_SYS_CACHE_SYNC;
        sys_data.cache_sync_param.handle=pIonHandle;
        switch(ctrl)
        {
            case IMEM_CACHECTRL_ENUM_FLUSH:
                sys_data.cache_sync_param.sync_type=ION_CACHE_FLUSH_BY_RANGE;
                break;
               case IMEM_CACHECTRL_ENUM_INVALID:
                   sys_data.cache_sync_param.sync_type=ION_CACHE_INVALID_BY_RANGE;
                   break;
               default:
                   LOG_ERR("ERR cmd(%d)",ctrl);
                   break;
          }
        if(ion_custom_ioctl(mIonDrv, ION_CMD_SYSTEM,&sys_data))
        {
            IMEM_ERR("CAN NOT DO SYNC, memID/(0x%x)",pInfo->memID);
            //decrease handle ref count if cache fail
            if(ion_free(mIonDrv,pIonHandle))
            {
                IMEM_ERR("ion_free fail");
                return -1;
            }
            return -1;
        }

        //c. decrease handle ref count
        if(ion_free(mIonDrv,pIonHandle))
        {
            IMEM_ERR("ion_free fail");
            return -1;
        }
    }
    IMEM_DBG("-");
    return err;
#endif
    return 0;
}
//-----------------------------------------------------------------------------
#if defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)
//m4u
MINT32 IMemDrvImp::allocM4UMemory(
    MUINTPTR     virtAddr,
    MUINT32     size,
    MUINT32*    m4uVa,
    MINT32      memID)
{
    MINT32 ret = 0;
    //
    if(mpM4UDrv == NULL)
    {
        IMEM_DBG("Null M4U driver");
        return -1;
    }
    //

    IMEM_DRV_DELAY
    //
#if(ISP_HAL_PROFILE)
    MINT32 startTime = 0, endTime = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    startTime = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
    //
    M4U_MODULE_ID_ENUM eM4U_ID = M4U_PORT_IMGI;
    //
    ret = mpM4UDrv->m4u_alloc_mva(
            eM4U_ID,
            virtAddr,
            size,
            M4U_PROT_READ|M4U_PROT_WRITE,
            M4U_FLAGS_SEQ_ACCESS,
            m4uVa);
    if(ret != M4U_STATUS_OK)
    {
        IMEM_ERR("m4u_alloc_mva fail:[%d]",ret);
        goto EXIT;
    }
    //
    IMEM_DBG("m4uVa(0x%x)", *m4uVa);
    //
    IMEM_DBG("M4U Flush(0x%x)",virtAddr);
    IMEM_DRV_DELAY
    mpM4UDrv->m4u_cache_sync(eM4U_ID,
                            M4U_CACHE_FLUSH_BY_RANGE,
                            virtAddr,
                            size,
                            (MUINT32)(*m4uVa));
    //
#if(ISP_HAL_PROFILE)
    gettimeofday(&tv, NULL);
    endTime = tv.tv_sec * 1000000 + tv.tv_usec;
    IMEM_DBG("profile time(%d) ms", (endTime - startTime) / 1000);
#endif
    //
    EXIT:
    IMEM_INF("[StdM4U]VA(0x%x)/PA(0x%x)/size(0x%x)/mID(0x%x)", virtAddr, *m4uVa, size, memID);
    return ret;

}
//-----------------------------------------------------------------------------
MINT32 IMemDrvImp::freeM4UMemory(
    MUINTPTR    virtAddr,
    MUINT32     m4uVa,
    MUINT32     size,
    MINT32      memID)
{
    MINT32 ret = 0;
    //
    if(mpM4UDrv == NULL)
    {
        IMEM_DBG("Null M4U driver");
        return -1;
    }
    //
    IMEM_DBG("va(0x%x)/m4uVa(0x%x)/size(%d)", virtAddr, m4uVa, size);
    IMEM_DRV_DELAY
    //
#if(ISP_HAL_PROFILE)
    MINT32 startTime = 0, endTime = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    startTime = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
    //

    ret = mpM4UDrv->m4u_dealloc_mva(
            M4U_PORT_IMGI,
            virtAddr,
            size,
            m4uVa);
    //
#if(ISP_HAL_PROFILE)
    gettimeofday(&tv, NULL);
    endTime = tv.tv_sec * 1000000 + tv.tv_usec;
    IMEM_DBG("profile time(%d) ms", (endTime - startTime) / 1000);
#endif

    //
    IMEM_INF("[StdM4U]VA(0x%x)/PA(0x%x)/size(0x%x)/mID(0x%x)", virtAddr, m4uVa, size, memID);
    return ret;

}

#endif //#if defined (__ISP_USE_STD_M4U__) || defined (__ISP_USE_ION__)

//-----------------------------------------------------------------------------


