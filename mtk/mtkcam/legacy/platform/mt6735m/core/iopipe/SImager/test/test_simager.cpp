/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


#define LOG_TAG "SImagerTest"


//#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/Log.h>
#include <mtkcam/common.h>

//
#include <mtkcam/iopipe/SImager/ISImager.h>

extern "C" {
#include <pthread.h>
}

#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>


using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSSImager;

/*******************************************************************************
*
********************************************************************************/
#if 1
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)    printf("[%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    printf("[%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[%s] " fmt"\n", __FUNCTION__, ##arg)
#endif

#define TARGET_WIDTH    640
#define TARGET_HEIGHT   480


static pthread_t g_CliKeyThreadHandle;
static MBOOL g_bIsCLITest = MTRUE;

static ISImager *g_pISImager = NULL;


static EImageFormat g_eImgFmt[] = { eImgFmt_YUY2,
                                    eImgFmt_NV21,
                                    eImgFmt_I420,
                                    eImgFmt_YV16,
                                    eImgFmt_I422,
                                    eImgFmt_JPEG} ;
static MUINT32 g_eTransform[] = {0, eTransform_FLIP_H, eTransform_FLIP_V, eTransform_ROT_90, eTransform_ROT_180, eTransform_ROT_270};

//
static MUINT32 g_u4TargetFmt = 0;
static MUINT32 g_u4Transform = 0;
static MUINT32 g_u4TargetWidth = 0, g_u4TargetHeight = 0;
//
static MUINT32 g_u4SrcWidth = 0, g_u4SrcHeight = 0;


/******************************************************************************
*
*******************************************************************************/
static
MBOOL
allocBuf(IImageBuffer **pBuf, MSize const & imgSize, int const format)
{
    MY_LOGD("alloc buf %dx%d, format 0x%x", imgSize.w, imgSize.h, format);
    MSize bufStridesInPixels[3] = {0};
    MUINT32 bufStridesInBytes[3] = {0};
using namespace NSCam::Utils::Format;

    MUINT32 plane = queryPlaneCount(format);
    for (int i = 0; i < (int)queryPlaneCount(format); i++)
    {
        bufStridesInBytes[i] =
            (queryPlaneWidthInPixels(format,i, imgSize.w) * queryPlaneBitsPerPixel(format,i)) >> 3;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator::ImgParam imgParam(
            format, imgSize, bufStridesInBytes, bufBoundaryInBytes, plane
            );

    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    *pBuf = allocator->alloc_ion(LOG_TAG, imgParam);
    if  ( (*pBuf) == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return  MFALSE;
    }
    //
    MY_LOGD("ImgBitsPerPixel:%d BufSizeInBytes:%d %d", (*pBuf)->getImgBitsPerPixel(), (*pBuf)->getBufSizeInBytes(0), imgSize.size());
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
static
MBOOL
allocJpegBuf(IImageBufferHeap **pHeap, IImageBuffer **pBuf, int const imgWidth, int const imgHeight)
{
    //imgParam(MSize(w, h), imgWidth * imgHeight * 2, 0);
    IImageBufferAllocator::ImgParam imgParam(MSize(imgWidth, imgHeight), imgWidth * imgHeight * 2, 0);

    MY_LOGD("using allocator");
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    *pBuf = allocator->alloc("Test_Allocator_allocJpegBuf", imgParam);
/*    *pHeap = IIonImageBufferHeap::create("allocJpegBuf", imgParam);
    if  ( *pHeap == 0 ) {
        CAM_LOGE("NULL Jpeg Heap\n");
        return  MFALSE;
    }
    //
    *pBuf = (*pHeap)->createImageBuffer();
    if  ( *pBuf == 0 )
    {
        CAM_LOGE("NULL Jpeg Buffer\n");
        return  MFALSE;
    }
    */
    //
    return  MTRUE;
}



/////////////////////////////////////////////////////////////////////////
//! Nucamera commands
/////////////////////////////////////////////////////////////////////////
typedef struct CLICmd_t
{
    //! Command string, include shortcut key
    const char *pucCmdStr;

    //! Help string, include functionality and parameter description
    const char *pucHelpStr;

    //! Handling function
    //! \param a_u4Argc  [IN] Number of arguments plus 1
    //! \param a_pprArgv [IN] Array of command and arguments, element 0 is
    //!                       command string
    //! \return error code
    //FIXME: return MRESULT is good?
    MUINT32 (*handleCmd)(const int argc, char** argv);

} CLICmd;

/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4SetFormat_Cmd(int argc, char** argv)
{
    if (argc != 1)
    {
        printf("setFormat <format:0:YUY2, 1:NV21, 2:I420, 3:YV16, 4:I422 5:JPEG>\n");
        return 1;
    }
    g_u4TargetFmt = atoi(argv[0]);
    if (g_u4TargetFmt > 5)
    {
        g_u4TargetFmt = 0;
    }
    //g_pISImager->setFormat(g_eImgFmt[g_u4TargetFmt]);
    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4SetTransform_Cmd(int argc, char** argv)
{
    if (argc != 1)
    {
        printf("u4SetTransform_Cmd <Normal:0, flip_h:1,flip_v:2, rot_90:3, rot_180:4, rot_270:5>\n");
        return 1;
    }

    //g_pISImager->setTransform(g_eTransform[atoi(argv[0])]);
    g_u4Transform = atoi(argv[0]);
    // rotate 90/270, swap the target size
    //if (eTransform_ROT_90 == g_eTransform[atoi(argv[0])]
    //    || eTransform_ROT_270 == g_eTransform[atoi(argv[0])])
    //{
    //    //swap the width/height
    //    g_pISImager->setResize(MSize(g_u4TargetHeight, g_u4TargetWidth));
    //}
    //else
    //{
    //    g_pISImager->setResize(MSize(g_u4TargetWidth, g_u4TargetHeight));
    //}

    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4SetEnc_Cmd(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("setEnc <SOI> <Quality:0~100>\n");
        return 1;
    }

    g_pISImager->setEncodeParam(atoi(argv[0]), atoi(argv[1]));
    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4SetSize_Cmd(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("setSize <width> <height>\n");
        return 1;
    }
    g_u4TargetWidth = atoi(argv[0]);
    g_u4TargetHeight = atoi(argv[1]);
    //g_pISImager->setResize(MSize(g_u4TargetWidth, g_u4TargetHeight));

    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4SetROI_Cmd(int argc, char** argv)
{
    if (argc != 4)
    {
        printf("setROI <x> <y> <w> <h>\n");
        return 1;
    }
    MRect rROI;

    rROI.p.x = atoi(argv[0]);
    rROI.p.y = atoi(argv[1]);
    rROI.s.w = atoi(argv[2]);
    rROI.s.h = atoi(argv[3]);

    if ((rROI.p.x + rROI.s.w) > g_u4SrcWidth || (rROI.p.y + rROI.s.h) > g_u4SrcHeight)
    {
        printf("error ROI=(%d,%d,%d,%d)\n", rROI.p.x, rROI.p.y, rROI.s.w, rROI.s.h);
        return 1;
    }

    g_pISImager->setCropROI(rROI);

    return 0;
}

/******************************************************************************
*
*******************************************************************************/
static MUINT32 u4Execute_Cmd(int argc, char** argv)
{
    IImageBufferHeap *pDestBufferHeap = NULL;
    IImageBuffer *pDestBuffer = NULL;

    MUINT32 w = g_u4TargetWidth;
    MUINT32 h = g_u4TargetHeight;
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    printf("transform %d, format %d\n", g_u4Transform, g_u4TargetFmt);

    // rotate 90/270, swap the target size
    if (eTransform_ROT_90 == g_eTransform[g_u4Transform]
        || eTransform_ROT_270 == g_eTransform[g_u4Transform])
    {
        //swap the width/height
        w = g_u4TargetHeight;
        h = g_u4TargetWidth;
    }
    g_pISImager->setTransform(g_eTransform[g_u4Transform]);

    if (eImgFmt_JPEG == g_eImgFmt[g_u4TargetFmt])
    {
        //!allocJpegBuf(&pDestBufferHeap, &pDestBuffer, w, h)
        if (!allocJpegBuf(&pDestBufferHeap, &pDestBuffer, w, h))
        {
            printf("allocBuffer fail\n");
            return 0;
        }
    }
    else
    {
        if (!allocBuf(&pDestBuffer, MSize(w, h), g_eImgFmt[g_u4TargetFmt]))
        {
             printf("allocYuvBuffer fail \n");
             return 0;
        }
    }

    pDestBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_MASK|eBUFFER_USAGE_SW_MASK);

    g_pISImager->setTargetImgBuffer(pDestBuffer);

    g_pISImager->execute();

    //
    if (eImgFmt_JPEG == g_eImgFmt[g_u4TargetFmt])
    {
        pDestBuffer->saveToFile("/data/result.jpg");
    }
    else
    {
        char filename[256];
        sprintf(filename, "/data/result%dx%d.yuv", w, h);
        pDestBuffer->saveToFile(filename);
    }

    pDestBuffer->unlockBuf(LOG_TAG);

    allocator->free(pDestBuffer);
    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//!  The cli command for the manucalibration
//!
/////////////////////////////////////////////////////////////////////////
static CLICmd g_pSImager_Cmds[] =
{
    {"setFormat", "setFormat <format:0:YUY2, 1:NV21, 2:I420, 3:YV16, 4:I422, 5:JPEG>", u4SetFormat_Cmd},
    {"setTrans",  "setTrans  <Normal:0, flip_h:1,flip_v:2, rot_90:3, rot_180:4, rot_270:5>", u4SetTransform_Cmd},
    {"setEnc", "setEnc <SOI> <Quality:0~100>", u4SetEnc_Cmd},
    {"setSize", "setSize <width> <height>", u4SetSize_Cmd},
    {"setROI",  "setROI <x> <y> <w> <h>", u4SetROI_Cmd},
    {"do", "do", u4Execute_Cmd},
    {NULL, NULL, NULL}
};


/////////////////////////////////////////////////////////////////////////
//
//  thread_exit_handler () -
//! @brief the CLI key input thread, wait for CLI command
//! @param sig: The input arguments
/////////////////////////////////////////////////////////////////////////
static void thread_exit_handler(int sig)
{
    printf("This signal is %d \n", sig);
    pthread_exit(0);
}

/////////////////////////////////////////////////////////////////////////
//
//  vSkipSpace () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
static void vSkipSpace(char **ppInStr)
{
    char *s = *ppInStr;

    while (( *s == ' ' ) || ( *s == '\t' ) || ( *s == '\r' ) || ( *s == '\n' ))
    {
        s++;
    }

    *ppInStr = s;
}


//  vHelp () -
//! @brief skip the space of the input string
//! @param ppInStr: The point of the input string
/////////////////////////////////////////////////////////////////////////
static void vHelp()
{
    printf("\n***********************************************************\n");
    printf("* SImager CLI Test                                                  *\n");
    printf("* Current Support Commands                                *\n");
    printf("===========================================================\n");

    printf("help/h    [Help]\n");
    printf("exit/q    [Exit]\n");

    int i = 0;
    for (i = 0; ; i++)
    {
        if (NULL == g_pSImager_Cmds[i].pucCmdStr)
        {
            break;
        }
        printf("%s    [%s]\n", g_pSImager_Cmds[i].pucCmdStr,
                               g_pSImager_Cmds[i].pucHelpStr);
    }
}

/////////////////////////////////////////////////////////////////////////
//
//  cliKeyThread () -
//! @brief the CLI key input thread, wait for CLI command
//! @param a_pArg: The input arguments
/////////////////////////////////////////////////////////////////////////
static void* cliKeyThread (void *a_pArg)
{
    char urCmds[256] = {0};

    //! ************************************************
    //! Set the signal for kill self thread
    //! this is because android don't support thread_kill()
    //! So we need to creat a self signal to receive signal
    //! to kill self
    //! ************************************************
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    int rc = sigaction(SIGUSR1,&actions,NULL);

    while (1)
    {
        printf("Input Cmd#");
        fgets(urCmds, 256, stdin);

        //remove the '\n'
        urCmds[strlen(urCmds)-1] = '\0';
        char *pCmds = &urCmds[0];
        //remove the space in the front of the string
        vSkipSpace(&pCmds);

        //Ignore blank command
        if (*pCmds == '\0')
        {
            continue;
        }

        //Extract the Command  and arguments where the argV[0] is the command
        MUINT32 u4ArgCount = 0;
        char  *pucStrToken, *pucCmdToken;
        char  *pucArgValues[25];

        pucStrToken = (char *)strtok(pCmds, " ");
        while (pucStrToken != NULL)
        {
            pucArgValues[u4ArgCount++] =(char*) pucStrToken;
            pucStrToken = (char*)strtok (NULL, " ");
        }

        if (u4ArgCount == 0)
        {
            continue;
        }

        pucCmdToken = (char*) pucArgValues[0];

        //parse the command
        if ((strcmp((char *)pucCmdToken, "help") == 0) ||
            (strcmp((char *)pucCmdToken, "h") == 0))
        {
            vHelp();
        }
        else if ((strcmp((char *)pucCmdToken, "exit") == 0) ||
                  (strcmp((char *)pucCmdToken, "q") == 0))
        {
            printf("Exit From CLI\n");
            g_bIsCLITest = MFALSE;
            break;
        }
        else
        {
            MBOOL bIsFoundCmd = MFALSE;
            for (MUINT32 u4CmdIndex = 0; ; u4CmdIndex++)
            {
                if(NULL == g_pSImager_Cmds[u4CmdIndex].pucCmdStr)
                {
                    break;
                }
                if (strcmp((char *)pucCmdToken, g_pSImager_Cmds[u4CmdIndex].pucCmdStr) == 0)
                {
                    bIsFoundCmd = MTRUE;
                    g_pSImager_Cmds[u4CmdIndex].handleCmd(u4ArgCount - 1, &pucArgValues[1]);
                    break;
                }
            }
            if (bIsFoundCmd == MFALSE)
            {
                printf("Invalid Command\n");
            }
        }

    }

    return 0;
}

/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    printf("SImager Test \n");
    char *filename;
    int srcFmt = 0;
    IImageBufferAllocator *allocator = IImageBufferAllocator::getInstance();
    //
    if (argc != 5)
    {
        printf("Usage: simagertest <filename> <width> <height> <format> \n");
        return 0;
    }
    filename = argv[1];
    g_u4SrcWidth = g_u4TargetWidth = atoi(argv[2]);
    g_u4SrcHeight = g_u4TargetHeight = atoi(argv[3]);
    srcFmt = atoi(argv[4]);
    if (srcFmt > 5)
    {
        srcFmt = 0;
    }

    if (g_u4SrcWidth > 5000 || (int)g_u4SrcWidth < 0)
    {
        g_u4SrcWidth = 0;
    }
    if (g_u4SrcHeight > 5000 || (int)g_u4SrcHeight < 0)
    {
        g_u4SrcHeight = 0;
    }

    IImageBuffer *pSrcImageBuff = NULL ;

    if (! allocBuf(&pSrcImageBuff, MSize(g_u4SrcWidth, g_u4SrcHeight), g_eImgFmt[srcFmt]))
    {
        MY_LOGE("Alloc Src YUV Buffer fail");
        return 0;
    }

    pSrcImageBuff->loadFromFile(filename);
    printf("load image:%s\n", filename);

    MY_LOGD("lock src");
    pSrcImageBuff->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_MASK|eBUFFER_USAGE_SW_MASK);

    // (1). Create Instance
    g_pISImager = ISImager::createInstance(pSrcImageBuff);
    if (g_pISImager == NULL)
    {
        MY_LOGE("Null ISImager Obj \n");
        return 0;
    }

    // init setting
    g_pISImager->setTransform(0);
    //
    g_pISImager->setEncodeParam(1, 90);
    //
    g_pISImager->setCropROI(MRect(MPoint(0,0), MSize(g_u4SrcWidth, g_u4SrcHeight)));
    //

    vHelp();

    pthread_create(& g_CliKeyThreadHandle, NULL, cliKeyThread, NULL);

    //!***************************************************
    //! Main thread wait for exit
    //!***************************************************
    while (g_bIsCLITest== MTRUE)
    {
        usleep(100000);
    }

    g_pISImager->destroyInstance();
    pSrcImageBuff->unlockBuf(LOG_TAG);
    allocator->free(pSrcImageBuff);
    return 0;
}
