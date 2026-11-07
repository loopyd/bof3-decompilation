#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801F33F8
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path and, while the shared gate byte at 0x801490C7 is clear,
 *           runs the shared front-end startup helper func_8015C088 and then
 *           requests primary scene state 5 with secondary state 0 in the shared
 *           route bytes 0x80146874/0x80146875. The gate is read with `lb` (so
 *           the cell is s8) and that load is hoisted above the status store;
 *           `bnez` jumps to the epilogue, so the gate-clear path falls through
 *           its two byte stores into the frame restore. The 0x18-byte frame
 *           only keeps $ra across the helper call and the call delay slot stays
 *           a nop because the helper takes no arguments. Takes no arguments and
 *           returns nothing.
 *
 * Name evidence: 0x80146874/0x80146875 are the shared primary/secondary scene
 * state request bytes (bootScenarioState and the requestPrimaryState* family
 * name them that way), func_8015C088 is the shared front-end startup helper
 * used by the whole world family, and D_801490C7 is the shared gate byte read
 * by the setStatusFFFF* handlers. The word at 0x801F5B5C inside this payload
 * points at 0x801F33F8, so the overlay reaches it as entry 1 of the ascending
 * handler table at 0x801F5B58 rather than through an in-image jal; the roles
 * stay structural because that table carries no prototype.
 *
 * Shape: the prefix is the already-exact sibling
 * setStatusFFFFSubstate1IfGateElse0World04Area19913_801F4478 (store 0xFFFF,
 * then branch on the gate byte), which fixes the statement order and the
 * hoisted gate load; the gate-clear body carries this function's own helper
 * call and its 5/0 constants, so it is written from the original bytes. No
 * byte-identical twin exists: the 0x50-byte span was searched for across
 * out/binaries and matched no other payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndRequestPrimaryState5Substate0WhenGateClearWorld02Area11113_801F33F8(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_80146874 = 5;
    D_80146875 = 0;
  }
}
