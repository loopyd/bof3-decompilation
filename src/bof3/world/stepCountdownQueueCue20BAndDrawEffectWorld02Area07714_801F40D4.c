#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;

/* Overlay draw helper at 0x801F4B80: loads work words +0x34/+0x38/+0x3C, takes
 * two rand offsets and submits the resulting quad through the shared prim
 * helpers, so the span it draws is offset from the published work position. */
void func_801F4B80(void);
/* Shared EXE-side cue dispatcher. */
void func_8015DF18(u32 arg0);
/* Shared EXE-side work-area reset. */
void func_80196070(void);

/* @source 0x801F40D4
 * @behavior Overlay work-byte-1 handler, entry 1 of the local handler table at
 *           0x801F6270: reads the work pointer from scratchpad cell 0x1F800044
 *           and, whenever the signed countdown word at offset 0x0C is a
 *           multiple of 30, dispatches the front-end cue 0x20B through
 *           func_8015DF18. It re-reads the work pointer and, while that
 *           countdown modulo 30 stays below 15, runs the overlay draw helper at
 *           0x801F4B80, then re-reads the work pointer once more, decrements
 *           the countdown and calls func_80196070, which resets the scratchpad
 *           work area, when the countdown reaches zero. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra and $s0, which
 *           holds the 0x88888889 reciprocal shared by both modulo words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepCountdownQueueCue20BAndDrawEffectWorld02Area07714_801F40D4(void) {
  if (*(s32*)(workRecordCursor1F800044 + 0x0C) % 30 == 0) {
    func_8015DF18(0x20B);
  }
  if (*(s32*)(workRecordCursor1F800044 + 0x0C) % 30 < 15) {
    func_801F4B80();
  }
  if (--*(s32*)(workRecordCursor1F800044 + 0x0C) == 0) {
    func_80196070();
  }
}
