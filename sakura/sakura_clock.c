#include <iodefine.h>

void sakura_clock_setup(void)
{
  int i;

  /* Enable writing to clock generation circuit registers. */
  SYSTEM.PRCR.WORD = 0xa501;

  /* Stop sub-clock. */
  SYSTEM.SOSCCR.BYTE = 0x01;

  /* Set main oscillator settling time to 10ms (131072 cycles @ 12MHz). */
  SYSTEM.MOSCWTCR.BYTE = 0b01101;

  /* Set PLL circuit settling time to 10ms (2097152 cycles @ 192MHz). */
  SYSTEM.PLLWTCR.BYTE = 0b01110;

  /* Set PLL circuit to x16. */
  SYSTEM.PLLCR.WORD = 0x0F00;

  /* Start the external 12Mhz oscillator. */
  SYSTEM.MOSCCR.BYTE = 0x00;

  /* Turn on the PLL. */
  SYSTEM.PLLCR2.BYTE = 0x00;

  /* Wait 12ms (~2075op/s @ 125KHz). */
  for(i = 0; i < 2075; i++)
  {   
      asm("nop");
  }

  /* Clock Description              Frequency
   * ----------------------------------------
   * PLL Clock frequency...............192MHz
   * System Clock Frequency.............96MHz
   * Peripheral Module Clock B..........48MHz
   * FlashIF Clock......................48MHz
   * External Bus Clock.................48MHz */
  SYSTEM.SCKCR.LONG = 0x21021211;

  /* Clock Description              Frequency
   * ----------------------------------------
   * USB Clock..........................48MHz
   * IEBus Clock........................24MHz */
  SYSTEM.SCKCR2.WORD = 0x0033;

  /* Set the clock source to PLL. */
  SYSTEM.SCKCR3.WORD = 0x0400;

  /* Stop external bus. */
  SYSTEM.SYSCR0.WORD = 0x5A01;

  /* Disable writing to clock generation circuit registers. */
  SYSTEM.PRCR.WORD = 0xa500;
}
 
