/*********************************************************************************
*
* copied from Arduino devkit Scribo.pde
*
**********************************************************************************/
#include "ErrorCodes.h"
#include <unistd.h>
#include <stdint.h>

#define SCRIBO_VERSION_MAJOR "1"
#define SCRIBO_VERSION_MINOR "71"
#define SCRIBO_RELEASE_DATE "20120906"

#define SCRIBO_PLATFORM "Climax"

#define SCRIBO "Scribo"

//Keep in the same format: name, version, release date and platform
//TODO #define SCRIBO_VERSION_STRING SCRIBO "," SCRIBO_VERSION_MAJOR "." SCRIBO_VERSION_MINOR "," SCRIBO_RELEASE_DATE "," SCRIBO_PLATFORM "," SCIPIO_VERSION_STRING
#define SCRIBO_VERSION_STRING "wimtest"
#define SCRIBO_BAUDRATE (500000)
#define TWI_BUFFER_LENGTH 0x200

#define HCTRL_UART_BUFFER_LENGTH  TWI_BUFFER_LENGTH + 16 //I2C buffer + overhead
//Fixed buffers on the heap for performance reasons
uint8_t _I2C_rx_buffer[TWI_BUFFER_LENGTH];
uint8_t _UART_rx_buffer[HCTRL_UART_BUFFER_LENGTH];  

/******************************************************************************
 * serial & time wrappers
 */
static uint8_t *serialReadBuffer, serialWriteBuffer[512];
static int serialReadIdx=0, serialReadLen=0;
static int serialWriteIdx=0;

int serialAvailable(void){
	return serialReadLen-serialReadIdx;
}
uint8_t serialRead(void){
	uint8_t data = serialReadBuffer[serialReadIdx];

	if (serialReadIdx++>serialReadLen) {
		printf("!!!!!!!!!!serialRead underun\n");
		return 0;
	}
	return data;
}
void serialWriteBuf(uint8_t *buf, int cnt){

	if (serialWriteIdx+cnt > sizeof(serialWriteBuffer)) {
		printf("!!!!!!!!!!serialWrite buffer overrun\n");
	}

	memcpy(&serialWriteBuffer[serialWriteIdx], buf, cnt);

	serialWriteIdx += cnt;

}
void serialWrite(uint8_t c){
	serialWriteBuf(&c,1);
}
void serialFlush(void){

}
void delay(int ms) {
	usleep(ms*1000); //convert to microseconds
	return;
}
//unsigned long micros() {
//	struct timeval now;
//
//}
/******************************************************************************
 * i2c command wrappers
 */

uint16_t I2Cspeed(uint32_t freq_Hz) // TODO I2Cspeed
{
  //SCL frequency = CPU Clock frequency / (16 + 2 * (TWBR) * (Prescaler)
  //Prescaler defaults to 1 (one)
  if (freq_Hz == 0) return 400000;

  if (freq_Hz > 1000000) return 1;
  if (freq_Hz <= 30418) return 2;
//  TWBR = ((CPU_FREQ / freq_Hz) - 16) / 2;
//  uSecsPerByte = 1E7 / I2Cspeed(); //(8 bits + ACK + pause) * freq / one us
  return 0;
}

_Bool i2c_Read(int bus, int addr, void* data, int size, int* nr);
uint16_t ReadI2C(uint8_t sla, uint16_t *cnt, uint8_t *data, uint16_t sz)
{
  uint16_t err = eNone;

  if (!i2c_Read(0, sla, data, sz, cnt))
	  err = eNoResponse;

  if (*cnt < sz )
	  err = eMissingI2CData;

  return err;
}
_Bool i2c_Write(int bus, int addrWr, void* dataWr, int sizeWr);
uint16_t WriteI2C(uint8_t sla, uint16_t cnt, uint8_t *data)
{
  //Format = 'w'(16) + sla(8) + cnt(16) + data(8 * cnt) + 0x02
  uint16_t idx = 0, err = eNone;

  err = i2c_Write(0, sla, data, cnt) ? eNone : eI2C_SLA_NACK; //TODO err only nak
//  switch (err)
//  {
//    case 1: err = eBufferOverRun;
//      break;
//    case 2: err = eI2C_SLA_NACK;
//      break;
//    case 3: err = eI2C_DATA_NACK;
//      break;
//    case 4: err = eI2C_UNSPECIFIED;
//      break;
//  }
  return err;
}
/******************************************************************************
 * pin command wrappers
 */

