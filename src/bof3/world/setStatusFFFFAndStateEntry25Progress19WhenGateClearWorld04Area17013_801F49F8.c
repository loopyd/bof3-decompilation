#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;

void func_8015C088(void);

/* @source 0x801F49F8
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path and, while the shared gate byte at 0x801490C7 is zero,
 *           runs the shared front-end startup helper func_8015C088 and then
 *           requests state entry 0x25 through the shared state byte at
 *           0x801448EB with progress 0x19 in the shared byte at 0x801448EC. The
 *           gate is read with `lb` (so the cell is s8) and that load is hoisted
 *           above the status store; `bnez` jumps to the epilogue, so the
 *           gate-clear path falls through its two byte stores into the frame
 *           restore. The 0x18-byte frame only keeps $ra across the helper call,
 *           and that call delay slot stays a nop because the helper takes no
 *           arguments. Each store materialises the 0x8014 page in $at on its
 *           own, and the two constants each need their own `addiu` because
 *           neither store writes zero. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x801448EB is the state-entry byte and 0x801448EC the progress
 * byte of the shared tuple the setStatusFFFFAndStateEntry* handlers write,
 * 0x801490C7 is the shared gate byte those handlers branch on, and 0x8015C088
 * is the shared front-end startup helper the whole world family runs. The
 * payload names this body only from a table word: 0x801F6464 holds 0x801F49F8
 * as the third of the six handler pointers 0x801F4950, 0x801F49A4, 0x801F49F8,
 * 0x801F4A4C, 0x801F4A7C and 0x801F4A9C, and the table-head pointer at
 * 0x801F64A8 targets 0x801F645C, so no in-image `jal` reaches it and its roles
 * stay structural. The progress digit follows the family's hex spelling, so
 * 19 here is 0x19.
 *
 * Shape: statement order, operand forms, offset spelling and declaration style
 * are taken from the two exact siblings earlier in the same handler table,
 * setStatusFFFFAndStateEntry25Progress5WhenGateClearWorld04Area17013_801F4950
 * and setStatusFFFFAndStateEntry25ProgressFWhenGateClearWorld04Area17013_801F49A4,
 * whose 0x54-byte bodies equal this 0x54-byte span word for word except for the
 * single `addiu` that materialises the second stored constant (5 there, 0xF in
 * the middle body, 0x19 here). That one differing word makes them shape evidence
 * rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry25Progress19WhenGateClearWorld04Area17013_801F49F8(
    void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x25;
    D_801448EC = 0x19;
  }
}
