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
#define LOG_TAG "SeninfDrvImp"
//
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <utils/threads.h>
#include <cutils/atomic.h>

#include <mtkcam/common.h>

#include <mtkcam/drv/isp_reg.h>
#include "seninf_reg.h"
#include "seninf_drv_imp.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"


//
/******************************************************************************
*
*******************************************************************************/
#define SENSOR_DEV_NAME     "/dev/kd_camera_hw"
#define ISP_DEV_NAME         "/dev/camera-isp"

//#define FPGA (1)//ToDo: remove after FPGA

#define CAM_APCONFIG_RANGE 0x1000
#define CAM_ISP_RANGE 0xA000
#define CAM_MIPIRX_CONFIG_RANGE 0x100
#define CAM_MIPIRX_ANALOG_RANGE 0x3000
#define CAM_MIPIPLL_RANGE 0x100
#define CAM_GPIO_RANGE 0x1000
#define CAM_PLL_RANGE 0x100    //ToDo:remove


/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrv::createInstance()
{
    return SeninfDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrv*
SeninfDrvImp::
getInstance()
{
    //LOG_MSG("[getInstance] \n");
    static SeninfDrvImp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
SeninfDrvImp::
destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::SeninfDrvImp() :
    SeninfDrv()
{
    LOG_MSG("[SeninfDrvImp] \n");

    mUsers = 0;
    mfd = 0;
    m_fdSensor = -1;
    tg1GrabWidth = 0;
    tg1GrabHeight = 0;
    tg2GrabWidth = 0;
    tg2GrabHeight = 0;
}

/*******************************************************************************
*
********************************************************************************/
SeninfDrvImp::~SeninfDrvImp()
{
    LOG_MSG("[~SeninfDrvImp] \n");
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::init()  //kk test
{
    LOG_MSG("[init]: Entry count %d \n", mUsers);
    MBOOL result;
    MINT32 cam_isp_addr = 0x15000000;
    MINT32 pll_base_hw = 0x10000000;    //ToDo:remove
    MINT32 mipiRx_config = 0x1500C000;
    MINT32 mipiRx_analog = 0x10215000;  /* Base address of CSI0 */
    MINT32 gpio_base_addr = 0x10211000; /* GPIO Baseaddress */

    Mutex::Autolock lock(mLock);

    //
    if (mUsers > 0) {
        LOG_MSG("  Has inited \n");
        android_atomic_inc(&mUsers);
        return 0;
    }

    // Open isp driver
#if USE_ISP_OPEN_FD
    m_pIspDrv = IspDrv::createInstance();
    if (NULL == m_pIspDrv)
    {
        LOG_ERR("IspDrv::createInstance() fail \n");
        return -1;
    }
    mfd = m_pIspDrv->isp_fd_open("seninf_drv");
    LOG_MSG("IspDrv mfd(%d) \n", mfd);
#else
    mfd = open(ISP_DEV_NAME, O_RDWR);
#endif
    if (mfd < 0) {
        LOG_ERR("error open kernel driver, %d, %s\n", errno, strerror(errno));
        return -1;
    }
    //Open sensor driver
    m_fdSensor = open(SENSOR_DEV_NAME, O_RDWR);
    if (m_fdSensor < 0) {
        LOG_ERR("[init]: error opening  %s \n",  strerror(errno));
        return -13;
    }

    mpIspHwRegAddr = (unsigned int *) mmap(0, CAM_ISP_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, cam_isp_addr);
    if (mpIspHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -4;
    }

    // mmap seninf reg
    //mpSeninfHwRegAddr = (unsigned int *) mmap(0, SENINF_BASE_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, SENINF_BASE_HW);
    mpSeninfHwRegAddr = (unsigned int *) mmap(0, SENINF_BASE_RANGE, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, SENINF_BASE_HW);
    if (mpSeninfHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -5;
    }
    //ToDo:remove after SMT
    // mmap pll reg
    //mpPLLHwRegAddr = (unsigned int *) mmap(0, CAM_PLL_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, pll_base_hw);
    mpPLLHwRegAddr = (unsigned int *) mmap(0, CAM_PLL_RANGE, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, pll_base_hw);
    if (mpPLLHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(2), %d, %s \n", errno, strerror(errno));
        return -6;
    }


    // mipi rx analog address
    //mpCSI2RxAnalogRegStartAddr = (unsigned int *) mmap(0, CAM_MIPIRX_ANALOG_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, mipiRx_analog);
    mpCSI2RxAnalogRegStartAddrAlign = (unsigned int *) mmap(0, CAM_MIPIRX_ANALOG_RANGE, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, mipiRx_analog);
    if (mpCSI2RxAnalogRegStartAddrAlign == MAP_FAILED) {
        LOG_ERR("mmap err(5), %d, %s \n", errno, strerror(errno));
        return -9;
    }
     //MMAP only support Page alignment(0x1000), ReMap to CSI2 base addr 0x1021_5800
    mpCSI2RxAnalogRegStartAddr = mpCSI2RxAnalogRegStartAddrAlign + (0x800/4);

    //gpio
    //mpGpioHwRegAddr = (unsigned int *) mmap(0, CAM_GPIO_RANGE, (PROT_READ|PROT_WRITE|PROT_NOCACHE), MAP_SHARED, mfd, gpio_base_addr);
    mpGpioHwRegAddr = (unsigned int *) mmap(0, CAM_GPIO_RANGE, (PROT_READ|PROT_WRITE), MAP_SHARED, mfd, gpio_base_addr);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(6), %d, %s \n", errno, strerror(errno));
        return -10;
    }

    //ToDo:remove after SMT
    mpIPllCon0RegAddr = mpPLLHwRegAddr + (0x60 /4);

    // Debug mpIPllCon0RegAddr, to check the setting flow of mlck pll is correct
    //LOG_MSG("mpIPllCon0RegAddr %x, State: init  \n", *mpIPllCon0RegAddr);

    mpCAMIODrvRegAddr = mpGpioHwRegAddr + (0x970 / 4); // DRV_CFG1
    LOG_MSG("mpCAMIODrvRegAddr %p = 0x%x, State: init  \n", mpCAMIODrvRegAddr, *mpCAMIODrvRegAddr);

    //6595 force Seninf_mux_en first to avoid standby mode w/ data output
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_MUX_EN) = 1;
    SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_EN) = 1;
    SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_EN) = 1;



    mpCSI2RxAnalog0RegAddr = mpCSI2RxAnalogRegStartAddr;
    mpCSI2RxAnalog1RegAddr = mpCSI2RxAnalogRegStartAddr + (0x400/4);
    mpCSI2RxAnalog2RegAddr = mpCSI2RxAnalogRegStartAddr + (0x800/4);

    //set CMMCLK mode 1
    //*(mpGpioHwRegAddr + (0xEA0/4)) &= 0xFFF8;
    //*(mpGpioHwRegAddr + (0xEA0/4)) |= 0x0001;

    // Read CIS efuse value
    {
        #define DEV_IOC_MAGIC       'd'
        #define READ_DEV_DATA       _IOR(DEV_IOC_MAGIC,  1, unsigned int)
        unsigned int devinfo_data = 45; /* CSI section*/

        mCSI = 0; /* initial CSI value*/

        int fd = open("/dev/devmap", O_RDONLY, 0);
        if(fd < 0)
        {
            LOG_ERR("/dev/devmap kernel open fail, errno(%d):%s",errno,strerror(errno));
        }
        else
        {
            if(ioctl(fd, READ_DEV_DATA, &devinfo_data) == 0)
            {
                mCSI = devinfo_data;
                LOG_MSG("csi_value(0x%08x)", mCSI);
            }
            else
            {
                LOG_ERR("Get devinfo_data fail");
            }
            close(fd);
        }
    }

    android_atomic_inc(&mUsers);

    LOG_MSG("[init]: Exit count %d \n", mUsers);


    return 0;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::uninit()
{
    LOG_MSG("[uninit]: %d \n", mUsers);

    //MBOOL result;//6593
    unsigned int temp = 0;

    Mutex::Autolock lock(mLock);

    //

    if (mUsers <= 0) {
        // No more users
        return 0;
    }

    // More than one user
    android_atomic_dec(&mUsers);

    if (mUsers == 0) {
        // Last user

        setSeninf1NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2
        setSeninf2NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // disable CSI2
        //setSeninf3NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0);   // disable CSI2
        //setSeninf4Parallel(0,0);
        //setMclk1(0, 0, 0, 0, 0, 0, 0);
        //setMclk2(0, 0, 0, 0, 0, 0, 0);
        //setMclk3(0, 0, 0, 0, 0, 0, 0);

        //set CMMCLK mode 0
        //*(mpGpioHwRegAddr + (0xEA0/4)) &= 0xFFF8;

        // Jessy added for debug mpIPllCon0RegAddr, to check the setting flow of mlck pll is correct
        //LOG_MSG("mpIPllCon0RegAddr %x, State: uinit \n", *mpIPllCon0RegAddr);


        //
        if ( 0 != mpIspHwRegAddr ) {
            if(munmap(mpIspHwRegAddr, CAM_ISP_RANGE)!=0){
                LOG_ERR("mpIspHwRegAddr munmap err, %d, %s \n", errno, strerror(errno));
            }
            mpIspHwRegAddr = NULL;
        }


        if ( 0 != mpSeninfHwRegAddr ) {
            if(munmap(mpSeninfHwRegAddr, SENINF_BASE_RANGE)!=0) {
                LOG_ERR("mpSeninfHwRegAddr munmap err, %d, %s \n", errno, strerror(errno));
            }
            mpSeninfHwRegAddr = NULL;
         }


        //ToDo:remove after SMT
        if ( mpIPllCon0RegAddr ) {
            //(*mpIPllCon0RegAddr) |= 0x01; //Power Down, No need
        }

        //ToDo:remove after SMT
        if ( 0 != mpPLLHwRegAddr ) {
            if(munmap(mpPLLHwRegAddr, CAM_PLL_RANGE)!=0){
                  LOG_ERR("mpPLLHwRegAddr munmap err, %d, %s \n", errno, strerror(errno));
            }
            mpPLLHwRegAddr = NULL;
        }


        if ( 0 != mpCSI2RxAnalogRegStartAddrAlign ) {
            if(munmap(mpCSI2RxAnalogRegStartAddrAlign, CAM_MIPIRX_ANALOG_RANGE)!=0){
                LOG_ERR("mpCSI2RxAnalogRegStartAddr munmap err, %d, %s \n", errno, strerror(errno));
            }
            mpCSI2RxAnalogRegStartAddrAlign = NULL;
            mpCSI2RxAnalogRegStartAddr = NULL;
        }

        if ( 0 != mpGpioHwRegAddr ) {
            if(munmap(mpGpioHwRegAddr, CAM_GPIO_RANGE)!=0) {
                LOG_ERR("mpGpioHwRegAddr munmap err, %d, %s \n", errno, strerror(errno));
            }
            mpGpioHwRegAddr = NULL;
        }

        //
         LOG_MSG("[uninit]: %d, mfd(%d) \n", mUsers, mfd);
        //
        if (mfd > 0) {
#if USE_ISP_OPEN_FD
            m_pIspDrv->isp_fd_close(mfd);
            m_pIspDrv->destroyInstance();
            m_pIspDrv = NULL;
#else
            close(mfd);
#endif
            mfd = -1;
        }

        if (m_fdSensor > 0) {
            close(m_fdSensor);
            m_fdSensor = -1;
        }
    }
    else {
        LOG_ERR("  Still users \n");
    }


    return 0;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk1(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    static MUINT32 mMclk1User = 0;
    if(1 == pcEn)
        mMclk1User++;
    else
        mMclk1User--;

#if 0
    // Enable Camera PLL first
    if (mclkSel == CAM_PLL_48_GROUP) {
        //48MHz    //ToDo:remove after SMT
        (*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
        (*mpIPllCon0RegAddr) |= 0x1;
    }
    else if (mclkSel == CAM_PLL_52_GROUP) {
        //208MHz    //ToDo:remove after SMT
        (*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
        (*mpIPllCon0RegAddr) |= 0x2;
    }
#endif

#if 1
    if(pcEn == 1 && mMclk1User == 1){
        sensorMclk.on = 1;
        if (mclkSel == CAM_PLL_48_GROUP) {
            //48MHz    //ToDo:remove after SMT
            //(*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
            //(*mpIPllCon0RegAddr) |= 0x1;
            sensorMclk.freq = MCLK_48MHZ_GROUP;
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
                LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
            }

        }
        else if (mclkSel == CAM_PLL_52_GROUP) {
            //208MHz    //ToDo:remove after SMT
            //(*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
            //(*mpIPllCon0RegAddr) |= 0x2;
            sensorMclk.freq = MCLK_52MHZ_GROUP;
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
               LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
            }
        }

        SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, PCEN) = pcEn;
    }
    else if(pcEn == 0 && mMclk1User == 0){
        sensorMclk.on = 0;
        sensorMclk.freq = MCLK_48MHZ_GROUP;
        ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
        if (ret < 0) {
           LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
        }
        SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, PCEN) = pcEn;
    }
 #endif
    LOG_MSG("[setTg1PhaseCounter] pcEn(%d) clkPol(%d) CamPll(0x%x) mMclk1User(%d)\n",(MINT32)pcEn,(MINT32)clkPol,(MUINT32)*mpIPllCon0RegAddr,mMclk1User);
    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt

    //Seninf Top pclk clear gating
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN) = 1;
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN) = 1;

    SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKRS) = clkRiseEdge;
    SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKFL) = clkFallEdge;
    SENINF_BITS(pSeninf, SENINF_TG1_SEN_CK, CLKCNT) = clkCnt;


    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKFL_POL) = (clkCnt & 0x1) ? 0 : 1;
    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, CLKPOL) = clkPol;

    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, TGCLK_SEL) = 1;//force PLL due to ISP engine clock dynamic spread
    //SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, ADCLK_EN) = 1;//FPGA experiment
    SENINF_BITS(pSeninf, SENINF_TG1_PH_CNT, PAD_PCLK_INV) = padPclkInv;


