#ifndef __WOD_CHANNEL_H__
#define __WOD_CHANNEL_H__

int wod_tcp_tx(char* txbuf, int txlen);
int wod_tcp_txrx(char* txbuf, int txlen, char *rxbuf, int *rxlen);

#endif // __WOD_CHANNEL_H__

