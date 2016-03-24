#ifndef _TFA9887_HEADFILE_
#define _TFA9887_HEADFILE_

#ifdef __cplusplus
extern "C" {
#endif

int StopNxpSpeaker();
int StartUpNxpSpeaker(unsigned  int SampleRate);
void MtkTfa98xxBypassOn(void);
void MtkTfa98xxSpeakerOn(void);
void MtkTfa98xxSpeakerOff(void);

int tfa9887_check_tfaopen(void);
int  tfa9887_init(void);
int  tfa9887_deinit(void);
void tfa9887_SpeakerOn(void);
void tfa9887_SpeakerOff(void);
void tfa9887_reset(void);
void tfa9887_setSamplerate(int samplerate);
void tfa9887_set_bypass_dsp_incall(int bypass);
void tfa9887_EchoReferenceConfigure(int config);


#ifdef __cplusplus
}
#endif

#endif //_TFA9887_HEADFILE_


