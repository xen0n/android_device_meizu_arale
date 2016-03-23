#ifndef __CCCI_INTERFACE_H__
#define __CCCI_INTERFACE_H__

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/************************************* device node set *************************************/
typedef enum {
    USR_MUXD_DATA = 0,    /* path for muxd */
    USR_FACTORY_DATA = USR_MUXD_DATA,    /* muxd does not work under factory mode */
    USR_FACTORY_SIM = USR_MUXD_DATA,    /* SIM test in factory mode */
    USR_FACTORY_IDLE = USR_MUXD_DATA,    /* idle test in factory mode */
    USR_FACTORY_RF = USR_MUXD_DATA,    /* RF test in factory mode */
    USR_MUXR_IOCTL,        /* ioctl only path for muxreport (TRM, world phone) */
    USR_RILD_IOCTL,        /* ioctl only path for rild (flight mode, 3G switch) */
    USR_FACTORY_SIM_IOCTL = USR_RILD_IOCTL,    /* ioctl only path for SIM test in factory mode (flight mode) */
    USR_FACTORY_IDLE_IOCTL = USR_RILD_IOCTL,    /* ioctl only path for IDLE test in factory mode (flight mode) */
    USR_TERS_IOCTL,        /* ioctl only path for terservice (regional phone) */
    USR_AUDIO_TX,        /* Tx path for audio */
    USR_AUDIO_RX,        /* Rx path for audio, it's the same with Tx if only one node is used */
    USR_META_DATA,        /* path for meta */
    USR_META_IOCTL,        /* ioctl only path for meta */
    USR_MDLOG_TX,        /* Tx path for mdlogger */
    USR_MDLOG_RX,        /* Rx path for mdlogger, it's the same with Tx if only one node is used */
    USR_MDLOG_IOCTL,    /* ioctl only path for mdlogger */
    USR_MDLOG_CTRL,        /* data control path for mdlogger using DHL protocol */
    USR_MDLOG_MONITOR,    /* data control path for mdlogger using DHL protocol */
    USR_AGPS_UART,        /* path for AGPS */
    USR_AGPS_IPC,        /* IPC path for AGPS */
    USR_GPS_IPC,        /* IPC path for GPS */
    USR_NET_0,
    USR_NET_1,
    USR_NET_2,
    USR_NET_3,
    USR_NET_4,
    USR_NET_5,
    USR_NET_6,
    USR_NET_7,
    USR_IMS_AUDIO,        /* audio path for IMS */
    USR_IMS_VEDIO,        /* vedio path for IMS */
    USR_IMS_CTRL,        /* control path for IMS */
    USR_IMS_DCTRL,        /* data control path for IMS */
    USR_ICUSB_DATA,        /* path for icusbd */
    USR_ICUSB_IOCTL,    /* ioctl only path for icusbd */
    USR_CCCI_FS,        /* path for ccci_fsd */
    USR_CCCI_CTRL,        /* path for md_init, read only */
    USR_MISC_TA_IPC,    /* IPC path for Modem Misc TA */
    USR_CCCI_RPC,        /* path for ccci_rpcd */
    USR_C2K_FLS,        /*for c2k flashlessd */
    USR_C2K_AT,        /*for c2k rild */
    USR_C2K_AT2,        /*for c2k rild */
    USR_C2K_AT3,        /*for c2k rild */
    USR_CCCI_MAX,
} CCCI_USER;            /* *_IOCTL path is only for ioctl commands, other pathes are capable of both data and ioctl */

typedef enum {
    MD_SYS1 = 0,
    MD_SYS2 = 1,
    MD_SYS3 = 2,
    MD_SYS5 = 4,
} CCCI_MD;

typedef enum {
    INVALID = 0,
    CCCI,            /* for 6575/77 */
    DUAL_CCCI,        /* for 6589/82/72/92/71 */
    ECCCI,            /* for 6595 */
    EEMCS,
    EDSDA,            /* for 6595 dsda */
} CCCI_VERSION;

typedef enum {
    MD1_CCCI = 0,
    MD1_DUAL_CCCI,
    MD1_ECCCI,
    MD2_DUAL_CCCI,
    MD3_CCCI,
    MD5_EEMCS,
    MD5_EDSDA,
    MD_TYPE_MAX,
} __MD_TYPE;

