//
// Unfortunetlly, the migration from MINIX/drivers/dpeth/dp.c is impossible,
// so this implement is original.
//

#include "minix-compatible.h"
#include "dp.h"
#include "eth.h"

#define BIOS_SEG 0 // dummy
static char SizeErrMsg[] = "illegal packet size";


/*
**  Name:	void dp_next_iovec(iovec_dat_t *iovp)
**  Function:	Retrieves data from next iovec element.
*/
void
dp_next_iovec(iovec_dat_t* iovp)
{
  iovp->iod_iovec_s -= IOVEC_NR;
  iovp->iod_iovec_addr += IOVEC_NR * sizeof(iovec_t);
  //get_userdata(iovp->iod_proc_nr, iovp->iod_iovec_addr,
  //     iovp->iod_iovec_s, iovp->iod_iovec);
  return;
}
/*
**  Name:	int calc_iovec_size(iovec_dat_t *iovp)
**  Function:	Compute the size of a request.
*/
static int calc_iovec_size(iovec_dat_t * iovp)
{
  int size, ix;
  size = ix = 0;
  do {
    size += iovp->iod_iovec[ix].iov_size;
    if (++ix >= IOVEC_NR) {
      dp_next_iovec(iovp);
      ix = 0;
    }
  } while (ix < iovp->iod_iovec_s); // Till all vectors added
  return size;
}

void
dp_write(dpeth_t* dep, eth_t* eth, int vectored /* boolean */)
{
  int size;
  if (dep->de_mode == DEM_ENABLED) {
  
    if (dep->de_flags & DEF_SENDING) {
      cprintf("%s: %s\n", dep->de_name, "send already in progress");
      panic("[do_write] panic");
    }
    if (vectored) {
      // get_userdata ...
      dep->de_write_iovec.iod_iovec_s = eth->size;
      dep->de_write_iovec.iod_iovec_addr = (vir_bytes)eth->addr;
      size = calc_iovec_size(&dep->de_write_iovec);
    } else {
      dep->de_write_iovec.iod_iovec[0].iov_addr = (vir_bytes)eth->addr;
      dep->de_write_iovec.iod_iovec[0].iov_size = size = eth->size;
      dep->de_write_iovec.iod_iovec_s = 1;
      dep->de_write_iovec.iod_iovec_addr = 0;
    }
    if (size < ETH_MIN_PACK_SIZE || size > ETH_MAX_PACK_SIZE) {
      cprintf("%s: %s %d\n", dep->de_name, SizeErrMsg, size);
      panic("[dp_write] panic");
    }
    
    dep->de_flags |= DEF_SENDING;
    (*dep->de_sendf)(dep, FALSE, size);
    
  } else if (dep->de_mode == DEM_SINK) {
    dep->de_flags |= DEF_ACK_SEND;
  }
}

void
dp_read(dpeth_t* dep, eth_t* eth, int vectored /* boolean */)
{
  int size;
  if (dep->de_mode == DEM_ENABLED) {
  
    if (dep->de_flags & DEF_READING) {
      cprintf("%s: read already in progress\n", dep->de_name);
      panic("[dp_read] panic");
    }
    if (vectored) {
      // get_userdata ...
      dep->de_read_iovec.iod_iovec_s = eth->size;
      dep->de_read_iovec.iod_iovec_addr = (vir_bytes)eth->addr;
      size = calc_iovec_size(&dep->de_read_iovec);
    } else {
      dep->de_read_iovec.iod_iovec[0].iov_addr = (vir_bytes)eth->addr;
      dep->de_read_iovec.iod_iovec[0].iov_size = size = eth->size;
      dep->de_read_iovec.iod_iovec_s = 1;
      dep->de_read_iovec.iod_iovec_addr = 0;
    }
    if (size < ETH_MIN_PACK_SIZE || size > ETH_MAX_PACK_SIZE) {
      cprintf("%s: %s %d\n", dep->de_name, SizeErrMsg, size);
      panic("[dp_read] panic");
    }
    
    dep->de_flags |= DEF_READING;
    (*dep->de_recvf)(dep, FALSE, size);
    
  }
}


