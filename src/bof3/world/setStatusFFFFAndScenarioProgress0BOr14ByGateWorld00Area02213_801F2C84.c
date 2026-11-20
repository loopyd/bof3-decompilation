#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x801F2C84
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb` at 0x801F2C88, so the cell is s8), stores 0xFFFF into the
 *           shared status halfword D_801490A8 (`ori v0,0,0xFFFF` at
 *           0x801F2C8C, the span's only `sh`, at 0x801F2C94), then stores 0x0B
 *           into the shared scenario progress byte g_ScenarioProgress when the
 *           gate byte reads 0 and 0x14 when it reads 1; any other gate value
 *           leaves that byte untouched. The constant 1 tested at 0x801F2CA0
 *           fills the branch delay slot of the preceding equality test, the
 *           0x14 arm constant fills the delay slot of that test itself
 *           (0x801F2CA4), and both arms share one store site (`sb` at
 *           0x801F2CB8) reached by the `j` at 0x801F2CA8 that the other gate
 *           values take. Takes no arguments, calls nothing, returns nothing: a
 *           frameless leaf whose 0x40-byte span closes on `jr $ra` at
 *           0x801F2CBC with its delay `nop`.
 *
 *           Evidence from this span's own bytes: the gate cell 0x801490C7 is
 *           read once, signed; the halfword store to 0x801490A8 is the only
 *           write there; the byte store goes to 0x80146864, the shared scenario
 *           progress cell the composed shared map names; the two arm literals are
 *           0x0B for gate 0 and 0x14 for gate 1.
 *
 *           Shape: the sibling setStatusFFFFAndScenarioProgress3COr46ByGateWorld00Area02213_801F2C44
 *           carries the same body in this same payload. A word comparison of the
 *           two 0x40-byte spans (payload offsets 0x84 and 0x44, one overlay
 *           loading at 0x801F2C00) differs in exactly three words: the two arm
 *           immediates at span offsets 0x20 and 0x2C, and the internal `j`
 *           immediate at span offset 0x24, whose target moves by exactly the
 *           0x40 delta between the two spans. Statement order, operand forms and
 *           declaration style are transcribed from that sibling with this span's
 *           own literals. One scan of every payload under out/binaries found no
 *           other 0x40-byte span byte-identical to this body, so no clone was
 *           claimed.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress0BOr14ByGateWorld00Area02213_801F2C84(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 0x0B;
    break;
  case 1:
    g_ScenarioProgress = 0x14;
    break;
  default:
    break;
  }
}
