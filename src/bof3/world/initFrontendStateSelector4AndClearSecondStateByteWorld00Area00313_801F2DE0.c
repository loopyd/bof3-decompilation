#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448ED;

s32 func_8015C088(void);

/* @source 0x801F2DE0
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then
 *           writes 4 into the shared front-end state selector at 0x801448EB,
 *           which dispatchIndexedStateHandler indexes into the callback table
 *           D_801C84BC, and clears the shared byte at 0x801448ED, leaving the
 *           middle byte 0x801448EC untouched; takes no arguments and returns 0.
 *           The 0x18-byte frame only keeps $ra across the call, the delay slot
 *           behind that jal stays a nop because the helper takes no arguments,
 *           each byte store materialises the 0x8014 page in $at on its own, the
 *           cleared byte stores $zero directly, and $v0 carries the zero return
 *           so the 4 is materialised in $v1. The declared value-returning
 *           prototype of the helper is the measured difference: with the helper
 *           declared void the same body emits the constant into $v0 and matches
 *           11 of 13 instructions, while this prototype keeps $v0 holding the
 *           call result and matches all 13. No in-image jal or pointer word
 *           addresses 0x801F2DE0, so the overlay reaches this handler from
 *           outside its payload. The void sibling initArea11313State writes 4
 *           to 0x801448EB and 6 to 0x801448ED behind the same call.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 initFrontendStateSelector4AndClearSecondStateByteWorld00Area00313_801F2DE0(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 0;
  return 0;
}
