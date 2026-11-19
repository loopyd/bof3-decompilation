#include "bof3/bof3.h"

extern u16 D_801F59E4; /* @source 0x801F59E4 @kind data */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* @source 0x801F2C48
 * @behavior Byte-1 state handler that the overlay's first byte-1 dispatch
 *           table (0x801F5124, indexed by the dispatcher at 0x801F2C04) lists
 *           at entry 0. Reads the work record published at the scratchpad
 *           pointer cell 0x1F800044, stores zero into the target halfword at
 *           0x801F59E4 and increments the record state byte at 0x01, so the
 *           next dispatch takes entry 1 of the same table (0x801F2C6C). The
 *           halfword it clears is the only other cell those two handlers
 *           touch besides the work record: entry 1 reads it back as a flag bit
 *           15, a seven-bit counter at bits 8 to 14 and an eight-bit counter at
 *           bits 0 to 7, and advances the same state byte once that eight-bit
 *           counter reaches 15. Takes no arguments and returns nothing; the
 *           body is a leaf with no frame, the work pointer is loaded once into
 *           $v1 and both record accesses use it, and the halfword store goes
 *           through $at.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetStepCounterHalfwordAndAdvanceWorkByte1World03Area11813_801F2C48(void) {
  u8* work = D_1F800044;

  D_801F59E4 = 0;
  work[1]++;
}
