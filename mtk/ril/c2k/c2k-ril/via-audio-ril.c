#include "via-audio-ril.h"
#include <alsa/asoundlib.h>

#define LOG_TAG "Audio-RIL"
#include <utils/Log.h>


#define AUDIO_CODEC_ANAMIC_GAIN_HANDSET_L 1
#define AUDIO_CODEC_ANAMIC_GAIN_HANDSET_R 1
#define AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_L 1
#define AUDIO_CODEC_ANAMIC_GAIN_HANDFREE_R 1
#define AUDIO_CODEC_ANAMIC_GAIN_HEADSET_L 1
#define AUDIO_CODEC_ANAMIC_GAIN_HEADSET_R 1

#define AUDIO_CODEC_TXL2_VOL_HANDSET 16
#define AUDIO_CODEC_TXR2_VOL_HANDSET 16
#define AUDIO_CODEC_TXL2_VOL_HANDFREE 16
#define AUDIO_CODEC_TXR2_VOL_HANDFREE 16
#define AUDIO_CODEC_TXL2_VOL_HEADSET 16
#define AUDIO_CODEC_TXR2_VOL_HEADSET 16

#define AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDSET 42
#define AUDIO_CODEC_VOICE_DIGITAL_VOL_HANDFREE 42
#define AUDIO_CODEC_VOICE_DIGITAL_VOL_HEADSET 42
#define AUDIO_CODEC_VOICE_DIGITAL_VOL_BLUETOOTH 12

#define AUDIO_CODEC_SIDETONE_GAIN_HANDSET 20
#define AUDIO_CODEC_SIDETONE_GAIN_HANDFREE 20
#define AUDIO_CODEC_SIDETONE_GAIN_HEADSET 20
#define AUDIO_CODEC_SIDETONE_GAIN_BLUETOOTH 20

#define AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDSET 13
#define AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HANDFREE 25
#define AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_HEADSET 25
#define AUDIO_CODEC_VOICE_AUDIO_MIX_VOL_EARPIECE 22

#define AUDIO_CODEC_VOICE_DIGITAL_CAPTURE_VOL_BLUETOOTH 12

