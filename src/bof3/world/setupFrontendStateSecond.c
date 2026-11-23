#include "bof3/bof3.h"

extern s8 D_801448EB; /* @source 0x801448EB @kind data */
extern s8 D_801448ED; /* @source 0x801448ED @kind data */

/* Shared front-end startup helper of the main executable, also reached from the
 * neighbouring handlers of this target.
 */
s32 func_8015C088(void);

/* @source 0x801F2E68
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then writes
 *           4 into the shared front-end state selector at 0x801448EB, which
 *           dispatchIndexedStateHandler indexes into the handler table
 *           D_801C84BC, and 8 into the shared byte at 0x801448ED, leaving the
 *           middle byte 0x801448EC untouched; takes no arguments and returns 0.
 *           The 0x18-byte frame only keeps $ra across the call, the delay slot
 *           behind that jal stays a nop because the helper takes no arguments,
 *           and each byte store materialises the 0x8014 page in $at on its own,
 *           so the constant 8 is re-materialised into $v1 after the first
 *           store. The value-returning prototype of the helper keeps $v0 clear
 *           through the call and the explicit zero return compiles to the
 *           addu $v0,$zero,$zero ahead of jr $ra. The role of the 8 index is not
 *           established beyond its position in the sibling run. No in-image jal
 *           or pointer word addresses 0x801F2E68, so the overlay reaches this
 *           handler from outside its payload.
 *
 * Name evidence: six exact lifts of this same body in other targets name
 * themselves after the two stores they make - initFrontendStateSelector4AndClearSecondStateByteWorld00Area00313_801F2DE0,
 * initFrontendStateSelector4AndSecondStateByte0DWorld00Area03713_801F3ADC,
 * initFrontendStateSelector4AndSecondStateByte4World01Area05013_801F2DA0,
 * initFrontendStateSelector4AndSecondStateByte2World01Area05513_801F2D70,
 * initFrontendStateSelector4AndSecondStateByte1World01Area06113_801F2D38 and
 * initFrontendStateSelector4AndSecondStateByte7World01Area07413_801F2E94 - so
 * this span takes that form with its own area, disc and stored value.
 *
 * Shape: statement order, operand forms and declaration style are taken from
 * that exact sibling, whose body differs only in the second stored constant;
 * every literal and both byte addresses here are read from this span's own
 * bytes. Seven spans in other payloads carry this body with that one store
 * constant changed (world00/area037/13 at 0x801F2E34, 0x801F2E6C, 0x801F2EA4
 * and 0x801F3ADC, world01/area041/13 at 0x801F4250, world01/area055/13 at
 * 0x801F2D70 and world01/area061/13 at 0x801F2D38), which proves the shape
 * across the family.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setupFrontendStateSecond(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 8;
  return 0;
}
