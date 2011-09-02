//
// Ethernet driver manager
//

// minix
#include "minix-compatible.h"
#include "dp.h"
extern int dp_init(int base, dpeth_t* dep);

#include "../traps.h"
#include "../fs.h"
#include "../file.h"
#include "../spinlock.h"


// lock
static struct {
  struct spinlock lock;
  int locking;
} cons;

// available device miner value 'n' maps 'dp_conf[dp_avail[n]]'
static int dp_avail[DE_PORT_NR];

typedef struct dp_conf {
  int base;
  int irq;
} dp_conf_t;

static dp_conf_t dp_conf[DE_PORT_NR] = {
  // QEMU ISA NE2000
  { 0x300,  IRQ_ETH1 },
  // QEMU PCI NE2000
  { 0xC100, IRQ_ETH2 },
  // BOCHS ISA NE2000
  { 0x240,  IRQ_ETH3 },
};

static dpeth_t de_table[DE_PORT_NR];


// Interrupt
void
ethintr(int irq)
{
  int i, conf;
  for (i = 0; i < DE_PORT_NR; ++i) {
    if (dp_conf[i].irq == irq)
      break;
  }
  if (i == DE_PORT_NR) {
    cprintf("[ethintr] inavailable IRQ=%d\n", irq);
    goto bad;
  }
  conf = i;
  for (i = 0; i < DE_PORT_NR; ++i) {
    if (dp_avail[i] == conf)
      break;
  }
  if (i == DE_PORT_NR) {
    cprintf("[ethintr] inavailable port=%d\n", conf);
    goto bad;
  }

  //interrupt dp_conf[conf]
  return;

bad:
  panic("[ethintr] fatal error");
}

int
ethread(struct inode* ip, char* p, int n /* unused */)
{
	//eth* ep = (eth*)p;
	// read dp_conf[dp_avail[ip->minor]]
  return 0;
}

int
ethwrite(struct inode* ip, char* p, int n /* unused */)
{
	//eth* ep = (eth*)p;
	// write dp_conf[dp_avail[ip->minor]]
  return 0;
}

// Initialize all ethernet drivers (called in 'mainc()' (in main.c))
void
ethinit()
{
  int i;
  char name[] = "eth#";
  int ai = 0; // available port index

  initlock(&cons.lock, "eth");

  // Initialize the NE2000 device driver
  for (i = 0; i < DE_PORT_NR; ++i) {
    cprintf("[ethinit] initialize port %d.\n", i);
    // first setup de_table[i]
    memset(&de_table[i], 0, sizeof(dpeth_t));
    name[3] = '0' + ai;
    strncpy(de_table[i].de_name, name, strlen(name)+1);
    de_table[i].de_irq = dp_conf[i].irq;
    if (dp_init(dp_conf[i].base, &de_table[i])) {
      dp_avail[ai++] = i;
      //devsw[ETHERNET].write = ethernetwrite;
      //devsw[ETHERNET].read = ethernetread;
      //picenable(IRQ_ETH0);
      //ioapicenable(IRQ_ETH0);
    }
  }

  return;
}