struct ccci_node {
    CCCI_USER user_id;
    char node_name[MD_TYPE_MAX][32];    /* Attention, max length of device node name is 32 */
};

#define INVALID_NAME "/dev/null"

static struct ccci_node ccci_node_list[] = {
    /*ccci_user_id *//*md1_ccci_name *//*md1_dual_ccci_name *//*md1_eccci_name *//*md2_dual_ccci_name *//*md3_ccci_name *//*md5_eemcs_dev_name *//*md5_edsda_dev_name */
    {USR_MUXD_DATA,
     {"/dev/ttyC0", "/dev/ttyC0", "/dev/ttyC0", "/dev/ccci2_tty0",
      INVALID_NAME, "/dev/eemcs_mux", "/dev/emd_cfifo1"}},
    {USR_MUXR_IOCTL,
     {"/dev/ccci_uem_tx", "/dev/ccci_ioctl0", "/dev/ccci_ioctl0",
      "/dev/ccci2_ioctl0", INVALID_NAME, "/dev/eemcs_muxrp",
      "/dev/emd_ctl1"}},
    {USR_RILD_IOCTL,
     {INVALID_NAME, "/dev/ccci_ioctl1", "/dev/ccci_ioctl1",
      "/dev/ccci2_ioctl1", "/dev/ccci3_ioctl1", "/dev/eemcs_ril",
      "/dev/emd_ctl2"}},
    {USR_TERS_IOCTL,
     {INVALID_NAME, "/dev/ccci_ioctl2", "/dev/ccci_ioctl2",
      "/dev/ccci2_ioctl2", INVALID_NAME, "/dev/eemcs_ioctl", INVALID_NAME}},
    {USR_AUDIO_TX,
     {"/dev/ccci_pcm_tx", "/dev/ccci_pcm_tx", "/dev/ccci_aud",
      "/dev/ccci2_aud", "/dev/ccci3_aud", "/dev/eemcs_aud", INVALID_NAME}},
    {USR_AUDIO_RX,
     {"/dev/ccci_pcm_rx", "/dev/ccci_pcm_rx", "/dev/ccci_aud",
      "/dev/ccci2_aud", "/dev/ccci3_aud", "/dev/eemcs_aud", INVALID_NAME}},
    {USR_META_DATA,
     {"/dev/ttyC1", "/dev/ttyC1", "/dev/ttyC1", "/dev/ccci2_tty1",
      "/dev/ccci3_tty1", "/dev/eemcs_md_log", INVALID_NAME}},
    {USR_META_IOCTL,
     {INVALID_NAME, "/dev/ccci_ioctl4", "/dev/ccci_ioctl4",
      "/dev/ccci2_ioctl4", INVALID_NAME, INVALID_NAME, "/dev/emd_ctl3"}},
    {USR_MDLOG_TX,
     {"/dev/ccci_md_log_tx", "/dev/ccci_md_log_tx", "/dev/ttyC1",
      "/dev/ccci2_tty1", "/dev/ccci3_tty1", "/dev/eemcs_md_log",
      INVALID_NAME}},
    {USR_MDLOG_RX,
     {"/dev/ccci_md_log_rx", "/dev/ccci_md_log_rx", "/dev/ttyC1",
      "/dev/ccci2_tty1", "/dev/ccci3_tty1", "/dev/eemcs_md_log",
      INVALID_NAME}},
    {USR_MDLOG_IOCTL,
     {"/dev/ccci_uem_rx", "/dev/ccci_ioctl0", "/dev/ttyC1",
      "/dev/ccci2_tty1", "/dev/ccci3_tty1", "/dev/eemcs_md_log",
      "/dev/emd_ctl4"}},
    {USR_MDLOG_CTRL,
     {INVALID_NAME, INVALID_NAME, "/dev/ccci_md_log_ctrl",
      "/dev/ccci2_md_log_ctrl", "/dev/ccci3_md_log_ctrl",
      "/dev/eemcs_md_log_ctrl", "/dev/emd_ctl4"}},
    {USR_MDLOG_MONITOR,
     {INVALID_NAME, INVALID_NAME, "/dev/ccci_mdl_monitor", INVALID_NAME,
      INVALID_NAME, INVALID_NAME, INVALID_NAME}},
    {USR_AGPS_UART,
     {"/dev/ttyC2", "/dev/ttyC2", "/dev/ttyC2", "/dev/ccci2_tty2",
      "/dev/ccci3_tty2", "/dev/eemcs_ipc_uart", INVALID_NAME}},
    {USR_AGPS_IPC,
     {"/dev/ccci_ipc_1220_0", "/dev/ccci_ipc_1220_0",
      "/dev/ccci_ipc_1220_0", "/dev/ccci2_ipc_0", INVALID_NAME,
      "/dev/eemcs_ipc_0", INVALID_NAME}},
    {USR_GPS_IPC,
     {INVALID_NAME, "/dev/ccci_ipc_2", "/dev/ccci_ipc_2",
      "/dev/ccci2_ipc_2", INVALID_NAME, "/dev/eemcs_ipc_2", INVALID_NAME}},
    {USR_NET_0,
     {"ccmni0", "ccmni0", "ccmni0", "cc2mni0", "cc3mni0", "ccemni0",
      INVALID_NAME}},
    {USR_NET_1,
     {"ccmni1", "ccmni1", "ccmni1", "cc2mni1", "cc3mni1", "ccemni1",
      INVALID_NAME}},
    {USR_NET_2,
     {"ccmni2", "ccmni2", "ccmni2", "cc2mni2", "cc3mni2", "ccemni2",
      INVALID_NAME}},
    {USR_NET_3,
     {"ccmni3", "ccmni3", "ccmni3", "cc2mni3", "cc3mni3", "ccemni3",
      INVALID_NAME}},
    {USR_NET_4,
     {"ccmni4", "ccmni4", "ccmni4", "cc2mni4", "cc3mni4", "ccemni4",
      INVALID_NAME}},
    {USR_NET_5,
     {"ccmni5", "ccmni5", "ccmni5", "cc2mni5", "cc3mni5", "ccemni5",
      INVALID_NAME}},
    {USR_NET_6,
     {"ccmni6", "ccmni6", "ccmni6", "cc2mni6", "cc3mni6", "ccemni6",
      INVALID_NAME}},
    {USR_NET_7,
     {"ccmni7", "ccmni7", "ccmni7", "cc2mni7", "cc3mni7", "ccemni7",
      INVALID_NAME}},
    {USR_IMS_AUDIO,
     {INVALID_NAME, INVALID_NAME, "/dev/ccci_imsa", "/dev/ccci2_imsa",
      INVALID_NAME, "/dev/eemcs_imsa", INVALID_NAME}},
    {USR_IMS_VEDIO,
     {INVALID_NAME, INVALID_NAME, "/dev/ccci_imsv", "/dev/ccci2_imsa",
      INVALID_NAME, "/dev/eemcs_imsv", INVALID_NAME}},
    {USR_IMS_CTRL,
     {INVALID_NAME, INVALID_NAME, "/dev/ccci_imsc", "/dev/ccci2_imsa",
      INVALID_NAME, "/dev/eemcs_imsc", INVALID_NAME}},
    {USR_IMS_DCTRL,
     {INVALID_NAME, INVALID_NAME, "/dev/ccci_imsdc", "/dev/ccci2_imsa",
      INVALID_NAME, "/dev/eemcs_imsdc", INVALID_NAME}},
    {USR_ICUSB_DATA,
     {INVALID_NAME, "/dev/ttyC3", "/dev/ttyC3", "/dev/ccci2_tty3",
      INVALID_NAME, INVALID_NAME, INVALID_NAME}},
    {USR_ICUSB_IOCTL,
     {INVALID_NAME, "/dev/ccci_ioctl3", "/dev/ccci_ioctl3",
      "/dev/ccci2_ioctl3", INVALID_NAME, INVALID_NAME, INVALID_NAME}},
    {USR_CCCI_FS,
     {"/dev/ccci_fs", "/dev/ccci_fs", "/dev/ccci_fs", "/dev/ccci2_fs",
      "/dev/ccci3_fs", "/dev/eemcs_fs", INVALID_NAME}},
    {USR_CCCI_CTRL,
     {"/dev/ccci_uem_rx", "/dev/ccci_monitor", "/dev/ccci_monitor",
      "/dev/ccci2_monitor", "/dev/ccci3_monitor", "/dev/eemcs_ctrl",
      "/dev/emd_ctl0"}},
    {USR_MISC_TA_IPC,
     {INVALID_NAME, INVALID_NAME, "/dev/ccci_ipc_4", "/dev/ccci2_ipc_4",
      INVALID_NAME, INVALID_NAME, INVALID_NAME}},
    {USR_CCCI_RPC,
     {"/dev/ccci_rpc", "/dev/ccci_rpc", "/dev/ccci_rpc", "/dev/ccci2_rpc",
      INVALID_NAME, "/dev/eemcs_rpc", INVALID_NAME}},
    {USR_C2K_FLS,
     {INVALID_NAME, INVALID_NAME, INVALID_NAME, INVALID_NAME,
      "/dev/ccci3_fls", INVALID_NAME, INVALID_NAME}},
    {USR_C2K_AT,
     {INVALID_NAME, INVALID_NAME, INVALID_NAME, INVALID_NAME,
      "/dev/ccci3_at", INVALID_NAME, INVALID_NAME}},
    {USR_C2K_AT2,
     {INVALID_NAME, INVALID_NAME, INVALID_NAME, INVALID_NAME,
      "/dev/ccci3_at2", INVALID_NAME, INVALID_NAME}},
    {USR_C2K_AT3,
     {INVALID_NAME, INVALID_NAME, INVALID_NAME, INVALID_NAME,
      "/dev/ccci3_at3", INVALID_NAME, INVALID_NAME}},
};

