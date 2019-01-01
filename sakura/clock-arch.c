#include "clock-arch.h"
#include "sakura_timer.h"

clock_time_t clock_time(void)
{
  return sakura_timer_uptime() * 10;
}

