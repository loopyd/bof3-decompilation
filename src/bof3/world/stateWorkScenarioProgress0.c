#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable. This target's
 * generated undefined-symbols map already binds the address, so the body
 * needs no map row for it. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */

/* @source 0x801F2C80
 * @behavior Overlay step handler, entry 1 of the work-byte-2 handler table at
 *           0x801F3DFC: reads the shared scenario progress byte and, only while
 *           it stands at 0x0D, advances the dispatch step by incrementing byte
 *           0x02 of the work record published at the scratchpad pointer cell
 *           0x1F800044. On every other progress value it leaves that byte alone
 *           and returns, so the step holds until the progress reaches 0x0D.
 *           Neighbouring entries of the same table place this handler in the
 *           overlay's palette setup: entry 0
 *           (stateAfterScalePass)
 *           advances the same step byte after running the palette block scaler
 *           with the zero scale, and entry 2
 *           (advancePaletteScaleRescaleBlocksThenResetWorkAreaAtUnityWorld04Area19813_801F2CB8)
 *           advances work-record byte 0x09, scales the palette blocks by it and
 *           calls func_80196070 once that byte reaches 0x80. This body therefore
 *           waits rather than drawing.
 *           It takes no arguments, returns nothing and reserves no stack
 *           frame, which is what keeps the record pointer in the caller-saved
 *           registers across the whole body. The scratchpad cell is read
 *           through the shared scratchpad macro, the same access form the
 *           exact dispatch table source of this target uses for that cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkScenarioProgress0(void) {
  if (g_ScenarioProgress == 0x0D) {
    SPAD_PTR_SLOT(u8, 0x44)[2]++;
  }
}