uint16_t SetPin(uint8_t output, uint8_t num, uint16_t val) //TODO pins
{
  uint16_t err = eNone;
//  int16_t pin = PinMapping(num);
//  if (pin < 0) err = eInvalidPinNumber;
//  else
//  {
//    if (num < 0x80)
//    {
//      pinMode(pin, output ? OUTPUT : INPUT);
//      digitalWrite(pin, val);
//    }
//    else
//    {
//      //this bord has no analog output pins.
//      err = eInvalidPinMode;
//    }
//  }
//  return err;
  return eInvalidPinNumber;
}

uint16_t GetPin(uint8_t num, uint16_t *val)	//TODO pins
{
  uint16_t err = eNone;
//  int16_t pin = PinMapping(num);
//  if (pin < 0) err = eInvalidPinNumber;
//  else
//  {
//    uint8_t mode = 0;
//    getPinMode(pin, &mode);
//    if (num < 0x80)
//    {
//      *val = digitalRead(pin);
//    }
//    else
//    {
//      *val = (unsigned int) analogRead(pin);
//    }
//    if (mode == OUTPUT) *val |= 0x8000;
//  }
//  return err;
  return eInvalidPinNumber;
}


/*
 *
 */
static void hexdump(char *str, const unsigned char * data, int num_write_bytes) //TODO cleanup/consolidate all hexdumps
{
	int i;

	printf("%s: ", str);
	for(i=0;i<num_write_bytes;i++)
	{
		printf("0x%02x ", data[i]);
	}
	printf("\n");
}

const uint8_t cTerminator = 0x02;  //All host commands are terminated with 0x02

uint8_t Expect(uint16_t NrOfBytes)
{
  /*
  uint32_t timedue = micros() + 12 * NrOfBytes + 2E7 * (NrOfBytes + 1) / SCRIBO_BAUDRATE; 
  while (serialAvailable() < NrOfBytes && micros() < timedue);
  */
//  uint32_t starttime = micros();
//  uint32_t timedue = 12 * NrOfBytes + 2E7 * (NrOfBytes + 1) / SCRIBO_BAUDRATE;
//  while ((serialAvailable() < NrOfBytes) && ((micros() - starttime) < timedue));
//  return serialAvailable() >= NrOfBytes;
	return 1; //TODO check if char timeout necessary for climax
}

uint16_t ReadTerminator()
{
  //Carefull. Several returns here.
  uint8_t term = 0;
  Expect(1);
  term = serialRead();
  if (term == 0xFF) return eMissingTerminator;
  else if (term != cTerminator) return eBadTerminator;
  return eNone;
}

void Result(uint16_t cmd, uint16_t err, uint8_t *data, uint16_t sz)
{
  serialWrite(cmd & 0xFF);
  serialWrite(cmd >> 8);
  serialWrite(err & 0xFF);
  serialWrite(err >> 8);
  if (!data) sz = 0;
  serialWrite(sz & 0xFF);
  serialWrite(sz >> 8);
  if (sz) serialWriteBuf(data, sz);
  serialWrite (cTerminator);
}

void SendVersionInfo()
{
  //Return version info
  //For C programmers this looks like a bug, 
  //but the sizeof() function returns the number of chars, not the size of the pointer
  //delay(1);
  Result('v', ReadTerminator(), SCRIBO_VERSION_STRING, sizeof(SCRIBO_VERSION_STRING));
}

void ChangeI2Cspeed()
{
  //Set I2C speed
  //format: 'sp'(16) + speed(32) + 0x02 
  uint32_t freq = 0;
  uint32_t oneByte = 0;
  uint16_t err = eNone;
  if (Expect(4)) 
  {
    freq = serialRead();
    oneByte = serialRead();
    freq += oneByte << 8;
    oneByte = serialRead();
    freq += oneByte << 16;
    oneByte = serialRead();
    freq += oneByte << 24;
    err |= ReadTerminator();
    if (err == eNone && freq) 
    {
      err = I2Cspeed(freq);
      if (err > 1)  err = eI2CspeedTooLow; 
      if (err == 1) err = eI2CspeedTooHigh; 
    }
  }
  else err |= eComErr | eBadFormat | eInvalidLength;
  freq = I2Cspeed(0);
  Result('sp', err, (uint8_t *) &freq, sizeof(freq));
}
 
