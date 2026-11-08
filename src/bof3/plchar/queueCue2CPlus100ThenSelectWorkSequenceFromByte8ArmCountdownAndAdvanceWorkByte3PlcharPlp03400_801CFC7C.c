#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell once before frame creation, re-reads it after the cue
 * call and again before each of the two stores, because a store through the
 * pointer may alias the cell itself; the source keeps the named-symbol form and
 * evaluates it at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Cue queue routine of the shared EXE image, outside this payload. The original
 * calls it through a plain `jal` with the work halfword 0x2C plus 0x100 in $a0,
 * computes that argument before the frame is created, and the added constant
 * issues in the call's own delay slot. The exact sibling
 * adjustEvenRouteByteThenAdvanceStepWhenRouteSampleAbove64ElseClaimWorkProbeLayersSelectSequence42PlcharPlp02500_801CE770
 * of this family declares the same word argument and passes the same
 * expression. */
void func_8015DF18(u32 arg0);

/* Work-sequence selector of the shared EXE image, outside this payload. The
 * original passes the sequence ((work byte 8 - 1) / 2 + 0x42) truncated to a
 * byte, which keeps both divisions signed, and masks the value in the call's
 * own delay slot. The same family sibling passes the same expression. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CFC7C
 * @behavior Overlay step dispatched on work-record byte 3: the task tables of
 *           this payload place its address at the pointer word 0x801D0334, which
 *           is entry 0 of the twenty-eight-word handler run based at 0x801D02F4,
 *           so dispatchWorkByte3Table0334PlcharPlp03400_801CFC38 reaches it when
 *           work byte 3 reads zero. It reads the work record published at the
 *           scratchpad pointer cell 0x1F800044 and, in order, queues the cue
 *           taken from the record's unsigned halfword 0x2C plus 0x100 through
 *           func_8015DF18, selects the shared work sequence
 *           ((record byte 8 - 1) / 2 + 0x42) truncated to a byte through
 *           func_8014D8D4, arms the countdown byte 0x0A with 8 and sets the
 *           dispatch byte 3 to 1. Both calls answer nothing it keeps; it has no
 *           `jal` caller and takes no arguments. The subtraction leaves the
 *           division signed, so a record byte 8 of zero halves -1 to zero and
 *           still selects sequence 0x42. The pointer cell is loaded before the
 *           frame is created and re-read after the cue call and before each of
 *           the two stores; the 0x18-byte frame keeps only $ra at 0x10, the
 *           countdown constant is materialised after the second call and the
 *           dispatch constant 1 is materialised before $ra is restored. The
 *           120-byte span at payload offset 0x187C sits exactly between the two
 *           declared rows that bound it. The nearest exact sibling
 *           refreshLayersThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp26700_801CF75C
 *           repeats this tail statement order with a layer refresh instead of
 *           the cue call and countdown 0xB, and
 *           adjustEvenRouteByteThenAdvanceStepWhenRouteSampleAbove64ElseClaimWorkProbeLayersSelectSequence42PlcharPlp02500_801CE770
 *           supplied both shared call shapes, so the shape is evidenced across
 *           overlays while the constants here come from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue2CPlus100ThenSelectWorkSequenceFromByte8ArmCountdownAndAdvanceWorkByte3PlcharPlp03400_801CFC7C(
    void) {
  func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
  func_8014D8D4((u8)((D_1F800044[8] - 1) / 2 + 0x42));
  D_1F800044[0xA] = 8;
  D_1F800044[3] = 1;
}
