#ifndef _SAKURA_TIMER_H
#define _SAKURA_TIMER_H

void sakura_timer_setup(void);
unsigned char sakura_timer_read(void);
void sakura_timer_set(unsigned char countdown);
unsigned long int sakura_timer_uptime(void);

#endif /* _SAKURA_TIMER_H */
