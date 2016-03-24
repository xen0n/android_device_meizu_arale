/*
 * lxI2c.c
 *
 *  Created on: Apr 21, 2012
 *      Author: wim
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <inttypes.h>   //TODO fix/converge types
//#include <linux/stat.h>
#include <fcntl.h>
#include "../../../climax/inc/i2c-dev.h"

#include "../../inc/lxScribo.h"
#include "../../../Tfa98xx/inc/Tfa98xx.h"   // for i2c slave address
#include <utils/Log.h>

#define LOG_TAG "lxI2C"

int i2c_trace = 1;

static unsigned char  tfa98xxI2cSlave = 0x36; // global for i2c access

static void hexdump(int num_write_bytes, const unsigned char *data)
{
    int i;
    for (i = 0; i < num_write_bytes; i++)
    {
        ALOGD("0x%02x ", data[i]);
    }

}

void  lxI2cSlave(int fd, int slave)
{
    // open the slave
    //ALOGD("lxI2cSlave");
    int res = ioctl(fd, I2C_SLAVE, slave);
    if (res < 0)
    {
        /* TODO ERROR HANDLING; you can check errno to see what went wrong */
        ALOGW(stderr, "Can't open i2c slave:0x%02x\n", slave);
        //_exit(res);
    }

    if (i2c_trace)
    {
        //ALOGD("I2C slave=0x%02x\n", tfa98xxI2cSlave);
    }
}

int lxI2cWriteRead(int fd, int NrOfWriteBytes, const uint8_t *WriteData,
                   int NrOfReadBytes, uint8_t *ReadData)
{
    struct i2c_smbus_ioctl_data args;
    uint8_t buf[256];
    int ln;
    //ALOGD("lxI2cWriteRead");

    // TODO redo checking
    /*
    if (WriteData[0] != (tfa98xxI2cSlave << 1))
    {
        ALOGD("wrong slave 0x%02x iso0x%02x\n", WriteData[0] >> 1, tfa98xxI2cSlave);
        return 0;
    }
    */
    lxI2cSlave(fd, tfa98xxI2cSlave);

    if (NrOfWriteBytes & i2c_trace)
    {
        //ALOGD("lxI2cWriteRead %d:", NrOfWriteBytes);
        //hexdump(NrOfWriteBytes, WriteData);
    }

    if (NrOfWriteBytes > 2)
    {
        ln = write(fd, &WriteData[1],  NrOfWriteBytes - 1);
    }

    if (NrOfReadBytes)   // read registers
    {
        //if ( (ReadData[0]>1) != (WriteData[0]>1) ) // if block read is different
        //      write(fd, &ReadData[0],  1);
        write(fd, &WriteData[1], 1); //write sub address
        ln = read(fd,  &ReadData[1], NrOfReadBytes - 1);
    }

    if (NrOfReadBytes & i2c_trace)
    {
        //ALOGD("R %d:", NrOfReadBytes);
        //hexdump(NrOfReadBytes, ReadData);
    }
    if (ln < 0)
    {
        ALOGE("i2c slave error");
    }
    //ALOGD("lxI2cWriteRead return =%d NrOfReadBytes = %d",ln+1,NrOfReadBytes);
    return (ln + 1);
}

int lxI2cWrite(int fd, int size, uint8_t *buffer)
{
    uint8_t cmd[5], *ptr, slave, term;
    int length, status, total = 0;
    //ALOGD("lxI2cWrite");
    return lxI2cWriteRead(fd, size, buffer, 0, NULL);
}

int lxI2cInit(char *devname)
{
    int fd =0 , res;
    int retrycount = 20 ;
    ALOGD("lxI2cInit devname = %s",devname);
    fd = open(devname, O_RDWR | O_NONBLOCK, 0);
    ALOGW(stderr, "lxI2cInit \n");
    while((retrycount > 0) && ( fd <0) )
    {
         if (fd < 0)
        {
            ALOGW("Can't open i2c bus:%s\n", devname);
            usleep(20*1000);
             //_exit(1);
        }
        fd = open(devname, O_RDWR | O_NONBLOCK, 0);
        ALOGD("retry lxI2cInit  retrycount =%d",retrycount);
        retrycount--;
    }
    lxI2cSlave(fd, tfa98xxI2cSlave);
    return fd;
}


