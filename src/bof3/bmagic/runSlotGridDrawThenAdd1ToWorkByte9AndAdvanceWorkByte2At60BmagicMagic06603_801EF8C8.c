#include "bof3/bof3.h"

/* Slot-grid draw routine of this overlay at 0x801EF984 (0x344 bytes, still an
 * asm span of this target). It walks the three-by-eight battler-slot grid and,
 * for every slot the predicate at func_801DB524 accepts, publishes the slot row
 * and index into work bytes 0xB and 0xA, writes the slot position triplet over
 * work words 0x34 / 0x38 / 0x3C from the record stride at 0x801EB664 plus the
 * corner offsets at 0x801F008C, and draws the slot through func_801EFFB0 and
 * func_801EFCC8 under a PopMatrix. Its trailing loop raises each of the four
 * per-slot counters at 0x801F00B0 while three times the slot index stays below
 * work byte 9, and masks a counter with 0xF when work byte 2 is 1. The call sets
 * no argument registers and discards no return value. */
extern void func_801EF984(void); /* @source 0x801EF984 */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. The cell is re-materialised (lui / lw) after the draw call and again
 * inside the taken branch, so both accesses are fresh loads of the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF8C8
 * @behavior Entry 1 of this overlay's three-address work-byte-2 handler table
 *           based at 0x801F0080 (0x801EF878, this body, 0x801EF930), which the
 *           dispatcher at 0x801EF834 indexes with the work object byte 0x2, so
 *           byte 0x2 holds 1 while this body runs. It first runs the
 *           target-local slot-grid draw routine at 0x801EF984 through a
 *           0x18-byte frame that keeps only $ra at 0x10; that call returns
 *           nothing and its result is unused. It then adds one to scratchpad
 *           work-object byte 9 modulo 256 (lbu / addiu 1 / sb) and tests the
 *           truncated byte it just stored (andi 0xFF of the stored register, no
 *           second load of the cell) against 0x60 for equality. Only when that
 *           byte is 0x60 does it re-read the pointer cell with its own lui / lw
 *           pair and increment the object's byte 2 modulo 256 (lbu / addiu 1 /
 *           sb), which advances the byte-2 handler past the last grid step. It
 *           returns nothing through the jr $ra at 0x801EF928, reads or writes no
 *           other state, and keeps no locals beyond the saved $ra.
 *
 *           A byte search of every payload under out/binaries finds no second
 *           copy of this 0x68-byte span, so the clone lever is declined. The
 *           span also has a frame and a jr $ra, so the data and carve levers do
 *           not apply. The body is word-for-word the shape of the exact lift
 *           runReadinessHelperThenAdd4ToWorkByte9AndAdvanceWorkByte2At10Bmagic
 *           Magic17303_801F0648: the frame, the two pointer-cell
 *           re-materialisations, the andi test and the guarded byte-2 increment
 *           are identical, and the only differing words are the jal target,
 *           the addend and the equality immediate. The statement order, operand
 *           forms, cell-reload discipline and declaration style come from that
 *           lift, while the addend 1, the equality 0x60 and the grid-draw
 *           callee are this span's own. The roles of work-object bytes 9 and 2
 *           beyond their table dispatch are not established, so both stay
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSlotGridDrawThenAdd1ToWorkByte9AndAdvanceWorkByte2At60BmagicMagic06603_801EF8C8(
    void) {
  func_801EF984();
  if ((g_battle_work[9] += 1) == 0x60) {
    g_battle_work[2]++;
  }
}