/**
 * ccci_get_version - get version of CCCI driver
 *
 * RETURNS:
 * see the defination of CCCI_VERSION
 */
static inline CCCI_VERSION ccci_get_version(void)
{
    int fd, ret;
    char ver_str[2];
    fd = open("/sys/kernel/ccci/version", O_RDONLY);
    if (fd < 0)
        return CCCI;
    ret = read(fd, ver_str, sizeof(ver_str));
    close(fd);
    if (ret < 1)
        return INVALID;
    return (CCCI_VERSION) (ver_str[0] - '0');
}

/**
 * ccci_get_node_name() - find the device node of a certain modem for a
 * certain user.
 * @usr_id: ID of the caller
 * @md_id: ID of the modem you want to operate on
 *
 * RETURNS:
 * pointer to a suitable device node name string
 *
 * Attention, here we assume ccci_node_list[] use the index as CCCI_USER,
 * if this assumption is broken, rewrite this funtion.
 */
static inline char *ccci_get_node_name(CCCI_USER usr_id, CCCI_MD md_id)
{
    char *name = NULL;

    switch (md_id) {
    case MD_SYS1:
        switch (ccci_get_version()) {
        case CCCI:
            name = ccci_node_list[usr_id].node_name[MD1_CCCI];
            break;
        case DUAL_CCCI:
            name = ccci_node_list[usr_id].node_name[MD1_DUAL_CCCI];
            break;
        case EDSDA:
        case ECCCI:
            name = ccci_node_list[usr_id].node_name[MD1_ECCCI];
            break;
        case INVALID:
        default:
            name = NULL;
            break;
        };
        break;
    case MD_SYS2:
        name = ccci_node_list[usr_id].node_name[MD2_DUAL_CCCI];
        break;
    case MD_SYS3:
        name = ccci_node_list[usr_id].node_name[MD3_CCCI];
        break;
    case MD_SYS5:
        switch (ccci_get_version()) {
        case EDSDA:
            name = ccci_node_list[usr_id].node_name[MD5_EDSDA];
            break;
        default:
            name = ccci_node_list[usr_id].node_name[MD5_EEMCS];
            break;
        }
        break;
    default:
        name = NULL;
        break;
    };
    if (name && strcmp(name, INVALID_NAME) == 0)
        name = NULL;
    return name;
}

