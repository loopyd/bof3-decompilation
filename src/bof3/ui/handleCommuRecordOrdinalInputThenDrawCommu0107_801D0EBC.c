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

/* Shared overlay step byte in main RAM; every overlay handler advances it by one
 * as its own step completes, and this body rolls it back on the cancel mask. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* Commu request step byte the confirm paths set and the cancel path rolls back;
 * the dispatcher at 0x801D0E80 indexes the run at 0x801D4BE4 with it. */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

/* @source 0x8014E640 @kind unknown
 * Shared pad-mask decoder: returns the direction bits of the button mask it is
 * given.
 */
extern s32 func_8014E640(u32 mask);

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

extern void drawRowMarkerOutlineCommu0107_801D36FC(u8 row, u8 selected);
extern void drawCommuEntryListScreen(void);
/* The marker emitter narrows only its y argument with a signed halfword, so x
 * is declared wide here and the constant x arguments keep their bare `addiu`
 * form; the selected flag is the byte the emitter masks on entry. */
extern void drawMarkerTriangleCommu0107_801D3984(s32 x, s16 y, u8 selected);
extern void drawRecordSlotChannelBox(s16 x, s16 y, u8 slot, s32 selected);
extern u8 countActiveRecordsByKind(s32 kind);
extern u8 findActiveRecordSlotByKind(u8 kind, u8 ordinal);

/* @source 0x801D0EBC
 * @behavior Handler at entry 6 of the dispatch table at 0x801D4BCC, the same
 * dispatch shape as the entry list handler at 0x801D1158 one entry later. It
 * reads the shared pad-state word 0x80145AA8 once and dispatches on its masks.
 * While that word shares a bit with the cancel mask 0x80145AC4 it queues
 * resident sound cue 0x106 through the shared cue dispatcher at 0x8015DF18,
 * reports result code 0xB5 through the request result halfword 0x801D4E0C and
 * rolls the shared overlay step byte 0x801F294C back by one. While it shares a
 * bit with the confirm mask 0x80145AC2 it queues resident cue 0x104, reports
 * result code 0x44 through the same halfword, copies the shared list row
 * 0x801448ED into the request row byte 0x801D4E28 and advances the commu request
 * step byte 0x801F294D by one. When neither mask matches it takes the record
 * count of the list row kind through countActiveRecordsByKind, decodes the
 * 0xA000 bits of the re-read pad-state word through the shared decoder at
 * 0x8014E640 and steps the ordinal byte 0x801D4E24 inside that count: bit 0x2000
 * adds one and wraps to 0 when the signed byte reaches the count, bit 0x8000
 * subtracts one and wraps to count minus one when the signed byte goes negative,
 * no other bit moves it, and the ordinal byte landing on a different value
 * queues resident cue 0x100. Every path then runs drawCommuEntryListScreen and draws the
 * list row 0x801448ED with the selected flag 1 through
 * drawRowMarkerOutlineCommu0107_801D36FC, and then draws the record of the
 * ordinal byte: while the list row is zero through
 * drawMarkerTriangleCommu0107_801D3984 at 0x28 plus 8 times the signed ordinal on
 * x and 0x39 on y, and the slot box through drawRecordSlotChannelBox at 0x30 plus that same
 * sum on x and 0x42 on y; otherwise through the same pair at 0xFA plus 8 times
 * the ordinal modulo 5 on x and 0x44 plus 54 times the list row minus 9 plus 7
 * times the ordinal divided by 5 on y, and the slot box at 0xA8 plus the same
 * modulo term on x and 8 below that y. Both marker calls select flag 0, and the
 * box takes
 * the slot findActiveRecordSlotByKind returns for the list row and the ordinal
 * truncated to its low byte. The 0x20-byte frame only holds s0, s1, s2 and ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleCommuRecordOrdinalInputThenDrawCommu0107_801D0EBC(void) {
  u16 pad;
  u16 *padWord;
  u8 count;
  u8 ordinal;
  u8 *row;
  s32 direction;
  s32 x;
  s32 y;
  s32 ordOffset;
  s8 col;
  s8 line;

  /* The pad word is read twice, the second time after the record count call, so
   * the front end keeps its address in a register across that call. */
  padWord = &D_80145AA8;
  pad = *padWord;
  if (pad & D_80145AC4) {
    dispatchSoundCue(0x106);
    D_801D4E0C = 0xB5;
    D_801F294C = D_801F294C - 1;
  } else if (pad & D_80145AC2) {
    dispatchSoundCue(0x104);
    D_801D4E0C = 0x44;
    D_801D4E28 = D_801448ED;
    D_801F294D++;
  } else {
    count = countActiveRecordsByKind(D_801448ED);
    direction = func_8014E640(*padWord & 0xA000);
    ordinal = D_801D4E24;
    if (direction & 0x2000) {
      D_801D4E24 = ordinal + 1;
      if ((s8)(ordinal + 1) == count) {
        D_801D4E24 = 0;
      }
    } else if (direction & 0x8000) {
      D_801D4E24 = ordinal - 1;
      if ((s8)(ordinal - 1) < 0) {
        D_801D4E24 = count - 1;
      }
    }
    if (ordinal != D_801D4E24) {
      dispatchSoundCue(0x100);
    }
  }
  drawCommuEntryListScreen();
  row = &D_801448ED;
  drawRowMarkerOutlineCommu0107_801D36FC(*row, 1);
  if (*row == 0) {
    /* 8 times the signed ordinal byte, sign extended before the scale. */
    ordinal = D_801D4E24;
    ordOffset = ((s32)ordinal << 24) >> 21;
    drawMarkerTriangleCommu0107_801D3984(ordOffset + 0x28, 0x39, 0);
    drawRecordSlotChannelBox(ordOffset + 0x30, 0x42,
                              findActiveRecordSlotByKind(*row, D_801D4E24), 0);
  } else {
    col = (s8)D_801D4E24 % 5;
    line = (s8)D_801D4E24 / 5;
    x = col * 8;
    y = (*row - 9) * 54 + line * 7;
    drawMarkerTriangleCommu0107_801D3984(x + 0xFA, y + 0x44, 0);
    drawRecordSlotChannelBox(x + 0xA8, y + 0x4C,
                              findActiveRecordSlotByKind(*row, D_801D4E24), 0);
  }
}
