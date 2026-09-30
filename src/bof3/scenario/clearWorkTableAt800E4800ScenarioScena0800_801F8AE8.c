#include "bof3/bof3.h"


/* @source 0x801F8AE8
 * @behavior Overlay init helper: clears the lead byte of each of the 0x8
 * entries of the 0x1C-byte-stride record table at 0x800E4800 (the main-RAM work
 * area this overlay shares with the world and scenario overlays), walking the
 * table with an unsigned byte counter in a bottom-tested loop; it takes no
 * arguments, returns nothing and reads no state. Its 44 bytes have the same
 * shape as clearWorkTableAt800E4800ScenarioScena0100_801F7AB8 (0x801F7AB8 in
 * emi/scenario/scena01/00) but a different entry count (0x8) and stride (0x1C),
 * so it is not a byte-identical duplicate of that function; this target owns
 * its own address and boundary. Its only caller inside this payload is the
 * function at 0x801F79B0, which is the first word of a pointer table at
 * 0x801FE8D8 and afterwards advances the scratch work object's byte at offset
 * 0x02 and stores 0x40 to that object's halfword at offset 0x5A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800ScenarioScena0800_801F8AE8(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x1Cu;
    i += 1u;
  } while (i < 0x8u);
}