#if(defined(FPGA))
    //JR //ToDo: remove
    *(mpSeninfHwRegAddr+0x200/4) = 0xa0000001;
    *(mpSeninfHwRegAddr+0x204/4) = 0x00030002; // 12/2 = 6Mhz
    //*(mpSeninfHwRegAddr+0x204/4) = 0x000010001; // 12/1 = 12Mhz
    *(mpSeninfHwRegAddr+0x100/4) |= 0x1000;
#endif
    usleep(100);



    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk2(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    static MUINT32 mMclk2User = 0;
    if(1 == pcEn)
        mMclk2User++;
    else
        mMclk2User--;
#if 0
    // Enable Camera PLL first
    if (mclkSel == CAM_PLL_48_GROUP) {
        //48MHz    //ToDo:remove after SMT
        (*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
        (*mpIPllCon0RegAddr) |= 0x1;
    }
    else if (mclkSel == CAM_PLL_52_GROUP) {
        //208MHz    //ToDo:remove after SMT
        (*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
        (*mpIPllCon0RegAddr) |= 0x2;
    }
#endif
#if 1
        if(pcEn == 1 && mMclk2User == 1){
            sensorMclk.on = 1;
            if (mclkSel == CAM_PLL_48_GROUP) {
                //48MHz    //ToDo:remove after SMT
                //(*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
                //(*mpIPllCon0RegAddr) |= 0x1;
                sensorMclk.freq = MCLK_48MHZ_GROUP;
                ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
                if (ret < 0) {
                    LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
                }

            }
            else if (mclkSel == CAM_PLL_52_GROUP) {
                //208MHz    //ToDo:remove after SMT
                //(*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
                //(*mpIPllCon0RegAddr) |= 0x2;
                sensorMclk.freq = MCLK_52MHZ_GROUP;
                ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
                if (ret < 0) {
                   LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
                }
            }
            SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN) = pcEn;
        }
        else if(pcEn == 0 && mMclk2User == 0) {
            sensorMclk.on = 0;
            sensorMclk.freq = MCLK_48MHZ_GROUP;
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
               LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
            }
            SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN) = pcEn;
        }
 #endif


    LOG_MSG("[setTg2PhaseCounter] pcEn(%d) clkPol(%d) CamPll(0x%x) mMclk2User(%d)\n",(MINT32)pcEn,(MINT32)clkPol,(MUINT32)*mpIPllCon0RegAddr,mMclk2User);
    //
       clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt

    //Seninf Top pclk clear gating
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN) = 1;
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN) = 1;

    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKCNT) = clkCnt;       // Sensor master clock falling edge control
    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKRS ) = clkRiseEdge;  // Sensor master clock rising edge control
    SENINF_BITS(pSeninf, SENINF_TG2_SEN_CK, CLKFL ) = clkFallEdge;//fpga


    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKFL_POL   ) = (clkCnt & 0x1) ? 0 : 1;    // Sensor clock falling edge polarity
    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, CLKPOL      ) = clkPol;    // Sensor master clock polarity control

    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, TGCLK_SEL   ) = 1;//force PLL due to ISP engine clock dynamic spread
    //SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, ADCLK_EN    ) = 1;    // Enable sensor master clock (mclk) output to sensor. Note that to set sensor master clock driving setting,
    //SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PCEN        ) = pcEn;    // TG phase counter enable control
    SENINF_BITS(pSeninf, SENINF_TG2_PH_CNT, PAD_PCLK_INV) = padPclkInv;    // Pixel clock inverse in PAD side


#if(defined(FPGA))
    //JR //ToDo: remove
    *(mpSeninfHwRegAddr+0x600/4) = 0xa0000001;
    *(mpSeninfHwRegAddr+0x604/4) = 0x00010001; // 12/2 = 6Mhz
    //*(mpSeninfHwRegAddr+0x204/4) = 0x000000000; // 12/1 = 12Mhz
    *(mpSeninfHwRegAddr+0x100/4) |= 0x1000;

#endif
    usleep(100);


    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk3(
    unsigned long pcEn, unsigned long mclkSel,
    unsigned long clkCnt, unsigned long clkPol,
    unsigned long clkFallEdge, unsigned long clkRiseEdge,
    unsigned long padPclkInv
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    ACDK_SENSOR_MCLK_STRUCT sensorMclk;
    static MUINT32 mMclk3User = 0;
    if(1 == pcEn)
        mMclk3User++;
    else
        mMclk3User--;
#if 0
   // Enable Camera PLL first
   if (mclkSel == CAM_PLL_48_GROUP) {
       //48MHz    //ToDo:remove after SMT
       (*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
       (*mpIPllCon0RegAddr) |= 0x1;
   }
   else if (mclkSel == CAM_PLL_52_GROUP) {
       //208MHz    //ToDo:remove after SMT
       (*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
       (*mpIPllCon0RegAddr) |= 0x2;
   }
#endif
#if 1
        if(pcEn == 1 && mMclk3User == 1){
            sensorMclk.on = 1;
            if (mclkSel == CAM_PLL_48_GROUP) {
                //48MHz    //ToDo:remove after SMT
                //(*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
                //(*mpIPllCon0RegAddr) |= 0x1;
                sensorMclk.freq = MCLK_48MHZ_GROUP;
                ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
                if (ret < 0) {
                    LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
                }

            }
            else if (mclkSel == CAM_PLL_52_GROUP) {
                //208MHz    //ToDo:remove after SMT
                //(*mpIPllCon0RegAddr) &= 0xFFFFFFF8;
                //(*mpIPllCon0RegAddr) |= 0x2;
                sensorMclk.freq = MCLK_52MHZ_GROUP;
                ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
                if (ret < 0) {
                   LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
                }
            }
            SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, PCEN) = pcEn;
        }
        else if(pcEn == 0 && mMclk3User == 0){
            sensorMclk.on = 0;
            sensorMclk.freq = MCLK_48MHZ_GROUP;
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_MCLK_PLL,&sensorMclk);
            if (ret < 0) {
               LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_MCLK_PLL\n");
            }
            SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, PCEN) = pcEn;
        }
 #endif


    LOG_MSG("[setTg3PhaseCounter] pcEn(%d) clkPol(%d) CamPll(0x%x) mMclk3User(%d)\n",(MINT32)pcEn,(MINT32)clkPol,(MUINT32)*mpIPllCon0RegAddr,mMclk3User);

    //
    clkRiseEdge = 0;
    clkFallEdge = (clkCnt > 1)? (clkCnt+1)>>1 : 1;//avoid setting larger than clkCnt

    //Seninf Top pclk clear gating
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF1_PCLK_EN) = 1;
    SENINF_BITS(pSeninf, SENINF_TOP_CTRL, SENINF2_PCLK_EN) = 1;

    SENINF_BITS(pSeninf, SENINF_TG3_SEN_CK, CLKRS) = clkRiseEdge;
    SENINF_BITS(pSeninf, SENINF_TG3_SEN_CK, CLKFL) = clkFallEdge;
    SENINF_BITS(pSeninf, SENINF_TG3_SEN_CK, CLKCNT) = clkCnt;


    SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, CLKFL_POL) = (clkCnt & 0x1) ? 0 : 1;
    SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, CLKPOL) = clkPol;

    SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, TGCLK_SEL) = 1;//force PLL due to ISP engine clock dynamic spread
    //SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, ADCLK_EN) = 1;//FPGA experiment
    //SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, PCEN) = pcEn;//FPGA experiment
    SENINF_BITS(pSeninf, SENINF_TG3_PH_CNT, PAD_PCLK_INV) = padPclkInv;

#if(defined(FPGA))
    //JR //ToDo: remove
    *(mpSeninfHwRegAddr+0x600/4) = 0xa0000001;
    *(mpSeninfHwRegAddr+0x604/4) = 0x00010001; // 12/2 = 6Mhz
    //*(mpSeninfHwRegAddr+0x204/4) = 0x000000000; // 12/1 = 12Mhz
    *(mpSeninfHwRegAddr+0x500/4) |= 0x1000;
#endif
    usleep(100);


    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1GrabRange] \n");
    tg1GrabWidth = pixelEnd - pixelStart;
    tg1GrabHeight = lineEnd - lineStart;

    // TG Grab Win Setting
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_E) = pixelEnd;
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_PXL, PXL_S) = pixelStart;
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_E) = lineEnd;
    ISP_BITS(pisp, CAM_TG_SEN_GRAB_LIN, LIN_S) = lineStart;

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg2GrabRange] \n");
    tg2GrabWidth = pixelEnd - pixelStart;
    tg2GrabHeight = lineEnd - lineStart;

    // TG Grab Win Setting
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_E) = pixelEnd;
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_PXL, PXL_S) = pixelStart;
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_E) = lineEnd;
    ISP_BITS(pisp, CAM_TG2_SEN_GRAB_LIN, LIN_S) = lineStart;

    return ret;
}
/*******************************************************************************
*
********************************************************************************/

int SeninfDrvImp::setSV1GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1GrabRange] \n");

    // TG Grab Win Setting
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_PXL, PXL_E) = pixelEnd;
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_PXL, PXL_S) = pixelStart;
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_LIN, LIN_E) = lineEnd;
    ISP_BITS(pisp, CAMSV_TG_SEN_GRAB_LIN, LIN_S) = lineStart;

    return ret;
}

/*******************************************************************************
*
********************************************************************************/

int SeninfDrvImp::setSV2GrabRange(
    unsigned long pixelStart, unsigned long pixelEnd,
    unsigned long lineStart, unsigned long lineEnd
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg2GrabRange] \n");

    // TG Grab Win Setting
    ISP_BITS(pisp, CAMSV_TG2_SEN_GRAB_PXL, PXL_E) = pixelEnd;
    ISP_BITS(pisp, CAMSV_TG2_SEN_GRAB_PXL, PXL_S) = pixelStart;
    ISP_BITS(pisp, CAMSV_TG2_SEN_GRAB_LIN, LIN_E) = lineEnd;
    ISP_BITS(pisp, CAMSV_TG2_SEN_GRAB_LIN, LIN_S) = lineStart;

    return ret;
}




/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg1ViewFinderMode(
    unsigned long spMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg1ViewFinderMode] \n");
    //
    ISP_BITS(pisp, CAM_TG_SEN_MODE, CMOS_EN) = 1;
    ISP_BITS(pisp, CAM_TG_SEN_MODE, SOT_MODE) = 1;

    ISP_BITS(pisp, CAM_TG_VF_CON, SPDELAY_MODE) = 1;
    ISP_BITS(pisp, CAM_TG_VF_CON, SINGLE_MODE) = spMode;
    //ISP_BITS(pisp, CAM_TG_VF_CON, SP_DELAY) = spDelay;

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2ViewFinderMode(
    unsigned long spMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setTg2ViewFinderMode] \n");
    //
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, CMOS_EN) = 1;
    ISP_BITS(pisp, CAM_TG2_SEN_MODE, SOT_MODE) = 1;

    ISP_BITS(pisp, CAM_TG2_VF_CON, SPDELAY_MODE) = 1;
    ISP_BITS(pisp, CAM_TG2_VF_CON, SINGLE_MODE) = spMode;
    //ISP_BITS(pisp, CAM_TG2_VF_CON, SP_DELAY) = spDelay;

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSV1ViewFinderMode(
    unsigned long spMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setSV1ViewFinderMode] \n");
    //
    ISP_BITS(pisp, CAMSV_TG_SEN_MODE, CMOS_EN) = 1;
    ISP_BITS(pisp, CAMSV_TG_SEN_MODE, SOT_MODE) = 1;

    ISP_BITS(pisp, CAMSV_TG_VF_CON, SPDELAY_MODE) = 1;
    ISP_BITS(pisp, CAMSV_TG_VF_CON, SINGLE_MODE) = spMode;
    //ISP_BITS(pisp, CAMSV_TG_VF_CON, SP_DELAY) = spDelay;

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSV2ViewFinderMode(
    unsigned long spMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    LOG_MSG("[setSV2ViewFinderMode] \n");
    //
    ISP_BITS(pisp, CAMSV_TG2_SEN_MODE, CMOS_EN) = 1;
    ISP_BITS(pisp, CAMSV_TG2_SEN_MODE, SOT_MODE) = 1;

    ISP_BITS(pisp, CAMSV_TG2_VF_CON, SPDELAY_MODE) = 1;
    ISP_BITS(pisp, CAMSV_TG2_VF_CON, SINGLE_MODE) = spMode;
    //ISP_BITS(pisp, CAMSV_TG2_VF_CON, SP_DELAY) = spDelay;

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::sendCommand(int cmd, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    int ret = 0;

    LOG_MSG("[sendCommand] cmd: 0x%x \n", cmd);
    switch (cmd) {
    case CMD_SET_DEVICE:
        mDevice = arg1;
        break;

    case CMD_GET_SENINF_ADDR:
        //LOG_MSG("  CMD_GET_ISP_ADDR: 0x%x \n", (int) mpIspHwRegAddr);
        *(unsigned long *) arg1 = (unsigned long) mpSeninfHwRegAddr;
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}



/*******************************************************************************
*
********************************************************************************/

int SeninfDrvImp::setTg1Cfg(
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb,
    unsigned int twoPxlMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    if(1 == twoPxlMode) {
        ISP_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS) = 1;
        ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1, TWO_PIX) = 1;

    }
    else {
        ISP_BITS(pisp, CAM_TG_SEN_MODE, DBL_DATA_BUS) = 0;
        ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1, TWO_PIX) = 0;
    }

    //JPG sensor
    if ( JPEG_FMT != inDataType) {
        ISP_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN) = 0;

    }
    else {
        ISP_BITS(pisp, CAM_TG_PATH_CFG, JPGINF_EN) = 1;
    }

    ISP_BITS(pisp, CAM_TG_PATH_CFG, SEN_IN_LSB) = 0x0;//no matter what kind of format, set 0
    ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1, TG1_FMT) = inDataType;

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setTg2Cfg(
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb,
    unsigned int twoPxlMode
)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    if(1 == twoPxlMode) {
        ISP_BITS(pisp, CAM_TG2_SEN_MODE, DBL_DATA_BUS) = 1;
        ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1_D, TWO_PIX_D) = 1;

    }
    else {
        ISP_BITS(pisp, CAM_TG2_SEN_MODE, DBL_DATA_BUS) = 0;
        ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1_D, TWO_PIX_D) = 0;
    }

    //JPG sensor
    if ( JPEG_FMT != inDataType) {
        ISP_BITS(pisp, CAM_TG2_PATH_CFG, JPGINF_EN) = 0;

    }
    else {
        ISP_BITS(pisp, CAM_TG2_PATH_CFG, JPGINF_EN) = 1;
    }

    ISP_BITS(pisp, CAM_TG2_PATH_CFG, SEN_IN_LSB) = 0x0;//no matter what kind of format, set 0
    ISP_BITS(pisp, CAM_CTL_FMT_SEL_P1_D, TG1_FMT_D) = inDataType;


    return ret;

}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSV1Cfg(
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb,
    unsigned int twoPxlMode

)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

