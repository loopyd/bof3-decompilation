#include "bof3/bof3.h"

/* @source 0x801F38BC
 * @behavior Overlay init helper: clears the lead byte of each of the first 8
 *           entries of the 0x18-byte-stride work table at 0x800E4800 (the
 *           main-RAM work area this overlay shares with the world and scenario
 *           overlays), walking the block with an unsigned byte counter in a
 *           bottom-tested loop; it takes no arguments, returns nothing and reads
 *           no state. Its only caller inside this payload is func_801F30F8 at
 *           0x801F3134, which calls it when the scenario progress byte equals 10,
 *           immediately before walking the 0x20-entry, 0x2C-byte-stride block
 *           that starts at 0x800E48C0 - i.e. at 0x800E4800 + 8 * 0x18, which
 *           corroborates the entry count and stride. The 44 bytes are
 *           byte-identical to the already-exact
 *           clearWorkTableAt800E4800World02Area09813_801F5240 lift in
 *           emi/world02/area098/13, which is the naming evidence for the table
 *           this loop clears.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800World04Area17213_801F38BC(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x18u;
    i += 1u;
  } while (i < 8u);
}
