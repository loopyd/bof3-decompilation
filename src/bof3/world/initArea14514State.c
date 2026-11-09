#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448EC;
extern s8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F439C
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 0xF to the three consecutive shared mode bytes
 *           at 0x801448EB, 0x801448EC and 0x801448ED; takes no arguments and
 *           returns nothing. The byte at 0x801448EB is the front-end state
 *           selector indexed into the callback table, so this is the overlay's
 *           state-entry tuple write. The 0x18-byte frame only keeps $ra across
 *           the call, and each byte store materialises the 0x8014 page in $at
 *           on its own, which is how the psyq compiler emits separate
 *           shared-byte stores. The 0xF constant is loaded between the first
 *           and second store because its only consumer is the last store. No
 *           in-image jal or pointer word addresses 0x801F439C, so the overlay
 *           reaches this handler from outside its payload. Its 64 bytes are
 *           the shape of the exact siblings initArea14613State (0x801F2D50),
 *           initArea10513State (0x801F3140), initArea093State (0x801F2C04) and
 *           initArea07714State (0x801F599C), which differ only in the third
 *           stored byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea14514State(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 0xF;
}
