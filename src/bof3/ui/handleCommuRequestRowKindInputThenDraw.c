#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Shared pressed-button masks paired with the pad-state word: 0x80145AC2 is the
 * confirm mask and 0x80145AC4 the cancel mask in this family's pad tests. */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Shared commu entry count: the number of entry rows the panel draws, and the
 * bound the request row byte wraps inside. */
extern u8 D_801455C3; /* @source 0x801455C3 @kind data */

/* Kind byte and grid line byte of each of the eight stride-8 local record
 * slots; this body reads the slot the list row points at, at that row minus
 * one. */
extern u8 D_801457A8[][8]; /* @source 0x801457A8 @kind data */

/* Text id of each request row, indexed by the signed request row byte. */
extern u16 D_801D4BB8[]; /* @source 0x801D4BB8 @kind data */

/* Kind grid each request row selects, indexed by the signed request row byte;
 * 0xFF marks a row that has no target kind grid. */
extern u8 D_801D4C04[]; /* @source 0x801D4C04 @kind data */

/* Text id of each selectable kind grid, indexed by its kind byte. */
extern u16 D_801D4C10[]; /* @source 0x801D4C10 @kind data */

/* Request row byte of the current commu request. */
extern u8 D_801D4E28; /* @source 0x801D4E28 @kind data */

/* Kind byte of the first kind grid the request stacks. */
extern u8 D_801D4E1C; /* @source 0x801D4E1C @kind data */

/* Grid line offset of that kind grid. */
extern u8 D_801D4E2C; /* @source 0x801D4E2C @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* Commu request step byte: the dispatcher at 0x801D1328 indexes the table at
 * 0x801D4BEC with it, and entry two of that table is this body. */
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
extern void publishCommuEntryTextId(void);
extern void saveCommuRequestStateAndSelectStep5(void);
extern void drawRowMarkerOutlineCommu0107_801D36FC(u8 row, u8 selected);
extern void computeKindSlotOrigin(s16 *outX, s16 *outY, s32 kind, s32 ordinal);
/* The marker emitter narrows only its y argument with a signed halfword, so x
 * is declared wide here. */
extern void drawMarkerTriangleCommu0107_801D3984(s32 x, s16 y, u8 selected);
extern s16 computeEntryListOriginY(void);
extern void drawEntryListPanel(s16 x, s16 y, u8 mode);
extern void func_801647C4(u16 x, u16 y, s32 mode);

/* @source 0x801D1778
 * @behavior Input and redraw step of the commu01/07 commu request flow, entry
 * ten of the handler run at 0x801D4BCC and entry two of the dispatch table at
 * 0x801D4BEC that the dispatcher at 0x801D1328 indexes with the commu request
 * step byte 0x801F294D. It reads the shared pad-state word 0x80145AA8 once and
 * dispatches on its masks. While that word shares a bit with the cancel mask
 * 0x80145AC4 it queues resident sound cue 0x106, publishes the list row text id
 * of 0x801448ED through publishCommuEntryTextId and rewinds the commu request
 * step byte 0x801F294D to zero. While it shares a bit with the confirm mask
 * 0x80145AC2 it takes the kind grid of the request row by indexing the
 * row-to-kind table at 0x801D4C04 with the signed request row byte 0x801D4E28: a
 * 0xFF entry marks a row that has no target kind grid and saves the request
 * state through saveCommuRequestStateAndSelectStep5, while any other entry
 * records that kind into 0x801D4E1C, queues resident cue 0x103, reports the
 * kind's text id from the halfword table at 0x801D4C10 through the request
 * result halfword 0x801D4E0C and advances the commu request step byte by one;
 * before either, while the request row equals the kind byte of the record slot
 * at 8 times the list row minus one from the stride-8 slot run at 0x801457A8
 * less 4 it copies that slot's second byte into the grid line offset 0x801D4E2C
 * and otherwise clears that offset. When neither mask matches it decodes the
 * 0x5000 bits of the pad word through the shared decoder at 0x8014E640 and steps
 * the request row byte inside the shared entry count 0x801455C3: bit 0x1000
 * subtracts one and wraps to the count minus one when the signed byte goes
 * negative, bit 0x4000 adds one and wraps to zero when the signed byte reaches
 * the count, no other bit moves it, and a row landing on a different value
 * queues resident cue 0x100; that arm reports the new row's text id from the
 * halfword table at 0x801D4BB8 through the same result halfword. Every path then
 * runs func_801D3038, draws the list row marker of 0x801448ED with selected flag
 * 1 through drawRowMarkerOutlineCommu0107_801D36FC, computes the slot origin of
 * that row kind at ordinal 0 through computeKindSlotOrigin, draws the marker
 * triangle at that origin with selected flag 1, draws the entry-list panel in
 * mode 0 at 0x42 plus 52 times the quarter index of the list row on the origin
 * computeEntryListOriginY returns, and submits the cursor box through
 * func_801647C4 at 0x44 plus that same term rounded down to a multiple of four
 * on x and at that origin plus 4 plus sixteen per unit of the request row on y.
 * No state other than the result halfword, the request row byte, the kind and
 * grid-line bytes, the request step byte and the callee cells is written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleCommuRequestRowKindInputThenDraw(void) {
  u16 pad;
  s32 direction;
  u8 row;
  u8 kind;
  u8 step;
  u8 next;
  s8 srow;
  u8 *cursor;
  s16 x;
  s16 y;

  pad = D_80145AA8;
  if (pad & D_80145AC4) {
    dispatchSoundCue(0x106);
    publishCommuEntryTextId();
    D_801F294D = 0;
  } else if (pad & D_80145AC2) {
    srow = D_801D4E28;
    kind = D_801D4C04[srow];
    if (kind == 0xFF) {
      saveCommuRequestStateAndSelectStep5();
    } else {
      if (srow == D_801457A8[D_801448ED - 1][0] - 4) {
        D_801D4E2C = D_801457A8[D_801448ED - 1][1];
      } else {
        D_801D4E2C = 0;
      }
      srow = D_801D4E28;
      D_801D4E1C = D_801D4C04[srow];
      dispatchSoundCue(0x103);
      D_801D4E0C = D_801D4C10[D_801D4E1C];
      step = D_801F294D + 1;
      D_801F294D = step;
    }
  } else {
    direction = func_8014E640(pad & 0x5000);
    row = D_801D4E28;
    if (direction & 0x1000) {
      next = D_801D4E28 - 1;
      D_801D4E28 = next;
      if ((s8)next < 0) {
        D_801D4E28 = D_801455C3 - 1;
      }
    } else if (direction & 0x4000) {
      next = D_801D4E28 + 1;
      D_801D4E28 = next;
      if ((s8)next >= D_801455C3) {
        D_801D4E28 = 0;
      }
    }
    if (row != D_801D4E28) {
      dispatchSoundCue(0x100);
    }
    srow = D_801D4E28;
    D_801D4E0C = D_801D4BB8[srow];
  }
  func_801D3038();
  cursor = &D_801448ED;
  drawRowMarkerOutlineCommu0107_801D36FC(*cursor, 1);
  computeKindSlotOrigin(&x, &y, *cursor, 0);
  drawMarkerTriangleCommu0107_801D3984(x, y, 1);
  y = computeEntryListOriginY();
  drawEntryListPanel((s16)(52 * ((*cursor - 1) % 4) + 0x42), y, 0);
  func_801647C4((u16)((52 * ((*cursor - 1) % 4) + 0x44) & 0xFFFC),
                (u16)(y + 4 + (s8)D_801D4E28 * 16), 0);
}
