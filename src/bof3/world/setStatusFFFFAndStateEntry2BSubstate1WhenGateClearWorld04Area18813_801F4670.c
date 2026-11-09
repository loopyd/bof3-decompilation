#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F4670
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path (`ori v0,0,0xFFFF`, `sh`) and, while the shared gate byte
 *           at 0x801490C7 is zero (`lb`, so the cell is s8), runs the shared
 *           front-end startup helper func_8015C088 and then writes the shared
 *           state-entry tuple: 0x801448EB = 0x2B from `addiu v0,0,0x2B`, then
 *           0x801448EC = 0, which stores `$zero`, and 0x801448ED = 1 from a
 *           second `addiu v0,0,1`. The gate load is hoisted above the status
 *           store, `bnez` jumps to the epilogue so the gate-clear path falls
 *           through its three byte stores into the frame restore, and both the
 *           branch delay slot and the call delay slot stay nops. The 0x18-byte
 *           frame only keeps $ra across the helper call, and each store
 *           materialises the 0x8014 page in $at on its own. Takes no arguments
 *           and returns nothing.
 *
 * Name evidence: 0x801448EB is the state-entry byte of the shared tuple the
 * setStatusFFFFAndStateEntry* handlers write, with 0x801448EC as the progress
 * byte and 0x801448ED as its companion byte, 0x801490C7 is the shared gate byte
 * those handlers branch on, and 0x8015C088 is the shared front-end startup
 * helper the whole world family runs. The payload names this body only from a
 * table word: no in-image `jal` reaches 0x801F4670, so its roles stay
 * structural. The 0x801448ED role word and the entry digit follow this target's
 * own family spelling, so 0x2B here is 2B and 1 is Substate1. 0x801448ED is a
 * shared scratch cell that other paths reuse as a commu record row
 * (publishCommuEntryTextId reads it as `row`, dispatchSharedModeByteTable2C04
 * stores a record index there), so the family's companion-byte vocabulary is
 * convention rather than a proven role.
 *
 * Shape: statement order, operand forms, offset spelling and declaration style
 * are taken from the near-identical in-target sibling
 * setStatusFFFFAndStateEntry2BWhenGateClearWorld04Area18813_801F4618, whose
 * 0x58-byte body differs from this 0x5C-byte span only in the single carried
 * constant of its last store (0x801448ED = 0 there, needs no `addiu`, against 1
 * here), so that body is shape evidence rather than a clone. No byte-identical
 * twin exists: the seven payload handlers that store 0x2B into 0x801448EB with a
 * func_8015C088 call under out/splat differ from this 0x5C-byte span in their
 * constants and tail stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x2B;
    D_801448EC = 0;
    D_801448ED = 1;
  }
}