static void
first_init(dpeth_t* dep)
{
  // ToDo: do something...
  if (dep->de_linmem != 0)
    dep->de_memsegm = BIOS_SEG;
  else
    dep->de_linmem = 0xFFFF0000;

  // Make sure statisics are cleared
  memset((void*)&(dep->de_stat), 0, sizeof(eth_stat_t));

  // Device specific initialization
  (*dep->de_initf)(dep);

  return;
}

/**
 * This function corresponds do_init() in MINIX/device/dpeth/dp.c
 * @return OK: 1, NG: 0
 */ 
int
dp_init(int base, dpeth_t* dep)
{
  if (dep->de_mode == DEM_DISABLED) {
    dep->de_base_port = base;
    // corresponding update_conf function
    dep->de_mode = DEM_ENABLED;
    // ToDo: about memory. dep->de_linmem = ...
    if (dep->de_mode == DEM_ENABLED &&
        !ne_probe(dep)) { // Probe for NEx000
      cprintf("no ethernet card found at 0x%x\n",
              dep->de_base_port);
      return 0;
    }
    switch (dep->de_mode) {
    case DEM_DISABLED:
      return 0;
    case DEM_ENABLED: // Device is present and probed
      // Device
      if (dep->de_flags == DEF_EMPTY) {
        // These actions only the first time
        first_init(dep);
        dep->de_flags |= DEF_ENABLED;
      }
      dep->de_flags &= NOT(DEF_PROMISC | DEF_MULTI | DEF_BROAD);
      dep->de_flags |= DEF_PROMISC | DEF_MULTI | DEF_BROAD;
#if 0
      if (eth->mode & ETH_MODE_PROMISC_REQ)
        dep->de_flags |= DEF_PROMISC | DEF_MULTI | DEF_BROAD;
      if (eth->mode & ETH_MODE_MULTI_REQ)
        dep->de_flags |= DEF_MULTI;
      if (eth->mode & ETH_MODE_BROAD_REQ)
        dep->de_flags |= DEF_BROAD;
#endif
      (*dep->de_flagsf)(dep);
      break;
    case DEM_SINK: // Device not present (sink mode)
      // I don't understand..., so ignore
      return 0;
    default:
      return 0;
    }
  }
  return 1;
}

void
dp_interrupt(dpeth_t* dep)
{
  if (dep->de_mode == DEM_ENABLED) {
    dep->de_int_pending = 1;
    (*dep->de_interruptf)(dep);
    dep->de_int_pending = 0;
  }
}

void
dp_dump(dpeth_t* dep)
{
  cprintf("%s statistics:\n", dep->de_name);

  /* Network interface status  */
  cprintf("Status: 0x%x (%d)\n", dep->de_flags, dep->de_int_pending);

  (*dep->de_dumpstatsf)(dep);

  /* Transmitted/received bytes */
  cprintf("Tx bytes:  %d\n", dep->bytes_Tx);
  cprintf("Rx bytes:  %d\n", dep->bytes_Rx);

  /* Transmitted/received packets */
  cprintf("Tx OK:     %d\n", dep->de_stat.ets_packetT);
  cprintf("Rx OK:     %d\n", dep->de_stat.ets_packetR);

  /* Transmit/receive errors */
  cprintf("Tx Err:    %d\n", dep->de_stat.ets_sendErr);
  cprintf("Rx Err:    %d\n", dep->de_stat.ets_recvErr);

  /* Transmit unnerruns/receive overrruns */
  cprintf("Tx Und:    %d\n", dep->de_stat.ets_fifoUnder);
  cprintf("Rx Ovr:    %d\n", dep->de_stat.ets_fifoOver);

  /* Transmit collisions/receive CRC errors */
  cprintf("Tx Coll:   %d\n", dep->de_stat.ets_collision);
  cprintf("Rx CRC:    %d\n", dep->de_stat.ets_CRCerr);
  
  cprintf("\n");
}

