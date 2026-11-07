#include "bof3/bof3.h"

extern u8 D_801F4290[]; /* @source 0x801F4290 @kind data */
extern u8 D_801F4260[]; /* @source 0x801F4260 @kind data */

/* @source 0x801F3924
 * @behavior Reads the small column offset at 0x801F4290 indexed by the low
 * byte of arg2 and adds it to arg0 while the low byte of arg3 is below 5, and
 * otherwise adds the fixed offset 8 to arg0; it then returns the byte at
 * 0x801F4260 indexed by the low byte of that sum. arg1 is unused. Both tables
 * sit in this overlay's own tail data run: 0x801F4290 holds the column offsets
 * and 0x801F4260 the index block addressed as a row base plus that offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 lookupTable4260ByteByTable4290OffsetOr8World04Area19113_801F3924(u32 arg0, u32 arg1, u32 arg2, u32 arg3) {
  u32 index;

  if ((arg3 & 0xFF) < 5) {
    index = D_801F4290[arg2 & 0xFF] + arg0;
  } else {
    index = arg0 + 8;
  }
  return D_801F4260[index & 0xFF];
}
