#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448EC;
extern s8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F3BEC
 * @behavior Runs the shared overlay startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 2 to the three consecutive shared state bytes at
 *           0x801448EB, 0x801448EC and 0x801448ED. The byte at 0x801448EB is
 *           the front-end state selector, so this is the overlay's state-entry
 *           tuple write. Takes no arguments and returns nothing; its 0x18-byte
 *           frame only keeps $ra across the call, and each store materialises
 *           the 0x8014 page in $at on its own.
 *           Name evidence: this is the body of the exact siblings
 *           initArea10513State (0x801F3140) and initArea093State (0x801F2C04),
 *           which call the same helper and store 0x2C/0/0x09 and 0x2C/0/0x0B
 *           through the same three shared bytes; each measured exact when it
 *           was written. Only that last stored literal differs here, so their
 *           statement order, plain extern declarations and untagged helper
 *           prototype are transcribed and this body's own 2 is written in.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea10013State(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 2;
}
