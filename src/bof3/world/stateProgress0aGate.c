#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;

/* @source 0x801F2CF8
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8), stores 0xFFFF into the shared status
 *           halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then stores 0xA
 *           into the shared scenario progress byte g_ScenarioProgress when the
 *           gate byte reads 0 and 0x2 when it reads 1; any other gate value
 *           leaves that byte untouched. Each arm's constant is materialised in
 *           $v0 and the arm selecting 1 fills the branch delay slot of its test,
 *           and the two arm stores share one store site at the join. Takes no
 *           arguments, calls nothing, returns nothing; leaf with no frame.
 *           Shape evidence: the 64 bytes 0x801F2CF8..0x801F2D38 are identical to
 *           this target's own setStatusFFFFAndScenarioProgress2BOr2CByGate
 *           World02Area07813_801F2CB8 except the two arm constants, whose
 *           metadata records exact, so the statement shape, operand forms and
 *           cell spellings come from that sibling while the stored values come
 *           from these original bytes. The next span of this target,
 *           0x801F2D38..0x801F2D78, differs from these bytes only in the word of
 *           its internal `j`, which shifts by the 0x40 span delta, so it is the
 *           same body and not a second lift of this one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateProgress0aGate(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 0xA;
    break;
  case 1:
    g_ScenarioProgress = 0x2;
    break;
  default:
    break;
  }
}
