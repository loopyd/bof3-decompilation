#include "bof3/bof3.h"

/* @source 0x801F3474
 * @behavior Overlay init helper: clears the lead byte of each of the 0x80
 * entries of the 0x24-byte-stride record table at 0x800E5C00 (the main-RAM work
 * area this overlay shares with the world and scenario overlays), walking the
 * table with an unsigned halfword counter in a bottom-tested loop; it takes no
 * arguments, returns nothing and reads no state. That lead byte is the entry's
 * in-use flag: the scan at 0x801F3368 reads it with lbu, skips a clear entry and
 * clears it once the entry's byte-1 countdown expires, while the indexer at
 * 0x801F34A0 reaches an entry as index * 8 + index shifted left 2, the same
 * 0x24 stride, so this empties the whole table. Its only in-payload caller was
 * the init handler at 0x801F2C48 (jal at 0x801F2C68, right after the 0x800E4800
 * clear at 0x801F30E4). The 44-byte instruction shape is the same as that
 * 0x800E4800 clear in this same target up to its base and stride immediates,
 * but its body stayed an asm row when this span was seeded, so it supplied
 * shape and not a clone. The statement order and the unsigned halfword counter
 * were transcribed from clearWorkTableByte3At800E4800World04Area17413_801F2E5C
 * (0x801F2E5C in emi/world04/area174/13), whose metadata recorded it exact with
 * this counter width. No other payload in the tree builds the 0x800E5C00
 * addend, so this span has no byte-identical sibling anywhere.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800(void) {
  u8* work;
  u16 i;

  work = PSX_PTR(u8, 0x800E5C00u);
  i = 0u;

  do {
    *work = 0;
    work += 0x24u;
    i += 1u;
  } while (i < 0x80u);
}
