#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u8 D_80146874; /* @source 0x80146874 @kind unknown */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

void func_8015C088(void);

/* @source 0x801F2CA8
 * @behavior Entry 1 of the target handler table at 0x801F37AC (the table the
 * payload publishes at 0x801F37D0). Stores 0xFFFF into the shared status
 * halfword D_801490A8 on every path and, while the shared gate byte D_801490C7
 * is clear, runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 1 with secondary state 0x14 through the shared
 * route bytes D_80146874 and D_80146875. The gate is read with `lb` (so the cell
 * is s8) and that load is hoisted above the status store; `bnez` jumps to the
 * epilogue, so the gate-clear path falls through its two byte stores into the
 * frame restore. The 0x18-byte frame only keeps $ra across the helper call and
 * that call's delay slot stays a nop because the helper takes no arguments. Each
 * byte store materialises the 0x8014 page in $at on its own. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name evidence: the payload's own word run at 0x801F37AC lists this address
 * second after 0x801F2C80, so this span is a table entry rather than a `jal`
 * target and its roles stay structural. 0x80146874/0x80146875 are the shared
 * primary/secondary scene state request bytes that the requestPrimaryState*
 * family names that way, func_8015C088 is the shared front-end startup helper
 * the whole world family runs, D_801490C7 is the gate byte the neighbouring
 * setStatusFFFF* handlers of the same table branch on, and D_801490A8 is the
 * shared status halfword they store.
 *
 * Shape notes from the original bytes: the 0x54-byte span is byte-identical to
 * emi/world04/area191/13 0x801F2F40, proven by comparing the two 0x54-byte
 * payload spans at their own offsets (0xA8 here, 0x340 there), but that row is
 * still `asm`, so it carries no source to clone. The body is instead seeded from
 * the near-identical exact sibling
 * setStatusFFFFAndRequestPrimaryState5Substate0WhenGateClearWorld02Area11113_801F33F8,
 * which writes the same two route bytes with the same unconditional 0xFFFF
 * status store, the same hoisted gate load and the same gate-clear arm, and
 * differs only in the two stored constants; the 0x54-byte twins of
 * emi/world04/area170/13 (0x801F4950, 0x801F49A4, 0x801F49F8) differ from this
 * span in exactly four words, the two stored constants and the two store
 * displacements that name that area's own state tuple. This file therefore
 * takes the statement order and declaration style from those siblings and its
 * own cells, constants and status value from this span's bytes.
 */
void setSubstate14GateClear(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_80146874 = 1;
    D_80146875 = 0x14;
  }
}
