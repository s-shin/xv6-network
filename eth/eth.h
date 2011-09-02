
#define ETH_MODE_NOMODE         0x0
#define ETH_MODE_PROMISC_REQ    0x2
#define ETH_MODE_MULTI_REQ      0x4
#define ETH_BROAD_REQ           0x8

// for user's use
typedef struct {
  int type;
  int mode;
  char* data;
  int len;
} eth;






