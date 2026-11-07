#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EC;

/* @source 0x801F4A4C
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8) into a local and stores 0xFFFF into the
 *           shared status halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then
 *           stores 0x37 into the shared state byte at 0x801448EC while the gate
 *           byte is nonzero and 0x34 when it is zero. The gate value stays in
 *           $v1 and the selected constant in $v0, the nonzero arm fills the
 *           `bnez` delay slot and both arms share the single `sb`. Takes no
 *           arguments, calls nothing, returns nothing; leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The instruction shape is the exact sibling
 * setStatusFFFFAndByte801448EC0EIfGateElse0CWorld03Area15013_801F2CDC
 * (emi/world03/area150/13); a payload comparison of the two 0x30-byte ranges
 * differs only in the two selected-constant words (0x37/0x34 here against
 * 0x0E/0x0C there), so that sibling's statement order and declaration style are
 * copied while this target writes its own literals at its own boundary and
 * address. The nearer sibling setStatusFFFFAndScenarioProgress14IfGateElse0A
 * World03Area12013_801F42F0 differs in its destination symbol as well, so it is
 * shape evidence only.
 */
void setStatusFFFFAndByte801448EC37IfGateElse34World04Area17013_801F4A4C(void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  D_801448EC = (gate != 0) ? 0x37 : 0x34;
}
