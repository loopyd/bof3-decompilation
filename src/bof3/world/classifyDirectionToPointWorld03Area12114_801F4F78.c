#include "bof3/bof3.h"

/* @source 0x801468BC @kind unknown */
extern s32 D_801468BC[];
/* @source 0x801468C0 @kind unknown */
extern s32 D_801468C0[];

/* @source 0x801F4F78
 * @behavior Classifies the signs of the point's two coordinate differences
 * from words at 0x801468BC and 0x801468C0 in a byte-indexed, 152-byte-stride
 * record. Returns direction codes 0/4/6/2 for --/++/-+/+-, 3/7 for +0/-0,
 * 5 for 0+, and 1 for 0- or coincident points. Does not write memory.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 classifyDirectionToPointWorld03Area12114_801F4F78(s32 x, s32 y, u8 slot) {
  x -= D_801468BC[slot * 38];
  y -= D_801468C0[slot * 38];
  if (x < 0 && y < 0) {
    return 0;
  }
  if (x > 0 && y > 0) {
    return 4;
  }
  if (x < 0 && y > 0) {
    return 6;
  }
  if (x > 0 && y < 0) {
    return 2;
  }
  if (x > 0 && y == 0) {
    return 3;
  }
  if (x < 0 && y == 0) {
    return 7;
  }
  if (x == 0 && y > 0) {
    return 5;
  }
  return 1;
}
