#include "bof3/bof3.h"

/* @source 0x801F4974
 * @behavior Overlay init helper: clears the lead byte of each of the 0x20
 * entries of the 0x28-byte-stride record table at 0x800E4800 (the main-RAM work
 * area this overlay shares with the world and scenario overlays), walking the
 * table with an unsigned byte counter in a bottom-tested loop; it takes no
 * arguments, returns nothing and reads no state. Its 44 bytes are the same
 * shape and stride as clearWorkTableAt800E4800ScenarioScena0800_801F7DE8
 * (0x801F7DE8 in emi/scenario/scena08/00) but with an entry count of 0x20
 * instead of 0x40, so it is not a byte-identical duplicate of that function;
 * this target owns its own address and boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800World03Area13413_801F4974(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x28u;
    i += 1u;
  } while (i < 0x20u);
}
