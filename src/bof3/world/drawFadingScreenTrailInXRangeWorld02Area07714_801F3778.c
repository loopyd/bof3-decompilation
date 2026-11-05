#include "bof3/bof3.h"

/* One 4-byte trail pair: the x halfword the range and order tests read and the
 * y halfword the packed line argument carries in its high half. */
typedef struct ScreenTrailPair {
  s16 x;
  u16 y;
} ScreenTrailPair;

/* Same-target draw helpers. 0x801F3898 walks the pixels between two by-value
 * pairs and 0x801F3B48 emits one shaded primitive from a pair's halfwords. */
void func_801F3898(ScreenTrailPair first, ScreenTrailPair second, u8 shade);
void func_801F3B48(s16* point, u8 shade);

/* @source 0x801F3778
 * @behavior Walks count - 1 adjacent pairs of the 4-byte trail at the passed
 *           base, the base the caller 0x801F36E8 supplies as 0x800E4800 with a
 *           count of 0x48 and a window rect. For each pair whose second x lies
 *           in the half-open range [rect->x, rect->x + rect->w) it calls the
 *           line helper 0x801F3898 with both pairs so the current x is not
 *           below the next x, and otherwise calls the point helper 0x801F3B48
 *           on the second pair; the pair reaches the line helper as one word,
 *           (y << 16) | (u16)x. Both draws get the byte shade, seeded at 0x40
 *           and dropped by 2 after each iteration until it reaches zero. The
 *           pair pointer advances one entry per iteration. Returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFadingScreenTrailInXRangeWorld02Area07714_801F3778(
    u8 count, ScreenTrailPair* current, RECT* rect) {
  u8 shade;
  u8 i;

  shade = 0x40;
  for (i = 0; i < count - 1; i++) {
    if (current[1].x >= rect->x && current[1].x < rect->x + rect->w) {
      if (current[0].x >= current[1].x) {
        func_801F3898(current[0], current[1], shade);
      } else {
        func_801F3B48(&current[1].x, shade);
      }
    }
    if (shade != 0) {
      shade -= 2;
    }
    current++;
  }
}
