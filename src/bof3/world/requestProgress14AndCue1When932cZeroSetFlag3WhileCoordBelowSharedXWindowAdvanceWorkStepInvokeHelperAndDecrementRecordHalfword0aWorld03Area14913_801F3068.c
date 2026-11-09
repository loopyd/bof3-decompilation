#include "bof3/bof3.h"

/* Shared signed halfword at 0x8014932C. Step 2 of this overlay's 0x801F4580
 * handler run uses it as a run-once gate: the original loads it with a signed
 * halfword read and only a zero value enters the block. Store-free here, so the
 * role stays address-anchored.
 * @source 0x8014932C @kind data
 */
extern s16 D_8014932C;

/* Shared flag byte at 0x80147AD0. This step stores 3 into it once the shared
 * coordinate word has reached the window, while step 0 stores 1, so the role
 * stays address-anchored. Store width comes from the original's single sb.
 * @source 0x80147AD0 @kind data
 */
extern u8 D_80147AD0;

/* Shared scenario progress byte; the overlay handlers publish the next progress
 * value here and this step's value is 0x14. Store width comes from the
 * original's single sb.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

/* Shared record pointer cell at 0x80148210. Its +0x0A halfword is the countdown
 * every step of this script subtracts 2 from; the original loads the cell once
 * per step and stores through the loaded pointer.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Shared X channel word at 0x80149308, read into a signed register and widened
 * by the 0x20000 window this step compares against. Signed width comes from the
 * original's slt.
 * @source 0x80149308 @kind data
 */
extern s32 D_80149308;

/* Shared coordinate word at 0x80147A8C, head of the three-word run that
 * func_801F2EC4 copies into work record bytes 0x34 and on. Signed width comes
 * from the original's slt on the loaded value.
 * @source 0x80147A8C @kind data
 */
extern s32 D_80147A8C;

/* Resident front-end cue dispatcher; this step queues cue 1. */
void func_8015DF18(u16 cue_id);

/* Local step helper at 0x801F3140. The four handlers of the 0x801F4580 run call
 * it with -4, -3, -2 and -1, so step 2 passes -2. */
void func_801F3140(s16 step_code);

/* @source 0x801F3068
 * @behavior Step 2 of this overlay's four-entry handler run based at 0x801F4580,
 * which func_801F2EFC indexes with work byte 4. Reads the shared signed
 * halfword 0x8014932C once, before the frame is set up, and only when it is zero
 * stores 0x14 into the scenario progress byte, queues front-end cue 1 through
 * func_8015DF18 and increments byte 4 of the scratchpad work record published at
 * pointer cell 0x1F800044 so the dispatcher advances to step 3. Whether or not
 * that gate fires it then compares the shared coordinate word 0x80147A8C
 * against the shared X channel 0x80149308 widened by a 0x20000 window, stores 3
 * into the flag byte 0x80147AD0 while the coordinate is still below that window,
 * calls the local step helper func_801F3140 with -2 and subtracts 2 from the
 * +0x0A halfword of the record published through 0x80148210. Takes no arguments
 * and returns nothing; the 0x18-byte frame keeps $ra at +0x10 and the
 * original's four unfilled load delay slots stay nops.
 *
 * Name evidence: the role comes from the payload's own dispatch table. The four
 * words at 0x801F4580 are 0x801F2F40, 0x801F2FC4, 0x801F3068 and 0x801F3108
 * (payload offset 0x1980, read as little-endian words), and func_801F2EFC loads
 * the scratchpad work record, scales its byte 4 by four and calls the pointer at
 * that index, so this body is the third entry, step 2, and its own byte-4
 * increment is the step advance. No in-image caller exists beyond that table
 * word; the overlay is reached through its dispatch tables.
 *
 * Shape evidence: func_801F2F40 was an exact lift in this target at the time
 * this seed was written and shares the 0x18-byte frame, the shared-halfword gate
 * read before the prologue, the cue-then-advance byte-4 idiom, the -N call into
 * 0x801F3140 and the subtract-2 tail on the record's +0x0A halfword, so its
 * statement order, pointer-cell spelling and record-offset arithmetic are reused
 * here. func_801F2FC4, a row that was still asm, carries the same 0x80147A8C
 * against 0x80149308 plus 0x20000 window store and differs only in its own gate
 * (0x8014932C below 0x680), progress value (0xF), cue argument (0) and helper
 * code (-3), so it is shape evidence for that store and not a clone. The store
 * fires while the signed compare 0x80147A8C below 0x80149308 + 0x20000 still
 * holds, which is the polarity the original's slt plus beqz pair shows. No payload
 * under out/binaries carries these 160 bytes at any other offset, so no
 * byte-identical sibling exists to clone from.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestProgress14AndCue1When932cZeroSetFlag3WhileCoordBelowSharedXWindowAdvanceWorkStepInvokeHelperAndDecrementRecordHalfword0aWorld03Area14913_801F3068(
    void) {
  s16 countdown;
  u8* work;

  countdown = D_8014932C;
  if (countdown == 0) {
    g_ScenarioProgress = 0x14;
    func_8015DF18(1u);
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[4] += 1;
  }
  if (D_80147A8C < D_80149308 + 0x20000) {
    D_80147AD0 = 3;
  }
  func_801F3140(-2);
  D_80148210[5] -= 2;
}
