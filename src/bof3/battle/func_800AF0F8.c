#include "bof3/battle/battle15_internal.h"

/* @source 0x800AF0F8
 * @behavior maps two 16.16 positions onto the render cell table: the cell index
 * is the sum of both position halves minus the s16 reference pair at
 * D_80149320/D_80149322, plus the extra byte and one per nonzero fractional
 * half. An out-of-table cell, or a cell whose block does not fit below the
 * primitive-arena limit (D_80143D44 * 0x9000 + 0x80028fcc), rolls the primitive
 * cursor back by rows * stride bytes. Otherwise, for each of rows iterations it
 * finds the largest remaining key in list[0..rows), clears that key, ORs the
 * low 24 bits of the packet at packet + best * stride into the code word of the
 * packet cell held by the cell's render table entry (D_80142CC4), and stores
 * that packet back into the entry.
 * @status partial
 * @match 54.05
 * @residual none
 * Residual class: ordering and scheduling. The arithmetic, control flow, loop
 * bodies and calls match; the prologue statement order, the branch/delay-slot
 * placement of the two fractional carries, the load placement of the three
 * stack byte arguments and loop-invariant constant hoisting do not.
 */
void func_800AF0F8(u32 pos0, u32 pos1, s32 *list, u8* packet, u8 rows, u8 stride,
                   u8 extra) {
  u32 delta;
  u32 row;
  u32 i;
  s32 best;
  s32 *cell;
  u32 color;
  s32 *slot;
  u8* cursor;
  u8  size;
  u32 offset;
  s32 dy;

  delta = ((s32)pos0 >> 16) - D_80149320;
  dy = ((s32)pos1 >> 16) - D_80149322;
  if (pos0 & 0xFFFF) {
    dy++;
  }
  delta += dy;
  if (pos1 & 0xFFFF) {
    delta += extra + 1;
  } else {
    delta += extra;
  }

  cursor = g_PrimCursor;
  size = stride;
  if (delta >= 0x38) {
    g_PrimCursor -= rows * stride;
    return;
  }
  if ((u8*)(D_80143D44 * 0x9000 + 0x80028fcc) > cursor + size) {
    offset = ((D_80143D44 + 4) << 3) + delta * 0x30;
    row = 0;
    while (row < rows) {
      best = 0;
      i = 0;
      while (i < rows) {
        if (list[best] < list[i]) {
          best = i;
        }
        i++;
      }
      if (list[best] != 0) {
        list[best] = 0;
        slot = (s32*)(packet + best * size);
        cell = *(s32**)(D_80142CC4 + offset);
        *cell = (*cell & 0xFF000000) | ((u32)slot & 0xFFFFFF);
        *(s32**)(D_80142CC4 + offset) = slot;
      }
      row++;
    }
  } else {
    g_PrimCursor = cursor - rows * size;
  }
}
