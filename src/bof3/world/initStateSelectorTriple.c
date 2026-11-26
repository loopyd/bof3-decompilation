#include "bof3/bof3.h"

/* @source 0x801448EB @kind unknown */
extern u8 D_801448EB;
/* @source 0x801448EC @kind unknown */
extern u8 D_801448EC;
/* @source 0x801448ED @kind unknown */
extern u8 D_801448ED;

void func_8015C088(void);

/* @source 0x801F2C68
 * @behavior Runs the shared front-end startup helper at 0x8015C088 and then
 *           writes 0x2C, 0 and 0xD to the three consecutive shared mode bytes
 *           at 0x801448EB, 0x801448EC and 0x801448ED; takes no arguments and
 *           returns 0. The byte at 0x801448EB is the front-end state selector
 *           indexed into the callback table, so this is the overlay's
 *           state-entry tuple write. The 0x18-byte frame only keeps $ra across
 *           the call, each byte store materialises the 0x8014 page in $at on
 *           its own, and the 0xD for the third byte is re-materialised into
 *           $v1 right after the first store, before the middle byte takes the
 *           zero register. The explicit zero return is the addu $v0,$zero,$zero
 *           ahead of jr $ra. No in-image jal or leading pointer word addresses
 *           0x801F2C68, so the overlay reaches this handler from outside its
 *           payload. Its 64 bytes are the shared startup body of
 *           initArea10513State (world02/area105/13, 0x801F3140), which stores
 *           0x2C and 0 into the same two leading bytes and 9 into the third
 *           without a zero return; world00/area037/13 at 0x801F3ADC carries
 *           the 0xD third byte with that zero return in a two-store body.
 *           The helper sets flag bits; the inspected original callers discard
 *           its incidental address in $v0, so no result is declared here.
 * @status partial
 * @match 68.75
 * @residual Constant scratch register and final two byte-store scheduling differ.
 */
s32 initStateSelectorTriple(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 0xD;
  return 0;
}
