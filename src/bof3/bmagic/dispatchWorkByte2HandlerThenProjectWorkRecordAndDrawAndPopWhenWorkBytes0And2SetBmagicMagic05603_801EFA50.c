#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Word 7 of the in-image code pointer run at 0x801F0744, which is where this
 * frame driver's byte-2 handler table begins; the run's own first word keeps
 * the reviewed `textbin` row for 0x801F0744 and the generated
 * undefined_syms_auto.txt row names this interior word. The original indexes
 * from it without checking the byte-2 index, so the extern is declared at the
 * indexed word: entries 0x801EFAD8, 0x801EFB38, 0x801EFBA4 and 0x801EFBF4. */
extern void (*D_801F0760[])(void); /* @source 0x801F0760 @kind data */

/* Pushes the GTE matrix stack, projects the work-record position through
 * RotTrans and installs the resulting transform with RotMatrix, MulMatrix2,
 * SetRotMatrix and SetTransMatrix. Lifted from its own span; this row used to
 * be an asm row in this target. */
extern void projectWorkRecordPositionAndInstallTransformWithZeroRotationBmagicMagic05603_801EF410(
    void); /* @source 0x801EF410 @kind function */

/* Second draw pass of the work record. Reads the graph type, selects its
 * semi-transparency mode and scale through GetGraphType, SetDrawMode,
 * SetSemiTrans and func_8014E5A0, then projects a triangle with RotTransPers3
 * and emits it through the primitive queue. Still an asm row in this target. */
extern void func_801EFC74(void); /* @source 0x801EFC74 @kind function */

/* @source 0x801EFA50
 * @behavior One frame of the overlay's second work-object frame driver over the
 *           record published at the scratchpad pointer cell 0x1F800044. It reads
 *           the cell before its frame exists, takes the record's unsigned byte 2
 *           scaled by four and calls the entry at that index of the handler table
 *           based at 0x801F0760 with no arguments, without bounding the index. It
 *           re-reads the cell after the handler returns and tests the record's
 *           byte 0 and then its byte 2, both at unsigned byte width. Only when
 *           both are non-zero does it run the record's active arm: the transform
 *           installer at 0x801EF410, the semi-transparent triangle draw pass at
 *           0x801EFC74 and PopMatrix. The 0x18-byte frame keeps only $ra at 0x10,
 *           created after the cell read and before the dispatch index is widened.
 *           It takes no arguments, returns nothing and writes no other state.
 *
 *           The record's byte 0 is its raised-flag byte, which this arm tests
 *           without masking a bit, and byte 2 is the byte-2 step counter its own
 *           handlers advance. No jal in this payload names 0x801EFA50: the span
 *           occupies word 2 of the code pointer run at 0x801F0744, whose words 0
 *           and 1 carry the byte-2 frame driver 0x801EF1F4 and whose word 3
 *           carries 0x801EFF28, so its parameter roles stay structural.
 *
 *           Statement order, operand forms and declaration style follow the exact
 *           same-target sibling
 *           dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic05603_801EF1F4:
 *           one payload cmp of the two 0x88-byte spans left exactly two differing
 *           words, this span's own table word (0x801F0760 for 0x801F0754) and its
 *           own draw callee (0x801EFC74 for 0x801EF4C0), with every relative
 *           branch and the frame identical. The 0x88-byte body is not
 *           byte-identical to any span in the 398 payloads under out/binaries, so
 *           the clone lever was declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic05603_801EFA50(
    void) {
  D_801F0760[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    projectWorkRecordPositionAndInstallTransformWithZeroRotationBmagicMagic05603_801EF410();
    func_801EFC74();
    PopMatrix();
  }
}
