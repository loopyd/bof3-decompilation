#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448EC;
extern s8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F58F4
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 8 to the three consecutive shared mode bytes at
 *           0x801448EB, 0x801448EC and 0x801448ED; takes no arguments and
 *           returns nothing. The byte at 0x801448EB is the front-end state
 *           selector indexed into the callback table, so this is the overlay's
 *           state-entry tuple write. No word or call inside this payload names
 *           the span, so the overlay loader is its only caller and its
 *           argument roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea10313State(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 8;
}
