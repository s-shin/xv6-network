//
// Unfortunetlly, the migration from MINIX/drivers/dpeth/dp.c is impossible,
// so this implement is original.
//

#include "minix-compatible.h"
#include "dp.h"
#include "eth.h"

#define BIOS_SEG 0 // dummy


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
      // only multicast mode &  only broadcast mode are inavailable.
      dep->de_flags |= DEF_PROMISC | DEF_MULTI | DEF_BROAD;
      //(*dep->de_flagsf)(dep);
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



