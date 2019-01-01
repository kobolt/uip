#ifndef _SAKURA_UART_H
#define _SAKURA_UART_H

void sakura_uart0_setup(void);
void sakura_uart0_send(unsigned char *s, int len);
unsigned char sakura_uart0_recv(void);

#endif /* _SAKURA_UART_H */