void ExecuteReadI2C()
{
  //Format = 'r'(16) + sla(8) + cnt(16) + 0x02
  uint16_t err = eNone;
  Expect(3);
  uint8_t sla = serialRead();
  uint16_t cnt = serialRead();// cnt <<= 8;
  cnt += serialRead() << 8;
  if (sla > 127) err = eBadSlaveAddress; 
  if (sla == 0xFF) err = eBadFormat | eComErr; //Yes, the previous err might be overwritten. Intended
  if (cnt == 0xFFFF) err = eMissingReadCount | eBadFormat;
  err |= ReadTerminator(); 
  if (err == eNone) err = ReadI2C(sla, &cnt, _I2C_rx_buffer, TWI_BUFFER_LENGTH);
  Result('r', err, _I2C_rx_buffer, cnt);
}

void PurgeSerial()
{
  delay(3);
  serialFlush();
}

uint16_t ReadWriteCmd(uint8_t *sla, uint16_t *cnt, uint8_t *data, uint16_t sz)
{
  uint16_t err = eNone;
  //Format = ... + sla(8) + cnt(16) + data(8 * cnt)
  Expect(3);
  *sla = serialRead();
  *cnt = serialRead();
  *cnt += (serialRead() << 8);
  uint32_t starttime;
  if (*sla > 127) err = eBadSlaveAddress; 
  if (*sla == 0xFF) err = eBadFormat | eComErr; //Yes, the previous err might be overwritten. Intended
  if (*cnt == 0xFFFF) err = eMissingReadCount | eBadFormat;
  if (*cnt > HCTRL_UART_BUFFER_LENGTH) 
  {
    err = eBufferOverRun;
    //Throw away incoming data
    serialFlush(); //TODO check eBufferOverRun
//    starttime = micros();
//    uint32_t timedue = 1E8 * (*cnt + 1) / SCRIBO_BAUDRATE; // (8 databits + 1 stopbit) * cnt / baudrate / 1 us
//    while ((micros() - starttime) < timedue) serialFlush();
  }
  uint16_t idx = 0;
  if (err == eNone)
  {
    //uint32_t timeout = (micros() + 10L * ((uint32_t)*cnt) + (1E7L * (((uint32_t)*cnt) + 1) / SCRIBO_BAUDRATE)) * 10L; 
    // (8 databits + 1 stopbit + 1 startbit ) * (cnt + 1 extra)  / baudrate / 1 us  
    // * 10 = 900% margin + 10us for code execution
    const uint32_t timeout = 80E3L;
//    starttime = micros();//TODO serialRead timeout
    do
    {
      if (serialAvailable()) _UART_rx_buffer[idx++] = serialRead();
    }
    while ((idx < *cnt));
    //    while (((micros() - starttime) < timeout) && (idx < *cnt));//TODO serialRead timeout
    if (idx < *cnt) 
    {
      err = eMissingData;
      PurgeSerial();
    }
  }
  return err;
}

void ExecuteWriteI2C()
{
  uint8_t sla;
  uint16_t cnt;
  uint16_t err = ReadWriteCmd(&sla, &cnt, _UART_rx_buffer, HCTRL_UART_BUFFER_LENGTH);  
  if (err == eNone) err |= ReadTerminator();
  if (err == eNone) err = WriteI2C(sla, cnt, _UART_rx_buffer);
  Result('w', err, 0, 0);
}

