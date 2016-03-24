#define LOG_TAG "MtkCam/Custom_Internal"

//
#include <stdlib.h>
#include <stdio.h>
#include <Log.h>
//
#include <camera/MtkCameraParameters.h>
using namespace android;
//
#include <FeatureDef.h>
using namespace NSCameraFeature;
//
#include <custom_internal.h>


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

namespace android {
    namespace camera {
        namespace v1 {

int isAligned(int w, int h, int aligned)
{
    return w % aligned == 0 && h % aligned == 0;
}

int isAligned_16(int w, int h)
{
    return isAligned(w, h, 16);
}

int isSize4_3(int width, int height)
{
    if(width * 3 > INT_MAX - 1)
    {
        MY_LOGD("width: %d is too big!\n", width);
        return 0;
    }

    if(height * 3 > INT_MAX - 1)
    {
        MY_LOGD("height: %d is too big!\n", height);
        return 0;
    }

    if(width * 3 == height * 4)
        return 1;
    return 0;
}

int isSize3_2(int width, int height)
{
    if(width * 2 > INT_MAX - 1)
    {
        MY_LOGD("width:%d is too big!\n", width);
        return 0;
    }

    if(height * 3 > INT_MAX - 1)
    {
        MY_LOGD("height:%d is too big!\n", height);
        return 0;
    }

    if(width * 2 == height * 3)
        return 1;
    return 0;
}

int isSize5_3(int width, int height)
{
    if(width * 3 > INT_MAX - 1)
    {
        MY_LOGD("width:%d is too big!\n", width);
        return 0;
    }

    if(height * 5 > INT_MAX - 1)
    {
        MY_LOGD("height:%d is too big!\n", height);
        return 0;
    }

    if(width * 3 == height * 5)
        return 1;
    return 0;
}

int isSize16_9(int width, int height)
{
    if(width * 9 > INT_MAX - 1)
    {
        MY_LOGD("width:%d is too big!\n", width);
        return 0;
    }

    if(height * 16 > INT_MAX - 1)
    {
        MY_LOGD("height:%d is too big!\n", height);
        return 0;
    }

    if(width * 9 == height * 16)
        return 1;
    return 0;
}

int getNearestRatio(int w, int h)
{
    double ratio = ((double)w) / h;

    if(abs(ratio - (16.) / 9) <= 1e-7)
        return SIZE_RATIO_16_9;
    if(abs(ratio - (5.) / 3) <= 1e-7)
        return SIZE_RATIO_5_3;
    if(abs(ratio - (3.) / 2) <= 1e-7)
        return SIZE_RATIO_3_2;
    if(abs(ratio - (4.) / 3) <= 1e-7)
        return SIZE_RATIO_4_3;
    return SIZE_RATIO_4_3;
}


int getSizeRatio(int w, int h)
{
    if(isSize4_3(w, h))
        return SIZE_RATIO_4_3;
    if(isSize3_2(w, h))
        return SIZE_RATIO_3_2;
    if(isSize5_3(w, h))
        return SIZE_RATIO_5_3;
    if(isSize16_9(w, h))
        return SIZE_RATIO_16_9;
    return SIZE_RATIO_UNKNOWN;
}

int add4_3_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW = 320, baseH = 240;
    int i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    baseW = 256, baseH = 192;
    i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    baseW = 1600, baseH = 1200;
    i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    label_fail:
        return currCnt;
}

int add3_2_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW = 624, baseH = 416;
    int i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(3:2):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    label_fail:
        return currCnt;
}


int add5_3_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW = 320, baseH = 192;
    int i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(5:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    baseW = 400, baseH = 240;
    i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(5:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    label_fail:
        return currCnt;
}

int add16_9_PictureSizes(char **size_arr, int addedCntAfter, int max_w, int max_h)
{
    int currCnt = addedCntAfter;
    int baseW = 256, baseH = 144;
    int i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(16:9):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    baseW = 320, baseH = 180;
    i = 1;

    while(baseW <= max_w && baseH <= max_h)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;

        sprintf(temp, "%dx%d", baseW, baseH);
        size_arr[currCnt] = temp;

        MY_LOGD("Add one size(4:3):%s\n", size_arr[currCnt]);
        ++currCnt;

        ++i;
        baseW *= 2;
        baseH *= 2;
    }

    label_fail:
        return currCnt;
}




int generateSizeValues(int max_w, int max_h, char ***size_arr, int force_ratio)
{
    double ratio = ((double)max_w)/max_h;
    int ratioInt = getSizeRatio(max_w, max_h);
    int base;
    int i;
    int w, h;
    int ratioW, ratioH;
    int ret = 0;
    int new_ratio;
    int max_count = 128;        // todo: need modify
    int addedCntAfter = 0;

    MY_LOGD("enter func generateSizeValues");
    *size_arr = NULL;

    if(force_ratio >= SIZE_RATIO_4_3 && force_ratio <= SIZE_RATIO_16_9)
    {
        ratioInt = force_ratio;
    }

label_switch_ratio:
    switch(ratioInt)
    {
        case SIZE_RATIO_4_3:
            base = max_w/16/4;
            ratioW = 4;
            ratioH = 3;
        break;
        case SIZE_RATIO_3_2:
            base = max_w/16/3;
            ratioW = 3;
            ratioH = 2;
        break;
        case SIZE_RATIO_5_3:
            base = max_w/16/5;
            ratioW = 5;
            ratioH = 3;
        break;
        case SIZE_RATIO_16_9:
            base = max_w/16/16;
            ratioW = 16;
            ratioH = 9;
        break;
        default:
            ratioInt = getNearestRatio(max_w, max_h);
            goto label_switch_ratio;
        break;
    }
    *size_arr = (char **)malloc(max_count * sizeof(char *));
    if(!*size_arr)
        goto label_fail;
    memset(*size_arr, 0, max_count * sizeof(char *));


    #if 0
    for(i = 1; i <= base; ++i)
    {
        char *temp = (char *)malloc(128);   // should modify: 128
        if(!temp)
            goto label_fail;
        w = i * 16 * ratioW;
        h = i * 16 * ratioH;

        sprintf(temp, "%dx%d", w, h);
        (*size_arr)[i - 1] = temp;
        //MY_LOGD("Add one size:%s\n", (*size_arr)[i - 1]);
        ++ret;
    }
    #else

        addedCntAfter = add4_3_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add5_3_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add3_2_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);
        addedCntAfter = add16_9_PictureSizes(*size_arr, addedCntAfter, max_w, max_h);
        ret = addedCntAfter;
    #endif

