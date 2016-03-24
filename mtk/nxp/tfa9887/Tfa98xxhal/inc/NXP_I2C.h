#ifndef NXP_I2C_H
#define NXP_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* A glue layer.
 * The NXP components will use the functions defined in this API to do the actual calls to I2C
 * Make sure you implement this to use your I2C access functions (which are SoC and OS dependent)
 */

typedef enum
{
  NXP_I2C_UnassignedErrorCode,
  NXP_I2C_Ok,
  NXP_I2C_NoAck,
  NXP_I2C_SclStuckAtOne,
  NXP_I2C_SdaStuckAtOne,
  NXP_I2C_SclStuckAtZero,
  NXP_I2C_SdaStuckAtZero,
  NXP_I2C_TimeOut,
  NXP_I2C_ArbLost,
  NXP_I2C_NoInit,
  NXP_I2C_Disabled,
  NXP_I2C_UnsupportedValue,
  NXP_I2C_UnsupportedType,
  NXP_I2C_NoInterfaceFound,
  NXP_I2C_NoPortnumber,
  NXP_I2C_ErrorMaxValue
} NXP_I2C_Error_t;

/* The maximum I2C message size allowed for read and write buffers, incl the slave address */
#define NXP_I2C_MAX_SIZE 254

/* Execute an I2C write transaction
   W <slave_address> <data_1> ... <data_n> P
   The slave address (data_0) and subaddress to use are inside the data
*/
NXP_I2C_Error_t NXP_I2C_Write(  int num_write_bytes,
                                unsigned char data[] );

/* Execute a write, followed by I2C restart and a read of num_read_bytes bytes.
   The read_buffer must be big enough to contain num_read_bytes.
   W <slave_address> <sub_address> <other write data> RS <slave address> <data to read> P
   The slave address and subaddress to use are inside the data
*/
NXP_I2C_Error_t NXP_I2C_WriteRead(  int num_write_bytes,
									                  unsigned char write_data[],
									                  int num_read_bytes,
									                  unsigned char read_buffer[] );

/* when enabled, the low level I2C transactions are displayed using printf */
NXP_I2C_Error_t NXP_I2C_EnableLogging(int bEnable);
#ifdef WIN32
/* Load the i2c interface */
NXP_I2C_Error_t init_I2C();
#endif
#ifdef __cplusplus
}
#endif

#endif // NXP_I2C_H
