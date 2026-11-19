#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448ED;

s32 func_8015C088(void);

/* @source 0x801F2D38
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then
 * writes 4 into the shared front-end state selector byte at 0x801448EB and 1
 * into the shared second state byte at 0x801448ED, leaving the middle byte
 * 0x801448EC untouched; takes no arguments and returns 0. The 0x18-byte frame
 * only keeps $ra across the call, the delay slot behind that jal stays a nop
 * because the helper takes no arguments, and each byte store materialises the
 * 0x8014 page in $at on its own, so this span's constant 1 is re-materialised
 * into $v1 after the first store. The value-returning prototype of the helper
 * keeps $v0 for the call result, and the explicit zero return compiles to the
 * addu $v0,$zero,$zero before jr $ra. Shape: the 56 bytes at 0x801F2D38 equal
 * the 56 bytes of the world01 sibling
 * initFrontendStateSelector4AndSecondStateByte2World01Area05513_801F2D70 (that
 * payload's 0x170 offset, payload cmp) except one word, the immediate of the
 * second addiu into $v1, which is this span's own 1 against that sibling's 2,
 * with every other word equal; that sibling carried a recorded exact
 * measurement and supplied the statement order, the plain extern declarations
 * and the value-returning helper prototype. The role of the 1 in that byte is
 * not established beyond its position. No in-image jal or pointer word
 * addresses 0x801F2D38, so the overlay reaches this handler from outside its
 * payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 initFrontendStateSelector4AndSecondStateByte1World01Area06113_801F2D38(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 1;
  return 0;
}
