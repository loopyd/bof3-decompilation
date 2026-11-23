#include "bof3/bof3.h"

/* @source 0x801F30E4
 * @behavior Overlay init helper: clears the lead byte of each of the 0x80
 * entries of the 0x28-byte-stride record table at 0x800E4800 (the main-RAM work
 * area this overlay shares with the world and scenario overlays), walking the
 * table with an unsigned halfword counter in a bottom-tested loop; it takes no
 * arguments, returns nothing and reads no state. Its only in-payload caller is
 * the init handler at 0x801F2C48, whose jal at 0x801F2C60 reaches it before the
 * 0x800E5C00 clear at 0x801F3474. The scan at 0x801F2FC4 walks the same table
 * with the same 0x80-entry, 0x28-byte stride and reads that lead byte with lbu
 * as the entry's live flag, storing it back to zero once the entry's byte-1
 * countdown expires, so this empties the whole table. The 44-byte body matches
 * the 0x800E5C00 clear in this target and the 0x800E4800 clears in
 * emi/world04/area174/13 and emi/scenario/scena01/00 up to its base low
 * halfword, its stride and its counter width; a comparison against every one of
 * the 398 extracted payloads found no byte-identical span outside its own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkTableStride28(void) {
  u8* work;
  u16 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x28u;
    i += 1u;
  } while (i < 0x80u);
}
