#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_80146875;

/* @source 0x801F4478
 * @behavior Overlay state setter: stores 0xFFFF into the shared status halfword
 *           at 0x801490A8 on every path, then mirrors the shared gate byte at
 *           0x801490C7 into the front-end sub-state byte at 0x80146875 - 0 when
 *           the gate is clear and 1 when it is nonzero. The gate is read with
 *           `lb` (so the cell is s8) and that load is hoisted above the status
 *           store; `bnez` targets the nonzero store, whose 1 is materialised in
 *           $v0 inside the branch delay slot, and the clear path stores $zero
 *           before jumping over it. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFSubstate1IfGateElse0World04Area19913_801F4478(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    D_80146875 = 0;
  } else {
    D_80146875 = 1;
  }
}
