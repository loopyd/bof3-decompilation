#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;

/* @source 0x801F42F0
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8), stores 0xFFFF into the shared status
 *           halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then stores 0x14
 *           into the shared scenario progress byte g_ScenarioProgress while
 *           the gate byte is nonzero, and 0xA when it is zero. The mapped
 *           progress is a conditional expression written straight into the
 *           cell, which is why the gate load is left in v1 and the selected
 *           constant in v0. No arguments, no return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress14IfGateElse0AWorld03Area12013_801F42F0(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  g_ScenarioProgress = (gate != 0) ? 0x14 : 0xA;
}
