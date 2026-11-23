#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x801F2C04
 * @behavior Reads the shared signed byte D_801490C7, stores 0xFFFF into the
 *           shared status halfword D_801490A8, then stores 1 into the shared
 *           scenario progress byte g_ScenarioProgress when that byte reads 0
 *           and 2 when it reads 1, leaving the progress byte untouched for any
 *           other value. Takes no arguments, calls nothing and returns nothing:
 *           a frameless leaf whose 0x40-byte span closes on `jr $ra` and its
 *           delay `nop`.
 *
 *           Byte evidence from this span: `lui`/`lb` reads 0x801490C7 signed,
 *           `ori $v0,$zero,0xFFFF` fills the single `sh` to 0x801490A8, the two
 *           arm constants are materialised in $v0 and fill the branch delay
 *           slots of the two equality tests, both arms merge on one `sb` to
 *           0x80146864, and any other value falls straight to the return.
 *
 *           Seed: the sibling overlay span of
 *           setStatusFFFFAndScenarioProgress64Or65ByGateWorld00Area01013_801F2C04
 *           sits at the same payload offset under the same load address, and a
 *           word-by-word read of the two original payload spans showed the two
 *           0x40-byte ranges equal except this span's own two arm immediates
 *           (0x0001 and 0x0002 against 0x0064 and 0x0065 there). That sibling's
 *           lift was exact, and its statement order, `switch` topology and
 *           declaration style are transcribed here with this span's own cell
 *           bindings and arm constants. No clone was claimed: a scan of every
 *           EMI payload under out/binaries found this 0x40-byte range in this
 *           target's own binary only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusOr2Gate(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 1;
    break;
  case 1:
    g_ScenarioProgress = 2;
    break;
  default:
    break;
  }
}
