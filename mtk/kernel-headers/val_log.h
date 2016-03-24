#ifndef _VAL_LOG_H_
#define _VAL_LOG_H_

#include <utils/Log.h>
#include <cutils/log.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#define MFV_LOG_ERROR           /* /< error */
#ifdef MFV_LOG_ERROR
#define MFV_LOGE(...) LOG_PRI(ANDROID_LOG_ERROR, "VDO_LOG", __VA_ARGS__);   /* /< show error log */
#define VDO_LOGE(...) LOG_PRI(ANDROID_LOG_ERROR, "VDO_LOG", __VA_ARGS__);   /* /< show error log */
#else
#define MFV_LOGE(...)           /* /< NOT show error log */
#define VDO_LOGE(...)           /* /< NOT show error log */
#endif

#define MFV_LOG_WARNING         /* /< warning */
#ifdef MFV_LOG_WARNING
#define MFV_LOGW(...) LOG_PRI(ANDROID_LOG_WARN, "VDO_LOG", __VA_ARGS__);    /* /< show warning log */
#define VDO_LOGW(...) LOG_PRI(ANDROID_LOG_WARN, "VDO_LOG", __VA_ARGS__);    /* /< show warning log */
#else
#define MFV_LOGW(...)           /* /< NOT show warning log */
#define VDO_LOGW(...)           /* /< NOT show warning log */
#endif

/* #define MFV_LOG_DEBUG         ///< debug information */
#ifdef MFV_LOG_DEBUG
#define MFV_LOGD(...) LOG_PRI(ANDROID_LOG_DEBUG, "VDO_LOG", __VA_ARGS__);   /* /< show debug information log */
#define VDO_LOGD(...) LOG_PRI(ANDROID_LOG_DEBUG, "VDO_LOG", __VA_ARGS__);   /* /< show debug information log */
#else
#define MFV_LOGD(...)           /* /< NOT show debug information log */
#define VDO_LOGD(...)           /* /< NOT show debug information log */
#endif

#define MFV_LOG_INFO            /* /< information */
#ifdef MFV_LOG_INFO
#define MFV_LOGI(...) LOG_PRI(ANDROID_LOG_INFO, "VDO_LOG", __VA_ARGS__);    /* /< show information log */
#define VDO_LOGI(...) LOG_PRI(ANDROID_LOG_INFO, "VDO_LOG", __VA_ARGS__);    /* /< show information log */
#else
#define MFV_LOGI(...)           /* /< NOT show information log */
#define VDO_LOGI(...)           /* /< NOT show information log */
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _VAL_LOG_H_ */
