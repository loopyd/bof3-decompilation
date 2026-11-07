#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 g_ScenarioProgress;
extern u8 D_80146875;
/* @source 0x80148208 @kind unknown */
extern u8* D_80148208;

/* @source 0x801F2F84
 * @behavior Resets the shared status halfword D_801490A8 to 0xFFFF and then
 *           branches on the shared gate byte D_801490C7, read with a signed
 *           byte read so the cell is s8. When the gate byte is zero it stores
 *           0xB into the unsigned halfword at +0x7E of the work record
 *           published through the pointer cell D_80148208, stores 0xF into the
 *           shared scenario progress byte g_ScenarioProgress, and requests
 *           sub-state 3 through the shared sub-state byte D_80146875. When the
 *           gate byte is nonzero it requests sub-state 8 instead and touches
 *           nothing else. The 0xB store fills the branch delay slot and the
 *           two sub-state stores share one store site at the join. Takes no
 *           arguments, calls nothing, returns nothing; leaf with no frame.
 *
 * Name evidence: the status halfword at 0x801490A8, the gate byte at 0x801490C7
 * and the scenario progress byte at 0x80146864 are the shared world state cells
 * the exact siblings of this overlay family reset and map, and D_80146875 is
 * the shared sub-state byte those siblings request states through (see
 * setStatusFFFFAndScenarioProgress8IfGateElse0AState3Substate10World03Area14514_801F3C90
 * and setStatusFFFFAndStateEntryWhenGateClearWorld03Area14313_801F3B8C, which
 * keep the same reset-then-gate order). The +0x7E halfword of the work record
 * published through D_80148208 carries the WorkHalfword7E spelling that
 * setStatus35IfGateElseResetStatusAndIncrementWorkHalfword7EState0DSubstate8World03Area12013_801F4350
 * uses for the same cell of the same record.
 *
 * Shape evidence: no byte-identical sibling exists anywhere in the extracted
 * payload set (one search over out/binaries finds this span only in this
 * target), and the body has no call or relocation that another overlay could
 * share, so the range is seeded from shape siblings rather than cloned. The
 * reset stores the status halfword before the gate test and the zero arm is the
 * fall-through one, so the arm's first constant is materialised in the delay
 * slot of the test, exactly as the area143 sibling shows. The sub-state value
 * is selected by storing it in each arm rather than through a result local:
 * GCC tail-merges the two arm stores into the single store after the join,
 * which keeps each arm's constant in its own block and reproduced the original
 * on the sweep measurement and on the lane gate, while routing the value
 * through a local instead made the compiler place the else constant in the
 * branch delay slot and store through $a0 (11 of 21 instructions, 8 bytes
 * short).
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndWorkHalfword7E0BProgressFSubstate3IfGateElseSubstate8World03Area12813_801F2F84(
    void) {
  u8* record;

  D_801490A8 = 0xFFFF;
  if (D_801490C7 == 0) {
    record = D_80148208;
    *(u16*)(record + 0x7E) = 0xB;
    g_ScenarioProgress = 0xF;
    D_80146875 = 3;
  } else {
    D_80146875 = 8;
  }
}
