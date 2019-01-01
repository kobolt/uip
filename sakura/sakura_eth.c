#include <iodefine.h>
#include <phy.h>
#include "sakura_eth.h"
#include <string.h> /* memcpy() */

#define ETH_BUFSIZE 1536 /* Must be 32-byte aligned. */

#define RD0_RACT   0x80000000
#define RD0_RDLE   0x40000000
#define RD0_RFE    0x08000000
#define TD0_TACT   0x80000000
#define TD0_TDLE   0x40000000
#define TD0_TFP1   0x20000000
#define TD0_TFP0   0x10000000

typedef struct {
  uint32_t rd0;
  uint32_t rd1;
  uint8_t *rd2;
} rx_descriptor_s;

typedef struct {
  uint32_t td0;
  uint32_t td1;
  uint8_t *td2;
} tx_descriptor_s;

__attribute__((aligned(16))) static rx_descriptor_s eth_rx_desc;
__attribute__((aligned(16))) static tx_descriptor_s eth_tx_desc;
__attribute__((aligned(32))) static uint8_t eth_buffer_rx[ETH_BUFSIZE];
__attribute__((aligned(32))) static uint8_t eth_buffer_tx[ETH_BUFSIZE];

static void eth_software_reset(void)
{
  int i;
  for (i = 0; i < 0x10000; i++) {
    asm("nop");
  }
  EDMAC.EDMR.BIT.SWR = 1;
  for (i = 0; i < 0x10000; i++) {
    asm("nop");
  }
}

static void eth_wait_for_link(void)
{
  int16_t link_state;
  do {
    link_state = Phy_GetLinkStatus();
    asm("wait");
  } while (link_state == R_PHY_ERROR);
}

static int eth_link_auto_negotiate(void)
{
  uint16_t duplex = 0;
  uint16_t lpb = 0;
  uint16_t ppb = 0;
  uint16_t link_result = 0;

  link_result = Phy_Set_Autonegotiate(&duplex, &lpb, &ppb);
  if (R_PHY_OK != link_result) {
    return -1;
  }

  switch (duplex) {
  case PHY_LINK_100H:
    ETHERC.ECMR.BIT.DM  = 0;
    ETHERC.ECMR.BIT.RTM = 1;
    break;

  case PHY_LINK_10H:
    ETHERC.ECMR.BIT.DM  = 0;
    ETHERC.ECMR.BIT.RTM = 0;
    break;

  case PHY_LINK_100F:
    ETHERC.ECMR.BIT.DM  = 1;
    ETHERC.ECMR.BIT.RTM = 1;
    break;

  case PHY_LINK_10F:
    ETHERC.ECMR.BIT.DM  = 1;
    ETHERC.ECMR.BIT.RTM = 0;
    break;

  default:
    return -1;
  }

  return 0;
}

