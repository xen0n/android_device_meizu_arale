#ifndef __VOW_H__
#define __VOW_H__

/***********************************************************************************
** VOW Control Message
************************************************************************************/
#define VOW_DEVNAME "vow"
#define VOW_IOC_MAGIC    'a'

static char const *const kVOWDeviceName = "/dev/vow";

#if 0
//below is control message
#define TEST_VOW_PRINT            _IO(VOW_IOC_MAGIC,  0x00)
#define VOWEINT_GET_BUFSIZE       _IOW(VOW_IOC_MAGIC, 0x01, unsigned long)
#define VOW_GET_STATUS            _IOW(VOW_IOC_MAGIC, 0x02, unsigned long)
#define VOW_SET_CONTROL           _IOW(VOW_IOC_MAGIC, 0x03, unsigned long)
#define VOW_SET_SPEAKER_MODEL     _IOW(VOW_IOC_MAGIC, 0x04, unsigned long)
#define VOW_CLR_SPEAKER_MODEL     _IOW(VOW_IOC_MAGIC, 0x05, unsigned long)
#define VOW_SET_INIT_MODEL        _IOW(VOW_IOC_MAGIC, 0x06, unsigned long)
#define VOW_SET_FIR_MODEL         _IOW(VOW_IOC_MAGIC, 0x07, unsigned long)
#define VOW_SET_NOISE_MODEL       _IOW(VOW_IOC_MAGIC, 0x08, unsigned long)
#define VOW_SET_APREG_INFO        _IOW(VOW_IOC_MAGIC, 0x09, unsigned long)
#define VOW_SET_REG_MODE          _IOW(VOW_IOC_MAGIC, 0x0A, unsigned long)
#define VOW_FAKE_WAKEUP           _IOW(VOW_IOC_MAGIC, 0x0B, unsigned long)
#else


//below is control message
#define TEST_VOW_PRINT            0x00
#define VOWEINT_GET_BUFSIZE       0x01
#define VOW_GET_STATUS            0x02
#define VOW_SET_CONTROL           0x03
#define VOW_SET_SPEAKER_MODEL     0x04
#define VOW_CLR_SPEAKER_MODEL     0x05
#define VOW_SET_INIT_MODEL        0x06
#define VOW_SET_FIR_MODEL         0x07
#define VOW_SET_NOISE_MODEL       0x08
#define VOW_SET_APREG_INFO        0x09
#define VOW_SET_REG_MODE          0x0A
#define VOW_FAKE_WAKEUP           0x0B

#endif



/***********************************************************************************
** Type Define
************************************************************************************/
enum VOW_Control_Cmd
{
    VOWControlCmd_Init = 0,
    VOWControlCmd_ReadVoiceData,
    VOWControlCmd_EnableDebug,
    VOWControlCmd_DisableDebug,
};

typedef enum VOW_REG_MODE_T {
   VOW_MODE_MD32_VOW = 0,
   VOW_MODE_VOICECOMMAND,
   VOW_MODE_MULTIPLE_KEY,
   VOW_MODE_MULTIPLE_KEY_VOICECOMMAND
}VOW_REG_MODE_T;

typedef enum VOW_EINT_STATUS{
    VOW_EINT_DISABLE = -2,
    VOW_EINT_FAIL = -1,
    VOW_EINT_PASS = 0,
    VOW_EINT_RETRY = 1,
    NUM_OF_VOW_EINT_STATUS
}VOW_EINT_STATUS;

typedef enum VOW_FLAG_TYPE{
    VOW_FLAG_DEBUG,
    VOW_FLAG_PRE_LEARN,
    VOW_FLAG_DMIC_LOWPOWER,
    NUM_OF_VOW_FLAG_TYPE
}VOW_FLAG_TYPE;

typedef enum VOW_PWR_STATUS{
    VOW_PWR_OFF = 0,
    VOW_PWR_ON = 1,
    NUM_OF_VOW_PWR_STATUS
}VOW_PWR_STATUS;

typedef enum VOW_IPI_RESULT
{
   VOW_IPI_SUCCESS = 0,
   VOW_IPI_CLR_SMODEL_ID_NOTMATCH,
   VOW_IPI_SET_SMODEL_NO_FREE_SLOT,
}VOW_IPI_RESULT;
/*
struct VOW_EINT_DATA_STRUCT{
   int size;        // size of data section
   int eint_status; // eint status
   int id;
   char *data;      // reserved for future extension
}VOW_EINT_DATA_STRUCT;
*/

typedef struct{
   long  id;
   long  addr;
   long  size;
   void *data;
}VOW_MODEL_INFO_T;

#endif //__VOW_H__
