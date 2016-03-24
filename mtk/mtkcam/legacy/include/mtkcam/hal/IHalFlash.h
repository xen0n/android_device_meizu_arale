#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALFLASH_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALFLASH_H_


/******************************************************************************
 *
 ******************************************************************************/
class IHalFlash
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:     ////            Instantiation.

    virtual ~IHalFlash() {};

public:     ////            Operations.


    static  IHalFlash*    getInstance();

    /**
     * Get current torch status
     */
    virtual MINT32    getTorchStatus(MINT32 i4SensorOpenIndex)  = 0;

    /**
     * Turn on/off torch
     */
    virtual MINT32    setTorchOnOff(MINT32 i4SensorOpenIndex, MBOOL bEnable) = 0;
};

#endif    // _MTK_HARDWARE_INCLUDE_MTKCAM_HAL_IHALFLASH_H_