#if 1
    if(1 == twoPxlMode) {
        ISP_BITS(pisp, CAMSV_TG_SEN_MODE, DBL_DATA_BUS) = 1;
        ISP_BITS(pisp, CAMSV_CAMSV_FMT_SEL, IMGO_BUS_SIZE) = 3;

    }
    else {
        ISP_BITS(pisp, CAMSV_TG_SEN_MODE, DBL_DATA_BUS) = 0;
        ISP_BITS(pisp, CAMSV_CAMSV_FMT_SEL, IMGO_BUS_SIZE) = 1;
    }

    //JPG sensor
    if ( JPEG_FMT == inDataType) {
        ISP_BITS(pisp, CAMSV_CAMSV_FMT_SEL, IMGO_FORMAT) = 2;
        ISP_BITS(pisp, CAMSV_TG_PATH_CFG, JPGINF_EN) = 1;
    }
    else if(YUV422_FMT == inDataType) {
        ISP_BITS(pisp, CAMSV_CAMSV_FMT_SEL, IMGO_FORMAT) = 1;
        ISP_BITS(pisp, CAMSV_TG_PATH_CFG, JPGINF_EN) = 0;
    }
    else {
        ISP_BITS(pisp, CAMSV_CAMSV_FMT_SEL, IMGO_FORMAT) = 0;
        ISP_BITS(pisp, CAMSV_TG_PATH_CFG, JPGINF_EN) = 0;
    }

#endif
    ISP_BITS(pisp, CAMSV_TG_PATH_CFG, SEN_IN_LSB) = 0x0;
    ISP_BITS(pisp, CAMSV_CAMSV_FMT_SEL, TG1_FMT) = inDataType;

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSV2Cfg(
    TG_FORMAT_ENUM inDataType, SENSOR_DATA_BITS_ENUM senInLsb,
    unsigned int twoPxlMode

)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

#if 1
    if(1 == twoPxlMode) {
        ISP_BITS(pisp, CAMSV_TG2_SEN_MODE, DBL_DATA_BUS) = 1;
        ISP_BITS(pisp, CAMSV_CAMSV2_FMT_SEL, IMGO_BUS_SIZE) = 3;

    }
    else {
        ISP_BITS(pisp, CAMSV_TG2_SEN_MODE, DBL_DATA_BUS) = 0;
        ISP_BITS(pisp, CAMSV_CAMSV2_FMT_SEL, IMGO_BUS_SIZE) = 1;
    }

    //JPG sensor
    if ( JPEG_FMT == inDataType) {
        ISP_BITS(pisp, CAMSV_CAMSV2_FMT_SEL, IMGO_FORMAT) = 2;
        ISP_BITS(pisp, CAMSV_TG2_PATH_CFG, JPGINF_EN) = 1;
    }
    else if(YUV422_FMT == inDataType) {
        ISP_BITS(pisp, CAMSV_CAMSV2_FMT_SEL, IMGO_FORMAT) = 1;
        ISP_BITS(pisp, CAMSV_TG2_PATH_CFG, JPGINF_EN) = 1;
    }
    else {
        ISP_BITS(pisp, CAMSV_CAMSV2_FMT_SEL, IMGO_FORMAT) = 0;
        ISP_BITS(pisp, CAMSV_TG2_PATH_CFG, JPGINF_EN) = 1;
    }
#endif
    ISP_BITS(pisp, CAMSV_TG2_PATH_CFG, SEN_IN_LSB) = 0x0;
    ISP_BITS(pisp, CAMSV_CAMSV2_FMT_SEL, TG1_FMT) = inDataType;

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninfTopMuxCtrl(
    unsigned int seninfTopIdx, unsigned int seninfSrc
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;

        temp = SENINF_READ_REG(pSeninf,SENINF_TOP_MUX_CTRL);
        switch (seninfTopIdx){
            case 0:
                SENINF_WRITE_REG(pSeninf,SENINF_TOP_MUX_CTRL,((temp&0xFFF0)|(seninfSrc&0xF)));
                break;
            case 1:
                SENINF_WRITE_REG(pSeninf,SENINF_TOP_MUX_CTRL,((temp&0xFF0F)|((seninfSrc&0xF)<<4)));
                break;
            case 2:
                SENINF_WRITE_REG(pSeninf,SENINF_TOP_MUX_CTRL,((temp&0xF0FF)|((seninfSrc&0xF)<<8)));
                break;
            case 3:
                SENINF_WRITE_REG(pSeninf,SENINF_TOP_MUX_CTRL,((temp&0x0FFF)|((seninfSrc&0xF)<<12)));
                break;
            default:
                break;

        }

        return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf1VC(
    unsigned int vc0Id, unsigned int vc1Id,
    unsigned int vc2Id, unsigned int vc3Id,
    unsigned int vcNum
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;

        temp = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DI);
        SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DI,((vc3Id)<<24)|((vc2Id)<<16)|((vc1Id)<<8)|(vc0Id));

        temp = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DI_CTRL);
        switch(vcNum) {
            case 1:
                temp |= 0x3;
                break;
            case 2:
                temp |= 0x303;
                break;
            case 3:
                temp |= 0x30303;
                break;
            case 4:
                temp |= 0x3030303;
                break;
            default:
                LOG_ERR("virtual channel number incorrect = %d",vcNum);
                return -1;
                break;
        }
        SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DI_CTRL,temp);


        return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf2VC(
    unsigned int vc0Id, unsigned int vc1Id,
    unsigned int vc2Id, unsigned int vc3Id,
    unsigned int vcNum
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;

        temp = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DI);
        SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_DI,((vc3Id&0x3)<<24)|((vc2Id&0x3)<<16)|((vc1Id&0x3)<<8)|(vc0Id&0x3));

        temp = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DI_CTRL);
        switch(vcNum) {
            case 1:
                temp |= 0x1;
                break;
            case 2:
                temp |= 0x101;
                break;
            case 3:
                temp |= 0x10101;
                break;
            case 4:
                temp |= 0x1010101;
                break;
            default:
                LOG_ERR("virtual channel number incorrect = %d",vcNum);
                return -1;
                break;
        }
        SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_DI_CTRL,temp);


        return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf3VC(
    unsigned int vc0Id, unsigned int vc1Id,
    unsigned int vc2Id, unsigned int vc3Id,
    unsigned int vcNum
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;

        temp = SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DI);
        SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_DI,((vc3Id&0x3)<<24)|((vc2Id&0x3)<<16)|((vc1Id&0x3)<<8)|(vc0Id&0x3));

        temp = SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DI_CTRL);
        switch(vcNum) {
            case 1:
                temp |= 0x1;
                break;
            case 2:
                temp |= 0x101;
                break;
            case 3:
                temp |= 0x10101;
                break;
            case 4:
                temp |= 0x1010101;
                break;
            default:
                LOG_ERR("virtual channel number incorrect = %d",vcNum);
                return -1;
                break;
        }
        SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_DI_CTRL,temp);


        return ret;
}


