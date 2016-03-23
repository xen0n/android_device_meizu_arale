#ifndef ANDROID_MTK_GRAPHIC_BUFFER_UTIL_H
#define ANDROID_MTK_GRAPHIC_BUFFER_UTIL_H
#include <utils/Singleton.h>

#include <ui/PixelFormat.h>

#include <hardware/gralloc_extra.h>

namespace android {
// ---------------------------------------------------------------------------
class GraphicBuffer;

class BufferInfo
{
private:
    int mErr;

public:
    int mWidth;
    int mHeight;
    int mStride;
    int mFormat;
    buffer_handle_t mHandle;

    BufferInfo() : mErr(NO_ERROR)
    {
    }

    status_t getInfo(const buffer_handle_t& handle);
    status_t getInfo(const sp<GraphicBuffer>& gb);
};

class GraphicBufferUtil : public Singleton<GraphicBufferUtil>
{
public:
    static inline GraphicBufferUtil& get() { return getInstance(); }

    /** @brief dump the GraphicBuffer
     *
     * @param prefix the prefix of file name
     * @param dir the stored directory, default is "/data/"
     */
    void dump(const buffer_handle_t& handle, const char* prefix = "", const char* dir = "/data/");
    void dump(const sp<GraphicBuffer>& gb, const char* prefix = "", const char* dir = "/data/");
    void dump(const BufferInfo &handle, const char* prefix = "", const char* dir = "/data/");

    uint32_t getBitsPerPixel(int format);
    uint32_t getBitsPerPixel(buffer_handle_t handle);
    int getRealFormat(buffer_handle_t handle, PixelFormat* format);

    int drawLine(const buffer_handle_t& handle, uint8_t val, int ptn_w, int ptn_h, int pos);
    int drawLine(const sp<GraphicBuffer>& gb, uint8_t val, int ptn_w, int ptn_h, int pos);
    int drawLine(const BufferInfo &handle, uint8_t val, int ptn_w, int ptn_h, int pos);

private:
    friend class Singleton<GraphicBufferUtil>;

    GraphicBufferUtil();
    ~GraphicBufferUtil();
};

inline GraphicBufferUtil& getGraphicBufferUtil() {
    return GraphicBufferUtil::get();
}
// ---------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_MTK_GRAPHIC_BUFFER_UTIL_H
