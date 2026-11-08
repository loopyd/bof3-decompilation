#include "bof3/bof3.h"

void emitFourStepRingAtWorkPositionRadiusWorld03Area13413_801F4E64(
    void* center, s32 radius);

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; it consumes one cue id and returns
 * nothing.
 */
extern void dispatchSoundCue(u32 cue_id);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer after the helper call for the countdown
 * group and again for the state-byte advance, so the fixed-address cell symbol
 * keeps all three loads. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* The record the published cell points at, described only in the offsets this
 * body touches; its roles beyond the state, countdown and helper arguments
 * stay unknown. */
typedef struct WorkareaStepRecord {
  u8 unk_00;
  u8 mode_01;
  u8 pad_02[7];
  u8 timer_09;
  u8 pad_0a[2];
  u32 saved_0c;
  u32 saved_10;
  u32 saved_14;
  u8 pad_18[4];
  u32 unk_1c;
} WorkareaStepRecord;

/* @source 0x801F4624
 * @behavior Overlay work-record step handler, entry 5 of the work-byte-1
 * handler table based at 0x801F5828 that this target's exact
 * dispatchStateByte1Table5828World03Area13413_801F4344 indexes with the work
 * record's state byte at +0x01. The body hands the record's +0x0C block and the
 * word at +0x1C to the overlay helper at 0x801F4E64, then advances the word at
 * +0x1C by 0x1199; entry 0 at 0x801F4388 seeds that word from the copy of the
 * saved word at +0x3C and the earlier entries step it by 0x01000000 and
 * -0x00100000, so this entry takes over the interpolation with its own small
 * step. It then counts the byte at +0x09 down modulo 256 and, when that
 * countdown reaches zero, posts sound cue 0x20F through the resident dispatcher
 * at 0x8015DF18 and advances the record's state byte at +0x01 by one, so the
 * next frame dispatches entry 6 at 0x801F46AC. It never re-arms the countdown.
 * Takes no arguments and returns nothing; its 0x18-byte frame holds $s0 at
 * 0x10 and $ra at 0x14 across the call. The roles of the record's word at
 * +0x1C, of the +0x0C block and of the 0x1199 step and the 0x20F cue stay
 * described by offset.
 *
 * Name evidence: the only reference to 0x801F4624 in the payload is word 5
 * (offset 0x2C3C) of the pointer run based at 0x801F5828, whose eight words
 * name the state handlers 0x801F4388, 0x801F4400, 0x801F44C0, 0x801F453C,
 * 0x801F45A0, 0x801F4624, 0x801F46AC and 0x801F46CC. There is no jal caller,
 * which is the dispatch shape rather than missing evidence, and the advance of
 * the state byte at +0x01 towards entry 6 fixes that ordering.
 *
 * Shape evidence: world00/area026/13 holds a copy of these 0x88 bytes at
 * 0x801F2C94 (file offset 0x94) that differs in exactly two words - its helper
 * jal at 0x801F2CA4 (0x801F2D5C there, 0x801F4E64 here) and its cue immediate
 * at 0x801F2CE0 (0x20D there, 0x20F here) - so it is a relocation-and-literal
 * different sibling, shape evidence only and never clone eligibility. That
 * sibling's lifted source src/bof3/world/func_801F2C94_area02613.c supplies the
 * statement order, the cell reload points and the decrement-in-test spelling;
 * this function's own helper, step and cue come from its original bytes. No
 * payload among the 396 binaries under out/binaries carries a byte-identical
 * copy of this span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkWord1CBy1199AndRunHelperThenDispatchCue20FAndAdvanceWorkByte1World03Area13413_801F4624(
    void) {
  WorkareaStepRecord* state;

  state = (WorkareaStepRecord*)D_1F800044;
  emitFourStepRingAtWorkPositionRadiusWorld03Area13413_801F4E64(&state->saved_0c,
                                                               (s32)state->unk_1c);
  state->unk_1c += 0x1199u;

  if (--((WorkareaStepRecord*)D_1F800044)->timer_09 == 0) {
    dispatchSoundCue(0x20F);
    ((WorkareaStepRecord*)D_1F800044)->mode_01++;
  }
}
