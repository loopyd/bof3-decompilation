#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Shared pressed-button masks paired with the pad-state word: 0x80145AC2 is the
 * confirm mask and 0x80145AC4 the cancel mask in this family's pad tests. */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Kind and grid line bytes of each of the eight stride-8 local record slots;
 * this body reads the slot the list row points at, at that row minus one, and
 * takes its kind byte at 0 and its grid line byte at 2. */
extern u8 D_801457A8[][8]; /* @source 0x801457A8 @kind data */

/* Per-kind entry of the kind grid this step moves a line cursor in: the first
 * byte is the number of grid lines the line cursor wraps inside, the second the
 * kind grid paired with this one, 0xFF when the kind has no pair. */
extern u8 D_801D4C18[][2]; /* @source 0x801D4C18 @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* Kind byte of the kind grid this step moves a line cursor in; 0xFF marks a
 * grid that is not selected. */
extern u8 D_801D4E1C; /* @source 0x801D4E1C @kind data */

/* Kind byte of the grid paired with 0x801D4E1C, taken from that kind's table
 * entry once the confirm path resolves the pair. */
extern u8 D_801D4E20; /* @source 0x801D4E20 @kind data */

/* Request row byte of the current commu request; read as a signed byte. */
extern s8 D_801D4E28; /* @source 0x801D4E28 @kind data */

/* Line cursor of the kind grid 0x801D4E1C, stepped by the pad direction. */
extern u8 D_801D4E2C; /* @source 0x801D4E2C @kind data */

/* Grid line byte the confirm path copies out of the list row's record slot. */
extern u8 D_801D4E30; /* @source 0x801D4E30 @kind data */

/* Commu request step byte: the dispatcher at 0x801D1328 indexes the table at
 * 0x801D4BEC with it, and entry three of that table is this body. */
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

extern void func_801D3038(void);
extern void saveCommuRequestStateAndSelectStep5(void);
extern void drawRowMarkerOutlineCommu0107_801D36FC(u8 row, u8 selected);
extern void computeKindSlotOrigin(s16 *outX, s16 *outY, s32 kind, s32 ordinal);
/* The marker emitter narrows only its y argument with a signed halfword, so x
 * is declared wide here. */
extern void drawMarkerTriangleCommu0107_801D3984(s32 x, s16 y, u8 selected);
extern s16 computeEntryListOriginY(void);
extern void drawEntryListPanel(s16 x, s16 y, u8 mode);
extern s16 drawKindGridPanel(s16 x, s16 y, u8 kind, u8 mode);
extern void func_801647C4(u16 x, u16 y, s32 mode);

