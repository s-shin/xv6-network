
// shin: make compatiblity between xv6 & minix
#include "../types.h"
#include "../x86.h"
#include "../defs.h"
typedef uchar u8_t;
typedef ushort u16_t;
typedef uint u32_t;
#define TRUE 1
#define FALSE 0
typedef unsigned long phys_bytes;
typedef unsigned long vir_bytes;
typedef u16_t port_t;
typedef long clock_t;
typedef struct {
  vir_bytes iov_addr; // address of an I/O buffer
  vir_bytes iov_size; // size of an I/O buffer
} iovec_t;
#define printf cprintf
#define sys_irqdisable(a) // ignore? or create ioapicdisable()?

#include "minix-include/ether.h"
#include "minix-include/eth_io.h"
#undef USE_IOPL

