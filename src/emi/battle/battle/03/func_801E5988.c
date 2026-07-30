#include "internal.h"

/* @behavior clears the current queued-slot entry's state bytes and late control
 * bytes.
 * @source 0x801E5988
 */
void func_801E5988(void) {
  ((volatile u8*)D_801EC2E0)[0x00] = 0u;
  ((volatile u8*)D_801EC2E0)[0x05] = 0u;
  ((volatile u8*)D_801EC2E0)[0x06] = 0u;
  ((volatile u8*)D_801EC2E0)[0x01] = 0u;
  ((volatile u8*)D_801EC2E0)[0x02] = 0u;
  ((volatile u8*)D_801EC2E0)[0x03] = 0u;
  ((volatile u8*)D_801EC2E0)[0x04] = 0u;
  ((volatile u8*)D_801EC2E0)[0x48] = 0u;
  ((volatile u8*)D_801EC2E0)[0x5d] = 0u;
  ((volatile u8*)D_801EC2E0)[0x5e] = 0u;
  ((u8*)D_801EC2E0)[0x5f] = 0u;
}
