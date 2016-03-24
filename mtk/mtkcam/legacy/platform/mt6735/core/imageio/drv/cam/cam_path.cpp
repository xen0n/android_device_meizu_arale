#define LOG_TAG "iio/path"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "cam_path.h"
#include <utils/Trace.h> //for systrace

using android::Mutex;

//

#if 0

#include <cutils/properties.h>  // For property_get().


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{Path}"
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(path);
//EXTERN_DBG_LOG_VARIABLE(path);

// Clear previous define, use our own define.
#undef ISP_PATH_VRB
#undef ISP_PATH_DBG
#undef ISP_PATH_INF
#undef ISP_PATH_WRN
#undef ISP_PATH_ERR
#undef ISP_PATH_AST
#define ISP_PATH_VRB(fmt, arg...)        do { if (path_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_PATH_DBG(fmt, arg...)        do { if (path_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_PATH_INF(fmt, arg...)        do { if (path_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_PATH_WRN(fmt, arg...)        do { if (path_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_PATH_ERR(fmt, arg...)        do { if (path_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_PATH_AST(cond, fmt, arg...)  do { if (path_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
#else
#include <cutils/properties.h>  // For property_get().

#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(path);
//EXTERN_DBG_LOG_VARIABLE(path);

#endif


/*-----------------------------------------------------------------------------
    MACRO Definition
  -----------------------------------------------------------------------------*/
#if 0
#define PRINT_ELEMENT_INVOLVED( _title_str_, _isplist_, _ispcount_, _mask_, _b_from_head_ )\
{\
    int  _i;    char _temp_str[512];\
    _temp_str[0] = '\0';\
    strcat( _temp_str, _title_str_ );\
    if( _b_from_head_ ) {\
        for( _i = 0; _i < _ispcount_; _i++ )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_isplist_[_i]->name_Str());\
        }\
    } else {\
        for( _i = (_ispcount_-1) ; _i >= 0 ; _i-- )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_isplist_[_i]->name_Str());\
        }\
    }\
    ISP_PATH_DBG("%s",_temp_str);\
}
#else
#define PRINT_ELEMENT_INVOLVED
#endif

/*-----------------------------------------------------------------------------
    Functions
  -----------------------------------------------------------------------------*/

CamPath_B::
CamPath_B():CQ(CAM_ISP_CQ_NONE)
{
    DBG_LOG_CONFIG(imageio, path);
    CQ_D = CAM_ISP_CQ_NONE;
}


int CamPath_B::start( void* pParam )
{
    ISP_PATH_DBG("{%s}::Start E" DEBUG_STR_BEGIN "", this->name_Str() );

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;

    if( this->_start( pParam ) < 0 ) {
        ISP_PATH_ERR("[ERROR] start(%s) ",this->name_Str());
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::Start""", this->name_Str() );

EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}

int CamPath_B::stop( void* pParam )
{
    ISP_PATH_INF("{%s}::Stop E"DEBUG_STR_BEGIN"", this->name_Str() );

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;

    if( this->_stop( pParam ) < 0 ){
        ISP_PATH_ERR("[ERROR] stop(%s) ",this->name_Str());
        //return -1;
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::Stop""", this->name_Str() );

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}
//
int CamPath_B::dumpRegister( void* pParam )
{
    ISP_PATH_DBG("%s::dumpRegister E"DEBUG_STR_BEGIN"", this->name_Str() );


    int     i;
    IspFunction_B**  isplist;
    int             ispcount;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    for( i = 0; i < ispcount; i++ )
    {
        if( i == 0 )//Use 1st isp element to dump mmsys1 system register
            isplist[i]->dumpRegister(1);

        isplist[i]->dumpRegister(0);
    }

    ISP_PATH_DBG(DEBUG_STR_END"%s::dumpRegister""", this->name_Str() );

    EXIT:

    ISP_PATH_DBG(":X ");

    return 0;

}