/************************************* IOCTL command set *************************************/
/* CCCI == EEMCS*/
#define CCCI_IOC_MAGIC 'C'
#define CCCI_IOC_MD_RESET                _IO(CCCI_IOC_MAGIC, 0) /* mdlogger META/muxreport */
#define CCCI_IOC_GET_MD_STATE            _IOR(CCCI_IOC_MAGIC, 1, unsigned int) /* audio */
#define CCCI_IOC_PCM_BASE_ADDR            _IOR(CCCI_IOC_MAGIC, 2, unsigned int) /* audio */
#define CCCI_IOC_PCM_LEN                _IOR(CCCI_IOC_MAGIC, 3, unsigned int) /* audio */
#define CCCI_IOC_FORCE_MD_ASSERT        _IO(CCCI_IOC_MAGIC, 4) /* muxreport  mdlogger */
#define CCCI_IOC_ALLOC_MD_LOG_MEM        _IO(CCCI_IOC_MAGIC, 5) /* mdlogger */
#define CCCI_IOC_DO_MD_RST                _IO(CCCI_IOC_MAGIC, 6) /* md_init */
#define CCCI_IOC_SEND_RUN_TIME_DATA        _IO(CCCI_IOC_MAGIC, 7) /* md_init */
#define CCCI_IOC_GET_MD_INFO            _IOR(CCCI_IOC_MAGIC, 8, unsigned int) /* md_init */
#define CCCI_IOC_GET_MD_EX_TYPE            _IOR(CCCI_IOC_MAGIC, 9, unsigned int) /* mdlogger */
#define CCCI_IOC_SEND_STOP_MD_REQUEST    _IO(CCCI_IOC_MAGIC, 10) /* muxreport */
#define CCCI_IOC_SEND_START_MD_REQUEST    _IO(CCCI_IOC_MAGIC, 11) /* muxreport */
#define CCCI_IOC_DO_STOP_MD                _IO(CCCI_IOC_MAGIC, 12) /* md_init */
#define CCCI_IOC_DO_START_MD            _IO(CCCI_IOC_MAGIC, 13) /* md_init */
#define CCCI_IOC_ENTER_DEEP_FLIGHT        _IO(CCCI_IOC_MAGIC, 14) /* RILD  factory */
#define CCCI_IOC_LEAVE_DEEP_FLIGHT        _IO(CCCI_IOC_MAGIC, 15) /* RILD  factory */
#define CCCI_IOC_POWER_ON_MD            _IO(CCCI_IOC_MAGIC, 16) /* md_init */
#define CCCI_IOC_POWER_OFF_MD            _IO(CCCI_IOC_MAGIC, 17) /* md_init */
#define CCCI_IOC_POWER_ON_MD_REQUEST    _IO(CCCI_IOC_MAGIC, 18)
#define CCCI_IOC_POWER_OFF_MD_REQUEST    _IO(CCCI_IOC_MAGIC, 19)
#define CCCI_IOC_SIM_SWITCH                _IOW(CCCI_IOC_MAGIC, 20, unsigned int) /* RILD  factory */
#define CCCI_IOC_SEND_BATTERY_INFO        _IO(CCCI_IOC_MAGIC, 21) /* md_init */
#define CCCI_IOC_SIM_SWITCH_TYPE        _IOR(CCCI_IOC_MAGIC, 22, unsigned int) /* RILD */
#define CCCI_IOC_STORE_SIM_MODE            _IOW(CCCI_IOC_MAGIC, 23, unsigned int)    /* RILD */
#define CCCI_IOC_GET_SIM_MODE            _IOR(CCCI_IOC_MAGIC, 24, unsigned int)    /* RILD */
#define CCCI_IOC_RELOAD_MD_TYPE            _IO(CCCI_IOC_MAGIC, 25)    /* META  md_init // muxreport */
#define CCCI_IOC_GET_SIM_TYPE            _IOR(CCCI_IOC_MAGIC, 26, unsigned int)    /* terservice */
#define CCCI_IOC_ENABLE_GET_SIM_TYPE    _IOW(CCCI_IOC_MAGIC, 27, unsigned int)    /* terservice */
#define CCCI_IOC_SEND_ICUSB_NOTIFY        _IOW(CCCI_IOC_MAGIC, 28, unsigned int)    /* icusbd */
#define CCCI_IOC_SET_MD_IMG_EXIST        _IOW(CCCI_IOC_MAGIC, 29, unsigned int)    /* md_init */
#define CCCI_IOC_GET_MD_IMG_EXIST        _IOR(CCCI_IOC_MAGIC, 30, unsigned int)    /* META */
#define CCCI_IOC_GET_MD_TYPE            _IOR(CCCI_IOC_MAGIC, 31, unsigned int)    /* RILD */
#define CCCI_IOC_STORE_MD_TYPE            _IOW(CCCI_IOC_MAGIC, 32, unsigned int)    /* RILD */
#define CCCI_IOC_GET_MD_TYPE_SAVING        _IOR(CCCI_IOC_MAGIC, 33, unsigned int)    /* META */
#define CCCI_IOC_GET_EXT_MD_POST_FIX    _IOR(CCCI_IOC_MAGIC, 34, unsigned int)    /* char[32] emcs_fsd  mdlogger */
#define CCCI_IOC_FORCE_FD                _IOW(CCCI_IOC_MAGIC, 35, unsigned int)    /* RILD(6577) */
#define CCCI_IOC_AP_ENG_BUILD            _IOW(CCCI_IOC_MAGIC, 36, unsigned int)    /* md_init(6577) */
#define CCCI_IOC_GET_MD_MEM_SIZE        _IOR(CCCI_IOC_MAGIC, 37, unsigned int)    /* md_init(6577) */
#define CCCI_IOC_UPDATE_SIM_SLOT_CFG    _IOW(CCCI_IOC_MAGIC, 38, unsigned int)    /* RILD */
#define CCCI_IOC_GET_CFG_SETTING        _IOW(CCCI_IOC_MAGIC, 39, unsigned int)    /* md_init */
#define CCCI_IOC_SET_MD_SBP_CFG         _IOW(CCCI_IOC_MAGIC, 40, unsigned int)    /* md_init */
#define CCCI_IOC_GET_MD_SBP_CFG         _IOW(CCCI_IOC_MAGIC, 41, unsigned int)    /* md_init */
#define CCCI_IOC_GET_MD_PROTOCOL_TYPE    _IOR(CCCI_IOC_MAGIC, 42, char[16])    /*metal tool to get modem protocol type: AP_TST or DHL */
#define CCCI_IOC_SEND_SIGNAL_TO_USER    _IOW(CCCI_IOC_MAGIC, 43, unsigned int)    /* md_init */
#define CCCI_IOC_IGNORE_MD_EXCP         _IO(CCCI_IOC_MAGIC, 44)    /* RILD */
#define CCCI_IOC_RESET_MD1_MD3_PCCIF    _IO(CCCI_IOC_MAGIC, 45)    /* md_init */
#define CCCI_IOC_RESET_AP                 _IOW(CCCI_IOC_MAGIC, 46, unsigned int)    /* md_init */

