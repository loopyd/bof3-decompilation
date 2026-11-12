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
 * request row inside its record kind. This body reads it as a signed byte. */
extern u8 D_801D4E24; /* @source 0x801D4E24 @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

extern void restoreSavedCommuRequestStateAndStepBack(void);
extern void publishCommuEntryTextId(void);
extern void publishCommuEntryTextIdForRow(u8 *rowPtr);
extern s32 commitCommuEntryRequestIntoActiveRecord(s32 offset);
extern void func_801D3B0C(u8 *rowPtr);
extern void func_801D36FC(u8 row, u8 selected);
extern void func_801D3038(void);
/* The marker emitter narrows only its y argument with a signed halfword, so x
 * is declared wide here and the constant x arguments keep their bare `addiu`
 * form; the selected flag is the byte the emitter masks on entry. */
extern void drawMarkerTriangleCommu0107_801D3984(s32 x, s16 y, u8 selected);
extern void func_801D28A4(s16 x, s16 y, u8 slot, s32 selected);
extern u8 findActiveRecordSlotByKind(u8 kind, u8 ordinal);

/* @source 0x801D1158
 * @behavior Handler at entry 7 of the handler run at 0x801D4BCC, the same
 * dispatch shape as the entry list handler at 0x801D1630. It reads the shared
 * pad-state word 0x80145AA8 once and dispatches on its masks. While that word
 * shares a bit with the cancel mask 0x80145AC4 it queues resident sound cue
 * 0x106 through the shared cue dispatcher at 0x8015DF18, publishes the list row
 * text id of 0x801448ED through publishCommuEntryTextId and then closes the
 * request through restoreSavedCommuRequestStateAndStepBack. While it shares a
 * bit with the confirm mask 0x80145AC2 it commits the request row through
 * commitCommuEntryRequestIntoActiveRecord with offset 0 and stores result code
 * 0xB5 into the halfword
 * 0x801D4E0C when the returned low byte is nonzero. When neither mask matches
 * it moves the request row byte 0x801D4E28 through func_801D3B0C and publishes
 * that row text id through publishCommuEntryTextIdForRow. Every path then runs
 * func_801D3038, draws the list row 0x801448ED with the selected flag 1 and the
 * request row 0x801D4E28 with 0 through func_801D36FC, and while no commit
 * happened draws the marker through drawMarkerTriangleCommu0107_801D3984 with the selected flag 1: at
 * 0x28 plus 8 times the signed ordinal byte 0x801D4E24 on x and 0x39 on y while
 * the list row cursor is zero, otherwise at 0xFA plus 8 times that byte modulo 5
 * on x and 0x44 plus 54 times the cursor minus 9 plus 7 times that byte divided
 * by 5 on y. It then draws the record slot cursor box at (0xA0, 0x3E) with the
 * selected flag 1 through func_801D28A4 for the slot
 * findActiveRecordSlotByKind returns for the cursor and that byte. No state
 * other than the result halfword, the request row byte, the list row cursor and
 * the callee cells is written, and the 0x20-byte frame only holds s0, s1 and ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleCommuEntryListInputThenDrawCommu0107_801D1158(void) {
  u16 pad;
  s32 committed;
  u8 *row;
  s32 lineInset;
  s8 col;
  s8 line;

  pad = D_80145AA8;
  committed = 0;
  if (pad & D_80145AC4) {
    dispatchSoundCue(0x106);
    publishCommuEntryTextId();
    restoreSavedCommuRequestStateAndStepBack();
  } else if (pad & D_80145AC2) {
    committed = commitCommuEntryRequestIntoActiveRecord(0);
    if (committed & 0xFF) {
      D_801D4E0C = 0xB5;
    }
  } else {
    row = &D_801D4E28;
    func_801D3B0C(row);
    publishCommuEntryTextIdForRow(row);
  }
  func_801D3038();
  row = &D_801448ED;
  func_801D36FC(*row, 1);
  func_801D36FC(D_801D4E28, 0);
  if ((committed & 0xFF) == 0) {
    if (*row == 0) {
      drawMarkerTriangleCommu0107_801D3984(*(s8 *)&D_801D4E24 * 8 + 0x28, 0x39,
                                           1);
    } else {
      /* The 0x44 inset reaches the line term as a value the front end cannot
       * fold: written as a literal it moves onto the cursor term and lands one
       * instruction earlier (measured, 115 of 116 instructions). */
      lineInset = 0x44;
      col = (s8)D_801D4E24 % 5;
      line = (s8)D_801D4E24 / 5;
      drawMarkerTriangleCommu0107_801D3984(
          col * 8 + 0xFA, (*row - 9) * 54 + (line * 7 + lineInset), 1);
    }
    func_801D28A4(0xA0, 0x3E, findActiveRecordSlotByKind(D_801448ED, D_801D4E24),
                  1);
  }
}
