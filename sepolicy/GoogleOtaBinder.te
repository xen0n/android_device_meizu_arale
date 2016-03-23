# ==============================================
# Policy File of /system/binGoogleOtaBinder Executable File 


# ==============================================
# Type Declaration
# ==============================================

type GoogleOtaBinder_exec , exec_type, file_type;
type GoogleOtaBinder ,domain;

# ==============================================
# Android Policy Rule
# ==============================================

# ==============================================
# NSA Policy Rule
# ==============================================

# ==============================================
# MTK Policy Rule
# ==============================================

#permissive GoogleOtaBinder;
init_daemon_domain(GoogleOtaBinder)
#unconfined_domain(GoogleOtaBinder)

# Date : 2014/09/10
# Operation : Migration
# Purpose : allow Binder IPC
binder_use(GoogleOtaBinder)
binder_service(GoogleOtaBinder)

allow GoogleOtaBinder ota_agent_service:service_manager add;
# /dev/block/mmcblko
allow GoogleOtaBinder para_block_device:blk_file { write read open };
allow GoogleOtaBinder mmcblk0_block_device:blk_file { write read open };

allow GoogleOtaBinder block_device:dir search;
#/dev/misc
allow GoogleOtaBinder misc_device:chr_file { write read open ioctl};
#for kmsg
allow GoogleOtaBinder kmsg_device:chr_file { write read open ioctl};
#for nand
allow GoogleOtaBinder mtd_device:dir search;
allow GoogleOtaBinder mtd_device:chr_file { read write open rw_file_perms};

