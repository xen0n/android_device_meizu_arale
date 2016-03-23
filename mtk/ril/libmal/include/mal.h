#ifndef __MAL_INTERFACE_H__
#define __MAL_INTERFACE_H__

#include "simmngr_interface.h"
#include "datamngr_interface.h"


void* mal_init ();

void* mal_init2 ();

int mal_exit (void *o);

int mal_execute_atcmd (void *o, char *cmd, char *response, int response_len);

int mal_ril_connect (void *o);

int mal_ril_disconnect (void *o);

int mal_get_ps_reg_state (char *buf, unsigned int len);

int mal_get_ps_reg_state_ex (int type, char *buf, unsigned int buf_len);

int mal_set_ps_format(int long_format);

#endif /* __MAL_INTERFACE_H__ */
