#include "bof3/ui/shop00_internal.h"

/* @source 0x801DF6A8
 * @behavior shop screen draw called with no arguments by the phase step
 *           func_801DE40C, the cursor/confirm handler func_801DE4C8 and the
 *           sub-step handler func_801DE88C: it first emits the sprite grid
 *           through func_801DBD24 with the main-RAM index byte D_80144953,
 *           reads the visible-list length through func_801BDB7C(0) and draws
 *           the shop panel strip through func_801DAB90 with the fixed panel
 *           arguments 0x14, 0x12, 0x118 and 0x13 plus the main-RAM CLUT-bank
 *           byte D_80144952. While the slot index stays below that length it
 *           draws each visible slot through the field-list emitter
 *           func_801D826C with the fixed x 0x11, the even row offset
 *           (slot * 54 + 0x3E) & 0xFFFE, the field-record index
 *           D_80181B10[D_80144F5A[slot]] and bit 1 of that record's flag byte
 *           D_8014496F shifted down to 0/1, then formats the one-based slot
 *           ordinal through sprintf into the shared text buffer D_80145AD4
 *           with the EMI-local "%d" format string D_801D0E60 and draws it
 *           through the main-exe text emitter func_80150098 at x 0x1A by the
 *           sign-extended row y slot * 54 + 0x43 with the alignment constant
 *           that same flag bit selects (7 when set, 0 when clear). Afterwards
 *           it emits the slidable off-screen slot region through func_801DF00C
 *           at the fixed position (0x96, 0x3B) and marks both shop selections
 *           through emitFlatLineRectangleBorder with the D_801E5BC8[slot]
 *           [column] coordinate pair, the sprite/palette constants 0x7E/0x30
 *           when the cell's flag byte is set and 0x80/0x34 when it is clear:
 *           first the saved pair D_8014865A/D_8014865D unless the saved kind
 *           byte still holds the 0x7F "no saved selection" sentinel, with the
 *           selection argument 0, then the live cursor pair
 *           D_80148658/D_80148659 with the selection argument 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Unlike the sliding sibling drawNumberedShopSlotListThenPanelStrip every
 * offset here is a constant: the original materializes 0x11, 0x1A and 0x96
 * directly, so no frame-timer pointer group exists. The field-record flag byte
 * is read twice per slot - once as the caller's bit-1 test and once for the
 * text alignment - because sprintf sits between the two uses and the original
 * reloads it rather than keeping it live. The saved slot index D_8014865D is
 * declared u8 for the site that reads it zero-extended; this function indexes
 * the geometry table with the signed byte the original loads here, so the
 * signed view is local to this use. */
void drawShopSlotListWithSelectionCursors(void) {
  u8 count;
  u8 slot;

  slot = 0;
  func_801DBD24(D_80144953);
  count = func_801BDB7C(0);
  func_801DAB90(0x14, 0x12, 0x118, 0x13, D_80144952);
  if (count != 0) {
    do {
      u8 record;
      s32 offset;

      record = D_80181B10[D_80144F5A[slot]];
      offset = record * 164;
      func_801D826C(0x11, (u32)(slot * 54 + 0x3E) & 0xFFFE, record,
                    (D_8014496F[offset] >> 1) & 1, 0);
      sprintf((char*)D_80145AD4, D_801D0E60, slot + 1);
      func_80150098(0x1A, (s16)(slot * 54 + 0x43),
                    (D_8014496F[offset] & 2) ? 7 : 0, D_80145AD4);
      slot++;
    } while (slot < count);
  }
  func_801DF00C(0x96, 0x3B);
  if (D_8014865A != 0x7F) {
    ShopSelectionCell* cell;

    cell = &D_801E5BC8[*(s8*)&D_8014865D][D_8014865A];
    if (cell->unk_04 != 0) {
      emitFlatLineRectangleBorder(cell->x, cell->y, 0x7E, 0x30, 0, 6);
    } else {
      emitFlatLineRectangleBorder(cell->x, cell->y, 0x80, 0x34, 0, 6);
    }
  }
  {
    ShopSelectionCell* cell;

    cell = &D_801E5BC8[D_80148659][D_80148658];
    if (cell->unk_04 != 0) {
      emitFlatLineRectangleBorder(cell->x, cell->y, 0x7E, 0x30, 1, 6);
    } else {
      emitFlatLineRectangleBorder(cell->x, cell->y, 0x80, 0x34, 1, 6);
    }
  }
}
