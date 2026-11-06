#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;
extern u8 D_80146874;
extern u8 D_80146875;

/* @source 0x801F3C90
 * @behavior Overlay reset handler: reads the shared gate byte D_801490C7 with a
 *           signed byte read and stores 0xFFFF into the shared status halfword
 *           D_801490A8. When the gate byte is nonzero it stores 8 into the
 *           shared scenario progress byte g_ScenarioProgress and jumps to the
 *           single return; when the gate byte is zero it stores 0xA into that
 *           progress byte and also requests primary state 3 with sub-state 10
 *           by storing 3 into D_80146874 and 0xA into D_80146875. The nonzero
 *           arm is the fall-through one, so the stored 8 is materialised in the
 *           branch delay slot, and the zero arm's two 0xA stores share one
 *           constant register. D_80146874 and D_80146875 are the shared
 *           primary-state and sub-state bytes the scenario overlays request
 *           states through. Takes no arguments, calls nothing, returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress8IfGateElse0AState3Substate10World03Area14514_801F3C90(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  if (gate != 0) {
    g_ScenarioProgress = 8;
  } else {
    g_ScenarioProgress = 0xA;
    D_80146874 = 3;
    D_80146875 = 0xA;
  }
}
