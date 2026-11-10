#include "bof3/bof3.h"

/* Slot-grid draw routine of this overlay at 0x801EF984 (0x344 bytes, still an
 * asm span of this target). It walks the three-by-eight battler-slot grid and
 * draws every slot the predicate at func_801DB524 accepts; the two handler
 * bodies of this overlay at 0x801EF8C8 and 0x801EF930 both run it first. The
 * call sets no argument registers and discards no return value. */
extern void func_801EF984(void); /* @source 0x801EF984 */

/* Shared battle helper called from many overlay step handlers, including the
 * ones this overlay reaches at 0x801EEF00 and 0x801EF108. It lives outside
 * this payload and only its address is established, so it keeps its
 * address-qualified name. */
extern void func_801E5988(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. The cell is re-materialised (lui / lw) once after the draw call, and
 * that single load feeds the byte increment and the equality test. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF930
 * @behavior Entry 2 of this overlay's three-address work-byte-2 handler table
 *           based at 0x801F0080, whose three words are 0x801EF878, 0x801EF8C8
 *           and this body, and which the dispatcher at 0x801EF834 indexes with
 *           the work object byte 0x2; entry 1 advances that byte from 1 to 2,
 *           so this body runs while byte 0x2 holds 2. It first runs the
 *           target-local slot-grid draw routine at 0x801EF984 through a
 *           0x18-byte frame that keeps only $ra at 0x10; that call returns
 *           nothing and its result is unused. It then adds one to scratchpad
 *           work-object byte 9 modulo 256 (lbu / addiu 1 / sb) and tests the
 *           truncated byte it just stored (andi 0xFF of the stored register, no
 *           second load of the cell) against 0x80 for equality. Only when that
 *           byte is 0x80 does it invoke the shared battle helper at 0x801E5988
 *           with no argument registers. It returns nothing through the jr $ra
 *           at 0x801EF97C and reads or writes no other state.
 *
 *           A byte search of every payload under out/binaries found exactly one
 *           copy of this 0x54-byte span, this one, so the clone lever was
 *           declined. The span has a frame and a jr $ra, so the data and carve
 *           levers did not apply. The body is word-for-word the shape of the
 *           exact lift runSlotGridDrawThenAdd1ToWorkByte9AndAdvanceWorkByte2
 *           At60BmagicMagic06603_801EF8C8 of this same target: the frame, the
 *           post-call cell materialisation, the andi test and the guarded
 *           branch are identical, and the only differing words are the equality
 *           immediate and the taken arm, which here is a call instead of the
 *           work-byte-2 increment. Statement order, operand forms, cell-reload
 *           discipline and declaration style come from that lift, while the
 *           equality 0x80 and the helper callee are this span's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add1ToWorkByte9AndInvokeHelperAt80BmagicMagic06603_801EF930(void) {
  func_801EF984();
  if ((g_battle_work[9] += 1) == 0x80) {
    func_801E5988();
  }
}