/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf1MuxCtrl(
    unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int twoPxlMode
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;


        //JL for MT6593
        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_MUX_EN) = 1;


        //ToDo:
        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_SRC_SEL) = inSrcTypeSel;


        if(1 == twoPxlMode) {
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_PIX_SEL) = 1;
        }
        else {
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_PIX_SEL) = 0;
        }

        if(JPEG_FMT != inDataType) {
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FULL_WR_EN) = 1;
        }
        else {
            SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FULL_WR_EN) = 0;
        }

        if ((CSI2 == inSrcTypeSel)||(MIPI_SENSOR <= inSrcTypeSel)) {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FLUSH_EN) = 0x3B;
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_PUSH_EN) = 0x3F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }
        else {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FLUSH_EN) = 0x1B;
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_PUSH_EN) = 0x1F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }

        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_HSYNC_POL) = hsPol;
        SENINF_BITS(pSeninf, SENINF1_MUX_CTRL, SENINF_VSYNC_POL) = vsPol;



    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf2MuxCtrl(
    unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int twoPxlMode
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;


        //JL for MT6593
        SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_MUX_EN) = 1;


        //ToDo:
        SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_SRC_SEL) = inSrcTypeSel;


        if(1 == twoPxlMode) {
            SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_PIX_SEL) = 1;
        }
        else {
            SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_PIX_SEL) = 0;
        }

        if(JPEG_FMT != inDataType) {
            SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_FULL_WR_EN) = 1;
        }
        else {
            SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_FULL_WR_EN) = 0;
        }

        if ((CSI2 == inSrcTypeSel)||(MIPI_SENSOR <= inSrcTypeSel)) {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_FLUSH_EN) = 0x3B;
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_PUSH_EN) = 0x3F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }
        else {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_FLUSH_EN) = 0x1B;
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_PUSH_EN) = 0x1F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }

        SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_HSYNC_POL) = hsPol;
        SENINF_BITS(pSeninf, SENINF2_MUX_CTRL, SENINF_VSYNC_POL) = vsPol;



    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf3MuxCtrl(
    unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int twoPxlMode
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;


        //JL for MT6593
        SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_MUX_EN) = 1;


        //ToDo:
        SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_SRC_SEL) = inSrcTypeSel;


        if(1 == twoPxlMode) {
            SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_PIX_SEL) = 1;
        }
        else {
            SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_PIX_SEL) = 0;
        }

        if(JPEG_FMT != inDataType) {
            SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_FULL_WR_EN) = 1;
        }
        else {
            SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_FULL_WR_EN) = 0;
        }

        if ((CSI2 == inSrcTypeSel)||(MIPI_SENSOR <= inSrcTypeSel)) {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_FLUSH_EN) = 0x3B;
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_PUSH_EN) = 0x3F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }
        else {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_FLUSH_EN) = 0x1B;
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_PUSH_EN) = 0x1F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }

        SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_HSYNC_POL) = hsPol;
        SENINF_BITS(pSeninf, SENINF3_MUX_CTRL, SENINF_VSYNC_POL) = vsPol;



    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf4MuxCtrl(
    unsigned long hsPol, unsigned long vsPol,
    SENINF_SOURCE_ENUM inSrcTypeSel, TG_FORMAT_ENUM inDataType,
    unsigned int twoPxlMode
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;


        //JL for MT6593
        SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_MUX_EN) = 1;


        //ToDo:
        SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_SRC_SEL) = inSrcTypeSel;


        if(1 == twoPxlMode) {
            SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_PIX_SEL) = 1;
        }
        else {
            SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_PIX_SEL) = 0;
        }

        SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_FULL_WR_EN) = 1;

        if ((CSI2 == inSrcTypeSel)||(MIPI_SENSOR <= inSrcTypeSel)) {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_FLUSH_EN) = 0x3B;
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_PUSH_EN) = 0x3F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_FLUSH_EN) = 0x38;
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_PUSH_EN) = 0x3E;
            }
        }
        else {
            if(JPEG_FMT != inDataType) {
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_FLUSH_EN) = 0x1B;
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_PUSH_EN) = 0x1F;
            }
            else {
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_FLUSH_EN) = 0x18;
                SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, FIFO_PUSH_EN) = 0x1E;
            }
        }

        SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_HSYNC_POL) = hsPol;
        SENINF_BITS(pSeninf, SENINF4_MUX_CTRL, SENINF_VSYNC_POL) = vsPol;




    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf1Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;

        SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_CTRL, PAD2CAM_DATA_SEL) = padSel;

        if (inSrcTypeSel >= 8) {
            SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; // NCSI2
        }
        else {
            SENINF_BITS(pSeninf, SENINF1_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; // NCSI2
        }
        //temp = SENINF_READ_REG(pSeninf,SENINF1_CTRL);
        //SENINF_WRITE_REG(pSeninf,SENINF1_CTRL,temp|0xE);//reset
        //SENINF_WRITE_REG(pSeninf,SENINF1_CTRL,temp&0xFFFFFF81);//clear reset

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf2Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;

        //0x8100
        SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_EN) = 1;
        SENINF_BITS(pSeninf, SENINF2_CTRL, PAD2CAM_DATA_SEL) = padSel;
        if (inSrcTypeSel >= 8) {
            SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; // NCSI2
        }
        else {
            SENINF_BITS(pSeninf, SENINF2_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; // NCSI2
        }

        //temp = SENINF_READ_REG(pSeninf,SENINF2_CTRL);
        //SENINF_WRITE_REG(pSeninf,SENINF2_CTRL,temp|0xE);//reset
        //SENINF_WRITE_REG(pSeninf,SENINF2_CTRL,temp&0xFFFFFF81);//clear reset

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf3Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;


        SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_EN) = 1;
        SENINF_BITS(pSeninf, SENINF3_CTRL, PAD2CAM_DATA_SEL) = padSel;
        if (inSrcTypeSel >= 8) {
            SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; // NCSI2
        }
        else {
            SENINF_BITS(pSeninf, SENINF3_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; // NCSI2
        }

        //temp = SENINF_READ_REG(pSeninf,SENINF3_CTRL);
        //SENINF_WRITE_REG(pSeninf,SENINF3_CTRL,temp|0xE);//reset
        //SENINF_WRITE_REG(pSeninf,SENINF3_CTRL,temp&0xFFFFFF81);//clear reset

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf4Ctrl(
    PAD2CAM_DATA_ENUM padSel, SENINF_SOURCE_ENUM inSrcTypeSel
)
{
        int ret = 0;
        seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
        unsigned int temp = 0;


        SENINF_BITS(pSeninf, SENINF4_CTRL, SENINF_EN) = 1;
        SENINF_BITS(pSeninf, SENINF4_CTRL, PAD2CAM_DATA_SEL) = padSel;
        if (inSrcTypeSel >= 8) {
            SENINF_BITS(pSeninf, SENINF4_CTRL, SENINF_SRC_SEL) = 8; // NCSI2
        }
        else {
            SENINF_BITS(pSeninf, SENINF4_CTRL, SENINF_SRC_SEL) = inSrcTypeSel; // NCSI2
        }

        //temp = SENINF_READ_REG(pSeninf,SENINF4_CTRL);
        //SENINF_WRITE_REG(pSeninf,SENINF4_CTRL,temp|0xE);//reset
        //SENINF_WRITE_REG(pSeninf,SENINF4_CTRL,temp&0xFFFFFF81);//clear reset

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf1NCSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlaneNum, unsigned long ncsi2En,
    unsigned long dataheaderOrder, unsigned long mipi_type,
    unsigned long HSRXDE, unsigned long dpcm
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    unsigned int Isp_clk = 0;
    unsigned int  msettleDelay = 0;
    //Get ISP CLK
    ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_ISP_CLK,&Isp_clk);
    if (ret < 0) {
        LOG_ERR("ERROR:KDIMGSENSORIOC_X_GET_ISP_CLK\n");
    }

    //ToDo: add GPIO config & MIPI RX config
    if(ncsi2En == 1) {  // enable CSI2
         // enable mipi lane
#if 1//ToDo: for real chip
        temp = *(mpCSI2RxAnalog0RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
        mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog0RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
        mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog0RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//clock lane input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//data lane 0 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//data lane 1 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0c/4));//data lane 2 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//data lane 3 input select mipi
        mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog0RegAddr + (0x10/4));


        // CSI Power On Timing
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x24/4));
        usleep(30);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x20/4));//bit0:RG_CSI0_LDO_CORE_EN,bit1:RG_CSI0_LNRD_HSRX_BCLK_INVERT
        mt65xx_reg_writel(temp|0x00000003, mpCSI2RxAnalog0RegAddr + (0x20/4));
        usleep(1);
        temp = *(mpCSI2RxAnalog0RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN
        mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog0RegAddr + (0x10/4));
        //CSI efuse calibration
        //0x1021_5804[15:12] use 0x1020_61A8 [3:0]
        //0x1021_5808[15:12] use 0x1020_61A8 [7:4]
        //0x1021_580C[15:12] use 0x1020_61A8 [11:8]
        //0x1021_5810[15:12] use 0x1020_61A8 [15:12]
        if(mCSI != 0)
        {
            temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));
            mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x0000000f)<<12), mpCSI2RxAnalog0RegAddr + (0x04/4));
            temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));
            mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x000000f0)<<8), mpCSI2RxAnalog0RegAddr + (0x08/4));
            temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));
            mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x00000f00)<<4), mpCSI2RxAnalog0RegAddr + (0x0C/4));
            temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));
            mt65xx_reg_writel((temp&(~0xf000))|(mCSI&0x0000f000), mpCSI2RxAnalog0RegAddr + (0x10/4));

            LOG_MSG("CSI-0 EFUSE 0x04=0x%x, 0x08=0x%x, 0x0c=0x%x, 0x10=0x%x\n",*(mpCSI2RxAnalog0RegAddr + (0x04/4))
            ,*(mpCSI2RxAnalog0RegAddr + (0x08/4))
            ,*(mpCSI2RxAnalog0RegAddr + (0x0c/4))
            ,*(mpCSI2RxAnalog0RegAddr + (0x10/4)));
        }

        //CSI Offset calibration
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 1;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 1;

        // CSI2 offset calibration
        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 1; // 0x15008338[0]= 1'b1;
        temp = SENINF_READ_REG(pSeninf, MIPI_RX_CON3C_CSI0);
        SENINF_WRITE_REG(pSeninf, MIPI_RX_CON3C_CSI0, 0x1541); // 0x1500833C[31:0]=32'h1541;

        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_HW_CAL_START) = 1;//MIPI_RX_HW_CAL_START

        LOG_MSG("CSI-0 calibration start,MIPIType=%d,HSRXDE=%d\n",(int)mipi_type, (int)HSRXDE);

        usleep(500);
        if(!(( SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI0)& 0x10001) && (SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI0) & 0x101))){
         LOG_ERR("CSI-0 calibration failed!, NCSI2Config Reg 0x44=0x%x, 0x48=0x%x\n",SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI0),SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI0));
         //ret = -1;
        }
        // disable SW control mode
        SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI0, MIPI_RX_SW_CTRL_MODE) = 0; // 0x15008338[0]= 1'b0;

        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 0;
        SENINF_BITS(pSeninf, SENINF1_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 0;

        LOG_MSG("CSI-0 calibration end !\n");

#endif
        // 1.12v 400Mhz, 1.0v 317Mhz
        // ISP CLK = 346Mhz , settel delay count =  (x ns *  300 M )/1000
        if(dataSettleDelay == 0)
            dataSettleDelay = 85;
        msettleDelay = dataSettleDelay * 300 /1000;

      /*if(Isp_clk == 317)
        {
            msettleDelay = dataSettleDelay * 317 /1000 + 1;
        }
        else
        {
            msettleDelay = dataSettleDelay * 400 /1000;
        }*/

        LOG_MSG("TermDelay:%d SettleDelay:%d ClkTermDelay:%d Vsync:%d lane_num:%d NCSI2_en:%d HeaderOrder:%d SettleDelay(cnt):%d ISP_clk:%d dpcm:%d\n",
            (int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlaneNum,
            (int) ncsi2En, (int)dataheaderOrder, (int)msettleDelay, (int)Isp_clk,(int)dpcm);

        //DPCM Enable
        switch(dpcm)
        {
            case 0x30:
                temp = 0x80;
                break;
            case 0x31:
                temp = 0x100;
                break;
            case 0x2a:
                temp = 0xf000;
                break;
            default :
                temp = 0x00;
                break;
        }
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_DPCM, temp);//CSI2 must be slected when sensor clk output

        if(mipi_type == 1) // For CSI2
        {
            //0x8100
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL, temp&0xFFFF0FFF);//CSI2 must be slected when sensor clk output
            //0x8364
            temp = (msettleDelay&0xFF)<<16;
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_DELAY, temp);
            //0x8360
            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL,(temp|(1<<5)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
            //0x8368
            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTEN);
            SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_INTEN,temp|0x000F);//turn on all interrupt
        }
        else // for NCSI2
        {
            temp = SENINF_READ_REG(pSeninf, SENINF1_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF1_CTRL,((temp&0xFFFF0FFF)|0x8000));//nCSI2 must be slected when sensor clk output
            //temp = SENINF_READ_REG(pSeninf, SENINF1_MUX_CTRL);

            //temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFEEF); // disable CSI2 first & disable HSRX_DET_EN (use settle delay setting instead auto)
            //SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFE7F); // disable CSI2 first & disable HSRX_DET_EN (use settle delay setting instead auto)
            /*Disable HS detect function */
            HSRXDE = 1;
            if(HSRXDE == 1)
            {
                // disable HSRX_DET_EN  bit7:HSRX_DET_EN
                temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
                SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&(~0x00000080));
            }
            else
            {
                // Enable HSRX_DET_EN  bit7:HSRX_DET_EN
                temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
                SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp|(0x00000080));
            }

#if (defined(FPGA))
            //JL : settle parameter
            //temp = (3&0xFF)<<8;
            temp = (1&0xFF)<<8;
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, temp);

            temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL,(temp|(1<<26)|(1<<16)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
#else
            temp = (msettleDelay&0xFF)<<8;  //Fixed@145ns/4ns(ISP clock)
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_LNRD_TIMING, temp);
            temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL,(temp|(1<<26)|(dataheaderOrder<<16)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
#endif
            SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_INT_EN,0x80007FFF);//turn on all interrupt
        }

            temp = SENINF_READ_REG(pSeninf,SENINF1_MUX_CTRL);
            SENINF_WRITE_REG(pSeninf,SENINF1_MUX_CTRL,temp|0x3);//reset
            SENINF_WRITE_REG(pSeninf,SENINF1_MUX_CTRL,temp&0xFFFFFFFC);//clear reset

            //turn on virtual channel
            //temp = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DI_CTRL);
            //SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DI_CTRL,temp|0x1);
            // temp solution : force enable clk lane / data lane
            //temp = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_HSRX_DBG);
            //SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_HSRX_DBG,temp|0x1f);

    }
    else {
        // disable NCSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_NCSI2_CTL);
        SENINF_WRITE_REG(pSeninf, SENINF1_NCSI2_CTL, temp&0xFFFFFFE0); // disable CSI2 first
        // disable CSI2
        temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFE0); // disable CSI2 first

        //disable mipi BG
        temp = *(mpCSI2RxAnalog0RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalog0RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN off
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog0RegAddr + (0x10/4));

        // disable mipi pin
        temp = *(mpCSI2RxAnalog0RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog0RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//clock lane input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//data lane 0 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//data lane 1 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//data lane 2 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//data lane 3 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog0RegAddr + (0x10/4));
    }



    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf2NCSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlaneNum, unsigned long ncsi2En,
    unsigned long dataheaderOrder, unsigned long mipi_type,
    unsigned long HSRXDE, unsigned long dpcm
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    unsigned int  msettleDelay = 0;

    //ToDo: add GPIO config & MIPI RX config

     if(ncsi2En == 1) {  // enable CSI2
          // enable mipi lane
#if 1//ToDo: for real chip
         temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x50/4));

         temp = *(mpCSI2RxAnalog1RegAddr + (0x00));//clock lane input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog1RegAddr + (0x00));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//data lane 0 input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog1RegAddr + (0x04/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//data lane 1 input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog1RegAddr + (0x08/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x0c/4));//data lane 2 input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog1RegAddr + (0x0C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//data lane 3 input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog1RegAddr + (0x10/4));



         // CSI Power On Timing
         temp = *(mpCSI2RxAnalog1RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog1RegAddr + (0x24/4));
         usleep(30);
         temp = *(mpCSI2RxAnalog1RegAddr + (0x20/4));//bit0:RG_CSI0_LDO_CORE_EN,bit1:RG_CSI0_LNRD_HSRX_BCLK_INVERT
         mt65xx_reg_writel(temp|0x00000003, mpCSI2RxAnalog1RegAddr + (0x20/4));
         usleep(1);
         temp = *(mpCSI2RxAnalog1RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog1RegAddr);
         temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog1RegAddr + (0x04/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog1RegAddr + (0x08/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog1RegAddr + (0x0C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog1RegAddr + (0x10/4));
         //CSI1 efuse calibration
         //0x1021_5c04[15:12] use 0x1020_61A8 [19:16]
         //0x1021_5c08[15:12] use 0x1020_61A8 [23:20]
         //0x1021_5c0C[15:12] use 0x1020_61A8 [27:24]
         //0x1021_5c10[15:12] use 0x1020_61A8 [31:28]
         if(mCSI != 0)
         {
             temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x000f0000)>>4), mpCSI2RxAnalog1RegAddr + (0x04/4));
             temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x00f00000)>>8), mpCSI2RxAnalog1RegAddr + (0x08/4));
             temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x0f000000)>>12), mpCSI2RxAnalog1RegAddr + (0x0C/4));
             temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0xf0000000)>>16), mpCSI2RxAnalog1RegAddr + (0x10/4));

             LOG_MSG("CSI-1 EFUSE 0x04=0x%x, 0x08=0x%x, 0x0c=0x%x, 0x10=0x%x\n", *(mpCSI2RxAnalog1RegAddr + (0x04/4))
             ,*(mpCSI2RxAnalog1RegAddr + (0x08/4))
             ,*(mpCSI2RxAnalog1RegAddr + (0x0c/4))
             ,*(mpCSI2RxAnalog1RegAddr + (0x10/4)));

         };

         //CSI Offset calibration
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 1;

         // CSI2 offset calibration
         //SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI1, MIPI_RX_SW_CTRL_MODE) = 1; // 0x15008338[0]= 1'b1;
         //temp = SENINF_READ_REG(pSeninf, MIPI_RX_CON3C_CSI1);
         //SENINF_WRITE_REG(pSeninf, MIPI_RX_CON3C_CSI1, 0x1541); // 0x1500833C[31:0]=32'h1541;

         SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI1, MIPI_RX_HW_CAL_START) = 1;//MIPI_RX_HW_CAL_START
         LOG_MSG("CSI-1 calibration start,MIPIType=%d,HSRXDE=%d\n",(int)mipi_type, (int)HSRXDE);

         usleep(500);
         if(!(( SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI1)& 0x10001) && (SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI1) & 0x101))){
          LOG_ERR("CSI-1 calibration failed!, NCSI2Config Reg 0x44=0x%x, 0x48=0x%x\n",SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI1),SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI1));
          //ret = -1;
         }
         // disable SW control mode
         //SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI1, MIPI_RX_SW_CTRL_MODE) = 0; // 0x15008338[0]= 1'b0;

         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF2_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 0;

         LOG_MSG("CSI-1 calibration end !\n");
