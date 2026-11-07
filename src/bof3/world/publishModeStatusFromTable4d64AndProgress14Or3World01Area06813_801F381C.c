#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u16 D_801F4D64[]; /* @source 0x801F4D64 @kind table */
extern u8 g_ScenarioProgress;

/* @source 0x801F381C
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F4D64 and
 *           publishes it into the shared status halfword D_801490A8. On the
 *           same mode value it then stores 0x14 into the shared scenario
 *           progress byte g_ScenarioProgress when the mode reads 0 and 3 when
 *           it reads 1, and any other mode leaves that byte untouched. Takes no
 *           arguments and returns nothing; leaf, no frame.
 *           Constant/variance evidence: the body follows the shape of two exact
 *           siblings, publishModeStatusFromTable539cAndProgress5World01Area06714_801F3E7C
 *           for the signed mode read and the table-index status publish, and
 *           setStatusFFFFAndScenarioProgress2BOr2CByGateWorld02Area07813_801F2CB8
 *           for the switch on that mode whose merged store site at the join
 *           takes each arm constant in a branch delay slot. This function's own
 *           literals -- the table base 0x801F4D64 and the mapped values 0x14
 *           and 3 -- come from these original bytes, and the 80-byte span has
 *           no byte-identical copy in any shipped payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable4d64AndProgress14Or3World01Area06813_801F381C(
    void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F4D64[mode];
  switch (mode) {
  case 0:
    g_ScenarioProgress = 0x14;
    break;
  case 1:
    g_ScenarioProgress = 3;
    break;
  default:
    break;
  }
}
