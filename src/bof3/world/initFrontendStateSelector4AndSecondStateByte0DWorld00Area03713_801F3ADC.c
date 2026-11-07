#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448ED;

s32 func_8015C088(void);

/* @source 0x801F3ADC
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then
 *           writes 4 into the shared front-end state selector at 0x801448EB,
 *           which dispatchIndexedStateHandler indexes into the handler table
 *           D_801C84BC, and 0xD into the shared byte at 0x801448ED, leaving the
 *           middle byte 0x801448EC untouched; takes no arguments and returns 0.
 *           The 0x18-byte frame only keeps $ra across the call, the delay slot
 *           behind that jal stays a nop because the helper takes no arguments,
 *           and each byte store materialises the 0x8014 page in $at on its own,
 *           so the constant 0xD is re-materialised into $v1 after the first
 *           store. The value-returning prototype of the helper keeps $v0 clear
 *           through the call and the explicit zero return compiles to the
 *           addu $v0,$zero,$zero in the branch slot before jr $ra. This is the
 *           same body as the world00 sibling
 *           initFrontendStateSelector4AndClearSecondStateByteWorld00Area00313_801F2DE0,
 *           which differs only in storing 0 through $zero into 0x801448ED, and
 *           the in-target asm neighbours func_801F3A34, func_801F3A6C and
 *           func_801F3AA4 emit the identical sequence with 0xB, 0xC and 0xE in
 *           that second byte. The role of the 0xD index is not established
 *           beyond its position in that run. No in-image jal or pointer word
 *           addresses 0x801F3ADC, so the overlay reaches this handler from
 *           outside its payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 initFrontendStateSelector4AndSecondStateByte0DWorld00Area03713_801F3ADC(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 0xD;
  return 0;
}
