#include "bof3/scenario/sce10eff_internal.h"

/* @behavior advances byte 11 and counts down byte 9 of the scratchpad work,
 * advancing byte 3 when the countdown reaches zero.
 * @source 0x801D1138
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D1138(void) {
  u8* work;
  u32 slot_offset;

  slot_offset = 0x44u;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  work[0x0b]++;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  work[0x09]--;
  if (work[0x09] == 0u) {
    work = PSX_REF(u8*, SPAD_BASE + slot_offset);
    work[0x03]++;
  }
}
