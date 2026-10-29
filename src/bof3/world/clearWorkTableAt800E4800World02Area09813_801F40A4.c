#include "bof3/bof3.h"

/* @source 0x801F40A4
 * @behavior Overlay helper: clears the lead byte of each of the 0x18 entries of
 *           the 0x38-byte-stride work table at 0x800E4800 (the main-RAM work
 *           area this overlay shares with the world and scenario overlays),
 *           walking the table with an unsigned byte counter in a bottom-tested
 *           loop; it takes no arguments, returns nothing and touches no other
 *           state. Its only caller inside this payload is func_801F3540 at
 *           0x801F3540, a state handler that clears this table before seeding
 *           the scratchpad work object at 0x1F800044 and queuing cue 0x204. The
 *           44 bytes are byte-identical to the already-exact
 *           clearWorkTableAt800E4800World02Area08213_801F3910 lift in
 *           emi/world02/area082/13 (same base and loop shape; stride 0x14 and
 *           0x80 entries there), which is the naming evidence for the table
 *           this loop clears.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800World02Area09813_801F40A4(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x38u;
    i += 1u;
  } while (i < 0x18u);
}
