#include "bof3/bof3.h"

extern u8 D_801462ED;

/* @source 0x800C2160
 * @behavior Raises bit 0x20 of the global flag byte at 0x801462ED (the flag
 * byte five above the shared battle flag halfword 0x801462E8 that the sibling
 * lifts in this overlay read) and returns; it reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlagBit20BossBoss02513_800C2160(void) {
  u8* flags;

  flags = &D_801462ED;
  *flags |= 0x20;
}
