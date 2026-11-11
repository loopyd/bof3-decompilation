#include "bof3/bof3.h"

extern u8 D_801448EB;
extern u8 D_801448EC;
extern u8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F3F70
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 5 to the three consecutive shared mode bytes at
 *           0x801448EB, 0x801448EC and 0x801448ED; takes no arguments and
 *           returns nothing. The byte at 0x801448EB is the front-end state
 *           selector indexed into the callback table, so this is the overlay's
 *           state-entry tuple write. The 0x18-byte frame only keeps $ra across
 *           the call, and each byte store materialises the 0x8014 page in $at
 *           on its own, which is how the psyq compiler emits separate
 *           shared-byte stores. The 5 constant is loaded between the first and
 *           second store because its only consumer is the last store. No
 *           in-image jal or pointer word addresses 0x801F3F70, so the overlay
 *           reaches this handler from outside its payload. A masked payload
 *           scan found ten 64-byte siblings of this body that differ in exactly
 *           one word, the third stored constant (0x801448ED = 0x0A, 0x01,
 *           0x07, 0x0B, 0x02, 0x08, 0x09, 0x0E, 0x0F and 0x04), with the same
 *           call word; five of them were lifted exact as initArea14613State
 *           (0x801F2D50), initArea10513State (0x801F3140), initArea14514State
 *           (0x801F439C), initArea07714State (0x801F599C) and
 *           initArea093State (0x801F2C04), and this body follows their
 *           statement order with its own constant.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea11713State(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 5;
}
