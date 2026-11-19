#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress;

/* @source 0x801F2C04
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8), stores 0xFFFF into the shared status
 *           halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then stores 0x64
 *           into the shared scenario progress byte g_ScenarioProgress when the
 *           gate byte reads 0 and 0x65 when it reads 1; any other gate value
 *           leaves that byte untouched. Each arm's constant is materialised in
 *           $v0 and fills the branch delay slot of the test that selects it, and
 *           the two arm stores share one store site at the join reached by the
 *           `j` the non-zero, non-one gate takes. Takes no arguments, calls
 *           nothing, returns nothing; leaf with no frame.
 *
 *           Evidence from this span's own bytes: the gate cell 0x801490C7 is
 *           read once, the halfword store to 0x801490A8 is the only write there,
 *           the byte store goes to 0x80146864 - the shared scenario progress
 *           cell the composed shared map names - and the two arm literals are
 *           0x64 for gate 0 and 0x65 for gate 1.
 *
 *           Shape: the same body was lifted exact at 0x801F2CB8 in
 *           emi/world00/area015/13, and a word comparison of the two 0x40-byte
 *           spans in out/binaries differs in one word only, that span's internal
 *           `j` immediate, which shifts by exactly the 0xB4 offset between the
 *           two spans. A byte-identical copy of this span also sits at offset 4
 *           of out/binaries/emi/world00/area010/13.bin, where it is still an
 *           `asm` row and so has no authored source to clone; the clone
 *           adapter was therefore not run and the statement order above was
 *           transcribed from the exact area015 sibling.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress64Or65ByGateWorld00Area00513_801F2C04(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 0x64;
    break;
  case 1:
    g_ScenarioProgress = 0x65;
    break;
  default:
    break;
  }
}
