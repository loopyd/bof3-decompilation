#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x801F2C04
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8), stores 0xFFFF into the shared status
 *           halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then stores 0x1E
 *           into the shared scenario progress byte g_ScenarioProgress when the
 *           gate byte reads 0 and 0x32 when it reads 1; any other gate value
 *           leaves that byte untouched. Each arm's constant is materialised in
 *           $v0 and fills the branch delay slot of the test that selects it, and
 *           the two arm stores share one store site at the join reached by the
 *           `j` the non-zero, non-one gate takes. Takes no arguments, calls
 *           nothing, returns nothing; leaf with no frame, and its 0x40-byte span
 *           closes on `jr $ra` with its delay `nop`.
 *
 *           Evidence from this span's own bytes: the gate cell 0x801490C7 is
 *           read once with `lb`, the halfword store to 0x801490A8 is the only
 *           write there, the byte store goes to 0x80146864 - the shared scenario
 *           progress cell the composed shared map names - and the two arm
 *           literals are 0x1E for gate 0 and 0x32 for gate 1.
 *
 *           Shape: the sibling
 *           setStatusFFFFAndScenarioProgress64Or65ByGateWorld00Area01013_801F2C04
 *           carries the same body. A word comparison of the two 0x40-byte spans
 *           in out/binaries (offset 4 of each payload, both overlays loading at
 *           0x801F2C00) differs in exactly two bytes, the arm immediates at span
 *           offsets 0x20 and 0x2C, so the statement order, operand forms and
 *           declaration style are transcribed here with this span's own
 *           literals; the same body was lifted at 0x801F2CB8 in
 *           emi/world00/area015/13, where the only word difference is the
 *           internal `j` immediate. No span of this body is byte-identical to an
 *           authored source, so no clone was claimed.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateProgress1eGate(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 0x1E;
    break;
  case 1:
    g_ScenarioProgress = 0x32;
    break;
  default:
    break;
  }
}