#endif
        // ISP CLK = 346Mhz , settel delay count =  (x ns *  300 M )/1000
        if(dataSettleDelay == 0)
            dataSettleDelay = 85;
        
        msettleDelay = dataSettleDelay * 300 /1000;

       LOG_MSG("TermDelay:%d SettleDelay:%d ClkTermDelay:%d Vsync:%d lane_num:%d CSI2_en:%d HeaderOrder:%d SettleDelay(cnt):%d dpcm:%d\n",
         (int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlaneNum,
         (int) ncsi2En, (int)dataheaderOrder, (int)msettleDelay,(int)dpcm);


        if(mipi_type == 1) // For CSI2
        {
            //0x8500
            temp = SENINF_READ_REG(pSeninf, SENINF2_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF2_CTRL, temp&0xFFFF0FFF);//CSI2 must be slected when sensor clk output
            //0x8764
            temp = (msettleDelay&0xFF)<<16;
            SENINF_WRITE_REG(pSeninf, SENINF2_CSI2_DELAY, temp);
            //0x8760
            temp = SENINF_READ_REG(pSeninf, SENINF2_CSI2_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF2_CSI2_CTRL,(temp|(1<<5)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
            //0x8768
            temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_INTEN);
            SENINF_WRITE_REG(pSeninf, SENINF2_CSI2_INTEN,temp|0x000F);//turn on all interrupt
        }
        else // for NCSI2
        {
            temp = SENINF_READ_REG(pSeninf, SENINF2_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF2_CTRL,((temp&0xFFFF0FFF)|0x8000));//nCSI2 must be slected when sensor clk output

            //temp = SENINF_READ_REG(pSeninf, SENINF2_NCSI2_CTL);
            //SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_CTL, temp&0xFFFFFEEF); // disable CSI2 first & disable HSRX_DET_EN (use settle delay setting instead auto)
            HSRXDE = 1;
            if(HSRXDE == 1)
            {
                // disable HSRX_DET_EN  bit7:HSRX_DET_EN
                temp = SENINF_READ_REG(pSeninf, SENINF2_NCSI2_CTL);
                SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_CTL, temp&(~0x00000080));
            }
            else
            {
                // Enable HSRX_DET_EN  bit7:HSRX_DET_EN
                temp = SENINF_READ_REG(pSeninf, SENINF2_NCSI2_CTL);
                SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_CTL, temp|(0x00000080));
            }
  
#if (defined(FPGA))
            temp = (msettleDelay&0xFF)<<8;
            SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_LNRD_TIMING, temp);

            temp = SENINF_READ_REG(pSeninf, SENINF2_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_CTL,(temp|(1<<26)|(1<<16)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
#else
            temp = (msettleDelay&0xFF)<<8;  //Fixed@145ns/4ns(ISP clock)
            SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_LNRD_TIMING, temp);

            temp = SENINF_READ_REG(pSeninf, SENINF2_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_CTL,(temp|(1<<26)|(dataheaderOrder<<16)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
#endif
            SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_INT_EN,0x80007FFF);//turn on all interrupt
        }

        temp = SENINF_READ_REG(pSeninf,SENINF2_MUX_CTRL);
        SENINF_WRITE_REG(pSeninf,SENINF2_MUX_CTRL,temp|0x3);//reset
        SENINF_WRITE_REG(pSeninf,SENINF2_MUX_CTRL,temp&0xFFFFFFFC);//clear reset
     }
     else {
         // disable CSI2
         temp = SENINF_READ_REG(pSeninf, SENINF2_NCSI2_CTL);
         SENINF_WRITE_REG(pSeninf, SENINF2_NCSI2_CTL, temp&0xFFFFFFE0); // disable NCSI2 first
         // disable CSI2
         temp = SENINF_READ_REG(pSeninf, SENINF1_CSI2_CTRL);
         SENINF_WRITE_REG(pSeninf, SENINF1_CSI2_CTRL, temp&0xFFFFFFE0); // disable CSI2 first

#if 1//ToDo: for real chip
         //disable mipi BG
         temp = *(mpCSI2RxAnalog1RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN off
         mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog1RegAddr + (0x24/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN off
         mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog1RegAddr + (0x20/4));
         temp = *(mpCSI2RxAnalog1RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN off
         mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog1RegAddr);
         temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN off
         mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog1RegAddr + (0x04/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN off
         mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog1RegAddr + (0x08/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//RG_CSI0_LNRD2_LDO_OUT_EN off
         mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog1RegAddr + (0x0C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//RG_CSI0_LNRD3_LDO_OUT_EN off
         mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog1RegAddr + (0x10/4));
         // disable mipi pin
         temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
         mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
         mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x50/4));


         temp = *(mpCSI2RxAnalog1RegAddr + (0x00));//clock lane input select hi-Z
         mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog1RegAddr + (0x00));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//data lane 0 input select hi-Z
         mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog1RegAddr + (0x04/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//data lane 1 input select hi-Z
         mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog1RegAddr + (0x08/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//data lane 2 input select hi-Z
         mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog1RegAddr + (0x0C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//data lane 3 input select hi-Z
         mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog1RegAddr + (0x10/4));
#endif
     }


    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf3NCSI2(
    unsigned long dataTermDelay, unsigned long dataSettleDelay,
    unsigned long clkTermDelay, unsigned long vsyncType,
    unsigned long dlaneNum, unsigned long ncsi2En,
    unsigned long dataheaderOrder, unsigned long mipi_type,
    unsigned long HSRXDE, unsigned long dpcm
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    unsigned int  msettleDelay = 0;
    //ToDo: add GPIO config & MIPI RX config


     if(ncsi2En == 1) {  // enable CSI2
          // enable mipi lane
#if 1//ToDo: for real chip
         temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x50/4));

         temp = *(mpCSI2RxAnalog2RegAddr + (0x00));//clock lane input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog2RegAddr + (0x00));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//data lane 0 input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog2RegAddr + (0x04/4));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//data lane 1 input select mipi
         mt65xx_reg_writel(temp|0x00000008, mpCSI2RxAnalog2RegAddr + (0x08/4));



         // CSI Power On Timing
         temp = *(mpCSI2RxAnalog2RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog2RegAddr + (0x24/4));
         usleep(30);
         temp = *(mpCSI2RxAnalog2RegAddr + (0x20/4));//bit0:RG_CSI0_LDO_CORE_EN,bit1:RG_CSI0_LNRD_HSRX_BCLK_INVERT
         mt65xx_reg_writel(temp|0x00000003, mpCSI2RxAnalog2RegAddr + (0x20/4));
         usleep(1);
         temp = *(mpCSI2RxAnalog2RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog2RegAddr);
         temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog2RegAddr + (0x04/4));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
         mt65xx_reg_writel(temp|0x00000001, mpCSI2RxAnalog2RegAddr + (0x08/4));
         //CSI2 efuse calibration
         //0x1021_5c04[15:12] use 0x1020_61A8 [19:16]
         //0x1021_5c08[15:12] use 0x1020_61A8 [23:20]
         //0x1021_5c0C[15:12] use 0x1020_61A8 [27:24]
         //0x1021_5c10[15:12] use 0x1020_61A8 [31:28]
         if(mCSI != 0)
         {
             temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x000f0000)>>4), mpCSI2RxAnalog2RegAddr + (0x04/4));
             temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x00f00000)>>8), mpCSI2RxAnalog2RegAddr + (0x08/4));
             temp = *(mpCSI2RxAnalog2RegAddr + (0x0C/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0x0f000000)>>12), mpCSI2RxAnalog2RegAddr + (0x0C/4));
             temp = *(mpCSI2RxAnalog2RegAddr + (0x10/4));
             mt65xx_reg_writel((temp&(~0xf000))|((mCSI&0xf0000000)>>16), mpCSI2RxAnalog2RegAddr + (0x10/4));

             LOG_MSG("CSI-2 EFUSE 0x04=0x%x, 0x08=0x%x, 0x0c=0x%x, 0x10=0x%x\n", *(mpCSI2RxAnalog2RegAddr + (0x04/4))
             ,*(mpCSI2RxAnalog2RegAddr + (0x08/4))
             ,*(mpCSI2RxAnalog2RegAddr + (0x0c/4))
             ,*(mpCSI2RxAnalog2RegAddr + (0x10/4)));

         };


         //CSI Offset calibration
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 1;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 1;

         // CSI2 offset calibration
         SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI2, MIPI_RX_SW_CTRL_MODE) = 1; // 0x15008338[0]= 1'b1;
         temp = SENINF_READ_REG(pSeninf, MIPI_RX_CON3C_CSI2);
         SENINF_WRITE_REG(pSeninf, MIPI_RX_CON3C_CSI2, 0x1541); // 0x15008b3C[31:0]=32'h1541;
         SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI2, MIPI_RX_HW_CAL_START) = 1;//MIPI_RX_HW_CAL_START
         LOG_MSG("CSI-2 calibration start,MIPIType=%d,HSRXDE=%d\n",(int)mipi_type, (int)HSRXDE);

         usleep(500);
         if(!(( SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI2)& 0x10001) && (SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI2) & 0x101))){
         LOG_ERR("CSI-2 calibration failed!, NCSI2Config Reg 0x44=0x%x, 0x48=0x%x\n",SENINF_READ_REG(pSeninf,MIPI_RX_CON44_CSI2),SENINF_READ_REG(pSeninf,MIPI_RX_CON48_CSI2));
         //ret = -1;
         }
         // disable SW control mode
         SENINF_BITS(pSeninf, MIPI_RX_CON38_CSI2, MIPI_RX_SW_CTRL_MODE) = 0; // 0x15008338[0]= 1'b0;

         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, CLOCK_LANE_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE0_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE1_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE2_HSRX_EN) = 0;
         SENINF_BITS(pSeninf, SENINF3_NCSI2_HSRX_DBG, DATA_LANE3_HSRX_EN) = 0;


         LOG_MSG("CSI-2 calibration end !\n");
#endif
         // ISP CLK = 346Mhz , settel delay count =  (x ns *  300 M )/1000
        if(dataSettleDelay == 0)
            dataSettleDelay = 85;
        
        msettleDelay = dataSettleDelay * 300 /1000;

        LOG_MSG("TermDelay:%d SettleDelay:%d ClkTermDelay:%d Vsync:%d lane_num:%d CSI2_en:%d HeaderOrder:%d SettleDelay(cnt):%d dpcm:%d\n",
            (int) dataTermDelay, (int) dataSettleDelay, (int) clkTermDelay, (int) vsyncType, (int) dlaneNum,
            (int) ncsi2En, (int)dataheaderOrder, (int)msettleDelay, (int)dpcm);
        if(mipi_type == 1) // For CSI2
        {
            //0x8900
            temp = SENINF_READ_REG(pSeninf, SENINF3_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF3_CTRL, temp&0xFFFF0FFF);//CSI2 must be slected when sensor clk output
            //0x8B64
            temp = (msettleDelay&0xFF)<<16;
            SENINF_WRITE_REG(pSeninf, SENINF3_CSI2_DELAY, temp);
            //0x8B60
            temp = SENINF_READ_REG(pSeninf, SENINF3_CSI2_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF3_CSI2_CTRL,(temp|(1<<5)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
            //0x8768
            temp = SENINF_READ_REG(pSeninf, SENINF3_CSI2_INTEN);
            SENINF_WRITE_REG(pSeninf, SENINF3_CSI2_INTEN,temp|0x000F);//turn on all interrupt
        }
        else // for NCSI2
        {
            temp = SENINF_READ_REG(pSeninf, SENINF3_CTRL);
            SENINF_WRITE_REG(pSeninf, SENINF3_CTRL,((temp&0xFFFF0FFF)|0x8000));//nCSI2 must be slected when sensor clk output

            //temp = SENINF_READ_REG(pSeninf, SENINF3_NCSI2_CTL);
            //SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_CTL, temp&0xFFFFFEEF); // disable CSI2 first & disable HSRX_DET_EN (use settle delay setting instead auto)
            HSRXDE = 1;
            if(HSRXDE == 1)
            {
                //disable HSRX_DET_EN  bit7:HSRX_DET_EN
                temp = SENINF_READ_REG(pSeninf, SENINF3_NCSI2_CTL);
                SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_CTL, temp&(~0x00000080));
            }
            else
            {
                // Enable HSRX_DET_EN  bit7:HSRX_DET_EN
                temp = SENINF_READ_REG(pSeninf, SENINF3_NCSI2_CTL);
                SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_CTL, temp|(0x00000080));
            }

#if (defined(FPGA))
            temp = (msettleDelay&0xFF)<<8;
            SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_LNRD_TIMING, temp);

            temp = SENINF_READ_REG(pSeninf, SENINF2_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_CTL,(temp|(1<<26)|(1<<16)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;

#else
            temp = (msettleDelay&0xFF)<<8;  //Fixed@145ns/4ns(ISP clock)
            SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_LNRD_TIMING, temp);

            temp = SENINF_READ_REG(pSeninf, SENINF3_NCSI2_CTL);
            SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_CTL,(temp|(1<<26)|(dataheaderOrder<<16)|(ncsi2En<<4)|(((1<<(dlaneNum+1))-1)))) ;
