#include "bof3/bof3.h"

/* @source 0x801F2E5C
 * @behavior Overlay init helper: clears byte 3 of each of the 0x20 records of
 * the 0x2C-byte-stride table at 0x800E4800 (the main-RAM work area this
 * overlay shares with the world and scenario overlays), walking the table with
 * an unsigned halfword counter in a bottom-tested loop; it takes no arguments,
 * returns nothing and touches no other state. Its only caller inside this
 * payload is the overlay state function at 0x801F2C48, which calls it when the
 * scratch work object's byte at offset 6 is nonzero and afterwards clears that
 * object's byte at offset 9 and increments its byte at offset 1. The 0x20-entry,
 * 0x2C-byte-stride, byte-3 shape differs from the byte-0 work-table clears in
 * emi/scenario (clearWorkTableAt800E4800...) and from the 0x28-stride clear at
 * 0x801F30E4 in emi/world04/area197/13, so this target owns its own address,
 * boundary and counter width.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableByte3At800E4800World04Area17413_801F2E5C(void) {
  u8* work;
  u16 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    work[3] = 0;
    work += 0x2Cu;
    i += 1u;
  } while (i < 0x20u);
}