/* EEMCS only*/
#define CCCI_IOC_BOOT_MD                _IO(CCCI_IOC_MAGIC,  100) /*mdinit */
#define CCCI_IOC_GATE_MD                _IO(CCCI_IOC_MAGIC,  101) /*nouser */
#define CCCI_IOC_ASSERT_MD                _IO(CCCI_IOC_MAGIC,  102) /*nouser */
#define CCCI_IOC_CHECK_STATE            _IOR(CCCI_IOC_MAGIC, 103, unsigned int) /*nouser */
#define CCCI_IOC_SET_STATE                _IOW(CCCI_IOC_MAGIC, 104, unsigned int)/*nouser */
#define CCCI_IOC_GET_MD_BOOT_INFO        _IOR(CCCI_IOC_MAGIC, 105, unsigned int)/*mdinit */
#define CCCI_IOC_START_BOOT                _IO(CCCI_IOC_MAGIC,  106) /*mdinit */
#define CCCI_IOC_BOOT_DONE                _IO(CCCI_IOC_MAGIC,  107) /*mdinit */
#define CCCI_IOC_REBOOT                    _IO(CCCI_IOC_MAGIC,  108) /*mdinit */
#define CCCI_IOC_MD_EXCEPTION            _IO(CCCI_IOC_MAGIC,  109) /*nouser */
#define CCCI_IOC_MD_EX_REC_OK            _IO(CCCI_IOC_MAGIC,  110) /*nouser */
#define CCCI_IOC_GET_RUNTIME_DATA        _IOR(CCCI_IOC_MAGIC, 111, char[1024])    /*mdinit */
#define CCCI_IOC_SET_HEADER                _IO(CCCI_IOC_MAGIC,  112) /*UT    */
#define CCCI_IOC_CLR_HEADER                _IO(CCCI_IOC_MAGIC,  113) /*UT    */
#define CCCI_IOC_SET_EXCEPTION_DATA        _IOW(CCCI_IOC_MAGIC, 114, char[1024]) /*nouser */
#define CCCI_IOC_GET_EXCEPTION_LENGTH    _IOR(CCCI_IOC_MAGIC, 115, unsigned int) /*nouser */
#define CCCI_IOC_SET_BOOT_STATE            _IOW(CCCI_IOC_MAGIC, 116, unsigned int) /*boot_IT */
#define CCCI_IOC_GET_BOOT_STATE            _IOR(CCCI_IOC_MAGIC, 117, unsigned int) /*boot_IT */
#define CCCI_IOC_WAIT_RDY_RST            _IO(CCCI_IOC_MAGIC,  118) /*mdinit For MD reset flow must wait for mux/fsd/mdlogger close port */
#define CCCI_IOC_DL_TRAFFIC_CONTROL        _IOW(CCCI_IOC_MAGIC, 119, unsigned int)    /* For MD LOGER to turn on/off downlink traffic */
#define CCCI_IOC_FLOW_CTRL_SETTING      _IOW(CCCI_IOC_MAGIC, 120, unsigned int)    /*set flow control limit&threshold of mdlog for debug */
#define CCCI_IOC_BOOT_UP_TIMEOUT        _IOW(CCCI_IOC_MAGIC, 121, unsigned int[2])    /*notify kernel of md boot up timeout */
#define CCCI_IOC_SET_BOOT_TO_VAL    _IOW(CCCI_IOC_MAGIC, 122, unsigned int)  /* Set boot time out value */

