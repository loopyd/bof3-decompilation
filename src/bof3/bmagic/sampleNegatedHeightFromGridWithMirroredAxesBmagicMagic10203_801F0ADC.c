#include "bof3/bof3.h"

/* Signed byte height grid this overlay samples. Its rows are fifteen bytes, so
 * a row step is 15 and the row above the sampled cell is reached through the
 * base minus fifteen that the original materialises before the index adds.
 * @source 0x801F0DDB @kind data
 */
extern s8 D_801F0DDB[16][15];

/* First-axis base subtracted from the scaled first coordinate, doubled.
 * @source 0x801F0CC4 @kind data
 */
extern s16 D_801F0CC4;

/* Second-axis base subtracted from the scaled second coordinate, doubled.
 * @source 0x801F0CC8 @kind data
 */
extern s16 D_801F0CC8;

/* Per-frame flag byte the overlay writes at init. Bit 0 selects whether each
 * scaled coordinate keeps its own axis base or takes the other one's.
 * @source 0x801F0CBC @kind data
 */
extern u8 D_801F0CBC;

/* @source 0x801F0ADC
 * @behavior Samples this overlay's signed byte height grid at D_801F0DDB for
 * one coordinate pair and returns the sampled height negated and scaled by
 * sixteen as a halfword. Both incoming coordinates are scaled by 64 and offset
 * by one of the two s16 axis bases, doubled: while bit 0 of D_801F0CBC is set
 * each coordinate keeps its own base, and while it is clear the pair crosses so
 * the first coordinate takes the second base. Each scaled coordinate is then
 * halved into a grid index and the pair's low bits pick the taps: both even
 * reads the cell plus the cell one row back one column on, x odd with y even
 * reads the column to the right plus the cell, x even with y odd reads that
 * column plus the row back one column on, and both odd reads only the column to
 * the right at double weight. A scaled coordinate at or past 0x20 in the first
 * axis or 0x1C in the second returns zero instead. All six callers in this
 * overlay add the returned halfword to a record word and store it into the
 * record's sprite Y halfword, which is where the negated vertical offset is
 * read off. The body is a leaf that takes two scalars and returns a signed
 * halfword, and keeps an eight byte frame whose only slot is the pad array
 * declared below: the original reserves and releases eight bytes of frame while
 * touching none of it, so the unused array reproduces that reservation.
 * @status partial
 * @match 60.00
 * @residual The first 34 instructions match. The remaining difference is
 * register allocation and tail scheduling inside the three two-tap arms: the
 * compiler here keeps the grid base in the opposite register to the original,
 * merges the second tap's load and the sum of two arms into one shared block
 * where the original keeps a copy in each arm, and therefore does not fold the
 * single-tap arm's doubling into the final shift.
 */
s16 sampleNegatedHeightFromGridWithMirroredAxesBmagicMagic10203_801F0ADC(
    s32 px,
    s32 py) {
  s32 x;
  s32 y;
  s32 h;
  s8 pad[8];

  if (D_801F0CBC & 1) {
    y = ((py + 0x4000) >> 6) - (D_801F0CC8 << 1);
    x = ((px + 0x4000) >> 6) - (D_801F0CC4 << 1);
  } else {
    y = ((px + 0x4000) >> 6) - (D_801F0CC4 << 1);
    x = ((py + 0x4000) >> 6) - (D_801F0CC8 << 1);
  }
  if (((u32)y >= 0x1C) | ((u32)x >= 0x20)) {
    return 0;
  }
  if (((y | x) & 1) == 0) {
    h = D_801F0DDB[x >> 1][y >> 1] + D_801F0DDB[(x >> 1) - 1][(y >> 1) + 1];
  } else if (y & 1) {
    if (x & 1) {
      h = D_801F0DDB[x >> 1][(y >> 1) + 1] << 1;
    } else {
      h = D_801F0DDB[x >> 1][(y >> 1) + 1] +
          D_801F0DDB[(x >> 1) - 1][(y >> 1) + 1];
    }
  } else if (x & 1) {
    h = D_801F0DDB[x >> 1][(y >> 1) + 1] + D_801F0DDB[x >> 1][y >> 1];
  } else {
    h = D_801F0DDB[x >> 1][(y >> 1) + 1] << 1;
  }
  (void)pad;
  return (s16)-(h << 3);
}
