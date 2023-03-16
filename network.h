#ifndef NETWORK_H
#define NETWORK_H

void net_close_sockets(void);
int  net_start_listen(CONFIG_t *pConfig);
int  net_poll(void);

#endif
