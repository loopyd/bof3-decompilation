#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448EC;
extern s8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F599C
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 7 to the three consecutive shared mode bytes at
 *           0x801448EB, 0x801448EC and 0x801448ED; takes no arguments and
 *           returns nothing. The byte at 0x801448EB is the front-end state
 *           selector indexed into the callback table by
 *           dispatchIndexedStateHandler, so this is the overlay's state-entry
 *           tuple write.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea07714State(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 7;
}
