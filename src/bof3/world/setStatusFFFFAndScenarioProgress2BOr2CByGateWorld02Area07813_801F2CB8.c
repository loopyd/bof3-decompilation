#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;

/* @source 0x801F2CB8
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8), stores 0xFFFF into the shared status
 *           halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then stores 0x2B
 *           into the shared scenario progress byte g_ScenarioProgress when the
 *           gate byte reads 0 and 0x2C when it reads 1; any other gate value
 *           leaves that byte untouched. Each arm's constant is materialised in
 *           $v0 and the arm selecting 1 fills the branch delay slot of its test,
 *           and the two arm stores share one store site at the join. Takes no
 *           arguments, calls nothing, returns nothing; leaf with no frame.
 *           Constant/variance evidence: the 64 bytes 0x801F2CB8..0x801F2CF8 are
 *           identical to the same-address function of BIN/WORLD00/AREA015.EMI#13
 *           except for the two arm constants (0x64/0x65 there), so the statement
 *           shape, operand forms and cell spellings come from that exact sibling
 *           while the stored values come from these original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress2BOr2CByGateWorld02Area07813_801F2CB8(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 0x2B;
    break;
  case 1:
    g_ScenarioProgress = 0x2C;
    break;
  default:
    break;
  }
}
