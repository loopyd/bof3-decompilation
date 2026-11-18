#include "bof3/world/area03004_internal.h"

extern u8 D_801C9E8A[]; /* @source 0x801C9E8A @kind table */

extern int sprintf(char* buffer, const char* format, ...);

/* @source 0x801D8364
 * @behavior draws the AREA030 lure column at the requested origin: submits the
 * 0x8B by 0x9E inset panel through func_801AE3F0 with shade 0x80 and the shared
 * CLUT-bank byte D_80144952 as its trailing argument, then walks the 0x80 slot
 * entries of the shared main-executable tables D_801451CC (record index) and
 * D_801453CC (count) and, for every slot whose 20-byte equipment record in
 * D_801C9E7C carries the kind byte 10 at +0x0E, draws that record's twelve
 * character name through func_8014F800 at (x + 5, row). The work-record byte
 * +0x0A is a countdown decremented per call: its low byte +0x38 is the first
 * visible row (one less while work byte +0x08 is clear), the drawn row limit is
 * 10 (9 in that case) and the column height reads 0x19 - 4 * count (0x19 - 4 *
 * (3 - count) in that case). The row the work word +0x10 names is drawn
 * highlighted with colour 7 and, while the shared byte 0x80145E94 reads two and
 * bit 0 of work byte +0x07 is set, a second unhighlighted copy of its name is
 * drawn two pixels up; the work halfword +0x3A is stamped with that row's slot
 * index. A slot count of two or more is formatted through "*%2d" into the
 * shared 0x80145AD4 buffer and drawn through func_80150098. The column closes
 * with func_801D90FC at the origin, the stretched panel column at x + 0x80
 * whose height loses six pixels per row beyond the ninth and whose origin
 * follows the work-record scroll byte, and the four-character "LURE" title of
 * the panel at (x + 0x37, y + 7).
 * @status partial
 * @match 97.15
 * @residual Count-X setup and the footer keep-copy still differ.
 * The compiled body is four bytes shorter than the original.
 */
void drawLureListColumnWorld00Area03004_801D8364(s16 arg0, s16 arg1) {
  u16 x;
  u16 y;
  u8 rowLimit;
  u8 scrollRows;
  s32 nameX;
  s32 shiftedNameX;
  s16 baseY;
  s16 lineY;
  s32 textY;
  s32 lineOffset;
  s32 rows;
  s32 drawn;
  s32 i;
  s32 textX;
  u8 scroll;
  s32 columnEnd;
  s32 columnY;
  s32 rawColumnY;
  u8* record;
  u8* columnWork;
  u8* buf;
  u8* slotCursor;

  x = arg0;
  y = arg1;
  func_801AE3F0(x, (u16)(y + 1), 0x8B, 0x9E, 0x80, D_80144952);

  {
    u8* scrollWork;
    scrollWork = D_1F800044;
    if (scrollWork[0x0A] != 0) {
      scrollWork[0x0A] = scrollWork[0x0A] - 1;
      scrollWork = D_1F800044;
      if (scrollWork[0x08] != 0) {
        scrollRows = scrollWork[0x38];
      } else {
        scrollRows = scrollWork[0x38] - 1;
      }
      scrollWork = D_1F800044;
      rowLimit = 10;
      if (scrollWork[0x08] != 0) {
        columnY = 0x19 - scrollWork[0x0A] * 4;
      } else {
        columnY = 0x19 - (3 - scrollWork[0x0A]) * 4;
      }
    } else {
      scrollRows = scrollWork[0x38];
      rowLimit = 9;
      columnY = 0x19;
    }
  }

  i = 0;
  rows = 0;
  drawn = 0;
  baseY = y;
  buf = D_80145AD4;
  slotCursor = D_801451CC;
  shiftedNameX = (u32)(u16)(x + 5) << 16;
  nameX = shiftedNameX >> 16;
  baseY += (u8)columnY;
  textY = 0;
  lineY = baseY;
  lineOffset = -2;

  for (; i < 0x80; i++, slotCursor++) {
    if (D_801C9E8A[*slotCursor * 20] == 10) {
      if ((rows >= scrollRows) && (drawn < rowLimit)) {
        if (D_80145E94[0] != 2) {
          goto unselected;
        }
        record = D_1F800044;
        if ((record[7] & 1) == 0) {
          goto unselected;
        }
        if (FIELD_REF(s32, record, 0x10) != drawn) {
          goto unselected;
        }
        func_8014F800(nameX, (s16)lineY, 7, 0xC,
                      (u32)&D_801C9E7C[*slotCursor * 20]);
        func_8014F800(nameX, (s16)(baseY + lineOffset), 0, 0xC,
                      (u32)&D_801C9E7C[*slotCursor * 20]);
        if (D_801453CC[i] >= 2) {
          sprintf((char*)buf, (const char*)D_801D0C14, D_801453CC[i]);
          textX = x + 0x68;
          func_80150098((s16)textX, (s16)(baseY + (s16)(textY + 4)), 7, buf);
          func_80150098((s16)textX, (s16)(baseY + (s16)(textY + 2)), 0, buf);
        }
        goto row_drawn;
unselected:
        func_8014F800(shiftedNameX >> 16, (s16)lineY, 0, 0xC,
                      (u32)&D_801C9E7C[*slotCursor * 20]);
        if (D_801453CC[i] >= 2) {
          sprintf((char*)buf, (const char*)D_801D0C14, D_801453CC[i]);
          func_80150098((s16)(x + 0x68), (s16)(baseY + (s16)(textY + 4)), 0, buf);
        }
row_drawn:
        record = D_1F800044;
        if (FIELD_REF(s32, record, 0x10) == drawn) {
          FIELD_REF(u16, record, 0x3A) = i;
        }
        lineY += 13;
        textY += 13;
        drawn += 1;
        lineOffset += 13;
      }
      rows += 1;
    }
  }

  {
    u8* selectedWork;
    s32 lastRow;
    lastRow = rows - 1;
    selectedWork = D_1F800044;
    FIELD_REF(s32, selectedWork, 0x1C) = lastRow;
    if (FIELD_REF(s32, selectedWork, 0x10) >= drawn) {
      if (((lastRow) != 0) && (drawn != 0)) {
        FIELD_REF(s32, selectedWork, 0x10) = drawn - 1;
      } else {
        FIELD_REF(s32, selectedWork, 0x10) = 0;
      }
    }
  }

  func_801D90FC((s16)x, (s16)y);

  i = 0;
  drawn = 0;
  for (; i < 0x80; i++) {
    columnWork = D_801C9E8A;
    columnWork += D_801451CC[i] * 20;
    if (*columnWork == 10) {
      drawn += 1;
    }
  }

  if (drawn < 10) {
    columnEnd = 0x72;
    columnY = 0x1C;
  } else {
    drawn -= 9;
    columnEnd = 0x72 - drawn * 6;
    columnWork = D_1F800044;
    rawColumnY = FIELD_REF(u16, columnWork, 0x38) * 6 + 0x1C;
    scroll = columnWork[0x0A];
    if (scroll != 0) {
      if (columnWork[0x08] != 0) {
        columnY = rawColumnY + scroll * 2;
      } else {
        columnY = rawColumnY - scroll * 2;
      }
    } else {
      columnY = rawColumnY;
    }
  }

  drawStretchedPanelColumnWorld00Area03004_801D8248(
      (s16)(x + 0x80), (s16)(y + (u8)columnY), (u8)(columnEnd & 0xFE));
  func_8014F800((s16)(x + 0x37), (s16)(y + 7), 0, 4, (u32)D_801E21D0);
}
