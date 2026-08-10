#include "bof3/scenario/sce10eff_internal.h"

/* @source 0x801D10C8
 * @behavior Advances scratch byte 11, then advances byte 10 by two while it is below 18; otherwise resets byte 9 to 60 and advances byte 3.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D10C8(void) {
  u8* scratch;
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[11]++;

  scratch = PSX_REF(u8*, SPAD_BASE + slot_offset);
  if (scratch[10] < 18) {
    scratch[10] += 2;
  } else {
    scratch[9] = 60;
    scratch = PSX_REF(u8*, SPAD_BASE + slot_offset);
    scratch[3]++;
  }
}
