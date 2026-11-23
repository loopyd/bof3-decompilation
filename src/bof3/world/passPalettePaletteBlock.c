#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads it once, ahead of the
 * frame's $ra save, and reads work byte 5 through that one load.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared main-executable palette block row helper, outside this payload. It
 * receives the 0x80035600 palette block base plus the work byte scaled by 0x40
 * and a second argument this overlay passes as 1; that argument's role is not
 * established here, so both parameter roles stay structural.
 */
void func_80166C48(u8* row, u32 flag); /* @source 0x80166C48 */

/* @source 0x801F4D54
 * @behavior Passes one 0x40-byte row of the shared palette block to the
 *           main-executable palette block helper func_80166C48. It loads the
 *           scratch work record once through the pointer cell 0x1F800044,
 *           reads work byte 0x05, scales it by 0x40 with a left shift of six
 *           and adds that offset to the fixed palette block base 0x80035600,
 *           so the callee receives the row that work byte selects, together
 *           with the second argument 1. There is no test and no branch: the
 *           call is the whole body, and the 0x18-byte frame exists only to keep
 *           $ra at 0x10 across it. The body takes no arguments and returns
 *           nothing. No jal in this payload names it, so its only in-image
 *           reference is the code-pointer word 0x801F6658, entry 18 of the
 *           dispatch table this overlay places at 0x801F6610, which makes it one
 *           of the overlay's table-dispatched state handlers. The role of work
 *           byte 0x05 is the same one the plchar overlay lifts record for this
 *           cell, the selector of a 0x40-byte palette block row, and it stays
 *           anchored to byte 0x05 here because nothing else in this payload
 *           reads it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void passPalettePaletteBlock(
    void) {
  func_80166C48((u8*)(0x80035600u + (D_1F800044[5] << 6)), 1);
}
