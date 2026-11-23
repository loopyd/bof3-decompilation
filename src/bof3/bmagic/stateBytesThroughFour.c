#include "bof3/bof3.h"

/* Work-record pointer cell. It is the last word of the overlay data run at
 * 0x801EF9C0 and the word just past the 96-record pool at 0x801EF9F8. A store
 * through the loaded pointer may alias the cell, so the original
 * re-materialises the cell address before each of this function's five byte
 * stores. */
extern u8 *D_801F05F8; /* @source 0x801F05F8 @kind data */

/* @source 0x801EF970
 * @behavior Clears work-record bytes 0x00 through 0x04, re-reading the pointer
 *           cell at 0x801F05F8 before each byte store. Takes no arguments and
 *           returns nothing. The caller at 0x801EF418 runs it when the
 *           frame-parity countdown of record byte 0x09 reaches zero. The 0x50
 *           bytes differ from the copy at emi/bmagic/magic226/03 address
 *           0x801F0BF0 only in the ten words that carry the two address halves
 *           of that scratchpad-bound pointer cell, so this relocation-different
 *           sibling is shape evidence only and the source was seeded from the
 *           original bytes rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateBytesThroughFour(void) {
  D_801F05F8[0] = 0;
  D_801F05F8[1] = 0;
  D_801F05F8[2] = 0;
  D_801F05F8[3] = 0;
  D_801F05F8[4] = 0;
}
