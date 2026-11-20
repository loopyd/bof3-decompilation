#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress;

/* @source 0x801F2C44
 * @behavior Reads the shared gate byte D_801490C7 with a signed byte load,
 *           stores 0xFFFF into the shared status halfword D_801490A8, then
 *           stores 0xC8 into the shared scenario progress byte
 *           g_ScenarioProgress when that byte reads 0 and 0xC9 when it reads 1,
 *           leaving the progress byte untouched for any other value. Takes no
 *           arguments, calls nothing and returns nothing: a frameless leaf
 *           whose 0x40-byte span closes on `jr $ra` and its delay `nop`.
 *
 *           Byte evidence from this span: `lui`/`lb` reads 0x801490C7 signed,
 *           `ori $v0,$zero,0xFFFF` fills the single `sh` to 0x801490A8, the two
 *           arm constants (0xC8 at 0x801F2C70, 0xC9 in the delay slot at
 *           0x801F2C64) are materialised in $v0 and fill the branch delay slots
 *           of the two equality tests, both arms merge on one `sb` to
 *           0x80146864, and any other gate value falls straight to the return.
 *
 *           Shape: this target's own 0x801F2C04 body was lifted exact with the
 *           same topology and differs from this span in three bytes only - the
 *           two stored arm constants (0x64/0x65 there, 0xC8/0xC9 here) and the
 *           internal `j` word that joins the arms to the return, whose target
 *           shifts by the 0x40 delta between the two spans. Its statement
 *           order, `switch` topology and declaration style are transcribed here
 *           with this span's own literals. A full 0x40-byte `cmp` also proved
 *           this span byte-identical to payload offset 0x44 of
 *           emi/world00/area010/13, whose authored source is the byte-identical
 *           twin; the clone adapter's dry run accepted that identity but
 *           rewrote the shared-map binding g_ScenarioProgress into the raw
 *           0x80146864 spelling, so the body is carried here from the twin with
 *           the binding this target's map and sibling source already use.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndScenarioProgressC8OrC9ByGateWorld00Area00513_801F2C44(
    void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (gate) {
  case 0:
    g_ScenarioProgress = 0xC8;
    break;
  case 1:
    g_ScenarioProgress = 0xC9;
    break;
  default:
    break;
  }
}
