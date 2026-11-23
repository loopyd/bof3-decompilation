#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448ED;

s32 func_8015C088(void);

/* @source 0x801F3AA4
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then
 *           writes 4 into the shared front-end state selector at 0x801448EB,
 *           which dispatchIndexedStateHandler indexes into the handler table
 *           D_801C84BC, and 0xE into the shared byte at 0x801448ED, leaving the
 *           middle byte 0x801448EC untouched; takes no arguments and returns 0.
 *           The 0x18-byte frame only keeps $ra across the call, the delay slot
 *           behind that jal stays a nop because the helper takes no arguments,
 *           and each byte store materialises the 0x8014 page in $at on its own,
 *           so the constant 0xE is re-materialised into $v1 after the first
 *           store. The value-returning prototype of the helper keeps $v0 clear
 *           through the call and the explicit zero return compiles to the
 *           addu $v0,$zero,$zero in the branch slot before jr $ra. This body is
 *           word-equal to the in-target siblings already lifted exact at
 *           0x801F3A34, 0x801F3A6C and 0x801F3ADC apart from that one 0xE
 *           immediate, and the four words 0x801F3A34, 0x801F3A6C, 0x801F3AA4,
 *           0x801F3ADC sit consecutively in a pointer run at payload offset
 *           0x32EA4 of emi/etc/game/00.bin, so the four form one run of
 *           front-end entry handlers in that order. That run fixes entry order
 *           only: the role of the 0xE value is not established beyond its
 *           position in it, and no in-image jal or pointer word addresses
 *           0x801F3AA4 inside this payload, so its argument roles stay
 *           structural and the overlay reaches this handler from outside it.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 initFrontendByte0e(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 0xE;
  return 0;
}
