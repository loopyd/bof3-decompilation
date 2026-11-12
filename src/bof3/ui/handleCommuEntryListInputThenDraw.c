#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Shared pressed-button masks paired with the pad-state word: 0x80145AC2 is the
 * confirm mask and 0x80145AC4 the cancel mask in this family's pad tests. */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Request row byte of the current commu request. */
extern u8 D_801D4E28; /* @source 0x801D4E28 @kind data */

/* Ordinal byte of the current commu request: the zero-based slot ordinal of the
 * request row inside its record kind. */
extern u8 D_801D4E24; /* @source 0x801D4E24 @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

extern void restoreSavedCommuRequestStateAndStepBack(void);
extern s32 commitCommuEntryRequestIntoActiveRecord(s32 offset);
extern void moveCommuRowCursorByPadDirection(u8 *rowPtr);
extern void drawRowMarkerOutlineCommu0107_801D36FC(u8 row, u8 selected);
extern void func_801D3038(void);
extern void drawMarkerTriangleCommu0107_801D3984(s16 x, s16 y, u8 selected);
extern void drawRecordSlotChannelBox(s16 x, s16 y, u8 slot, s32 selected);
void publishCommuEntryTextId(void);
void publishCommuEntryTextIdForRow(u8 *rowPtr);
void computeKindSlotOrigin(s16 *outX, s16 *outY, s32 kind, s32 ordinal);
u8 findActiveRecordSlotByKind(u8 kind, u8 ordinal);

/* @source 0x801D1630
 * @behavior Handler at entry 9 of the dispatch table at 0x801D4BCC. Reads the
 * shared pad-state word 0x80145AA8 once and dispatches on its masks. While that
 * word shares a bit with the cancel mask 0x80145AC4 it closes the request
 * through restoreSavedCommuRequestStateAndStepBack and publishes the list row
 * text id of 0x801448ED through publishCommuEntryTextId into the result
 * halfword 0x801D4E0C. While it
 * shares a bit with the confirm mask 0x80145AC2 it commits the request row
 * through commitCommuEntryRequestIntoActiveRecord with offset 1 and stores
 * result code 0xB5 into
 * 0x801D4E0C when the returned low byte is nonzero. When neither mask matches it
 * moves the request row byte 0x801D4E28 through
 * moveCommuRowCursorByPadDirection and publishes that row text id through
 * publishCommuEntryTextIdForRow. Every path then runs
 * func_801D3038, draws the list row 0x801448ED with the selected flag 1 and the
 * request row 0x801D4E28 with 0 through drawRowMarkerOutlineCommu0107_801D36FC, and when no commit
 * happened computes the slot origin of the list row kind at ordinal 0x801D4E24
 * through computeKindSlotOrigin, draws the marker at that origin advanced by 4
 * on x with the selected flag 1 through drawMarkerTriangleCommu0107_801D3984,
 * and draws the record slot cursor box at (0xA0, 0x3E) with the selected flag 1
 * through drawRecordSlotChannelBox for
 * the slot findActiveRecordSlotByKind returns for that kind and ordinal minus
 * one. No state other than the result halfword, the request row byte, the list
 * row cursor and the callee cells is written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleCommuEntryListInputThenDraw(void) {
  u16 pad;
  s32 committed;
  u8 *row;
  s16 x;
  s16 y;

  pad = D_80145AA8;
  committed = 0;
  if (pad & D_80145AC4) {
    restoreSavedCommuRequestStateAndStepBack();
    publishCommuEntryTextId();
  } else if (pad & D_80145AC2) {
    committed = commitCommuEntryRequestIntoActiveRecord(1);
    if (committed & 0xFF) {
      D_801D4E0C = 0xB5;
    }
  } else {
    row = &D_801D4E28;
    moveCommuRowCursorByPadDirection(row);
    publishCommuEntryTextIdForRow(row);
  }
  func_801D3038();
  row = &D_801448ED;
  drawRowMarkerOutlineCommu0107_801D36FC(*row, 1);
  drawRowMarkerOutlineCommu0107_801D36FC(D_801D4E28, 0);
  if ((committed & 0xFF) == 0) {
    computeKindSlotOrigin(&x, &y, *row, D_801D4E24);
    drawMarkerTriangleCommu0107_801D3984(x + 4, y, 1);
    drawRecordSlotChannelBox(0xA0, 0x3E,
                              findActiveRecordSlotByKind(*row, D_801D4E24 - 1),
                              1);
  }
}
