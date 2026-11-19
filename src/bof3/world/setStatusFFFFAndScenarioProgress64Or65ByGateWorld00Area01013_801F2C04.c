#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x801F2C04
 * @behavior Reads the shared byte D_801490C7 with a signed byte load, stores
 *           0xFFFF into the shared status halfword D_801490A8, then stores
 *           0x64 into the shared scenario progress byte g_ScenarioProgress
 *           when that byte reads 0 and 0x65 when it reads 1, leaving the
 *           progress byte untouched for any other value. Takes no arguments,
 *           calls nothing and returns nothing: a frameless leaf whose
 *           0x40-byte span closes on `jr $ra` and its delay `nop`.
 *
 *           Byte evidence from this span: `lui`/`lb` reads 0x801490C7 signed,
 *           `ori $v0,$zero,0xFFFF` fills the single `sh` to 0x801490A8, the two
 *           arm constants are materialised in $v0 and fill the branch delay
 *           slots of the two equality tests, both arms merge on one `sb` to
 *           0x80146864, and the other value falls straight to the return.
 *
 *           Shape: the sibling
 *           setStatusFFFFAndScenarioProgress64Or65ByGateWorld00Area01513_801F2CB8
 *           was lifted exact in its own overlay, and its payload span equals
 *           this 0x40-byte span word for word except the absolute `j` that
 *           joins the arms to the return (0x0807CB0F here against 0x0807CB3C
 *           there), a shift of exactly the 0xB4 delta between the two spans.
 *           Its statement order, `switch` topology and declaration style are
 *           transcribed here with this span's own cell bindings. This target's
 *           own neighbouring lifts read the same shared cells: D_801490C7 as
 *           the signed shared mode byte and D_801490A8 as the shared status
 *           halfword.
 *
 *           The shared gate-status macro is no fit: the status halfword takes
 *           0xFFFF on every path while the arm constants land in a different
 *           byte cell, selected by equality against 0 and 1 rather than by a
 *           zero/nonzero test.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgress64Or65ByGateWorld00Area01013_801F2C04(
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
