#include "bof3/bof3.h"

void clearWorkTableAt800E4800World03Area13413_801F4974(void);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer for every store group below. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* Overlay-private step word: zero in the shipped image, incremented once by the
 * handler below, and its low byte is only stamped into the work record's byte at
 * offset 0x06, whose consumer is still unknown, so the role stays
 * address-anchored. */
extern u32 D_801F5818; /* @source 0x801F5818 @kind data */

/* @source 0x801F4214
 * @behavior Overlay state handler: entry 0 (work state byte 1 = 0) of the
 * handler table based at 0x801F581C that this target's dispatcher
 * dispatchStateByte1Table581cWorld03Area13413_801F41D0 indexes with the work
 * record's byte at +1. It is the run's first-step initialiser: while the
 * overlay's private word at 0x801F5818 still reads zero it calls the work-table
 * clear helper at 0x801F4974, the routine that zeroes the lead byte of the 0x20
 * entries of the 0x28-byte-stride record table at 0x800E4800; it then arms the
 * work record's countdown byte at offset 0x09 with 0x80, increments that private
 * word, stamps the incremented word's low byte into the record's byte at offset
 * 0x06, and advances the record's state byte at offset 0x01 by one through a
 * byte-width add, so the next frame dispatches entry 1 at 0x801F4288. The
 * scratchpad pointer cell 0x1F800044 is addressed and reloaded again for each
 * of the three store groups, which the fixed-address cell symbol preserves
 * where the scratchpad slot macro would have been commoned into one base. It takes no
 * arguments and returns nothing; its 0x18-byte frame only holds $ra at 0x10
 * across the single call, whose delay slot stays a nop, and that call is reached
 * only from the zero path.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableOnFirstStepArmCountdownStampStepCountAndAdvanceWorkByte1World03Area13413_801F4214(
    void) {
  u8* work;
  u32 count;

  if (D_801F5818 == 0u) {
    clearWorkTableAt800E4800World03Area13413_801F4974();
  }
  work = D_1F800044;
  work[9] = 0x80;
  count = D_801F5818 + 1u;
  work = D_1F800044;
  D_801F5818 = count;
  work[6] = (u8)count;
  work = D_1F800044;
  work[1]++;
}
