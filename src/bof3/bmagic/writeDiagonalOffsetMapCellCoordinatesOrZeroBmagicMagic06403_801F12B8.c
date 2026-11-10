#include "bof3/bof3.h"

extern s16 D_80149320; /* @source 0x80149320 @kind data */
extern s16 D_80149322; /* @source 0x80149322 @kind data */
extern u8 D_80104001; /* @source 0x80104001 @kind data */
extern u16 D_80104002; /* @source 0x80104002 @kind data */

/* @source 0x801F12B8
 * @behavior Converts diagonal offsets to map-cell coordinates, writing the pair when inside the map border and its halfword cell is nonzero, or zeroing both output bytes otherwise.
 * @status partial
 * @match 83.02
 * @residual Address arithmetic register allocation and commutative operand ordering differ.
 */
void writeDiagonalOffsetMapCellCoordinatesOrZeroBmagicMagic06403_801F12B8(s32 diagonal, s32 offset, u8 *out) {
  s32 x;
  s32 y;
  u8 *map;

  x = D_80149320 + (diagonal + 1) / 2 + offset;
  y = D_80149322 + diagonal / 2 - offset;
  if (x > 0) {
    map = PSX_PTR(u8, 0x80104000);
    if (x < map[0] - 1 && y > 0 && y < D_80104001 - 1 &&
        *(u16 *)(y * map[0] * 2 + (D_80104002 * 4 + x * 2) + map) != 0) {
      out[0] = x;
      out[1] = y;
      return;
    }
  }
  out[0] = 0;
  out[1] = 0;
}
