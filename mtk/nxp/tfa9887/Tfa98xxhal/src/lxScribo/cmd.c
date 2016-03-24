/*
 * cmd.c
 *
 *  Created on: Sep 29, 2011
 *      Author: Rene Geraets
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ErrorCodes.h"
#include "cmd.h"

#if 0
#include "FreeRTOS.h"
#include "i2c.h"
#include "cdcuser.h"
#include "rprintf.h"
#include "ErrorCodes.h"
#include "lcd.h"
#include "gui.h"
#include "gpio.h"

#define SCRIBO_VERSION_STRING "Scribo,1.6,20110926,LPC1768"

#else

#include <stdio.h>
#include "../../inc/lxScribo.h"
#define SCRIBO_VERSION_STRING "Scribo,2.0,20120515,SocketServer"
//#define SCRIBO_VERSION_STRING "Scribo,1.71,20120209,Development board"//,Scipio,0.23,20120328,AT-Mega2561"

#endif //LPCDEMO

#define CMDSIZE 300
#define LOGERR 1

/*
 *  this is the host return write
 */
extern int activeSocket;	//TODO fix global
extern int socket_verbose;
#define VERBOSE if (socket_verbose)

void hostWrite(int fd, void* pBuffer, int size) {


	if (socket_verbose) {
		int i;
		uint8_t *data=pBuffer;
		printf("xmit: ");
		for (i = 0; i < size; i++)
			printf("0x%02x ",  data[i]);
		printf("\n");
	}

	write(fd, pBuffer, size);
}

static uint8_t cmdbuf[CMDSIZE];
static int idx;

static uint8_t resultbuf[512];

void Cmd_Init(void)
{
	idx = 0;
}


static void Result(uint16_t cmd, uint16_t err, uint16_t cnt)
{
	resultbuf[0] = cmd >> 8;
	resultbuf[1] = cmd & 0xFF;
	resultbuf[2] = err & 0xFF;
	resultbuf[3] = err >> 8;
	resultbuf[4] = cnt & 0xFF;
	resultbuf[5] = cnt >> 8;
	resultbuf[6 + cnt] = 0x02;
	hostWrite(activeSocket, resultbuf, 7 + cnt);
}


#define CMD(b1, b2) ((b1) | ((b2) << 8))