void ExecuteWriteRead()
{
  //Format = 'wr'(16) + sla(8) + w_cnt(16) + data(8 * w_cnt) + r_cnt(16) + 0x02
  //w_cnt = number of bytes to write, r_cnt = number of bytes to read.
  //Normally we would use a Restart between the write and read. 
  //Since this is a single I2C Master bus, a separate write and read will work as well. 
  uint8_t sla;
  uint16_t cntW, cntR;
  uint16_t err = ReadWriteCmd(&sla, &cntW, _UART_rx_buffer, HCTRL_UART_BUFFER_LENGTH);  
  Expect(2);
  cntR  = serialRead();
  cntR += (serialRead() << 8);
  if (cntR == 0xFFFF) err = eMissingReadCount | eBadFormat;
  if (err == eNone) err |= ReadTerminator();
  if (err == eNone) err = WriteI2C(sla, cntW, _UART_rx_buffer);
  if (err == eNone) err = ReadI2C(sla, &cntR, _I2C_rx_buffer, TWI_BUFFER_LENGTH);
  Result('wr', err, _I2C_rx_buffer, cntR);
}

void ExecutePinSet()
{
  /*
  Format = 'ps'(16) + id(8) + val(16) + 0x02
  id = pin number, defined by application board.
  id: 7 lsb's is number. msb=1->analog, msb=0->digital
  val: 15 lsb's: digital->0=low, else=high. analog = dac or adc value.
  val: msb=1->output, msb=0->input
  */
  //delay(1);
  Expect(3);
  uint8_t id = serialRead();
  uint16_t val = serialRead();
  val += (serialRead() << 8);
  uint16_t err = ReadTerminator();
  if (err == eNone) err = SetPin ((val & 0x8000) == 0x8000, id, val & 0x7FFF);
  Result('ps', err, 0, 0);
}

void ExecutePinRead()
{
  /*
  Format = 'ps'(16) + id(8) + 0x02
  id = pin number, defined by application board.
  id: 7 lsb's is number. msb=1->analog, msb=0->digital
  */
  Expect(1);
  uint16_t err = eNone;
  uint8_t id = serialRead();
  if (id == 0xFF) err = eBadFormat | eComErr;
  err |= ReadTerminator();
  uint16_t val;
  if (err == eNone) err = GetPin(id, &val);
  Result('pr', err, (uint8_t *)&val, sizeof(val));
}

void ReturnBufferLength()
{
  uint16_t sz = TWI_BUFFER_LENGTH - 1;
  Result('bl', ReadTerminator(), (uint8_t *) &sz, sizeof(sz));
}

void Reset()
{
	printf(">>>>>>>>>>>>>reset\n");
//  wdt_disable();
//  //Set the watchdog to 1 ms
//  wdt_enable(WDTO_15MS);
//  wdt_reset();
//  delay(60);
}

void ExecuteHostCmd(uint16_t cmd)
{
  switch(cmd)
  {
    case 'r':  //I2C read
      ExecuteReadI2C();
      break;
    case 'w':  //I2C write
      ExecuteWriteI2C();
      break;
    case 'wr': //I2C write-read
      //I2C write and read
      ExecuteWriteRead();
      break;
    case 'v':  //Version info
      SendVersionInfo();
      break;
    case 'sp': //Set I2C speed
      ChangeI2Cspeed();
      break;
    case 'bl': //Return I2C buffer length
      //Send buffer length
      ReturnBufferLength();
      break;
    case 'ps': //Pin set
      ExecutePinSet();
      break;
    case 'pr': //Pin read
      ExecutePinRead();
      break;
    case 'rs': //
      Result(cmd, eNone, 0, 0);
      Reset();
      break;
    default:   //Unsupported command
      Result(cmd, eBadCmd, 0, 0);
      //Reset();
      delay(5);
      serialFlush();
      break;
  }
}

//Read Host command, execute and return
void DoHost()
{
  uint16_t cmd = 0, err = eNone;
  if (serialAvailable() > 1)
  {
    cmd  = serialRead();
    cmd += (serialRead() << 8);
    ExecuteHostCmd(cmd);
    serialFlush();
  }
}
void CmdProcess(void* buf, int len)
{
	hexdump("CmdProcess",buf ,len);
	serialReadBuffer = buf;
	serialReadLen = len;
	serialReadIdx = 0;
	serialWriteIdx = 0;
	DoHost();
	UsbCdcWrite(0,serialWriteBuffer, serialWriteIdx);

}

