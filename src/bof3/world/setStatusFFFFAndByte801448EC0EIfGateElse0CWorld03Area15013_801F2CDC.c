#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EC;

/* @source 0x801F2CDC
 * @behavior Loads the shared gate byte D_801490C7 with a signed byte read
 *           (`lb`, so the cell is s8) into a local, stores 0xFFFF into the
 *           shared status halfword D_801490A8 (`ori v0,0,0xFFFF`, `sh`), then
 *           stores 0x0E into the shared state byte at 0x801448EC while the gate
 *           byte is nonzero and 0x0C when it is zero. The gate value stays in
 *           $v1 and the selected constant in $v0, the nonzero arm fills the
 *           `bnez` delay slot and both arms share the single `sb`. Takes no
 *           arguments, calls nothing, returns nothing; leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The instruction shape is byte-identical to the exact sibling
 * setStatusFFFFAndScenarioProgress14IfGateElse0AWorld03Area12013_801F42F0
 * (emi/world03/area120/13), which differs only in its two selected constants
 * and its destination symbol, so this target owns its own address and boundary
 * rather than a duplicate of that function.
 */
void setStatusFFFFAndByte801448EC0EIfGateElse0CWorld03Area15013_801F2CDC(void) {
  s32 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFFu;
  D_801448EC = (gate != 0) ? 0xE : 0xC;
}
