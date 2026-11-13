#include "bof3/ui/shop00_internal.h"

/* One two-byte row of the count-prefixed list the panel task root holds at
 * +0x20: the pair func_801DA9C4 resolves through its mode and index bytes. */
typedef struct ShopPriceRow {
  u8 kind;
  u8 index;
} ShopPriceRow;

/* The row list itself: the row count leading its rows. The single-element
 * array spells the rows' +1 offset without a flexible-array extension. */
typedef struct ShopPriceRowList {
  u8 count;
  ShopPriceRow rows[1];
} ShopPriceRowList;

/* Local view of the panel task root this step reads. Its prefix repeats the
 * shared panel layout (the two origin halfwords at +0x04 and +0x06, same as
 * func_801E3284); the bytes the row loop reads follow it and the row-list
 * pointer closes the run at +0x20. Only the offsets this body reads are named,
 * and the roles no byte proves stay address anchored. */
typedef struct ShopPriceListTask {
  u8 unk_00[4];
  u16 x;        /* +0x04 panel origin column */
  u16 field_06; /* +0x06 panel origin row */
  u8 unk_08;    /* +0x08 */
  u8 unk_09;    /* +0x09 */
  u8 unk_0A;    /* +0x0A nonzero forces the costly shade on every row */
  u8 unk_0B;    /* +0x0B percent the row value is scaled by */
  u8 unk_0C;    /* +0x0C index of the highlighted row */
  u8 unk_0D[0x13];
  ShopPriceRowList* rows; /* +0x20 count-prefixed row list */
} ShopPriceListTask;

/* @source 0x801E2F04
 * @behavior shop price list panel of the sliding shop panel: draws the 0xAD by
 *           0x9F shaded rectangle at (x + 3, y + 3) through func_801AE3F0 with
 *           task byte 0xA as the shade and the main-RAM CLUT-bank byte
 *           D_80144952 as the trailing argument, then one row per entry of the
 *           count-prefixed list the task holds at +0x20. Every entry is a
 *           (kind, index) pair: the row value is func_801DA9C4(kind, index)
 *           scaled by task byte 0xB through scaleByPercentFloor1 and formatted
 *           into the shared text buffer D_80145AD4 with the "%6dZ" string
 *           D_801D0E88. The row draws the icon cell
 *           D_801E5E0C[func_80165E04(kind, index) & 0xFF] through func_801AEA80
 *           at (x + 4, y + 3) with the mode byte, the record
 *           func_80165D48(kind, index), masked through func_801AF20C, as the
 *           handle of func_8014F800 at (x + 0xE, y), and the buffer text
 *           through func_80150098 at (x + 0x71, y + 3); both text emitters take
 *           the same shade argument. The shade is 7 and the mode byte 1 when
 *           task byte 0xA is nonzero or the main-RAM money word D_80144F50 is
 *           below the row value, and 0 and 0 otherwise. Rows step 0xD pixels
 *           down. The highlighted row (index == task byte 0xC) draws twice:
 *           while its shade is 0 it first draws the icon with the mode 1, the
 *           record and the buffer text with the literal shade 7 at the row's
 *           own y, then every highlighted row draws again with its computed
 *           shade and mode two pixels up, and the position is restored before
 *           the step. The panel frame func_801DD350 closes the body at (x, y)
 *           with the constants 0x14 and 0x13.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopPriceListPanel(PanelTask* task) {
  ShopPriceListTask* view;
  ShopPriceRowList* list;
  u16 base;
  u32 value;
  u32 icon;
  u8* record;
  u8 shade;
  s32 highlight;
  s32 y;
  u8 i;
  u8 kind;
  u8 index;

  view = (ShopPriceListTask*)task;
  func_801AE3F0((u16)(view->x + 3), (u16)(view->field_06 + 3), 0xAD, 0x9F,
                view->unk_0A, D_80144952);
  list = view->rows;
  y = view->field_06 + 8;
  for (i = 0; i < list->count; i++) {
    kind = list->rows[i].kind;
    index = list->rows[i].index;
    base = func_801DA9C4(kind, index);
    value = scaleByPercentFloor1(base, view->unk_0B);
    icon = func_80165E04(kind, index);
    record = func_80165D48(kind, index);
    if (view->unk_0A != 0 || (u32)D_80144F50 < value) {
      shade = 7;
      highlight = 1;
    } else {
      shade = 0;
      highlight = 0;
    }
    sprintf((char*)D_80145AD4, D_801D0E88, value);
    if (i == view->unk_0C) {
      if (shade == 0) {
        func_801AEA80((u16)(view->x + 4), (u16)(y + 3),
                      D_801E5E0C[icon & 0xFF], 1);
        func_8014F800((s16)(view->x + 0xE), (s16)y, 7,
                      (u8)func_801AF20C(record), (u32)record);
        func_80150098((s16)(view->x + 0x71), (s16)(y + 3), 7, D_80145AD4);
      }
      y -= 2;
    }
    func_801AEA80((u16)(view->x + 4), (u16)(y + 3), D_801E5E0C[icon & 0xFF],
                  highlight);
    func_8014F800((s16)(view->x + 0xE), (s16)y, shade, (u8)func_801AF20C(record),
                  (u32)record);
    func_80150098((s16)(view->x + 0x71), (s16)(y + 3), shade, D_80145AD4);
    if (i == view->unk_0C) {
      y += 2;
    }
    y += 0xD;
  }
  func_801DD350((s16)view->x, (s16)view->field_06, 0x14, 0x13);
}