int alsa_set_index(snd_ctl_t * handle,const char *name, unsigned int value, int index)
{
    int ret = 0;
    int count = 0;
    int i = 0;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;
    snd_ctl_elem_value_t *control;
    snd_ctl_elem_type_t type;
    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);
    if (!handle) {
        LOGE("Control not initialized");
        return -1;
    }


    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, name);
    snd_ctl_elem_info_set_id(info, id);

    ret = snd_ctl_elem_info(handle, info);
    if (ret < 0) {
        LOGE("Control '%s' cannot get element info: %d", name, ret);
        return -EINVAL;
    }

    count = snd_ctl_elem_info_get_count(info);
    if (index >= count) {
        LOGE("Control '%s' index is out of range (%d >= %d)", name, index, count);
        return -EINVAL;
    }
    if (index == -1)
        index = 0; // Range over all of them
    else
        count = index + 1; // Just do the one specified

    type = snd_ctl_elem_info_get_type(info);


    snd_ctl_elem_value_alloca(&control);

    snd_ctl_elem_info_get_id(info, id);
    snd_ctl_elem_value_set_id(control, id);

    for (i = 0; i < count; i++)
        switch (type) {
            case SND_CTL_ELEM_TYPE_BOOLEAN:
                snd_ctl_elem_value_set_boolean(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_INTEGER:
                snd_ctl_elem_value_set_integer(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_INTEGER64:
                snd_ctl_elem_value_set_integer64(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_ENUMERATED:
                snd_ctl_elem_value_set_enumerated(control, i, value);
                break;
            case SND_CTL_ELEM_TYPE_BYTES:
                snd_ctl_elem_value_set_byte(control, i, value);
                break;
            default:
                break;
        }

    ret = snd_ctl_elem_write(handle, control);
    return (ret < 0) ? -EINVAL : 1;
}

static int alsa_set(snd_ctl_t *  handle, const char *name, const char *value)
{
    int ret = 0;
    int i = 0;
    int items = 0;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_info_t *info;

    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_info_alloca(&info);

    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, name);
    snd_ctl_elem_info_set_id(info, id);

    ret = snd_ctl_elem_info(handle, info);
    if (ret < 0) {
        LOGE("Control '%s' cannot get element info: %d", name, ret);
        return -EINVAL;
    }
    items = snd_ctl_elem_info_get_items(info);

    for (i = 0; i < items; i++) {
        snd_ctl_elem_info_set_item(info, i);
        ret = snd_ctl_elem_info(handle, info);
        if (ret < 0) continue;
        if (strcmp(value, snd_ctl_elem_info_get_item_name(info)) == 0){
            return alsa_set_index(handle, name, i, -1);
            }
    }
    LOGE("Control '%s' has no enumerated value of '%s'", name, value);

    return -EINVAL;
}

int set_twl5030_path(int path)
{
    int ret = 0;
    snd_ctl_t *             mHandle;
    LOGD("%s %d path = %d",__FUNCTION__,__LINE__,path);
    ret = snd_ctl_open(&mHandle, "hw:00", 0);
    if (!mHandle) {
        LOGE("Control not initialized");
        return -1;
    }
    switch(path) {
        case VOICE_EARPIECE:
            LOGD("set earpiece");
#if 1
            //ret = alsa_set_index(mHandle, "Analog Capture Main Mic Switch", 1, 0);
            //ret = alsa_set_index(mHandle, "Main Microphone Bias Switch", 1, 0);
            ret = alsa_set_index(mHandle, "Earpiece Mixer AudioL2", 1,-1); // on
            //ret = alsa_set_index(mHandle, "Earpiece Playback Volume", 2, 0); //analog level 2 gain 0x02=6 db ??????????????3
#endif
			break;
        case VOICE_HEADSET:
            LOGD("set headset");
            usleep(20*1000);
            ret = alsa_set_index(mHandle, "ARXL2 DAC APGA EN", 0, 0);
            ret = alsa_set_index(mHandle, "ARXR2 DAC APGA EN", 0, 0);
            ret = alsa_set(mHandle, "Voice Sample Rate","8 kHz");
            ret = alsa_set(mHandle, "Codec Operation Mode", "Option 2 (voice/audio)");
            ret = alsa_set(mHandle, "Voice Clock Mode", "Slave Mode");
            ret = alsa_set(mHandle, "Voice I/O Swap", "VDX/VDR not swapped");
            ret = alsa_set(mHandle, "Voice Interface Mode", "Mode 1 (writing on PCM_VCK rising edge)");
            ret = alsa_set_index(mHandle, "Voice Tristate Switch", 0, 0);
            ret = alsa_set_index(mHandle, "Voice Input Switch", 1, 0);
            ret = alsa_set_index(mHandle, "Voice Ouput Switch", 1, 0);
            ret = alsa_set_index(mHandle, "Voice Switch", 1, 0);
            ret = alsa_set_index(mHandle, "Analog Capture Headset Mic Switch", 1, 0);
            ret = alsa_set_index(mHandle, "Headset Microphone Bias Switch", 1, 0);
            ret = alsa_set_index(mHandle, "ARXL2 DAC APGA EN", 1, 0);
            ret = alsa_set_index(mHandle, "ARXR2 DAC APGA EN", 1, 0);
            ret = alsa_set_index(mHandle, "HeadsetR Mixer AudioR2", 1, -1); // on
            ret = alsa_set_index(mHandle, "HeadsetL Mixer AudioL2", 1, -1); // on
            break;
        case VOICE_SPEAKER:
            LOGD("set speaker");
            usleep(20*1000);
            ret = alsa_set_index(mHandle, "Analog Capture Main Mic Switch", 1, 0);
            ret = alsa_set_index(mHandle, "Main Microphone Bias Switch", 1, 0);
            ret = alsa_set_index(mHandle, "HandsfreeR Switch", 1, -1); // on
            ret = alsa_set_index(mHandle, "HandsfreeL Switch", 1, -1); // on
            ret = alsa_set(mHandle, "HandsfreeR Mux", "AudioR2");
            ret = alsa_set(mHandle, "HandsfreeL Mux", "AudioL2");
            break;
    }
	if (mHandle) snd_ctl_close(mHandle);
    return ret;
}
