#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448ED;

s32 func_8015C088(void);

/* @source 0x801F2D70
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then
 *           writes 4 into the shared front-end state selector at 0x801448EB,
 *           which dispatchIndexedStateHandler indexes into the handler table
 *           D_801C84BC, and 2 into the shared byte at 0x801448ED, leaving the
 *           middle byte 0x801448EC untouched; takes no arguments and returns 0.
 *           The 0x18-byte frame only keeps $ra across the call, the delay slot
 *           behind that jal stays a nop because the helper takes no arguments,
 *           and each byte store materialises the 0x8014 page in $at on its own,
 *           so the constant 2 is re-materialised into $v1 after the first
 *           store. The value-returning prototype of the helper keeps $v0 for
 *           the call result and the explicit zero return compiles to the
 *           addu $v0,$zero,$zero before jr $ra. Shape: the 56 bytes at
 *           0x801F2D70 equal the 56 bytes of the world00 sibling
 *           initFrontendStateSelector4AndSecondStateByte0DWorld00Area03713_801F3ADC
 *           except one word, the immediate of the second addiu into $v1, which
 *           is this span's own 2 against that sibling's 0xD, with every other
 *           word equal (payload cmp); that sibling, which measured exact when it
 *           was written, supplies the statement order, the plain extern
 *           declarations and the value-returning helper prototype. The nearer
 *           world00 sibling
 *           initFrontendStateSelector4AndClearSecondStateByteWorld00Area00313_801F2DE0
 *           stores $zero into that second byte and is one instruction shorter,
 *           so it is shape evidence only. The role of the 2 in that byte is not
 *           established beyond its position. No in-image jal or pointer word
 *           addresses 0x801F2D70, so the overlay reaches this handler from
 *           outside its payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setupFrontendStateTwo(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 2;
  return 0;
}
