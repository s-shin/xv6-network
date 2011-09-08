
#define ETH_IOCTL_UNUSED
#define ETH_IOCTL_DUMP

// for user's use
typedef struct {
  uchar* addr;
  uint size;
} eth_t;

// dp.c
int dp_init(int base, dpeth_t* dep);
void dp_read(dpeth_t* dep, eth_t* eth, int vectored /* boolean */);
void dp_write(dpeth_t* dep, eth_t* eth, int vectored /* boolean */);
void dp_interrupt(dpeth_t* dep);
void dp_dump(dpeth_t* dep);



