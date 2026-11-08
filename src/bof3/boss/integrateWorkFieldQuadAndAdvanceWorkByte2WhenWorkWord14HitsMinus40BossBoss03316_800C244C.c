#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The original materializes the base once
 * per handler and keeps it in a caller-saved register for the whole body, and
 * it does not re-read the cell after any of the stores, so the source takes the
 * cell into one local and reaches every field through it. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Readiness poll helper of the main executable. The battle and bmagic lifts
 * declare the same prototype and discard or test its returned byte; this
 * handler discards it. */
extern u8 func_8014DAEC(void);

/* @source 0x800C244C
 * @behavior Work-step handler at index 5 of this overlay code-pointer run based
 *           at 0x800C27B4, which is the base word the step dispatcher
 *           func_800C1E70 loads and indexes with work byte 2 scaled by four; run
 *           word 5 at 0x800C27C8 holds this address, and no jal in this payload
 *           names it. It loads the scratch work object published at the pointer
 *           cell 0x1F800044 once and adds four deltas into four of that object
 *           fields, in this store order: word 0x0C into word 0x34, the
 *           pre-update word 0x14 into unsigned halfword 0x3E, word 0x10 into
 *           word 0x38, and word 0x18 into word 0x3C. It then advances word 0x14
 *           by word 0x20, and when that advanced word equals minus 0x40 it
 *           raises object byte 2, the handler index, by one. It finishes with
 *           its single call, the readiness helper func_8014DAEC, whose returned
 *           byte it discards. The word 0x14 read for the halfword 0x3E update is
 *           the one the original loads before itself stores word 0x14, which is
 *           why the body reads word 0x14 twice; the 0x18-byte frame keeps only
 *           the return address, and the body takes no arguments and returns
 *           nothing. The roles of the fields at 0x0C, 0x10, 0x14, 0x18, 0x20,
 *           0x34, 0x38, 0x3C and 0x3E are not established here, so they stay
 *           named by offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The two independent stores to 0x3E and 0x38 issue in that order, but the
 * source lists the 0x38 statement first: the scheduler orders the two hoisted
 * operand loads by RTL order, and only this statement order reproduces the
 * original load of 0x10 before the load of 0x14. */
void integrateWorkFieldQuadAndAdvanceWorkByte2WhenWorkWord14HitsMinus40BossBoss03316_800C244C(
    void) {
  u8 *work;

  work = D_1F800044;
  *(u32 *)(work + 0x34) += *(u32 *)(work + 0x0C);
  *(u32 *)(work + 0x38) += *(u32 *)(work + 0x10);
  *(u16 *)(work + 0x3E) += *(u32 *)(work + 0x14);
  *(u32 *)(work + 0x3C) += *(u32 *)(work + 0x18);
  *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
  if (*(s32 *)(work + 0x14) == -0x40) {
    work[2]++;
  }
  func_8014DAEC();
}
