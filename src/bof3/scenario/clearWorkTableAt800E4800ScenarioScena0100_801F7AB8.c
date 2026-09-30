#include "bof3/bof3.h"


/* @source 0x801F7AB8
 * @behavior Overlay init helper: clears the lead byte of each of the 0x80
 * entries of the 0x14-byte-stride work table at 0x800E4800 (the main-RAM work
 * area this overlay shares with the world and scenario overlays), walking the
 * table with an unsigned byte counter in a bottom-tested loop; it takes no
 * arguments, returns nothing and touches no other state. Its only caller inside
 * this payload is the overlay's state-0 handler at 0x801F73FC, which seeds the
 * scratchpad work object and then clears this table. The identical 44 bytes
 * appear at 0x801F75AC in emi/scenario/scena15/00 and at 0x801F3910 in
 * emi/world02/area082/13.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800ScenarioScena0100_801F7AB8(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x14u;
    i += 1u;
  } while (i < 0x80u);
}