#endif
            SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_INT_EN,0x80007FFF);//turn on all interrupt

        }
        temp = SENINF_READ_REG(pSeninf,SENINF3_MUX_CTRL);
        SENINF_WRITE_REG(pSeninf,SENINF3_MUX_CTRL,temp|0x3);//reset
        SENINF_WRITE_REG(pSeninf,SENINF3_MUX_CTRL,temp&0xFFFFFFFC);//clear reset

     }
     else
     {
         // disable CSI2
         temp = SENINF_READ_REG(pSeninf, SENINF3_NCSI2_CTL);
         SENINF_WRITE_REG(pSeninf, SENINF3_NCSI2_CTL, temp&0xFFFFFFE0); // disable NCSI2 first
         // disable CSI2
         temp = SENINF_READ_REG(pSeninf, SENINF3_CSI2_CTRL);
         SENINF_WRITE_REG(pSeninf, SENINF3_CSI2_CTRL, temp&0xFFFFFFE0); // disable CSI2 first

#if 1//ToDo: for real chip
        //disable mipi BG
        temp = *(mpCSI2RxAnalog2RegAddr + (0x24/4));//RG_CSI_BG_CORE_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog2RegAddr + (0x24/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x20/4));//RG_CSI0_LDO_CORE_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog2RegAddr + (0x20/4));
        temp = *(mpCSI2RxAnalog2RegAddr); //RG_CSI0_LNRC_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog2RegAddr);
        temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//RG_CSI0_LNRD0_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog2RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//RG_CSI0_LNRD1_LDO_OUT_EN
        mt65xx_reg_writel(temp&0xFFFFFFFE, mpCSI2RxAnalog2RegAddr + (0x08/4));
        // disable mipi pin
        temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4 ));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x50/4));


        temp = *(mpCSI2RxAnalog2RegAddr + (0x00));//clock lane input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog2RegAddr + (0x00));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//data lane 0 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog2RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//data lane 1 input select hi-Z
        mt65xx_reg_writel(temp&0xFFFFFFF7, mpCSI2RxAnalog2RegAddr + (0x08/4));
#endif
     }


    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setSeninf4Scam(
    unsigned int scamEn, unsigned int clkInv,
    unsigned int width, unsigned int height,
    unsigned int contiMode, unsigned int csdNum,
    unsigned int DDR_EN
)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    IMGSENSOR_GPIO_STRUCT mpgpio;
    int data_channel;
    LOG_MSG("Scam clkInv(%d),width(%d),height(%d),csdNum(%d),DDR_EN(%d)\n",clkInv,width,height,csdNum+1,DDR_EN);

    /* set GPIO config */
    if(scamEn == 1) {
        mpgpio.GpioEnable = 1;
        mpgpio.SensroInterfaceType = SENSORIF_SERIAL;
        mpgpio.SensorIndataformat = DATA_YUV422_FMT;
        ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
        if (ret < 0) {
           LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO\n");
        }

        //GPI
        temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x50/4));

        temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x4C/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
        mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x50/4));
    }
    else {
         mpgpio.GpioEnable = 0;
         mpgpio.SensroInterfaceType = SENSORIF_SERIAL;
         mpgpio.SensorIndataformat = DATA_YUV422_FMT;
         ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
         if (ret < 0) {
            LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO\n");
         }
        temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x50/4));

         temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x50/4));
    }

if(scamEn == 1) {
        /**/
        switch(csdNum) {
            case 0: /* 1 channel*/
                data_channel = 0;
                break;
            case 1: /* 2 channel*/
                data_channel = 1;
                break;
            case 2:/* 3 channel*/
                data_channel = 3;
                break;
            case 3:/* 4 channel*/
                data_channel = 2;
                break;
            default:
                data_channel = 0;
                break;
        }
        /*Reset for enable scam*/
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x10000);
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00);

        /*Sensor Interface control select to Serial camera*/
        temp = SENINF_READ_REG(pSeninf, SENINF4_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF4_CTRL,((temp&0xFFFF0FFF)|0x4000));

        /*SCAM control */
        temp = SENINF_READ_REG(pSeninf, SCAM1_CFG);
        temp &= 0x50000000;
        SENINF_WRITE_REG(pSeninf,SCAM1_CFG, temp|((data_channel&0x7)<<24)|(contiMode<<17)|(clkInv<<12)|(1<<8)|(1));
        SENINF_WRITE_REG(pSeninf,SCAM1_SIZE, ((height&0xFFF)<<16)|(width&0xFFF));
        if(width > 0xFFF || height > 0xFFF) {
            LOG_ERR("scam size incorrect, it must be smaller than 0xfff !!\n");
        }

        SENINF_WRITE_REG(pSeninf, SCAM1_CFG2, 1);
        /*DDR Mode */
        if(DDR_EN == 1)
        {
            unsigned int clkInv1 = 0; /*DDR mode : clkInv & clkInv1 need inverse*/
            if(clkInv == 0)
                clkInv1 = 1 ;
            else
                clkInv1 = 0;

            temp = SENINF_READ_REG(pSeninf, SCAM1_DDR_CTRL);
            temp &= 0x00000004;
            SENINF_WRITE_REG(pSeninf,SCAM1_DDR_CTRL, temp|((csdNum&0x3)<<28)|(clkInv1<<2)|(clkInv<<1));

        }
        /*Enable SCAM*/
        SENINF_WRITE_REG(pSeninf, SCAM1_LINE_ID_START, 0x00);
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x01);
    }
    else
    {
        SENINF_WRITE_REG(pSeninf, SCAM1_CON, 0x00);
    }

    temp = SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL);
    SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp|0x3);//reset
    SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp&0xFFFFFFFC);//clear reset

    return ret;

}


int SeninfDrvImp::setSeninf4Parallel(unsigned int parallelEn,unsigned int inDataType)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    unsigned int temp = 0;
    IMGSENSOR_GPIO_STRUCT mpgpio;

    if(parallelEn == 1) {
        if ((inDataType == RAW_8BIT_FMT)||(inDataType == YUV422_FMT)||(inDataType == RGB565_MIPI_FMT)||
            (inDataType == RGB888_MIPI_FMT)||(inDataType == JPEG_FMT)||(inDataType == RAW_10BIT_FMT)) {
#if 0 //ToDo: for real chip
            //cmmclk & cmpclk
            //temp = *(mpGpioHwRegAddr + (0x680/4));
            //mt65xx_reg_writel(((temp&0xFE07)|0x48), mpGpioHwRegAddr + (0x680/4));

            temp = *(mpGpioHwRegAddr + (0x870/4));
            mt65xx_reg_writel(((temp&0x003F)|0x1240), mpGpioHwRegAddr + (0x870/4));
            mt65xx_reg_writel(0x1249, mpGpioHwRegAddr + (0x880/4));
            temp = *(mpGpioHwRegAddr + (0x890/4));
            mt65xx_reg_writel(((temp&0xFFC0)|0x9), mpGpioHwRegAddr + (0x890/4));

            if(inDataType == RAW_10BIT_FMT) {//10bit data pin
                temp = *(mpGpioHwRegAddr + (0x890/4));
                mt65xx_reg_writel(((temp&0xFFF)|0x1000), mpGpioHwRegAddr + (0x890/4));
                temp = *(mpGpioHwRegAddr + (0x8A0/4));
                mt65xx_reg_writel(((temp&0xFFF8)|0x1), mpGpioHwRegAddr + (0x8A0/4));
            }
#endif
            mpgpio.GpioEnable = 1;
            mpgpio.SensroInterfaceType = SENSORIF_PARALLEL;
            mpgpio.SensorIndataformat = (INDATA_FORMAT_ENUM)inDataType;
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
            if (ret < 0) {
               LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO\n");
            }

            //GPI
            temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x4C/4));
            temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog1RegAddr + (0x50/4));

            temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x4C/4));
            temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for GPIO
            mt65xx_reg_writel(temp|0x1041041, mpCSI2RxAnalog2RegAddr + (0x50/4));

        }

        else {
            LOG_ERR("parallel interface only support 8bit/10bit !\n");
        }


        temp = SENINF_READ_REG(pSeninf, SENINF4_CTRL);
        SENINF_WRITE_REG(pSeninf, SENINF4_CTRL,((temp&0xFFFF0FFF)|0x3000));


         temp = SENINF_READ_REG(pSeninf,SENINF4_MUX_CTRL);
        SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp|0x3);//reset
        SENINF_WRITE_REG(pSeninf,SENINF4_MUX_CTRL,temp&0xFFFFFFFC);//clear reset
    }
    else {

#if 0
        //ToDo: for real chip
        //temp = *(mpGpioHwRegAddr + (0x680/4));
        //mt65xx_reg_writel((temp&0xFE07), mpGpioHwRegAddr + (0x680/4));
        temp = *(mpGpioHwRegAddr + (0x870/4));
        mt65xx_reg_writel((temp&0x003F), mpGpioHwRegAddr + (0x870/4));
        mt65xx_reg_writel(0x0, mpGpioHwRegAddr + (0x880/4));
        temp = *(mpGpioHwRegAddr + (0x890/4));
        mt65xx_reg_writel((temp&0xFC0), mpGpioHwRegAddr + (0x890/4));
        temp = *(mpGpioHwRegAddr + (0x8A0/4));
        mt65xx_reg_writel((temp&0xFFF8), mpGpioHwRegAddr + (0x8A0/4));

#endif
         mpgpio.GpioEnable = 0;
         mpgpio.SensroInterfaceType = SENSORIF_PARALLEL;
         mpgpio.SensorIndataformat = (INDATA_FORMAT_ENUM)inDataType;
         ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_GPIO,&mpgpio);
         if (ret < 0) {
            LOG_ERR("ERROR:KDIMGSENSORIOC_X_SET_GPIO\n");
         }

         temp = *(mpCSI2RxAnalog1RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog1RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog1RegAddr + (0x50/4));

         temp = *(mpCSI2RxAnalog2RegAddr + (0x4C/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x4C/4));
         temp = *(mpCSI2RxAnalog2RegAddr + (0x50/4));//GPI*_IES = 0 for MIPI
         mt65xx_reg_writel(temp&0xFEFBEFBE, mpCSI2RxAnalog2RegAddr + (0x50/4));

    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk1IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;

    if(mpCAMIODrvRegAddr != NULL) {
        *(mpCAMIODrvRegAddr) &= ~0x60;
        *(mpCAMIODrvRegAddr) |= (ioDrivingCurrent<<5); // [6:5] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
    }
    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(mpCAMIODrvRegAddr)));

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk2IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;

    if(mpCAMIODrvRegAddr != NULL) {
        *(mpCAMIODrvRegAddr) &= ~0x06;
        *(mpCAMIODrvRegAddr) |= (ioDrivingCurrent<<1); // [2:1] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
    }
    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(mpCAMIODrvRegAddr)));

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
int SeninfDrvImp::setMclk3IODrivingCurrent(unsigned long ioDrivingCurrent)
{
    int ret = 0;

    if(mpCAMIODrvRegAddr != NULL) {
        *(mpCAMIODrvRegAddr) &= ~0x6;
        *(mpCAMIODrvRegAddr) |= (ioDrivingCurrent<<1); // [2:1] = 0:2mA, 1:4mA, 2:6mA, 3:8mA
    }

    LOG_MSG("[setIODrivingCurrent]:%d 0x%08x\n", (int) ioDrivingCurrent, (int) (*(mpCAMIODrvRegAddr)));

    return ret;
}

int SeninfDrvImp::setTG1_TM_Ctl(unsigned int seninfSrc, unsigned int TM_En, unsigned int dummypxl,unsigned int vsync,
                                    unsigned int line,unsigned int pxl)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_WRITE_REG(pSeninf,SENINF_TG1_TM_CTL,0x00);
    if(seninfSrc == SENINF_1)
    {
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG1_TM_SIZE, TM_LINE) = line;
    }
    else if(seninfSrc == SENINF_2)
    {
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_SIZE, TM_LINE) = line;
    }
    else if(seninfSrc == SENINF_3)
    {
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_SIZE, TM_LINE) = line;
    }
    else if(seninfSrc == SENINF_4)
    {
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_EN) = TM_En;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_VSYNC) = vsync;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_DUMMYPXL) = dummypxl;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_PAT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_FMT) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_RST) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_SIZE, TM_PXL) = pxl;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_SIZE, TM_LINE) = line;
    }
    else
    {
        SENINF_BITS(pSeninf, SENINF_TG1_TM_CTL, TM_EN) = 0;
        SENINF_BITS(pSeninf, SENINF_TG2_TM_CTL, TM_EN) = 0;
        SENINF_BITS(pSeninf, SENINF_TG3_TM_CTL, TM_EN) = 0;
        SENINF_BITS(pSeninf, SENINF_TG4_TM_CTL, TM_EN) = 0;
    }
    return ret;
}


