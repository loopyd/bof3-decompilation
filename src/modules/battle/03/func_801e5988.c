#include "internal.h"

extern volatile Battle03QueuedSlot* DAT_801ec2e0;

/* does: clears the current queued-slot entry's state bytes and late control
 * bytes.
 * @source: 0x801e5988 FUN_801e5988
 */
void func_801e5988(void) {
  ((volatile u8*)DAT_801ec2e0)[0x00] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x05] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x06] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x01] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x02] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x03] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x04] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x48] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x5d] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x5e] = 0u;
  ((volatile u8*)DAT_801ec2e0)[0x5f] = 0u;
}
