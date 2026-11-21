#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F2C94
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path (`ori v0,0,0xFFFF`, `sh`) and, while the shared gate byte
 *           at 0x801490C7 is zero (`lb`, so the cell is s8), runs the shared
 *           front-end startup helper func_8015C088 and then writes the shared
 *           state-entry tuple: 0x801448EB = 0x13 from `addiu v0,0,0x13`, then
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
 * those handlers branch on, 0x801490A8 is the shared status halfword they
 * publish, and 0x8015C088 is the shared front-end startup helper the whole
 * world family runs. The entry digit follows the family hex spelling, so 0x13
 * is written 13, and the companion byte's nonzero value is spelled Substate1
 * as in setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670.
 * 0x801448ED is a shared scratch cell other paths reuse as a commu record row,
 * so the companion-byte vocabulary is family convention rather than a proven
 * role.
 *
 * Entry evidence: this span is word 1 of the area handler pointer run at
 * 0x801F4348 in this payload, whose 15 words end at 0x801F3170, so no in-image
 * `jal` reaches 0x801F2C94 and its roles stay structural.
 *
 * Shape: statement order, operand forms, offset spelling and declaration style
 * are taken from setStatusFFFFAndStateEntry2BSubstate1WhenGateClearWorld04Area18813_801F4670,
 * an exact lift in the world04/area188/13 overlay that loads at the same
 * 0x801F2C00 address: its 0x5C-byte span equals this one word for word except
 * the single carried entry constant (0x2B there, 0x13 here), so that body is
 * shape evidence rather than a clone and this body writes its own literal. The
 * nearest in-target sibling,
 * setStatusFFFFAndStateEntry13WhenGateClearWorld03Area13613_801F2C3C, is the
 * same body with 0x801448ED = 0, which stores `$zero` and needs no `addiu`.
 * No byte-identical twin exists: a scan of the `out/binaries` payloads finds the
 * 0x5C bytes of this span only at offset 0x94 of this target's own binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry13Substate1WhenGateClearWorld03Area13613_801F2C94(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x13;
    D_801448EC = 0;
    D_801448ED = 1;
  }
}
