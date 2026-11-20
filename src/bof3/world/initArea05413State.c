#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448EC;
extern s8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F2C04
 * @behavior Runs the shared overlay startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 0x0A to the three consecutive shared state bytes
 *           at 0x801448EB, 0x801448EC and 0x801448ED. The byte at 0x801448EB is
 *           the front-end state selector, so this is the overlay's state-entry
 *           tuple write. Takes no arguments and returns nothing; its 0x18-byte
 *           frame only keeps $ra across the call, and each store materialises
 *           the 0x8014 page in $at on its own. The 0x0A constant is loaded
 *           between the first and second store because its only consumer is the
 *           last store.
 *           Name evidence: this is the body of the exact siblings
 *           initArea10513State (0x801F3140), initArea10013State (0x801F3BEC) and
 *           initArea11713State (0x801F3F70), which call the same helper and
 *           store 0x2C/0/0x09, 0x2C/0/2 and 0x2C/0/5 through the same three
 *           shared bytes; each measured exact when it was written. A payload cmp
 *           of all 64 bytes of this span against each of those three measured
 *           every word equal except the third stored constant, so their
 *           statement order, plain extern declarations and untagged helper
 *           prototype are transcribed and this body's own 0x0A is written in.
 *           No in-image jal or pointer word addresses 0x801F2C04 (a scan of all
 *           2824 payload bytes finds none), so the overlay reaches this handler
 *           from outside its payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea05413State(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 0x0A;
}
