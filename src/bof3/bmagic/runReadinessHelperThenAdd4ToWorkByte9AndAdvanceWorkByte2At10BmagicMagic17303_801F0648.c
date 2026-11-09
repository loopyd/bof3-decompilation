#include "bof3/bof3.h"

/* Shared readiness helper of the main executable at 0x8014DAEC, declared with
 * the eight-bit return type its own battle header gives it. This handler
 * discards the returned byte and never re-reads it, so the call is a plain
 * statement before the work-object update. */
u8 func_8014DAEC(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. The cell is re-materialised (lui / lw) after the helper call and again
 * inside the taken branch, so both accesses are fresh loads of the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0648
 * @behavior Per-frame handler of this overlay's step dispatch: the word at
 *           0x801F09A4 is entry 7 of the code-pointer run based at 0x801F0988
 *           that this target's dispatcher indexes with work-object byte 1, so
 *           the table row proves the entry boundary and the no-argument ABI.
 *           It first runs the shared readiness helper at 0x8014DAEC through a
 *           0x18-byte frame that keeps only $ra at 0x10; the helper's
 *           eight-bit result is discarded. It then adds four to scratchpad
 *           work-object byte 9 modulo 256 (lbu / addiu 4 / sb) and tests the
 *           truncated byte it just stored (andi 0xFF of the stored register,
 *           no second load of the cell) against 0x10 for equality. Only when
 *           that byte is 0x10 does it re-read the pointer cell with its own
 *           lui / lw pair and increment the object's byte 2 modulo 256
 *           (lbu / addiu 1 / sb), advancing to the next byte-2 handler step.
 *           It returns nothing through the jr $ra at 0x801F06A8 and reads or
 *           writes no other state. The roles of work-object bytes 9 and 2 are
 *           not established, so both stay offset-anchored.
 *
 *           A search of every payload under out/binaries finds no second copy
 *           of this 0x68-byte span, so the clone lever is declined. The body
 *           from the pointer-cell load onward is the shape of the reviewed
 *           exact lift add4ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic00603
 *           _801F05F4: the 0x48 bytes from the pointer-cell load through the
 *           byte-2 store are identical word for word, because both spans
 *           branch over the same seven-word advance group, and only the
 *           epilogue differs, this span restoring $ra and the frame before
 *           the jr while that leaf has no frame. The statement form, operand
 *           order and cell-reload discipline come from that lift unchanged,
 *           and the discarded helper call is this span's own leading group.
 *           The near neighbour add4ToWorkByte9AndAdvanceWork
 *           Byte2At40BmagicMagic13103_801F053C compares against 0x40 instead,
 *           and consumeRandomDecayWorkHalfwords2EAnd30AdvanceByte9By4Advance
 *           WorkByte2At10BmagicMagic04403_801F05AC adds more state, so neither
 *           supplies this body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runReadinessHelperThenAdd4ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic17303_801F0648(
    void) {
  func_8014DAEC();
  if ((g_battle_work[9] += 4) == 0x10) {
    g_battle_work[2]++;
  }
}
