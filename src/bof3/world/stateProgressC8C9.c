#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x801F2C44
 * @behavior Reads the shared byte D_801490C7 with a signed byte load, stores
 *           0xFFFF into the shared status halfword D_801490A8, then stores
 *           0xC8 into the shared scenario progress byte g_ScenarioProgress
 *           when that byte reads 0 and 0xC9 when it reads 1, leaving the
 *           progress byte untouched for any other value. Takes no arguments,
 *           calls nothing and returns nothing: a frameless leaf whose
 *           0x40-byte span closes on `jr $ra` and its delay `nop`.
 *
 *           Byte evidence from this span: `lui`/`lb` reads 0x801490C7 signed,
 *           `ori $v0,$zero,0xFFFF` fills the single `sh` to 0x801490A8, the two
 *           arm constants (0xC8 at 0x801F2C70, 0xC9 in the delay slot at
 *           0x801F2C64) are materialised in $v0 and fill the branch delay slots
 *           of the two equality tests, both arms merge on one `sb` to
 *           0x80146864, and the other value falls straight to the return.
 *
 *           Shape: the sibling
 *           stateStatusProgress64Gate
 *           was lifted exact in this same overlay, and its 0x40-byte span
 *           differs from this one in exactly three words: the two stored arm
 *           constants (0x64/0x65 there, 0xC8/0xC9 here) and the absolute `j`
 *           that joins the arms to the return, which shifts by the 0x40 delta
 *           between the two spans (0x0807CB0F there, 0x0807CB1F here). Its
 *           statement order, `switch` topology and declaration style are
 *           transcribed here with this span's own literals. A payload scan for
 *           this span's first four words over `out/binaries` found one
 *           byte-identical twin, emi/world00/area005/13.bin at payload offset
 *           0x44, whose own row is still `asm`, so no lifted source existed to
 *           clone and this transcription carries the body instead.
 *
 *           The shared gate-status macro is no fit: the status halfword takes
 *           0xFFFF on every path while the arm constants land in a different
 *           byte cell, selected by equality against 0 and 1 rather than by a
 *           zero/nonzero test.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateProgressC8C9(
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