/* ext md*/
/*Block to get extern md assert flag for mdlogger */
#define CCCI_IOC_GET_MD_ASSERTLOG       _IOW(CCCI_IOC_MAGIC, 200, unsigned int)
/*get extern md assert log status */
#define CCCI_IOC_GET_MD_ASSERTLOG_STATUS  _IOW(CCCI_IOC_MAGIC, 201, unsigned int)
/*Enter dwonload extern md img mode */
#define CCCI_IOC_ENTER_MD_DL_MODE         _IOW(CCCI_IOC_MAGIC, 202, unsigned int)

#define CCCI_IPC_MAGIC 'P'    /* only for IPC user */
/* CCCI == EEMCS*/
#define CCCI_IPC_RESET_RECV        _IO(CCCI_IPC_MAGIC,0)
#define CCCI_IPC_RESET_SEND        _IO(CCCI_IPC_MAGIC,1)
#define CCCI_IPC_WAIT_MD_READY    _IO(CCCI_IPC_MAGIC,2)
#define CCCI_IPC_KERN_WRITE_TEST    _IO(CCCI_IPC_MAGIC,3)
#define CCCI_IPC_UPDATE_TIME        _IO(CCCI_IPC_MAGIC,4)
#define CCCI_IPC_WAIT_TIME_UPDATE    _IO(CCCI_IPC_MAGIC,5)
#define CCCI_IPC_UPDATE_TIMEZONE    _IO(CCCI_IPC_MAGIC,6)

#define CCCI_FS_IOC_MAGIC 'K'    /* only for ccci_fsd */
/* CCCI only*/
#define CCCI_FS_IOCTL_GET_INDEX    _IO(CCCI_FS_IOC_MAGIC, 1)
#define CCCI_FS_IOCTL_SEND        _IOR(CCCI_FS_IOC_MAGIC, 2, unsigned int)

#define CCCI_RPC_IOC_MAGIC        'R'
#define CCCI_RPC_IOCTL_GET_INDEX    _IO(CCCI_RPC_IOC_MAGIC, 1)
#define CCCI_RPC_IOCTL_SEND        _IOR(CCCI_RPC_IOC_MAGIC, 2, unsigned int)

/* API exported by ccci so*/
int query_kcfg_setting(char name[], char val[], int size);
int query_prj_cfg_setting(char name[], char val[], int size);

#endif /*__CCCI_INTERFACE_H__    */
