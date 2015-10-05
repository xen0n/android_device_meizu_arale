#ifndef KEXEC_ARCH_ARM_MACH_H
#define KEXEC_ARCH_ARM_MACH_H

#include <sys/types.h>


struct arm_mach
{
    int (*choose_dtb)(const char *dtb_img, off_t dtb_len, char **dtb_buf, off_t *dtb_length);
    int (*add_extra_regs)(void *dtb_buf);
    char *const boardnames[];
};

struct arm_mach *arm_mach_choose(const char *boardname);

#endif
