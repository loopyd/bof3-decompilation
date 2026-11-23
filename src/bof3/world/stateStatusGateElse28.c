#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;

/* @source 0x801F4320
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8), stores 0xFFFF into the shared status
 *           halfword D_801490A8 unconditionally (`ori v0,0,0xFFFF`, `sh`),
 *           then stores 0x1E into the shared scenario progress byte
 *           g_ScenarioProgress while the gate byte is nonzero, and 0x28 when
 *           it is zero. The mapped progress is a conditional expression
 *           written straight into the cell, which is why the gate load is
 *           left in v1 and the selected constant in v0. Leaf: no arguments,
 *           no return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusGateElse28(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  g_ScenarioProgress = (gate != 0) ? 0x1E : 0x28;
}
