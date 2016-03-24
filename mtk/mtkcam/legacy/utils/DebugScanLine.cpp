#define LOG_TAG "MtkCam/DebugScanLine"

#include <mtkcam/utils/DebugScanLine.h>
#include <cutils/properties.h>
//using namespace std;
//#include <cutils/atomic.h>
//#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include <mtkcam/Log.h>
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)

class DebugScanLineImp : public DebugScanLine
{
    public:
        DebugScanLineImp();
        ~DebugScanLineImp();

        virtual void    destroyInstance(void);
        virtual void    drawScanLine(int imgWidth, int imgHeight, void* virtAddr, int bufSize, int imgStride);

    private:
        int drawCount;



};

DebugScanLine::
~DebugScanLine()
{
}

DebugScanLineImp::
DebugScanLineImp()
{
    drawCount = 0;
}

DebugScanLineImp::
~DebugScanLineImp()
{
}

DebugScanLine*
DebugScanLine::
createInstance(void)
{
    DebugScanLineImp* pDebugScanLineImp = new DebugScanLineImp();
    return pDebugScanLineImp;
}

void
DebugScanLineImp::
destroyInstance(void)
{
    delete this;
}

void
DebugScanLineImp::
drawScanLine(int imgWidth, int imgHeight, void* virtAddr, int bufSize, int imgStride)
{
    char propertyValue[PROPERTY_VALUE_MAX] = {'\0'};
    int i, value = 0, height = 0, width = 0, widthShift = 0, speed = 0, lineHeight = 0, displacement = 0, fullscreen;

    if (bufSize < imgStride*imgHeight)
    {
        MY_LOGD("buffer size < stride*height, doesn't draw scan line");
        return;
    }

    property_get( "debug.cam.scanline.value", propertyValue, "255");
    value = atoi(propertyValue);
    property_get( "debug.cam.scanline.height", propertyValue, "100");
    height = atoi(propertyValue);
    property_get( "debug.cam.scanline.width", propertyValue, "0");
    width = atoi(propertyValue);
    property_get( "debug.cam.scanline.widthShift", propertyValue, "0");
    widthShift = atoi(propertyValue);
    property_get( "debug.cam.scanline.speed", propertyValue, "100");
    speed = atoi(propertyValue);

    lineHeight = imgHeight*height/800;
    displacement = (speed*drawCount/5)%(imgHeight-lineHeight);

    MY_LOGD("para:(w,h,s,VA,size)=(%d,%d,%d,0x%x,%d) prop:(v,h,w,s)=(%d,%d,%d,%d) line:(h,d)=(%d,%d)",
            imgWidth,
            imgHeight,
            imgStride,
            virtAddr,
            bufSize,
            value,
            width,
            height,
            speed,
            lineHeight,
            displacement);
            
    property_get( "debug.cam.scanline.fullscreen", propertyValue, "0");
    fullscreen = atoi(propertyValue);

    if(width > 0)
    {
        if (fullscreen)
        {
            for(i=0; i<imgHeight; i++)
            {
                memset((void*)(virtAddr+widthShift+imgStride*i),
                               value,
                               width);
            }
        }
        else
        {
            for(i=0; i<lineHeight; i++)
            {
                memset((void*)(virtAddr+widthShift+imgStride*displacement+imgStride*i),
                               value,
                               width);
            }
        }
    }
    else
    if (fullscreen)
    {
        memset((void*)virtAddr, value, bufSize);
    }
    else
    {
        memset((void*)(virtAddr+imgStride*displacement),
                       value,
                       imgStride*lineHeight);
    }
    drawCount += 1;
    return;
}
