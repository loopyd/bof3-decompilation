#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

extern s32 func_8015477C(s32 x, s32 y);

/* @source 0x801F38E0
 * @behavior State handler 0 of the local work-byte-1 dispatch table based at
 *           0x801F545C. It seeds the work object's cursor triple published at
 *           the scratchpad pointer cell 0x1F800044: word +0x34 takes the
 *           fixed-point constant 0xA0000 (10.0), word +0x38 takes 0x630000
 *           (99.0), and word +0x3C takes the shared distance helper
 *           func_8015477C evaluated at that same pair, shifted left 16 into
 *           fixed point. It then advances the work object's state byte at +1
 *           from 0 to 1, which makes the next dispatch of table 0x801F545C land
 *           on its only other entry, the cursor-quad renderer at 0x801F3938
 *           that copies this same triple into a point. The distance call
 *           re-reads word +0x34 from the work object rather than reusing the
 *           stored constant, and the constant 0x630000 is materialised twice
 *           because it is both the second word and the second argument. The
 *           work-object pointer is re-loaded from the cell after the call
 *           instead of being kept in a saved register, which is why the frame
 *           only holds $ra. Takes no arguments and returns nothing. The roles of
 *           the two constants beyond being a position pair are not established,
 *           so the name and this block describe the effect only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkCursorTripleAndAdvanceStateByte1World03Area14514_801F38E0(void) {
  *(s32 *)(D_1F800044 + 0x34) = 0xA0000;
  *(s32 *)(D_1F800044 + 0x38) = 0x630000;
  *(s32 *)(D_1F800044 + 0x3C) =
      func_8015477C(*(s32 *)(D_1F800044 + 0x34), 0x630000) << 16;
  D_1F800044[1] += 1;
}