    return ret;

label_fail:
    for(i = 0; i < base; ++i)
        if((*size_arr)[i])
            free((*size_arr)[i]);
    if(*size_arr)
        free(*size_arr);
    return ret;
}


char *getDefaultPicSize()
{
    return "128x128";
}

int parseStrToSize(const char *s, int *w, int *h)
{
    int ret;
    *w = *h = 0;

    ret = sscanf(s, "%dx%d", w, h);
    if(ret == 2)
        goto label_exit;

    ret = sscanf(s, "\"%dx%d\"", w, h);
    if(ret < 2)
    {
        MY_LOGD("size:%s invalid!\n", s);
        return 0;
    }

label_exit:
    if(ret == 2)
    {
        return 1;
    }

    MY_LOGD("parseStrToSize failed, s:[%s], set default value:640x480\n", ret, s);
    *w = 640;
    *h = 480;

    return 1;
}


const char **getPicSizes(char const *maxPicSize, int * pCnt, int facing)
{
    static bool HasGetMainPicSizes = false;
    static bool HasGetSubPicSizes = false;
    static char **main_arr, **sub_arr;

    int max_w, max_h;

    // back facing camera
    if(facing == 0 && !HasGetMainPicSizes) {
        parseStrToSize(maxPicSize, &max_w, &max_h);
        *pCnt = generateSizeValues(max_w, max_h, &main_arr, 0);

#if 0
        const char **arr = new const char *[4];
        arr[0] = "600x400";
        arr[1] = "1600x1200";
        arr[2] = "800x600";
        arr[3] = "2400x1800";
        *pCnt = 4;
#endif

        HasGetMainPicSizes = true;

        return (const char **)main_arr;
    }
    else if(facing == 1 && !HasGetSubPicSizes) {   // front facing camera
        parseStrToSize(maxPicSize, &max_w, &max_h);
        *pCnt = generateSizeValues(max_w, max_h, &sub_arr, 0);

        HasGetSubPicSizes = true;

        return (const char **)sub_arr;
    }

    return NULL;
}

void addExtraPictureSizes( char const* * defaultSizes, int defaultSize, char const* *extraSizes, int extraSize)
{

}

const char **getPrvSizes(char const *maxPrvSize, int * pCnt, int facing)
{
    static bool HasGetMainPrvSizes = false;
    static bool HasGetSubPrvSizes = false;
    static char **main_arr, **sub_arr;

    int max_w, max_h;

    // back facing camera
    if(facing == 0 && !HasGetMainPrvSizes) {
        parseStrToSize(maxPrvSize, &max_w, &max_h);
        *pCnt = generateSizeValues(max_w, max_h, &main_arr, 0);

#if 0
        const char **arr = new const char *[4];
        arr[0] = "600x400";
        arr[1] = "1600x1200";
        arr[2] = "800x600";
        arr[3] = "2400x1800";
        *pCnt = 4;
#endif

        HasGetMainPrvSizes = true;

        return (const char **)main_arr;
    }
    else if(facing == 1 && !HasGetSubPrvSizes) {   // front facing camera
        parseStrToSize(maxPrvSize, &max_w, &max_h);
        *pCnt = generateSizeValues(max_w, max_h, &sub_arr, 0);

        HasGetSubPrvSizes = true;

        return (const char **)sub_arr;
    }

    return NULL;
}

void addExtraPreviewSizes( char const* * defaultSizes, int defaultSize, char const* *extraSizes, int extraSize)
{

}


// { char const* szFType = "default-values"; char const* szFKey = MtkCameraParameters::KEY_PICTURE_SIZE; char const *szDefaultScene_ItemDefault[] = { getDefaultPicSize() }; int picSizesCnt; char const* *szDefaultScene_ItemList = getPicSizes(&picSizesCnt);  SceneKeyedMap sceneKeyedMap(szFType, FeatureInfo( szDefaultScene_ItemDefault[0], szDefaultScene_ItemList, picSizesCnt )); rFMap.add(FKEY_T(szFKey), sceneKeyedMap); do { if ( (0) ) { ((void)__android_log_print(ANDROID_LOG_DEBUG, "MtkCam/ParamsManager", "[%s] ""%s=%s""\r\n", __FUNCTION__, szFKey, sceneKeyedMap.getDefault().getDefaultItem().string())); } }while(0); }    \


        }// v1
    }   // camera
}// android

