#include "bof3/bof3.h"


/* @source 0x801F7DE8
 * @behavior Overlay init helper: clears the lead byte of each of the 0x40
 * entries of the 0x28-byte-stride record table at 0x800E4800 (the main-RAM work
 * area this overlay shares with the world and scenario overlays), walking the
 * table with an unsigned byte counter in a bottom-tested loop; it takes no
 * arguments, returns nothing and reads no state. Its 44 bytes have the same
 * shape as clearWorkTableAt800E4800ScenarioScena0100_801F7AB8 (0x801F7AB8 in
 * emi/scenario/scena01/00) but a different entry count (0x40) and stride
 * (0x28), so it is not a byte-identical duplicate of that function; this target
 * owns its own address and boundary. Its only caller inside this payload is the
 * function at 0x801F6F8C, which is the first word of a pointer table at
 * 0x801FE894 and afterwards advances the scratch work object's byte at offset
 * 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800ScenarioScena0800_801F7DE8(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x28u;
    i += 1u;
  } while (i < 0x40u);
}
