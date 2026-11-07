#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;

/* Scratch work-record pointer cell published by the overlay. The original
 * re-reads it once per access group, re-materializing its address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each read, so it is named through this
 * target's map instead of the fixed-address accessor. */
extern u8* workRecordCursor1F800044; /* @source 0x1F800044 @kind data */

/* Main-exe cue dispatcher shared with the other overlays; this target reaches it
 * through the global symbol at 0x8015DF18, which lies outside the payload. */
void func_8015DF18(u16 cue);

/* @source 0x801F4D6C
 * @behavior Overlay step handler called with no arguments. It reads the shared
 *           scenario progress byte at 0x80146864 and, only when that byte is
 *           0x32, queues main-exe sound cue 0x208 through the cue dispatcher at
 *           0x8015DF18, re-reads the scratchpad work pointer cell 0x1F800044
 *           and clears that work record byte at +0x09, then reads the cell a
 *           second time and advances the record dispatch byte at +0x02 by one,
 *           which passes control to the next handler. Every other progress
 *           value leaves the record untouched. The cell is re-read after the
 *           call because the stores through the loaded pointer can alias it.
 *           The 0x18-byte frame only keeps $ra across the call, and the
 *           function returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueCue208ResetWorkByte9AdvanceWorkByte2AtScenarioProgress32World02Area08213_801F4D6C(
    void) {
  if (g_ScenarioProgress == 0x32) {
    func_8015DF18(0x208);
    workRecordCursor1F800044[9] = 0;
    workRecordCursor1F800044[2]++;
  }
}
