#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448EC;

void func_8015C088(void);

/* @source 0x801F4168
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 0x1F and 0xA to the two consecutive shared mode bytes at
 *           0x801448EB and 0x801448EC; takes no arguments and returns nothing.
 *           The byte at 0x801448EB is the front-end state selector that
 *           dispatchIndexedStateHandler indexes into the callback table, so
 *           this is the overlay's state-entry tuple write. The 0x18-byte frame
 *           only keeps $ra across the call, and each store materialises the
 *           0x8014 page in $at on its own, which is how the psyq compiler emits
 *           separate shared-byte stores. No jal or pointer word in this payload
 *           addresses 0x801F4168, so the overlay reaches this handler from
 *           outside its payload.
 *
 * Name evidence: this is the shape of the exact siblings initArea14514State
 * (0x801F439C), initArea14613State, initArea10513State (0x801F3140),
 * initArea093State and initArea07714State, which all call func_8015C088 and
 * then write the same pair of shared mode bytes at 0x801448EB/0x801448EC;
 * they differ only in the stored tuple (0x2C/0/0xF, 0x2C/0/9, 0x2C/0). This
 * one writes 0x1F/0xA, matching the preceding gate variant func_801F4108,
 * which writes 0x1F/0 on its success path.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea14813State(void) {
  func_8015C088();
  D_801448EB = 0x1F;
  D_801448EC = 0xA;
}
