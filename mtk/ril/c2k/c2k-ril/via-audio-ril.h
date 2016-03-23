typedef enum {
    VOICE_EARPIECE = 3,
    VOICE_HEADSET = 4,
    VOICE_BLUETOOTH = 6,
    VOICE_SPEAKER     = 5,
} CP_VOICE_TYPE;               // according to VIA AT Spec

int set_twl5030_path(int path);
