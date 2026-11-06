#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F2D48
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 4 to the shared mode byte at 0x801448EB and 6 to the second
 *           shared mode byte at 0x801448ED, leaving the middle byte
 *           0x801448EC untouched; takes no arguments and returns nothing. The
 *           byte at 0x801448EB is the front-end state selector indexed into the
 *           callback table by dispatchIndexedStateHandler, so this is the
 *           overlay's state-entry selector write, and it is the shape of the
 *           exact siblings initArea093State (0x801F2C04), initArea10513State
 *           (0x801F3140), initArea14613State (0x801F2D50) and
 *           initArea07714State (0x801F599C) with the 0x801448EC store dropped.
 *           The 0x18-byte frame only keeps $ra across the call, the call's
 *           delay slot stays a nop because the helper takes no arguments, and
 *           each byte store materialises the 0x8014 page in $at on its own.
 *           No in-image jal or pointer word addresses 0x801F2D48, so the
 *           overlay reaches this handler from outside its payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea11313State(void) {
  func_8015C088();
  D_801448EB = 4;
  D_801448ED = 6;
}