int sakura_eth_setup(unsigned char mac[6])
{
  long int result;

  SYSTEM.PRCR.WORD = 0xa502; /* Enable writing to MSTP registers. */
  MSTP_EDMAC       = 0;      /* Disable module-stop state for EDMAC. */
  SYSTEM.PRCR.WORD = 0xa500; /* Disable writing to MSTP registers. */

  /* Clear port direction registers. */
  PORTA.PDR.BIT.B3 = 0;
  PORTA.PDR.BIT.B4 = 0;
  PORTA.PDR.BIT.B5 = 0;
  PORTB.PDR.BIT.B0 = 0;
  PORTB.PDR.BIT.B1 = 0;
  PORTB.PDR.BIT.B2 = 0;
  PORTB.PDR.BIT.B3 = 0;
  PORTB.PDR.BIT.B4 = 0;
  PORTB.PDR.BIT.B5 = 0;
  PORTB.PDR.BIT.B6 = 0;
  PORTB.PDR.BIT.B7 = 0;

  /* Clear port mode registers. */
  PORTA.PMR.BIT.B3 = 0;
  PORTA.PMR.BIT.B4 = 0;
  PORTA.PMR.BIT.B5 = 0;
  PORTB.PMR.BIT.B0 = 0;
  PORTB.PMR.BIT.B1 = 0;
  PORTB.PMR.BIT.B2 = 0;
  PORTB.PMR.BIT.B3 = 0;
  PORTB.PMR.BIT.B4 = 0;
  PORTB.PMR.BIT.B5 = 0;
  PORTB.PMR.BIT.B6 = 0;
  PORTB.PMR.BIT.B7 = 0;

  MPC.PWPR.BIT.B0WI  = 0; /* Enable the PFSWE modification. */
  MPC.PWPR.BIT.PFSWE = 1; /* Disable the PFS register protect. */

  /* Select pin functions. */
  MPC.PA3PFS.BYTE = 0x11; /* ET_MDIO     (input/output) */
  MPC.PA4PFS.BYTE = 0x11; /* ET_MDC      (output)       */
  MPC.PA5PFS.BYTE = 0x11; /* ET_LINKSTA  (input)        */
  MPC.PB0PFS.BYTE = 0x12; /* RMII_RXD1   (input)        */
  MPC.PB1PFS.BYTE = 0x12; /* RMII_RXD0   (input)        */
  MPC.PB2PFS.BYTE = 0x12; /* REF50CK     (input)        */
  MPC.PB3PFS.BYTE = 0x12; /* RMII_RX_ER  (input)        */
  MPC.PB4PFS.BYTE = 0x12; /* RMII_TXD_EN (output)       */
  MPC.PB5PFS.BYTE = 0x12; /* RMII_TXD0   (output)       */
  MPC.PB6PFS.BYTE = 0x12; /* RMII_TXD1   (output)       */
  MPC.PB7PFS.BYTE = 0x12; /* RMII_CRS_DV (input)        */

  MPC.PFENET.BIT.PHYMODE = 0; /* RMII ethernet mode. */

  MPC.PWPR.BIT.PFSWE = 0; /* Enable the PFS register protect. */
  MPC.PWPR.BIT.B0WI  = 1; /* Disable the PFSWE modification. */

  /* Switch port modes to peripheral use. */
  PORTA.PMR.BIT.B3 = 1;
  PORTA.PMR.BIT.B4 = 1;
  PORTA.PMR.BIT.B5 = 1;
  PORTB.PMR.BIT.B0 = 1;
  PORTB.PMR.BIT.B1 = 1;
  PORTB.PMR.BIT.B2 = 1;
  PORTB.PMR.BIT.B3 = 1;
  PORTB.PMR.BIT.B4 = 1;
  PORTB.PMR.BIT.B5 = 1;
  PORTB.PMR.BIT.B6 = 1;
  PORTB.PMR.BIT.B7 = 1;

  /* Initialize the PHY. */
  eth_software_reset();
  result = Phy_Init();
  if (R_PHY_OK != result) {
    return -1;
  }
  Phy_Start_Autonegotiate();

  ETHERC.ECSR.LONG = 0x00000037; /* Clear ETHERC status bits. */
  EDMAC.EESR.LONG  = 0x47FF0F9F; /* Clear EDMAC status bits. */

  /* Wait for link to come online. */
  eth_wait_for_link();

  /* Assign MAC address. */
  eth_software_reset();
  ETHERC.MAHR = (mac[0] << 24) | (mac[1] << 16) | (mac[2] <<  8) | mac[3];
  ETHERC.MALR.LONG = (mac[4] << 8) | mac[5];

  /* Initialize receive and transmit descriptors. */
  eth_rx_desc.rd0 = RD0_RACT | RD0_RDLE;
  eth_rx_desc.rd1 = (ETH_BUFSIZE << 16);
  eth_rx_desc.rd2 = &eth_buffer_rx[0];
  EDMAC.RDLAR = &eth_rx_desc;
  eth_tx_desc.td0 = TD0_TDLE;
  eth_tx_desc.td1 = 0;
  eth_tx_desc.td2 = &eth_buffer_tx[0];
  EDMAC.TDLAR = &eth_tx_desc;

  /* Hardware interface configuration. */
  ETHERC.RFLR.LONG  = 1518;       /* Ethernet length 1514 bytes + CRC. */
  ETHERC.IPGR.LONG  = 0x00000014; /* Intergap is 96-bit time. */
  EDMAC.EDMR.BIT.DE = 1;          /* Little endian mode. */
  EDMAC.TRSCER.LONG = 0x00000080; /* Don't reflect EESR.RMAF in RD0.RFS. */
  EDMAC.TFTR.LONG   = 0x00000000; /* Threshold of Tx FIFO. */
  EDMAC.FDR.LONG    = 0x00000707; /* Tx/Rx FIFO is 2048 bytes. */
  EDMAC.RMCR.LONG   = 0x00000001; /* RNR - Receive Request Bit Reset */

  /* Setup link. */
  result = eth_link_auto_negotiate();
  if (result == -1) {
    return -1;
  }
  
  ETHERC.ECMR.BIT.MPDE = 0; /* Disable Magic Packet detection. */
  ETHERC.ECMR.BIT.RXF = 0; /* Disable PAUSE frame receive. */
  ETHERC.ECMR.BIT.TXF = 0; /* Disable PAUSE frame transmit. */
  ETHERC.ECMR.BIT.RE = 1; /* Enable receive. */
  ETHERC.ECMR.BIT.TE = 1; /* Enable transmit. */

  EDMAC.EDRRR.LONG = 0x1; /* Enable initial EDMAC receive. */

  return 0;
}

unsigned int sakura_eth_recv(unsigned char *buf)
{
  unsigned long int rfl;
  unsigned long int rfe;

  if (RD0_RACT != (eth_rx_desc.rd0 & RD0_RACT)) {
    rfl = eth_rx_desc.rd1 & 0x0000FFFF;
    rfe = eth_rx_desc.rd0 & RD0_RFE;

    if (! rfe) {
      memcpy(buf, eth_rx_desc.rd2, rfl);
    }

    eth_rx_desc.rd0 = RD0_RACT | RD0_RDLE;
    EDMAC.EDRRR.LONG = 0x1; /* Re-enable EDMAC receive. */

    if (rfe) {
      return 0; /* Error! */
    } else {
      return rfl;
    }
  }

  return 0; /* No bytes read. */
}

void sakura_eth_send(unsigned char *buf, unsigned int len)
{
  memcpy(eth_tx_desc.td2, buf, len);

  eth_tx_desc.td1 = (len << 16);
  eth_tx_desc.td0 &= ~(TD0_TFP1 | TD0_TFP0);
  eth_tx_desc.td0 |= (TD0_TFP1 | TD0_TFP0 | TD0_TACT);
  EDMAC.EDTRR.LONG = 0x1; /* Re-enable EDMAC transmit. */
}

