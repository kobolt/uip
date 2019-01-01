#ifndef _SAKURA_ETH_H
#define _SAKURA_ETH_H

int sakura_eth_setup(unsigned char mac[6]);
unsigned int sakura_eth_recv(unsigned char *buf);
void sakura_eth_send(unsigned char *buf, unsigned int len);

#endif /* _SAKURA_ETH_H */
