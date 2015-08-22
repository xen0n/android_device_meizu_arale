#include <MtpTypes.h>
#include <system/audio.h>
#include <StrongPointer.h>
#include <media/IAudioFlinger.h>
#include <hardware/audio.h>
#include <stdlib.h>
#include <dlfcn.h>

extern "C"
{
	struct legacy_audio_module {
	    struct audio_module module;
	};

	static int legacy_adev_open(const hw_module_t *module, const char *name,
		                hw_device_t **device)
	{
		void *hdl;
		struct legacy_audio_module *hmi;

		hdl = dlopen("libaudio.primary.default.so", RTLD_NOW | RTLD_LOCAL);
		if (hdl == NULL) {
			ALOGE("MTK Audio loadLibrary() failed to open %s", dlerror());
			return -1;
		}

		hmi = (struct legacy_audio_module *)dlsym(hdl, HAL_MODULE_INFO_SYM_AS_STR);
		if (hmi == NULL) {
			ALOGE("MTK Audio loadLibrary() could not find symbol %s",HAL_MODULE_INFO_SYM_AS_STR);
			return -1;
		}

		return hmi->module.common.methods->open(module, name, device);
	}

	static struct hw_module_methods_t legacy_audio_module_methods = {
	open:
		legacy_adev_open
	};

	struct legacy_audio_module HAL_MODULE_INFO_SYM = {
	    module: {
		common: {
		    tag: HARDWARE_MODULE_TAG,
		    module_api_version: AUDIO_MODULE_API_VERSION_0_1,
		    hal_api_version: HARDWARE_HAL_API_VERSION,
		    id: AUDIO_HARDWARE_MODULE_ID,
		    name: "MTK Audio HW HAL",
		    author: "The Android Open Source Project",
		    methods: &legacy_audio_module_methods,
		    dso : NULL,
		    reserved : {0},
		},
	    },
	};
}
