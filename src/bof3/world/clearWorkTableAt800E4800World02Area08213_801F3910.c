#include "bof3/bof3.h"

/* @source 0x801F3910
 * @behavior Overlay helper: clears the lead byte of each of the 0x80 entries of
 *           the 0x14-byte-stride work table at 0x800E4800 (the main-RAM work
 *           area this overlay shares with the world and scenario overlays),
 *           walking the table with an unsigned byte counter in a bottom-tested
 *           loop; it takes no arguments, returns nothing and touches no other
 *           state. Its only caller inside this payload is func_801F3398 at
 *           0x801F3398. The 44 bytes are byte-identical to the already-exact
 *           clearWorkTableAt800E4800ScenarioScena0100_801F7AB8 lift in
 *           emi/scenario/scena01/00 (same base, stride and entry count), which
 *           is the naming evidence for the table this loop clears.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800World02Area08213_801F3910(void) {
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