int CamPath_B::end( void* pParam )
{
    ISP_PATH_DBG("{%s}: E "DEBUG_STR_BEGIN" ", this->name_Str() );

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);


    int ret = 0;

    if( this->_end( pParam ) < 0 ) {
        ISP_PATH_ERR("[ERROR] end(%s) ",this->name_Str());
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::End""", this->name_Str() );

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}

//#define IS_P1_CQ(cq) (CAM_ISP_CQ0 == cq || CAM_ISP_CQ0B == cq || CAM_ISP_CQ0C == cq || CAM_ISP_CQ0_D == cq || CAM_ISP_CQ0B_D == cq || CAM_ISP_CQ0C_D == cq)

int CamPath_B::_config( void* pParam )
{
    ISP_TRACE_CALL();

    ISP_PATH_DBG("[_config]: E cq(%d/%d)",this->CQ, this->CQ_D);


    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunction_B**  isplist;
    int             ispcount;
    struct CamPathPass2Parameter* pPass2 = (struct CamPathPass2Parameter*)pParam;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //global mutex here
    //control Tg1/Tg2 at same time.
    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->g_cam_path_Lock);
    //
    this->ispTopCtrl.ispDrvSwitch2Virtual(this->CQ, this->CQ_D,this->ispTopCtrl.burstQueIdx, this->ispTopCtrl.dupCqIdx);


    //config
    PRINT_ELEMENT_INVOLVED("[_config]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            ISP_PATH_DBG("[%d]<%s> bypass:",i,isplist[i]->name_Str());
            continue;
        }

        ISP_PATH_DBG("<%s> config:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->config() ) < 0 )
        {
            ISP_PATH_ERR("[ERROR] _config(%s) ",isplist[i]->name_Str());
            goto EXIT;
        }

        if (( CAM_ISP_CQ_NONE != this->CQ ) || ( CAM_ISP_CQ_NONE != this->CQ_D)) {
            if( ( ret_val = isplist[i]->write2CQ() ) < 0 )
            {
                ISP_PATH_ERR("[ERROR] _config CQ(%d) ",this->CQ);
                goto EXIT;
            }
        }
    }


EXIT:

    ISP_PATH_DBG(":X ");


    return ret_val;
}
//
int CamPath_B::_setZoom( void* pParam )
{
    ISP_PATH_DBG("_setZoom E ");


    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunction_B**  isplist;
    int             ispcount;


    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //setZoom (From isp to cdp)
    PRINT_ELEMENT_INVOLVED("[_setZoom]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            ISP_PATH_DBG("<%s> bypass:",isplist[i]->name_Str());
            continue;
        }

        ISP_PATH_DBG("<%s> setZoom:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->setZoom() ) < 0 ) {
            ISP_PATH_ERR("[ERROR] _setZoom(%s) ",isplist[i]->name_Str());
            //return ret_val;
            goto EXIT;
        }

    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret_val;
}
//
int CamPath_B::_start( void* pParam )
{
    ISP_PATH_DBG("_start E ");

    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunction_B**  isplist;
    int             ispcount;
    int             isp_start = ISP_PASS1;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //set all function register base
    //ret_val = IspStart( isplist , ispcount );

    if( ret_val < 0 ) {
        ISP_PATH_ERR("[ERROR] _start ");
        goto EXIT;
        //return ret_val;
    }

    //config (From isp to cdpl)
    /*PRINT_ELEMENT_INVOLVED("[config]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        ISP_PATH_DBG("<%s> config:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->config() ) < 0 )
        {
            return ret_val;
        }
    }*/

    //enable (From cdp to isp)
    PRINT_ELEMENT_INVOLVED("[enable]:", isplist, ispcount, 0, 0 );
    for( i = (ispcount-1) ; i >= 0 ; i-- )
    {
        //ISP_PATH_DBG("<%s> enable.",isplist[i]->name_Str() );
        if( ( ret_val = isplist[i]->enable((void*)pParam ) ) < 0 ) {
            ISP_PATH_ERR("[ERROR] _start enable ");
            goto EXIT;
            //return ret_val;
        }
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret_val;
}

int CamPath_B::_stop( void* pParam )
{
    ISP_PATH_DBG("_stop E ");

int ret = 0;
    int     ret_val = ISP_ERROR_CODE_OK;
    int     ret_val_temp = ISP_ERROR_CODE_OK;
    int     i;
    IspFunction_B**  isplist;
    int             ispcount;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //3.disable (From head to tail)
    PRINT_ELEMENT_INVOLVED("[disable]:", isplist, ispcount, 0, 1 );
    for( i = 0; i < ispcount; i++ )
    {
        //ISP_PATH_DBG("<%s> disable.",isplist[i]->name_Str() );
        if( ( ret_val_temp = isplist[i]->disable() ) < 0 )
            ret_val = ret_val_temp;
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}//


int CamPath_B::_registerIrq( NSImageio::NSIspio::Irq_t irq )
{
    return 0;
}
int CamPath_B::_waitIrq( NSImageio::NSIspio::Irq_t irq )
{
    return 0;
}


int CamPath_B::_end( void* pParam )
{
int ret = 0;
    return ret;
}

//
int CamPath_B::enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo,MINT32 const enqueCq,MINT32 const dupCqIdx)
{
    //ISP_PATH_DBG("+,dmaChannel(%d)",dmaChannel);

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    //
    if ( 0 != this->ispBufCtrl.enqueueHwBuf(dmaChannel, bufInfo,enqueCq,dupCqIdx) ) {
        ISP_PATH_ERR("ERROR:enqueueHwBuf");
        goto EXIT;
        //return -1;
    }

    EXIT:

    //ISP_PATH_DBG("-,");

    //
    return ret;
}


int CamPath_B::enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo, MVOID* private_data, MBOOL bImdMode )
{
    //ISP_PATH_DBG("+,dmaChannel(%d)",dmaChannel);

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    //
    if ( 0 != this->ispBufCtrl.enqueueHwBuf(dmaChannel, bufInfo, private_data,bImdMode) ) {
        ISP_PATH_ERR("ERROR:enqueueHwBuf");
        goto EXIT;
        //return -1;
    }

    EXIT:

    //ISP_PATH_DBG("-,");

    //
    return ret;
}



//
int CamPath_B::freePhyBuf( MUINT32 const mode, stISP_BUF_INFO bufInfo )
{
    ISP_PATH_DBG("freePhyBuf E ");

    //Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    if ( ePathFreeBufMode_SINGLE == mode ) {
        this->ispBufCtrl.freeSinglePhyBuf(bufInfo);
    }
    else if ( ePathFreeBufMode_ALL == mode ) {
        this->ispBufCtrl.freeAllPhyBuf();
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}

//
int CamPath_B::flushCqDescriptor( MUINT32 cq, MUINT32 burstQueIdx, MUINT32 dupCqIdx)
{
    ISP_PATH_INF("cq(%d),burstQueIdx(%d),dupCqIdx(%d)",cq,burstQueIdx,dupCqIdx);
    int ret = 0;

    this->ispTopCtrl.m_pIspDrvShell->CQBufferMutexLock();
    if (true == this->ispTopCtrl.m_pIspDrvShell->getPhyIspDrv()->checkCQBufAllocated())
    {
        if(cq < ISP_DRV_BASIC_CQ_NUM) {
            for(int i=0; i<CAM_DUMMY_; i++)
                this->ispTopCtrl.m_pIspDrvShell->cqDelModule((ISP_DRV_CQ_ENUM)cq, burstQueIdx, dupCqIdx, (CAM_MODULE_ENUM)i);
        } else {
            ISP_PATH_WRN("[warning]not support this cq(%d)",cq);
        }
    }
    this->ispTopCtrl.m_pIspDrvShell->CQBufferMutexUnLock();

    return ret;
}


int CamPath_B::getNr3dGain(ESoftwareScenario softScenario, MINT32 magicNum)
{
    MINT32 nr3dGain=0;
    MBOOL ret=MTRUE;

    ISP_PATH_INF("softScenario(%d),magicNum(0x%x)",softScenario,magicNum);

    ret = this->ispTuningCtrl.m_pIspDrvShell->getNr3dGain(softScenario, magicNum, nr3dGain);
    if(ret == MFALSE){
        ISP_PATH_ERR("[Error] getNr3dGain fail");
        nr3dGain = 0xffffffff;
    }

    return nr3dGain;
}



MBOOL CamPath_B::getEnTuningTag
    (MBOOL isTopEngine,
    ISP_DRV_CQ_ENUM cq,
    MUINT32 drvScenario,
    MUINT32 subMode,
    MUINT32 &tuningEn1Tag,
    MUINT32 &tuningEn2Tag,
    MUINT32 &tuningDmaTag)
{
    return this->ispTuningCtrl.getEnTuningTag(isTopEngine, cq, drvScenario, subMode, tuningEn1Tag, tuningEn2Tag, tuningDmaTag);
}


//
//