int SeninfDrvImp::setFlashA(unsigned long endFrame, unsigned long startPoint, unsigned long lineUnit, unsigned long unitCount,
            unsigned long startLine, unsigned long startPixel, unsigned long  flashPol)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;


    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN) = 0x0;

    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_POL) = flashPol;
    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_END_FRM) = endFrame;
    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_STARTPNT) = startPoint;

    ISP_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT_NO) = unitCount;
    ISP_BITS(pisp, CAM_TG_FLASHA_LINE_CNT, FLASHA_LUNIT) = lineUnit;

    ISP_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_PXL) =  startPixel;
    ISP_BITS(pisp, CAM_TG_FLASHA_POS, FLASHA_LINE) =  startLine;

    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASHA_EN) = 0x1;

    return ret;

}


int SeninfDrvImp::setFlashB(unsigned long contiFrm, unsigned long startFrame, unsigned long lineUnit, unsigned long unitCount, unsigned long startLine, unsigned long startPixel)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN) = 0x0;

    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_CONT_FRM) = contiFrm;
    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_START_FRM) = startFrame;
    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_STARTPNT) = 0x0;
    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_TRIG_SRC) = 0x0;

    ISP_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT_NO) = unitCount;
    ISP_BITS(pisp, CAM_TG_FLASHB_LINE_CNT, FLASHB_LUNIT) = lineUnit;

    ISP_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_PXL) = startPixel;
    ISP_BITS(pisp, CAM_TG_FLASHB_POS, FLASHB_LINE) =  startLine;

    ISP_BITS(pisp, CAM_TG_FLASHB_CTL, FLASHB_EN) = 0x1;

    return ret;
}

int SeninfDrvImp::setFlashEn(bool flashEn)
{
    int ret = 0;
    isp_reg_t *pisp = (isp_reg_t *) mpIspHwRegAddr;

    ISP_BITS(pisp, CAM_TG_FLASHA_CTL, FLASH_EN) = flashEn;

    return ret;

}


int SeninfDrvImp::setCCIR656Cfg(CCIR656_OUTPUT_POLARITY_ENUM vsPol, CCIR656_OUTPUT_POLARITY_ENUM hsPol, unsigned long hsStart, unsigned long hsEnd)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    if ((hsStart > 4095) || (hsEnd > 4095))
    {
        LOG_ERR("CCIR656 HSTART or HEND value err \n");
        ret = -1;
    }

    SENINF_BITS(pSeninf, CCIR656_CTL, CCIR656_VS_POL) = vsPol;
    SENINF_BITS(pSeninf, CCIR656_CTL, CCIR656_HS_POL) = hsPol;
    SENINF_BITS(pSeninf, CCIR656_H, CCIR656_HS_END) = hsEnd;
    SENINF_BITS(pSeninf, CCIR656_H, CCIR656_HS_START) = hsStart;

    return ret;
}


int SeninfDrvImp::setN3DCfg(unsigned long n3dEn, unsigned long i2c1En, unsigned long i2c2En, unsigned long n3dMode, unsigned long diffCntEn, unsigned long diffCntThr)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_BITS(pSeninf, N3D_CTL, N3D_EN) = n3dEn;
    SENINF_BITS(pSeninf, N3D_CTL, I2C1_EN) = i2c1En;
    SENINF_BITS(pSeninf, N3D_CTL, I2C2_EN) = i2c2En;
    SENINF_BITS(pSeninf, N3D_CTL, MODE) = n3dMode;

    return ret;
}


int SeninfDrvImp::setN3DI2CPos(unsigned long n3dPos)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_BITS(pSeninf, N3D_POS, N3D_POS) = n3dPos;

    return ret;
}


int SeninfDrvImp::setN3DTrigger(bool i2c1TrigOn, bool i2c2TrigOn)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    SENINF_BITS(pSeninf, N3D_TRIG, I2CA_TRIG) = i2c1TrigOn;
    SENINF_BITS(pSeninf, N3D_TRIG, I2CB_TRIG) = i2c2TrigOn;

    return ret;

}


int SeninfDrvImp::getN3DDiffCnt(MUINT32 *pCnt)
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    *pCnt = SENINF_READ_REG(pSeninf,N3D_DIFF_CNT);
    *(pCnt+1) = SENINF_READ_REG(pSeninf,N3D_CNT0);
    *(pCnt+2) = SENINF_READ_REG(pSeninf,N3D_CNT1);
    *(pCnt+3) = SENINF_READ_REG(pSeninf,N3D_DBG);

    return ret;

}

int SeninfDrvImp::checkSeninf1Input()
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;
#if 0
    temp = SENINF_READ_REG(pSeninf,SENINF1_DEBUG_4);
    LOG_MSG("[checkSeninf1Input]:size = 0x%x",temp);
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;

    if( (tempW >= tg1GrabWidth) && (tempH >= tg1GrabHeight)  ) {
        ret = 0;
    }
    else {
        ret = 1;
    }
#endif
    return ret;

}

int SeninfDrvImp::checkSeninf2Input()
{
    int ret = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;
    int temp=0,tempW=0,tempH=0;
#if 0
    temp = SENINF_READ_REG(pSeninf,SENINF2_DEBUG_4);
    LOG_MSG("[checkSeninf2Input]:size = 0x%x",temp);
    tempW = (temp & 0xFFFF0000) >> 16;
    tempH = temp & 0xFFFF;

    if( (tempW >= tg2GrabWidth) && (tempH >= tg2GrabHeight)  ) {
        ret = 0;
    }
    else {
        ret = 1;
    }
#endif
    return ret;

}