void CmdProcess(void* buf, int len)
{
	uint16_t cmd;
	int n;
	int st;
	uint8_t sla;
	uint16_t err;
	uint16_t cnt;
	uint16_t cntR;
	int nR;
	int processed;
	int v;

	st = 0;
	while (st < len)
	{
		do
		{
			processed = 0;
			n = CMDSIZE - idx;
			if (n > len - st)
			{
				n = len - st;
			}
			memcpy(cmdbuf + idx, buf + st, len);
			st += n;
			idx += n;
			if (idx >= 2)
			{
				cmd = CMD(cmdbuf[1], cmdbuf[0]);
				switch(cmd)
				{
				case CMD(0, 'r'):  //I2C read
					//Format = 'r'(16) + sla(8) + cnt(16) + 0x02
			    	if (idx >= 6)
			    	{
			    		err = eNone;
			    		// Expect(3);
			    		sla = cmdbuf[2];
			    		cnt = cmdbuf[3] | (cmdbuf[4] << 8);
						if (sla > 127) err = eBadSlaveAddress;
						if (sla == 0xFF) err = eBadFormat | eComErr; //Yes, the previous err might be overwritten. Intended
						if (cnt == 0xFFFF) err = eMissingReadCount | eBadFormat;
						if (cmdbuf[5] != 0x02)
						{
							err |= eBadTerminator;
						}
						n = 0;
						if (err != eNone)
						{
							cnt = 0;
						}
						else
						{
							if (cnt > sizeof(resultbuf) - 7)
							{
								cnt = sizeof(resultbuf) - 7;
								err = eBufferOverRun;
							}
							if (!i2c_Read(I2CBUS, sla, resultbuf + 6, cnt, &n))
							{
								err = eI2C_SLA_NACK;
							}
						}
						Result(cmd, err, n);
#if LOGERR
						if (err != eNone)
						{
							rprintf("R:%02X:%d - 0x%04X\r\n", sla, cnt, err);
						}
#endif
						processed = 6;
			    	}
			    	break;

				case CMD(0, 'w'):  //I2C write
					//Format = 'w'(16) + sla(8) + cnt(16) + data(8 * cnt) + 0x02
					if (idx >= 5)
					{
						err = eNone;
						sla = cmdbuf[2];
						cnt = cmdbuf[3] | (cmdbuf[4] << 8);
						if (sla > 127) err = eBadSlaveAddress;
						if (sla == 0xFF) err = eBadFormat | eComErr; //Yes, the previous err might be overwritten. Intended
						if (cnt == 0xFFFF) err = eMissingReadCount | eBadFormat;
						if (cnt > CMDSIZE - 6)
						{
							err = eBufferOverRun;
#if LOGERR
							rprintf("W:%02X:%d - 0x%04X\r\n", sla, cnt, err);
#endif
							processed = idx;
						}
						else
						{
							if (idx >= 6 + cnt)
							{
								if (cmdbuf[5 + cnt] != 0x02)
								{
									err |= eBadTerminator;
								}
								if (err == eNone)
								{
									if (!i2c_Write(I2CBUS, sla, cmdbuf + 5, cnt))
									{
										err = eI2C_SLA_NACK;
									}
								}
								Result(cmd, err, 0);
#if LOGERR
								if (err != eNone)
								{
									rprintf("W:%02X:%d - 0x%04X\r\n", sla, cnt, err);
								}
#endif
								processed = 6 + cnt;
							}
						}
					}
					break;

				case CMD('w', 'r'): //I2C write-read
					//Format = 'wr'(16) + sla(8) + w_cnt(16) + data(8 * w_cnt) + r_cnt(16) + 0x02
					if (idx >= 5)
					{
						err = eNone;
						sla = cmdbuf[2];
						cnt = cmdbuf[3] | (cmdbuf[4] << 8);
						if (sla > 127) err = eBadSlaveAddress;
						if (sla == 0xFF) err = eBadFormat | eComErr; //Yes, the previous err might be overwritten. Intended
						if (cnt == 0xFFFF) err = eMissingReadCount | eBadFormat;
						if (cnt > CMDSIZE - 8)
						{
							err = eBufferOverRun;
#if LOGERR
							rprintf("WR:%02X:%d - 0x%04X\r\n", sla, cnt, err);
#endif
							processed = idx;
						}
						else
						{
							if (idx >= 8 + cnt)
							{
								if (cmdbuf[7 + cnt] != 0x02)
								{
									err |= eBadTerminator;
								}
								cntR = cmdbuf[5 + cnt] | (cmdbuf[6 + cnt] << 8);
								if (cntR > sizeof(resultbuf) - 7)
								{
									cntR = sizeof(resultbuf) - 7;
									err = eBufferOverRun;
								}
								nR = 0;
								if (err == eNone)
								{
									if (!i2c_WriteRead(I2CBUS, sla, cmdbuf + 5, cnt, &n, resultbuf + 6, cntR, &nR))
									{
										err = eI2C_SLA_NACK;
									}
								}
								Result(cmd, err, nR);
#if LOGERR
								if (err != eNone)
								{
									rprintf("WR:%02X:%d,%d - 0x%04X\r\n", sla, cnt, cntR, err);
								}
#endif
								processed = 8 + cnt;
							}
						}
					}
					break;

				case CMD(0, 'v'):  //Version info
					//format: 'v'(16) + 0x02
					if (idx >= 3)
					{
						err = eNone;
						if (cmdbuf[2] != 0x02)
						{
							err = eBadTerminator;
						}
						memcpy(resultbuf + 6, SCRIBO_VERSION_STRING, strlen(SCRIBO_VERSION_STRING) + 1);
						Result(cmd, err, strlen(SCRIBO_VERSION_STRING) + 1);
#if LOGERR
						if (err != eNone)
						{
							rprintf("V - 0x%04X\r\n", err);
						}
#endif
						processed = 3;
					}
					break;

				case CMD('s','p'): //Set I2C speed
					//format: 'sp'(16) + speed(32) + 0x02
					if (idx >= 7)
					{
						int speed;
						int res;
						speed = cmdbuf[2] | (cmdbuf[3] << 8) | (cmdbuf[4] << 16) | (cmdbuf[5] << 24);
						err = eNone;
						if (cmdbuf[6] != 0x02)
						{
							err = eBadTerminator;
						}
						else
						{
							if (speed != 0)
							{
								if (speed < 50000)
								{
									err = eI2CspeedTooLow;
								}
								else if (speed > 400000)
								{
									err = eI2CspeedTooHigh;
								}
								else
								{
									i2c_SetSpeed(I2CBUS, speed);
								}
							}
							res = i2c_GetSpeed(I2CBUS);
							resultbuf[6] =  res         & 0xFF;
							resultbuf[7] = (res  >>  8) & 0xFF;
							resultbuf[8] = (res  >> 16) & 0xFF;
							resultbuf[9] = (res  >> 24) & 0xFF;
							Result(cmd, err, 4);
#if LOGERR
							if (err != eNone)
							{
								rprintf("SP:%d - 0x%04X\r\n", speed, err);
							}
#endif
							processed = 7;
						}
					}
					break;

				case CMD('b', 'l'): //Return I2C buffer length
					//format: 'bl'(16) + 0x02
					if (idx >= 3)
					{
						err = eNone;
						if (cmdbuf[2] != 0x02)
						{
							err = eBadTerminator;
						}

						resultbuf[6] = (CMDSIZE - 16) & 0xFF;
						resultbuf[7] = (CMDSIZE - 16) >> 8;
						Result(cmd, err, 2);
#if LOGERR
						if (err != eNone)
						{
							rprintf("BL - 0x%04X\r\n", err);
						}
#endif
						processed = 3;
					}
			      break;

				case CMD('p', 's'): //Pin set
					//format = 'ps'(16) + id(8) + val(16) + 0x02
					if (idx >= 6)
					{
						if (cmdbuf[2] == 9)
						{
							v = cmdbuf[3] + (cmdbuf[4] << 8);
							if (v != 0)
							{
								v = 1;
							}
							gui_SetValue(ITEM_ID_STANDALONE, v);
							Result(cmd, eNone, 0);
						}
						else
						{
							Result(cmd, eInvalidPinNumber, 0);
						}
						processed = 6;
					}
					break;

				case CMD('p', 'r'): //Pin read
					//format = 'pr'(16) + id(8) + 0x02
					if (idx >= 4)
					{
						if ((cmdbuf[2] >= 10) && (cmdbuf[2] <= 15))
						{
							resultbuf[6] = 0;
							resultbuf[7] = 0;
							Result(cmd, eNone, 2);
						}
						else if (cmdbuf[2] == 9)
						{
							if (!gui_GetValue(ITEM_ID_STANDALONE, &v))
							{
								v = 1;
							}
							resultbuf[6] = ((v != 0) ? 1 : 0);
							resultbuf[7] = 0;
							Result(cmd, eNone, 2);
						}
						else
						{
							Result(cmd, eInvalidPinNumber, 0);
						}
						processed = 4;
					}
					break;

				case CMD('r', 's'): //Reset
					/* no way! */
#if LOGERR
					rprintf("RS - 0x%04X\r\n", eBadCmd);
#endif
					Result(cmd, eBadCmd, 0);
					processed = 2;
					break;

				default:   //Unsupported command
#if LOGERR
					rprintf("%c%c: eBadCmd (%02X-%02X)\r\n", (cmd & 0xFF) == 0 ? ' ' : cmd & 0xFF, cmd >> 8, cmdbuf[1], cmdbuf[0]);
#endif
					Result(cmd, eBadCmd, 0);
					processed = idx;
					break;
				}
			}
			if ((processed != 0) & (processed < idx))
			{
				memmove(cmdbuf, cmdbuf + processed, idx - processed);
			}
			idx -= processed;
		} while((idx > 0) && (processed > 0));
	}
}
