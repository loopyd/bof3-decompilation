#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original loads it once, ahead of the handler table's stores, and reads the
 * object's unsigned byte 0x01 as the step selector, so the cell is read once
 * here too. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* This overlay's two work-byte-1 step handlers. The begin step at 0x801EEC5C
 * is the lift
 * queueActionPointerTriplet,
 * which calls func_801E590C with the constants 1 and 0x1A, copies the selected
 * record's fields into the bands at 0x801EC338, 0x801EC364 and 0x801EC3A4,
 * clears the work object's byte 0x0B and advances byte 0x01. 0x801EED1C is the
 * finish step and was still an `asm` row of this target's Splat when this body
 * was last measured: it acts only when byte 0x0B reads 0xFF, then calls
 * func_800A9820, sets bit 0x04 of the halfword at 0x801462E8 and calls
 * func_801E5988. */
void queueActionPointerTriplet(
    void);
void func_801EED1C(void);

/* @source 0x801EEC04
 * @behavior Two-entry step dispatcher of this overlay. It publishes the
 *           scratchpad work object at 0x1F800044 into a two-word local handler
 *           table, {0x801EEC5C, 0x801EED1C}, and calls the entry selected by the
 *           object's unsigned byte 0x01 with no arguments. The selector is not
 *           bounded, nothing of the selected step's result is returned, and the
 *           0x20-byte frame only keeps $ra at 0x18 across the indirect call; the
 *           work-cell load precedes the table's stores. No jal and no aligned
 *           pointer word in the payload names 0x801EEC04, so the overlay loader
 *           is its only caller and the body takes no arguments. Statement order,
 *           operand forms and declaration style follow the one body of the same
 *           0x58-byte shape, the exact lift
 *           dispatchWorkByte4LocalHandlersWorld04Area17413_801F4AB4: the two
 *           spans differ in three words only, the two addiu %lo immediates of
 *           the handler addresses and the selector load's own byte offset
 *           (0x01 here, 0x04 there), with the frame, the lui/lw pair of the
 *           scratchpad cell, the scaled stack indexing and the jalr all equal.
 *           The handler addresses are this payload's own, so they are written
 *           from its bytes and not copied from the sibling. No byte-identical
 *           copy of this span exists: the 0x58 payload bytes at 0x801EEC04
 *           occur in none of the 398 overlay payloads of this repository's
 *           binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchLocalHandlers(void) {
  void (*handlers[2])(void) = {
      queueActionPointerTriplet,
      func_801EED1C};

  handlers[g_battle_work[1]]();
}