int SeninfDrvImp::autoDeskewCalibrationSeninf1()
{
    int ret = 0;
    unsigned int temp = 0;
    MUINT32 lane_num=0,min_lane_code=0;
    MUINT8 lane0_code=0,lane1_code=0,lane2_code=0,lane3_code=0,i=0;
    MUINT8 clk_code=0;
    MUINT32 currPacket = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("autoDeskewCalibration start \n");

    temp = *(mpCSI2RxAnalog0RegAddr + (0x00));//disable clock lane delay
    mt65xx_reg_writel(temp&0xFFEFFFFF, mpCSI2RxAnalog0RegAddr + (0x00));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//disable data lane 2 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//disable data lane 3 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));


    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_DGB_SEL,0x12);//set debug port to output packet number
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_EN,0x80007FFF);//set interrupt enable //@write clear?

    //@add check default if any interrup error exist, if yes, debug and fix it first. if no, continue calibration
    //@add print ecc & crc error status
    if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0){
        LOG_ERR("autoDeskewCalibration Line %d, default input has error, please check it \n",__LINE__);
    }
    LOG_MSG("autoDeskewCalibration start interupt status = 0x%x\n",SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS));
    SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);

    //Fix Clock lane
    //set lane 0
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//enable data lane 0 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x04/4));

    for(i=0; i<=0xF; i++) {
        lane0_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        //@add read interrupt status to clear
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x04/4));
        //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
    usleep(5);

    //set lane 1
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//enable data lane 1 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x08/4));

    for(i=0; i<=0xF; i++) {
        lane1_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x08/4));
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
    usleep(5);

    //set lane 2
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//enable data lane 2 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x0C/4));

    for(i=0; i<=0xF; i++) {
        lane2_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x0C/4));
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//disable data lane 2 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
    usleep(5);

    //set lane 3
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//enable data lane 3 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x10/4));

    for(i=0; i<=0xF; i++) {
        lane3_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x10/4));
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//disable data lane 3 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));

  LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d \n",lane0_code,lane1_code,lane2_code,lane3_code);

    //find minimum data lane code
    min_lane_code = lane0_code;
    if(min_lane_code > lane1_code) {
        min_lane_code = lane1_code;
    }
    if(min_lane_code > lane2_code) {
        min_lane_code = lane2_code;
    }
    if(min_lane_code > lane3_code) {
        min_lane_code = lane3_code;
    }
    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d, minimum = %d \n",lane0_code,lane1_code,lane2_code,lane3_code,min_lane_code);


    //Fix Data lane
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));
    mt65xx_reg_writel(temp|(((lane0_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));
    mt65xx_reg_writel(temp|(((lane1_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));
    mt65xx_reg_writel(temp|(((lane2_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));
    mt65xx_reg_writel(temp|(((lane3_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x10/4));
    temp = *(mpCSI2RxAnalog0RegAddr);//enable clock lane delay
    mt65xx_reg_writel(temp|0x100000, mpCSI2RxAnalog0RegAddr );
    temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//enable data lane 0 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//enable data lane 1 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//enable data lane 2 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//enable data lane 3 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog0RegAddr + (0x10/4));

    LOG_MSG("autoDeskewCalibration start test 5 \n");

    for(i=0; i<=0xF; i++) {
        clk_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog0RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog0RegAddr );
        temp = *(mpCSI2RxAnalog0RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalog0RegAddr );
        while((currPacket == SENINF_READ_REG(pSeninf,SENINF1_NCSI2_DBG_PORT))){};
         //usleep(5);
         if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF1_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF1_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }

    if(clk_code < min_lane_code) {
        lane0_code = lane0_code -((min_lane_code+clk_code)>>1);
        lane1_code = lane1_code -((min_lane_code+clk_code)>>1);
        lane2_code = lane2_code -((min_lane_code+clk_code)>>1);
        lane3_code = lane3_code -((min_lane_code+clk_code)>>1);
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog0RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog0RegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalog0RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog0RegAddr);//clk code = 0
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog0RegAddr );
    }
    else {
        //data code keeps at DC[n]-min(DC[n])
        clk_code = (clk_code - min_lane_code)>>1;
        temp = *(mpCSI2RxAnalog0RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog0RegAddr );
        temp = *(mpCSI2RxAnalog0RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalog0RegAddr );
    }
    LOG_MSG("autoDeskewCalibration clk_code = %d, min_lane_code = %d\n",clk_code,min_lane_code);



    LOG_MSG("autoDeskewCalibration end \n");
    return ret;
}


int SeninfDrvImp::autoDeskewCalibrationSeninf2()
{
    int ret = 0;
    unsigned int temp = 0;
    MUINT32 lane_num=0,min_lane_code=0;
    MUINT8 lane0_code=0,lane1_code=0,lane2_code=0,lane3_code=0,i=0;
    MUINT8 clk_code=0;
    MUINT32 currPacket = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("autoDeskewCalibration start \n");

    temp = *(mpCSI2RxAnalog1RegAddr + (0x00));//disable clock lane delay
    mt65xx_reg_writel(temp&0xFFEFFFFF, mpCSI2RxAnalog1RegAddr + (0x00));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//disable data lane 2 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//disable data lane 3 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x10/4));


    SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_DGB_SEL,0x12);//set debug port to output packet number
    SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_EN,0x80007FFF);//set interrupt enable //@write clear?

    //@add check default if any interrup error exist, if yes, debug and fix it first. if no, continue calibration
    //@add print ecc & crc error status
    if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0){
        LOG_ERR("autoDeskewCalibration Line %d, default input has error, please check it \n",__LINE__);
    }
    LOG_MSG("autoDeskewCalibration start interupt status = 0x%x\n",SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS));
    SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_STATUS,0x7FFF);

    //Fix Clock lane
    //set lane 0
    temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//enable data lane 0 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x04/4));

    for(i=0; i<=0xF; i++) {
        lane0_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT);
        //@add read interrupt status to clear
        temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x04/4));
        //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x04/4));
    usleep(5);

    //set lane 1
    temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//enable data lane 1 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x08/4));

    for(i=0; i<=0xF; i++) {
        lane1_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x08/4));
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x08/4));
    usleep(5);

    //set lane 2
    temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//enable data lane 2 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x0C/4));

    for(i=0; i<=0xF; i++) {
        lane2_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x0C/4));
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//disable data lane 2 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x0C/4));
    usleep(5);

    //set lane 3
    temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//enable data lane 3 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x10/4));

    for(i=0; i<=0xF; i++) {
        lane3_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x10/4));
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//disable data lane 3 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog1RegAddr + (0x10/4));

  LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d \n",lane0_code,lane1_code,lane2_code,lane3_code);

    //find minimum data lane code
    min_lane_code = lane0_code;
    if(min_lane_code > lane1_code) {
        min_lane_code = lane1_code;
    }
    if(min_lane_code > lane2_code) {
        min_lane_code = lane2_code;
    }
    if(min_lane_code > lane3_code) {
        min_lane_code = lane3_code;
    }
    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, data2 = %d, data3 = %d, minimum = %d \n",lane0_code,lane1_code,lane2_code,lane3_code,min_lane_code);


    //Fix Data lane
    temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x10/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));
    mt65xx_reg_writel(temp|(((lane0_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));
    mt65xx_reg_writel(temp|(((lane1_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));
    mt65xx_reg_writel(temp|(((lane2_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));
    mt65xx_reg_writel(temp|(((lane3_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x10/4));
    temp = *(mpCSI2RxAnalog1RegAddr);//enable clock lane delay
    mt65xx_reg_writel(temp|0x100000, mpCSI2RxAnalog1RegAddr );
    temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//enable data lane 0 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//enable data lane 1 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x08/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//enable data lane 2 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x0C/4));
    temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//enable data lane 3 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog1RegAddr + (0x10/4));

    LOG_MSG("autoDeskewCalibration start test 5 \n");

    for(i=0; i<=0xF; i++) {
        clk_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog1RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog1RegAddr );
        temp = *(mpCSI2RxAnalog1RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalog1RegAddr );
        while((currPacket == SENINF_READ_REG(pSeninf,SENINF2_NCSI2_DBG_PORT))){};
         //usleep(5);
         if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF2_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF2_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }

    if(clk_code < min_lane_code) {
        lane0_code = lane0_code -((min_lane_code+clk_code)>>1);
        lane1_code = lane1_code -((min_lane_code+clk_code)>>1);
        lane2_code = lane2_code -((min_lane_code+clk_code)>>1);
        lane3_code = lane3_code -((min_lane_code+clk_code)>>1);
        temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog1RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x0C/4));
        mt65xx_reg_writel(temp|((lane2_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x0C/4));
        temp = *(mpCSI2RxAnalog1RegAddr + (0x10/4));
        mt65xx_reg_writel(temp|((lane3_code&0xF)<<25), mpCSI2RxAnalog1RegAddr + (0x10/4));
        temp = *(mpCSI2RxAnalog1RegAddr);//clk code = 0
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog1RegAddr );
    }
    else {
        //data code keeps at DC[n]-min(DC[n])
        clk_code = (clk_code - min_lane_code)>>1;
        temp = *(mpCSI2RxAnalog1RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog1RegAddr );
        temp = *(mpCSI2RxAnalog1RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalog1RegAddr );
    }
    LOG_MSG("autoDeskewCalibration clk_code = %d, min_lane_code = %d\n",clk_code,min_lane_code);



    LOG_MSG("autoDeskewCalibration end \n");
    return ret;
}


int SeninfDrvImp::autoDeskewCalibrationSeninf3()
{
    int ret = 0;
    unsigned int temp = 0;
    MUINT32 lane_num=0,min_lane_code=0;
    MUINT8 lane0_code=0,lane1_code=0,i=0;
    MUINT8 clk_code=0;
    MUINT32 currPacket = 0;
    seninf_reg_t *pSeninf = (seninf_reg_t *)mpSeninfHwRegAddr;

    LOG_MSG("autoDeskewCalibration start \n");

    temp = *(mpCSI2RxAnalog2RegAddr + (0x00));//disable clock lane delay
    mt65xx_reg_writel(temp&0xFFEFFFFF, mpCSI2RxAnalog2RegAddr + (0x00));
    temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog2RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog2RegAddr + (0x08/4));



    SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_DGB_SEL,0x12);//set debug port to output packet number
    SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_INT_EN,0x80007FFF);//set interrupt enable //@write clear?

    //@add check default if any interrup error exist, if yes, debug and fix it first. if no, continue calibration
    //@add print ecc & crc error status
    if((SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS)&0xFB8)!= 0){
        LOG_ERR("autoDeskewCalibration Line %d, default input has error, please check it \n",__LINE__);
    }
    LOG_MSG("autoDeskewCalibration start interupt status = 0x%x\n",SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS));
    SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_INT_STATUS,0x7FFF);

    //Fix Clock lane
    //set lane 0
    temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//enable data lane 0 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog2RegAddr + (0x04/4));

    for(i=0; i<=0xF; i++) {
        lane0_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DBG_PORT);
        //@add read interrupt status to clear
        temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog2RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalog2RegAddr + (0x04/4));
        //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//disable data lane 0 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog2RegAddr + (0x04/4));
    usleep(5);

    //set lane 1
    temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//enable data lane 1 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog2RegAddr + (0x08/4));

    for(i=0; i<=0xF; i++) {
        lane1_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog2RegAddr + (0x08/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalog2RegAddr + (0x08/4));
         //usleep(5);
         while((currPacket == SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DBG_PORT))){};
         if((SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }
    temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//disable data lane 1 delay
    mt65xx_reg_writel(temp&0xFEFFFFFF, mpCSI2RxAnalog2RegAddr + (0x08/4));
    usleep(5);


  LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d\n",lane0_code,lane1_code);

    //find minimum data lane code
    min_lane_code = lane0_code;
    if(min_lane_code > lane1_code) {
        min_lane_code = lane1_code;
    }

    LOG_MSG("autoDeskewCalibration data0 = %d, data1 = %d, minimum = %d \n",lane0_code,lane1_code,min_lane_code);


    //Fix Data lane
    temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog2RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//set to 0 first
    mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog2RegAddr + (0x08/4));

    temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));
    mt65xx_reg_writel(temp|(((lane0_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog2RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));
    mt65xx_reg_writel(temp|(((lane1_code-min_lane_code)&0xF)<<25), mpCSI2RxAnalog2RegAddr + (0x08/4));

    temp = *(mpCSI2RxAnalog2RegAddr);//enable clock lane delay
    mt65xx_reg_writel(temp|0x100000, mpCSI2RxAnalog2RegAddr );
    temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//enable data lane 0 delay
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog2RegAddr + (0x04/4));
    temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//enable data lane 1 dela
    mt65xx_reg_writel(temp|0x1000000, mpCSI2RxAnalog2RegAddr + (0x08/4));


    LOG_MSG("autoDeskewCalibration start test 5 \n");

    for(i=0; i<=0xF; i++) {
        clk_code = i;
        currPacket = SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DBG_PORT);
        temp = *(mpCSI2RxAnalog2RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog2RegAddr );
        temp = *(mpCSI2RxAnalog2RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalog2RegAddr );
        while((currPacket == SENINF_READ_REG(pSeninf,SENINF3_NCSI2_DBG_PORT))){};
         //usleep(5);
         if((SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS)&0xFB8)!= 0) {
            SENINF_WRITE_REG(pSeninf,SENINF3_NCSI2_INT_STATUS,0x7FFF);
            usleep(5);
            if((SENINF_READ_REG(pSeninf,SENINF3_NCSI2_INT_STATUS)&0xFB8)!= 0) { //double confirm error happen
                break;
            }
         }

    }

    if(clk_code < min_lane_code) {
        lane0_code = lane0_code -((min_lane_code+clk_code)>>1);
        lane1_code = lane1_code -((min_lane_code+clk_code)>>1);

        temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog2RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));//set to 0 first
        mt65xx_reg_writel(temp&0xE1FFFFFF, mpCSI2RxAnalog2RegAddr + (0x08/4));

        temp = *(mpCSI2RxAnalog2RegAddr + (0x04/4));
        mt65xx_reg_writel(temp|((lane0_code&0xF)<<25), mpCSI2RxAnalog2RegAddr + (0x04/4));
        temp = *(mpCSI2RxAnalog2RegAddr + (0x08/4));
        mt65xx_reg_writel(temp|((lane1_code&0xF)<<25), mpCSI2RxAnalog2RegAddr + (0x08/4));

        temp = *(mpCSI2RxAnalog2RegAddr);//clk code = 0
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog2RegAddr );
    }
    else {
        //data code keeps at DC[n]-min(DC[n])
        clk_code = (clk_code - min_lane_code)>>1;
        temp = *(mpCSI2RxAnalog2RegAddr);//set to 0 first
        mt65xx_reg_writel(temp&0xFE1FFFFF, mpCSI2RxAnalog2RegAddr );
        temp = *(mpCSI2RxAnalog2RegAddr);
        mt65xx_reg_writel(temp|((clk_code&0xF)<<21), mpCSI2RxAnalog2RegAddr );
    }
    LOG_MSG("autoDeskewCalibration clk_code = %d, min_lane_code = %d\n",clk_code,min_lane_code);



    LOG_MSG("autoDeskewCalibration end \n");
    return ret;
}




//ToDo: remove
#if 0
int SeninfDrvImp::setPdnRst(int camera, bool on)
{
    int tgsel;
    int ret = 0;
    MINT32 imgsys_cg_clr0 = 0x15000000;
    MINT32 gpio_base_addr = 0x10001000;


    switch(camera) {
        case 1:
            tgsel = 0;
            break;
        case 2:
            tgsel = 1;
            break;
        case 3:
            tgsel = 2;
            break;

        default:
            tgsel = 3;
            break;
    }
    LOG_MSG("camera = %d tgsel = %d, On = %d \n",camera, tgsel,on);

    // FPGA_GPIO_GPI    :  0x10001E80
    // FPGA_GPIO_GPO    :  0x10001E84
    // FPGA_GPIO_DIR    :  0x10001E88
    // sen0_pwrdn          gpio[0] ,  pin C3_13
    // sen0_rst            gpio[1] ,  pin C3_14
    // sen1_pwrdn          gpio[2] ,  pin C3_15
    // sen1_rst            gpio[3] ,  pin C3_16

    *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0000000F; // Set GPIO output
#if 0
    if (0 == tgsel){//Tg1
        if(1 == on){
            *(mpGpioHwRegAddr + (0xE84/4)) = 0x00000003; //Bit4 : Reset, Bit5 : PowerDown
        }
        else {
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
            //ToDo: JH test
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
        }
    }
    else if (1 == tgsel){
        if(1 == on){
            *(mpGpioHwRegAddr + (0xE84/4)) = 0x00000008; //RST : 1, PWD : 0

        }
        else {
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
            //ToDo: JH test
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
        }

    }
    else if (2 == tgsel){
        if(1 == on){
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000003; //Bit4 : Reset, Bit5 : PowerDown
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000001; //Bit4 : Reset, Bit5 : PowerDown
            //ToDo: JH test
            *(mpGpioHwRegAddr + (0xE84/4)) = 0x0000000B; //Bit4 : Reset, Bit5 : PowerDown

        }
        else {
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
            //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
            //ToDo:JH test
            *(mpGpioHwRegAddr + (0xE84/4)) = 0x00000004; //Bit4 : Reset, Bit5 : PowerDown
        }


    }
#else
    if (0 == tgsel){//Tg1
         if(1 == on){
             *(mpGpioHwRegAddr + (0xE84/4)) |= 0x00000003; //Bit4 : Reset, Bit5 : PowerDown  3
         }
         else {
             //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
             //ToDo: JH test
             *(mpGpioHwRegAddr + (0xE84/4)) &= 0xFFFFFFFC; //Bit4 : Reset, Bit5 : PowerDown
         }
     }
     else if (1 == tgsel){
         if(1 == on){
             *(mpGpioHwRegAddr + (0xE84/4)) |= 0x00000008; //RST : 1, PWD : 0

         }
         else {
             //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
             //ToDo: JH test
             *(mpGpioHwRegAddr + (0xE84/4)) &= 0xFFFFFFF3; //Bit4 : Reset, Bit5 : PowerDown
         }

     }
     else if (2 == tgsel){
         if(1 == on){
             //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000003; //Bit4 : Reset, Bit5 : PowerDown
             //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000001; //Bit4 : Reset, Bit5 : PowerDown
             //ToDo: JH test
             *(mpGpioHwRegAddr + (0xE84/4)) |= 0x0000000B; //Bit4 : Reset, Bit5 : PowerDown

         }
         else {
             //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
             //*(mpGpioHwRegAddr + (0xE84/4)) = 0x00000000; //Bit4 : Reset, Bit5 : PowerDown
             //ToDo:JH test
             *(mpGpioHwRegAddr + (0xE84/4)) &= 0xFFFFFFF0; //Bit4 : Reset, Bit5 : PowerDown
         }


     }

#endif
    LOG_MSG("Address = 0x%x, value = 0x%x\n",(mpGpioHwRegAddr + (0xE84/4)),*(mpGpioHwRegAddr + (0xE84/4)));




/*
    // mmap seninf clear gating reg
    mpCAMMMSYSRegAddr = (unsigned int *) mmap(0, CAM_MMSYS_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, imgsys_cg_clr0);
    if (mpCAMMMSYSRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(3), %d, %s \n", errno, strerror(errno));
        return -7;
    }

    *(mpCAMMMSYSRegAddr + (0x8/4)) |= 0x03FF; //clear gate


    // mmap seninf reg
    mpGpioHwRegAddr = (unsigned int *) mmap(0, CAM_GPIO_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, mfd, gpio_base_addr);
    if (mpGpioHwRegAddr == MAP_FAILED) {
        LOG_ERR("mmap err(1), %d, %s \n", errno, strerror(errno));
        return -1;
    }

    //set GPIO0~3 as output
    //GPIO 0: CMRST  C3_51
    //GPIO 1: CMPDN  C3_52
    //GPIO 2: CM1RST  C3_53
    //GPIO 3: CM1PDN  C3_54
    *(mpGpioHwRegAddr + (0xE84/4)) |= 0x000F;
    *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFF0;



    switch(camera) {
        case 1:
            tgsel = 0;
            break;
        case 2:
            tgsel = 1;
            break;

        default:
            tgsel = 0;
            break;
    }
    LOG_MSG("camera = %d tgsel = %d, On = %d \n",camera, tgsel,on);



    if (0 == tgsel){//Tg1
        if(1 == on){
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFFD;
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0001;

        }
        else {
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFFE;
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0002;
        }
    }
    else {
        if(1 == on){
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFF7;
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0004;

        }
        else {
            *(mpGpioHwRegAddr + (0xE88/4)) &= 0xFFFB;
            *(mpGpioHwRegAddr + (0xE88/4)) |= 0x0008;
        }

    }
  */
    return ret;

}
#endif




