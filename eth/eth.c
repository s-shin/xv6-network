#include "../types.h"
#include "../defs.h"
#include "../fs.h"
#include "../file.h"
#include "../traps.h"
#include "ne.h"
#include "eth.h"

static ne_t ne;

void
ethintr()
{
  ne_interrupt(&ne);
  return;
}

// not used because ioctl() isn't still implemented.
int
ethioctl(struct inode* ip, char* p, int n)
{
  switch (n) {
  case ETH_SET_RECV_CALLBACK:
    ne.recv_callback = (ne_callback_t)p;
    break;
  }
  return 0;
}

int
ethread(struct inode* ip, char* p, int n)
{
  return ne_pio_read(&ne, (uchar*)p, n);
}

int
ethwrite(struct inode* ip, char* p, int n)
{
  return ne_pio_write(&ne, (uchar*)p, n);
}

void
ethinit()
{
  int i;
  char name[] = "eth#";
  int ports[] = { 0x300, 0xC100, 0x240, 0x280, 0x320, 0x340, 0x360 };

  devsw[ETHERNET].write = ethwrite;
  devsw[ETHERNET].read = ethread;

  for (i = 0; i < NELEM(ports); ++i) {
    cprintf("Ethernet: Initialize port %d.\n", i);
    memset(&ne, 0, sizeof(ne));
    name[3] = '0' + i;
    strncpy(ne.name, name, strlen(name)+1);
    ne.irq = IRQ_ETH;
    ne.base = ports[i];
    if (ne_probe(&ne)) {
      ne_init(&ne);
      picenable(ne.irq);
      ioapicenable(ne.irq, 0);
      break;
    }
  }
  
  return;
}

