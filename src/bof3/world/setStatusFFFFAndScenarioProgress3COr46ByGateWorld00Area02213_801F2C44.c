#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x801F2C44
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb` at 0x801F2C48, so the cell is s8), stores 0xFFFF into the
 *           shared status halfword D_801490A8 (`ori v0,0,0xFFFF` at
 *           0x801F2C4C, the span's only `sh`, at 0x801F2C54), then stores 0x3C
 *           into the shared scenario progress byte g_ScenarioProgress when the
 *           gate byte reads 0 and 0x46 when it reads 1; any other gate value
 *           leaves that byte untouched. Each arm constant is materialised in
 *           $v0 and fills the branch delay slot of the equality test that
 *           selects it (0x3C at 0x801F2C70 after `beqz`, 0x46 in the delay slot
 *           at 0x801F2C64 after `beq`), and both arms share one store site
 *           (`sb` at 0x801F2C78) reached by the `j` at 0x801F2C68 that the
 *           other gate values take. Takes no arguments, calls nothing, returns
 *           nothing: a frameless leaf whose 0x40-byte span closes on `jr $ra`
 *           at 0x801F2C7C with its delay `nop`.
 *
 *           Evidence from this span's own bytes: the gate cell 0x801490C7 is
 *           read once, signed; the halfword store to 0x801490A8 is the only
 *           write there; the byte store goes to 0x80146864, the shared scenario
 *           progress cell the composed shared map names; the two arm literals are
 *           0x3C for gate 0 and 0x46 for gate 1.
 *
 *           Shape: the sibling
 *           setStatusFFFFAndScenarioProgress1EOr32ByGateWorld00Area02213_801F2C04
 *           carries the same body in this same payload. A word comparison of the
 *           two 0x40-byte spans (payload offsets 4 and 0x44, both overlays
 *           loading at 0x801F2C00) differs in exactly three words: the two arm
 *           immediates at span offsets 0x20 and 0x2C, and the internal `j`
 *           immediate at span offset 0x24, whose target address moves by exactly
 *           the 0x40 delta between the two spans. The statement order, operand
 *           forms and declaration style are transcribed here with this span's own
 *           literals. No 0x40-byte span byte-identical to this body exists in any
 *           payload under out/binaries, so no clone was claimed.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress3COr46ByGateWorld00Area02213_801F2C44(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 0x3C;
    break;
  case 1:
    g_ScenarioProgress = 0x46;
    break;
  default:
    break;
  }
}
