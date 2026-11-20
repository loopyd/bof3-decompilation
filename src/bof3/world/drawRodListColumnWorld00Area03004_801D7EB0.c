#include "bof3/world/area03004_internal.h"

extern int sprintf(char* buffer, const char* format, ...);
extern u8 D_801C9E8A[]; /* @source 0x801C9E8A @kind table */
extern const char D_801E21CC[]; /* @source 0x801E21CC @kind rodata */

/* @source 0x801D7EB0
 * @behavior Draws the AREA030 ROD list column at the requested origin.
 * Walks 128 slot indices and draws records whose byte +0x0E is 11,
 * publishing the selected slot index to work halfword +0x36. The row
 * named by work word +0x0C receives the highlighted drawing while the
 * shared mode byte is two and work byte 7 bit 0 is clear. Counts of two
 * or more are formatted through "*%2d". Publishes the matching-row count
 * to work word +0x18 and bounds an out-of-range upper selection to the
 * last row, or zero when there are no matching rows. Finishes with the
 * panel footer, stretched column and three-character ROD title.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRodListColumnWorld00Area03004_801D7EB0(s16 arg0, s32 arg1) {
  u16 x;
  u16 baseY;
  s32 nameX;
  s32 countX;
  s32 rowY;
  u16 panelY;
  s32 rowOffset;
  s32 rows;
  s32 slotIndex;
  s32 selectedRow;
  u8 count;
  u8* slot;
  u8* work;
  u8* buffer;
  u8* options;
  u8* nameRecords;

  x = arg0;
  baseY = arg1;
  options = &D_80144952;
  panelY = baseY + 1;
  func_801AE3F0(x, (u16)panelY, 0x8B, 0x9E, 0xF0, *options);
  slotIndex = 0;
  rows = 0;
  buffer = D_80145AD4;
  slot = PSX_PTR(u8, (u32)options + 0x87Au);
  nameRecords = D_801C9E7C;
  rowY = arg1 + 0x19;
  rowOffset = 0;
  nameX = (s16)(x + 5);
  countX = (s16)(x + 0x68);
  for (; slotIndex < 0x80; slotIndex++, slot++) {
    if (D_801C9E8A[*slot * 20] != 11) {
      continue;
    }
    work = D_1F800044;
    if (FIELD_REF(s32, work, 0x0C) == rows) {
      FIELD_REF(u16, work, 0x36) = (u16)slotIndex;
    }
    if (D_80145E94[0] != 2) {
      goto unselected;
    }
    work = D_1F800044;
    if ((work[7] & 1) != 0) {
      goto unselected;
    }
    if (FIELD_REF(s32, work, 0x0C) != rows) {
      goto unselected;
    }
    func_8014F800((s16)nameX, (s16)rowY, 7, 0x0C,
                 *slot * 20 + (u32)nameRecords);
    func_8014F800((s16)nameX, (s16)(baseY + 0x17 + rowOffset),
                 0, 0x0C, *slot * 20 + (u32)nameRecords);
    count = D_801453CC[slotIndex];
    if (count >= 2) {
      sprintf((char*)buffer, D_801D0C14, count);
      func_80150098((s16)countX, (s16)(baseY + 0x1D + rowOffset),
                    7, buffer);
      func_80150098((s16)countX, (s16)(baseY + 0x1B + rowOffset),
                    0, buffer);
    }
    goto row_drawn;

unselected:
    func_8014F800((s16)nameX, (s16)rowY, 0, 0x0C,
                 *slot * 20 + (u32)nameRecords);
    count = D_801453CC[slotIndex];
    if (count >= 2) {
      sprintf((char*)buffer, D_801D0C14, count);
      func_80150098((s16)countX, (s16)(baseY + 0x1D + rowOffset),
                    0, buffer);
    }

row_drawn:
    do {
      rowY += 0x0D;
    } while (0);
    rowOffset += 0x0D;
    rows++;
  }
  work = D_1F800044;
  selectedRow = FIELD_REF(s32, work, 0x0C);
  FIELD_REF(s32, work, 0x18) = rows;
  if (selectedRow >= rows) {
    if (rows == 0) {
      FIELD_REF(s32, work, 0x0C) = 0;
    } else {
      FIELD_REF(s32, work, 0x0C) = rows - 1;
    }
  }
  drawListColumnFrameWorld00Area03004((s16)x, (s16)baseY);
  drawStretchedPanelColumnWorld00Area03004_801D8248(
      (s16)(x + 0x80), (s16)(baseY + 0x1C), 0x72);
  func_8014F800((s16)(x + 0x3A), (s16)(baseY + 7),
               0, 3, (u32)D_801E21CC);
}
