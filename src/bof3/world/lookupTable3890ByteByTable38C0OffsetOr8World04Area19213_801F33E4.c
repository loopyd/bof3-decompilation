#include "bof3/bof3.h"

extern u8 D_801F38C0[]; /* @source 0x801F38C0 @kind table */
extern u8 D_801F3890[]; /* @source 0x801F3890 @kind table */

/* @source 0x801F33E4
 * @behavior Reads the small column offset at 0x801F38C0 indexed by the low
 * byte of arg2 and adds it to arg0 while the low byte of arg3 stays below 5,
 * and otherwise adds the fixed offset 8 to arg0; it then returns the byte at
 * 0x801F3890 indexed by the low byte of that sum. arg1 is unused. Both tables
 * sit in this payload's own bin `tail` segment, 48 bytes apart: 0x801F38C0
 * holds the column offsets and 0x801F3890 the 45-byte block of five 9-column
 * rows addressed as a row base plus one of those offsets. The single caller at 0x801F326C passes
 * a record base of nine times the slot index, the clamped gate byte as arg2 and
 * a record field byte as arg3, and masks the returned byte to a halfword. The
 * bytes match emi/world04/area191/13 0x801F3924 apart from the two table
 * immediates and that sibling's own branch target, so the body is a shape copy
 * of it with this overlay's addresses rather than a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 lookupTable3890ByteByTable38C0OffsetOr8World04Area19213_801F33E4(
    u32 arg0, u32 arg1, u32 arg2, u32 arg3) {
  u32 index;

  if ((arg3 & 0xFF) < 5) {
    index = D_801F38C0[arg2 & 0xFF] + arg0;
  } else {
    index = arg0 + 8;
  }
  return D_801F3890[index & 0xFF];
}
