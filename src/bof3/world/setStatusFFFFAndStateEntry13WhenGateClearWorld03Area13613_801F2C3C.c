#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F2C3C
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path (`ori v0,0,0xFFFF`, `sh`) and, while the shared gate byte
 *           at 0x801490C7 is zero (`lb`, so the cell is s8), runs the shared
 *           front-end startup helper func_8015C088 and then writes the shared
 *           state-entry tuple: 0x801448EB = 0x13 from one `addiu v0,0,0x13`,
 *           then 0x801448EC = 0 and 0x801448ED = 0, which both store `$zero`.
 *           The gate load is hoisted above the frame store, `bnez` jumps to the
 *           epilogue so the gate-clear path falls through its three byte stores
 *           into the frame restore, and both the branch delay slot and the call
 *           delay slot stay nops. The 0x18-byte frame only keeps $ra across the
 *           helper call, and each store materialises the 0x8014 page in $at on
 *           its own. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x801448EB is the state-entry byte of the shared tuple the
 * setStatusFFFFAndStateEntry* handlers write, with 0x801448EC and 0x801448ED as
 * its progress and substate companions, 0x801490C7 is the shared gate byte those
 * handlers branch on, 0x801490A8 is the shared status halfword they publish, and
 * 0x8015C088 is the shared front-end startup helper the whole world family runs.
 * The entry digit follows the family hex spelling, so 0x13 is written 13.
 *
 * Entry evidence: this span is word 0 of the area handler pointer run at
 * 0x801F4348 in this payload, whose 15 words end at 0x801F3170, so no in-image
 * `jal` reaches 0x801F2C3C and its roles stay structural.
 *
 * Shape: statement order, operand forms, offset spelling and declaration style
 * are taken from setStatusFFFFAndStateEntry2BWhenGateClear
 * World04Area18813_801F4618, an exact lift whose 0x58-byte span differs from
 * this one in the single word carrying the entry constant (0x2B there, 0x13
 * here) with every other word equal, so that body is shape evidence rather than
 * a clone and this body writes its own literal. The nearest in-target sibling,
 * setStatusFFFFAndStateEntry13Substate1WhenGateClearWorld03Area13613_801F2C94
 * at 0x801F2C94, writes the same 0x801448EB = 0x13 and
 * 0x801448EC = 0 but sets 0x801448ED = 1 through an extra `addiu`, which is why
 * the two zero stores here spell `$zero` directly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry13WhenGateClearWorld03Area13613_801F2C3C(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x13;
    D_801448EC = 0;
    D_801448ED = 0;
  }
}
