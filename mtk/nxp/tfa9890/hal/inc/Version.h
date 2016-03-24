#ifndef VERSION_H
#define VERSION_H

#include "ProductVersion.h"

#define VER_FILEVERSION             TFA98XX_HAL_REV_MAJOR,TFA98XX_HAL_REV_MINOR
#define VER_FILEVERSION_STR         TFA98XX_HAL_REV_STR "\0"

#ifdef _X64
#define VER_FILEDESCRIPTION_STR     "TFA98xx HAL 64 bit I2C interface\0"
#else   //_X64
#define VER_FILEDESCRIPTION_STR     "TFA98xx HAL 32 bit I2C interface\0"
#endif  //_X64

#define VER_INTERNALNAME_STR        "Tfa98xx_hal.dll\0"
#define VER_ORIGINALFILENAME_STR    "Tfa98xx_hal.dll\0"


#endif //VERSION_H