/* @source 0x801D1A94
 * @behavior Input and redraw step of the commu01/07 commu request flow, entry
 * eleven of the handler run at 0x801D4BCC and entry three of the dispatch table
 * at 0x801D4BEC that the dispatcher at 0x801D1328 indexes with the commu
 * request step byte 0x801F294D. Reads the shared pad-state word 0x80145AA8 once
 * and dispatches on its masks. While that word shares a bit with the cancel
 * mask 0x80145AC4 it queues resident sound cue 0x106, marks the kind grid byte
 * 0x801D4E1C empty with 0xFF and rolls the commu request step byte back by one.
 * While it shares a bit with the confirm mask 0x80145AC2 it indexes the kind
 * grid and pair table at 0x801D4C18 with the kind grid byte: an entry whose
 * second byte is 0xFF marks a kind with no paired grid and saves the request
 * state through saveCommuRequestStateAndSelectStep5, while any other entry has
 * that second byte copied into the paired grid byte 0x801D4E20; either way the
 * grid line byte 0x801D4E30 takes the third byte of the record slot at 8 times
 * the shared list row 0x801448ED minus one from the stride-8 slot run at
 * 0x801457A8 when the request row byte 0x801D4E28 equals that slot's first byte
 * minus 4 and takes zero otherwise, and the paired entry then queues resident
 * cue 0x103, reports result code 0x46 through the request result halfword
 * 0x801D4E0C and advances the commu request step byte by one. When neither mask
 * matches it decodes the 0x5000 bits of the pad word through the shared decoder
 * at 0x8014E640 and steps the grid line byte 0x801D4E2C inside the first byte
 * of that same table entry: bit 0x1000 subtracts one and wraps to that first
 * byte minus one when the signed result goes negative, bit 0x4000 adds one and
 * wraps to zero when the signed result reaches that first byte, no other bit
 * moves it, and a line landing on a different value queues resident cue 0x100.
 * Every path then runs func_801D3038, draws the list row marker of 0x801448ED
 * with selected flag 1 through drawRowMarkerOutlineCommu0107_801D36FC, computes
 * the slot origin of that row kind at ordinal 0 through computeKindSlotOrigin,
 * draws the marker triangle at that origin with selected flag 1, draws the
 * entry-list panel in mode 1 at 0x42 plus 52 times the quarter index of the list
 * row on the origin computeEntryListOriginY returns and, while the kind grid
 * byte is not 0xFF, draws that kind grid in mode 0 at 0x4A plus that same term
 * on the bottom of the entry list advanced by twelve pixels plus sixteen per
 * unit of the request row byte, then submits the grid line cursor box through
 * func_801647C4 at 0x4C plus that same term rounded down to a multiple of four
 * on x and at the bottom of that grid plus 4 plus sixteen per unit of the grid
 * line byte on y.
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleCommuKindGridLineInputThenDraw(void) {
  u16 pad;
  u8 *cursor;
  u8 line;
  u8 next;
  s32 direction;
  s8 srow;
  s16 x;
  s16 y;

  pad = D_80145AA8;
  if (pad & D_80145AC4) {
    dispatchSoundCue(0x106);
    D_801D4E1C = 0xFF;
    D_801F294D = D_801F294D - 1;
  } else if (pad & D_80145AC2) {
    if (D_801D4C18[D_801D4E1C][1] == 0xFF) {
      saveCommuRequestStateAndSelectStep5();
    } else {
      srow = D_801D4E28;
      if (srow == D_801457A8[D_801448ED - 1][0] - 4) {
        D_801D4E30 = D_801457A8[D_801448ED - 1][2];
      } else {
        D_801D4E30 = 0;
      }
      D_801D4E20 = D_801D4C18[D_801D4E1C][1];
      dispatchSoundCue(0x103);
      D_801D4E0C = 0x46;
      D_801F294D = D_801F294D + 1;
    }
  } else {
    direction = func_8014E640(pad & 0x5000);
    line = D_801D4E2C;
    if (direction & 0x1000) {
      next = D_801D4E2C - 1;
      D_801D4E2C = next;
      if ((s8)next < 0) {
        D_801D4E2C = D_801D4C18[D_801D4E1C][0] - 1;
      }
    } else if (direction & 0x4000) {
      next = D_801D4E2C + 1;
      D_801D4E2C = next;
      if ((s8)next >= D_801D4C18[D_801D4E1C][0]) {
        D_801D4E2C = 0;
      }
    }
    if (line != D_801D4E2C) {
      dispatchSoundCue(0x100);
    }
  }
  func_801D3038();
  cursor = &D_801448ED;
  drawRowMarkerOutlineCommu0107_801D36FC(*cursor, 1);
  computeKindSlotOrigin(&x, &y, *cursor, 0);
  drawMarkerTriangleCommu0107_801D3984(x, y, 1);
  y = computeEntryListOriginY();
  drawEntryListPanel((s16)(52 * ((*cursor - 1) % 4) + 0x42), y, 1);
  if (D_801D4E1C != 0xFF) {
    y = y + (12 + 16 * D_801D4E28);
    y = drawKindGridPanel((s16)(52 * ((*cursor - 1) % 4) + 0x4A), y,
                          D_801D4E1C, 0);
    func_801647C4((u16)((52 * ((*cursor - 1) % 4) + 0x4C) & 0xFFFC),
                  (u16)(y + 4 + (s8)D_801D4E2C * 16), 0);
  }
}
