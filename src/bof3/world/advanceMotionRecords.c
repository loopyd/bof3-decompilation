#include "bof3/world/area02414_internal.h"

/* @behavior integrates the local 27-entry motion table at `0x800e4940`: each
 * 0x18-byte record's z velocity gains the frame-parity bit of the shared frame
 * counter, then each of its three positions gains the matching velocity, and
 * the matching 0x28-byte target and source records are handed to
 * func_801F3708.
 * @source 0x801F362C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F362C(void) {
  u8*  source = WORLD00_AREA024_SOURCE_TABLE;
  u8*  target = WORLD00_AREA024_TARGET_BASE;
  u16* record = WORLD00_AREA024_MOTION_BASE;
  u8   i = 0u;

  do {
    record[6] = (u16)(record[6] + (D_80143E6C & 1));
    record[0] = (u16)(record[0] + record[4]);
    record[1] = (u16)(record[1] + record[5]);
    record[2] = (u16)(record[2] + record[6]);
    func_801F3708(target, source, (s16*)record);
    target += 0x28;
    source += 0x28;
    record += 0xC;
    i += 1u;
  } while (i < 0x1Bu);
}
