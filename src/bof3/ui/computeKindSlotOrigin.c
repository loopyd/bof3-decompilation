#include "bof3/bof3.h"

/* One entry of the six-entry offset table at 0x801D4DDC: the pixel offset of
 * one record slot from the grid cell of its kind. 4 bytes. */
typedef struct Commu0107SlotOffset {
  u16 x; /* +0x00 */
  u16 y; /* +0x02 */
} Commu0107SlotOffset;

/* The per-slot offsets of the entry grid, in slot order. */
extern Commu0107SlotOffset D_801D4DDC[]; /* @source 0x801D4DDC @kind data */

extern u8 countActiveRecordsByKind(s32 kind);

/* @source 0x801D4694
 * @behavior Computes the screen origin of one record slot of the commu01/07
 * entry grid and stores it through the two halfword pointers. The low byte of
 * kind selects column (kind - 1) % 4 of four 52-pixel columns beginning at
 * x 0x20 together with row band (kind - 1) >> 2 of 61-pixel rows beginning at
 * y 0x6B. An ordinal of zero takes the cell origin with a fixed 0xA inset on x.
 * Any other ordinal starts at slot 2 when the kind has two live records, at
 * slot 1 when it has three and at slot 0 otherwise, and adds the offset of
 * table entry 0x801D4DDC at that slot advanced by ordinal - 1. drawCommuEntryListScreen
 * walks ordinals 1 through the live record count of each kind 1 through 8 and
 * draws a piece at every result, func_801D1364 and func_801D1630 pass the
 * overlay byte 0x801D4E24 as the ordinal, and func_801D1778, func_801D1A94,
 * func_801D1E48 and func_801D2110 pass ordinal 0. All of them pass the shared
 * row byte 0x801448ED as the kind. Only the two output halfwords are written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void computeKindSlotOrigin(s16 *outX, s16 *outY, s32 kind, s32 ordinal) {
  s32 base;
  s32 row;
  s32 slot;
  s32 x;
  s32 y;
  s32 sumX;
  s32 sumY;
  u8 slots;

  slot = 0;
  kind &= 0xFF;
  base = kind - 1;
  x = base % 4 * 52 + 0x20;
  row = base >> 2;
  y = row * 61 + 0x6B;
  slots = countActiveRecordsByKind(kind);
  switch (slots) {
  case 2:
    slot = 2;
    break;
  case 3:
    slot = 1;
    break;
  }
  if (ordinal & 0xFF) {
    ordinal--;
    sumX = x + D_801D4DDC[slot + (ordinal & 0xFF)].x;
    *outX = sumX;
    sumY = y + D_801D4DDC[slot + (ordinal & 0xFF)].y;
    *outY = sumY;
  } else {
    *outX = x + 0xA;
    *outY = y;
  }
}
