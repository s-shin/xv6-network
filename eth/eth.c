//
// Ethernet driver manager
//


// minix
#include "minix-compatible.h"
#include "dp.h"
#include "eth.h"

#include "../traps.h"
#include "../fs.h"
#include "../file.h"
#include "../spinlock.h"
#include "../param.h"
#include "../mmu.h"
#include "../proc.h"

// lock
static struct spinlock lock;

// QEMU PCI: 0xC100, QEMU ISA: 0x300, BOCHS ISA: 240(default)
static int ports[] = { 0x300, 0xC100, 0x240, 0x280, 0x320, 0x340, 0x360 };
static dpeth_t de;

// Interrupt
void
ethintr(void)
{
  dp_interrupt(&de);
  wakeup(0);
  return;
}

int
ethread(struct inode* ip, char* p, int n)
{
  eth_t eth = { (uchar*)p, n };
  
  // actually, use ioctl()
  if (p == 0 || n == 0) {
    dp_dump(&de);
    return 0;
  }
  
  iunlock(ip);
  acquire(&lock);

  sleep(0, &lock);
  dp_read(&de, &eth, 0);
  
  release(&lock);
  ilock(ip);
  return de.bytes_Rx;
}

int
ethwrite(struct inode* ip, char* p, int n)
{
  eth_t eth = { (uchar*)p, n };

  iunlock(ip);
  acquire(&lock);
  
  dp_write(&de, &eth, 0);
  
  release(&lock);
  ilock(ip);
  return de.bytes_Tx;
}

// Initialize all ethernet drivers (called in 'mainc()' (in main.c))
void
ethinit()
{
  int i;
  char name[] = "eth#";
  int ai = 0; // available port index

  initlock(&lock, "eth");
  
  devsw[ETHERNET].write = ethwrite;
  devsw[ETHERNET].read = ethread;

  // Initialize the NE2000 device driver
  for (i = 0; i < NELEM(ports); ++i) {
    cprintf("[ethinit] initialize port %d.\n", i);
    memset(&de, 0, sizeof(de));
    name[3] = '0' + ai;
    strncpy(de.de_name, name, strlen(name)+1);
    de.de_irq = IRQ_ETH;
    if (dp_init(ports[i], &de)) {
      picenable(de.de_irq);
      ioapicenable(de.de_irq, 0);
      break;
    }
  }

  return;
}







