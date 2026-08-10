#include "bof3/scenario/sce10eff_internal.h"

/* @behavior advances scratch byte 10 by two while it is below 16; otherwise
 * sets scratch byte 9 to 60 and advances scratch byte 3.
 * @source 0x801D1EFC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D1EFC(void) {
  u8* work;
  u8 value;
  u32 slot_offset;

  slot_offset = 0x44u;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  value = work[0x0a];
  if (value < 0x10u) {
    work[0x0a] = (u8)(value + 2u);
  } else {
    work[0x09] = 60;
    work = PSX_REF(u8*, SPAD_BASE + slot_offset);
    work[0x03]++;
  }
}
