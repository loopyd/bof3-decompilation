#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801448EB;
extern u8 D_801448EC;

void func_8015C088(void);

/* @source 0x801F4980
 * @behavior Stores 0xFFFF into the shared status halfword at 0x801490A8 on
 *           every path and, while the shared gate byte at 0x801490C7 is zero,
 *           runs the shared front-end startup helper func_8015C088 and then
 *           requests state entry 0x3B through the shared state byte at
 *           0x801448EB with progress 0x0A in the shared byte at 0x801448EC. The
 *           gate is read with `lb` (so the cell is s8) and that load is hoisted
 *           above the status store; `bnez` jumps to the epilogue, so the
 *           gate-clear path falls through its two byte stores into the frame
 *           restore. The 0x18-byte frame only keeps $ra across the helper call,
 *           and that call delay slot stays a nop because the helper takes no
 *           arguments. Each byte store materialises the 0x8014 page in $at on
 *           its own, and both constants need their own `addiu` because neither
 *           store writes zero. Takes no arguments and returns nothing.
 *
 * Name evidence: 0x801448EB is the state-entry byte and 0x801448EC the progress
 * byte of the shared tuple the setStatusFFFFAndStateEntry* handlers write,
 * 0x801490C7 is the shared gate byte those handlers branch on, and 0x8015C088
 * is the shared front-end startup helper the world family runs. The payload
 * names this body only from a table word: 0x801F4F48 holds 0x801F4980 inside
 * the handler run 0x801F4F30..0x801F4F4C (0x801F46F8, 0x801F4764, 0x801F4808,
 * 0x801F4848, 0x801F48B4, 0x801F4920, 0x801F4980, 0x801F49D4), and no in-image
 * `jal` reaches it, so its roles stay structural.
 *
 * Shape: statement order, operand forms and declaration style are taken from
 * the near-identical sibling
 * setStatusFFFFAndStateEntry25Progress5WhenGateClearWorld04Area17013_801F4950,
 * whose 0x54-byte body differs from this 0x54-byte span in exactly two words,
 * its two stored constants (0x25 and 5 there, 0x3B and 0x0A here), so that
 * sibling is shape evidence rather than a clone. No byte-identical twin exists:
 * the span was compared against every payload under out/binaries both as raw
 * bytes and with call and branch immediates masked, and each scan matched only
 * its own offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFAndStateEntry3BProgressAWhenGateClearWorld04Area16813_801F4980(void) {
  D_801490A8 = 0xFFFFu;
  if (D_801490C7 == 0) {
    func_8015C088();
    D_801448EB = 0x3B;
    D_801448EC = 0xA;
  }
}
