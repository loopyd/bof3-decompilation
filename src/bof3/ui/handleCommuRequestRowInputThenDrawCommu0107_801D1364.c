#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Shared pressed-button masks paired with the pad-state word: 0x80145AC2 is the
 * confirm mask and 0x80145AC4 the cancel mask in this family's pad tests. */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Kind byte of each of the eight stride-8 local record slots. This body reads
 * the slot the list row points at, at that row minus one. */
extern u8 D_801457A8[][8]; /* @source 0x801457A8 @kind data */

/* Request row byte of the current commu request. */
extern u8 D_801D4E28; /* @source 0x801D4E28 @kind data */

/* Ordinal byte of the current commu request: the zero-based slot ordinal of the
 * request row inside its record kind. */
extern u8 D_801D4E24; /* @source 0x801D4E24 @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* The two kind-grid selector bytes of the commu request; 0xFF marks a grid row
 * no selectable record claims. */
extern u8 D_801D4E1C; /* @source 0x801D4E1C @kind data */
extern u8 D_801D4E20; /* @source 0x801D4E20 @kind data */

/* Grid line offsets of those two selector bytes. */
extern u8 D_801D4E2C; /* @source 0x801D4E2C @kind data */
extern u8 D_801D4E30; /* @source 0x801D4E30 @kind data */

/* Shared overlay step byte in main RAM; every overlay handler advances it by one
 * as its own step completes, and this body rolls it back on the cancel mask. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* Commu request step byte: the dispatcher at 0x801D1328 indexes the table at
 * 0x801D4BEC with it, and its entry zero is this body. */
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

extern void drawCommuEntryListScreen(void);
extern void drawRowMarkerOutlineCommu0107_801D36FC(u8 row, u8 selected);
extern void computeKindSlotOrigin(s16 *outX, s16 *outY, s32 kind, s32 ordinal);
extern void drawMarkerTriangleCommu0107_801D3984(s16 x, s16 y, u8 selected);
extern void drawRecordSlotChannelBox(s16 x, s16 y, u8 slot, s32 selected);
extern u8 countActiveRecordsByKind(s32 kind);
extern u8 findActiveRecordSlotByKind(u8 kind, u8 ordinal);

/* Representation notes: the slot run is read as 8-byte rows because the byte
 * spelling `D_801457A8[(D_801448ED - 1) * 8]` let the row minus one fold into the
 * base address, which measured 177 of 179 instructions; the row view keeps the
 * subtract ahead of the scale. The ordinal byte tests sign extend their operand
 * through the `ord` copy, and the advanced step byte is materialized before the
 * request row copy: writing the increment straight to the cell, or after the row
 * copy, moved its store earlier and measured 172 of 179. */

/* @source 0x801D1364
 * @behavior Entry zero of the dispatch table at 0x801D4BEC that the dispatcher
 * at 0x801D1328 indexes with the commu request step byte 0x801F294D, and entry
 * eight of the handler run at 0x801D4BCC. Reads the shared pad-state word
 * 0x80145AA8 once and dispatches on its masks. While that word shares a bit with
 * the cancel mask 0x80145AC4 it queues resident sound cue 0x106 through the
 * shared cue dispatcher at 0x8015DF18, reports result code 0xB5 through the
 * request result halfword 0x801D4E0C and rolls the shared overlay step byte
 * 0x801F294C back by one. While it shares a bit with the confirm mask 0x80145AC2
 * it queues resident cue 0x103 and takes the request ordinal byte 0x801D4E24:
 * when that byte is nonzero it reports result code 0x44 through the same
 * halfword, copies the shared list row 0x801448ED into the request row byte
 * 0x801D4E28 and advances the commu request step byte 0x801F294D by one, while a
 * zero ordinal byte starts the request kind grid instead, taking the kind byte of
 * the record slot at 8 times the list row minus one from the stride-8 slot run at
 * 0x801457A8 as the request row byte less 4, or zero when that kind byte is zero,
 * clearing both kind-grid selector bytes 0x801D4E1C and 0x801D4E20 to 0xFF,
 * clearing the two grid line offsets 0x801D4E2C and 0x801D4E30 and selecting
 * commu request step 2. When neither mask matches it decodes the 0xA000 bits of
 * the pad word through the shared decoder at 0x8014E640, takes the record count
 * of the list row kind through countActiveRecordsByKind and steps the ordinal
 * byte inside that count: bit 0x8000 subtracts one and wraps to the count itself
 * when the signed byte goes negative, bit 0x2000 adds one and wraps to zero when
 * the signed byte passes the count, no other bit moves it, and an ordinal byte
 * landing on a different value queues resident cue 0x100. Every path then runs
 * drawCommuEntryListScreen and draws the list row 0x801448ED with the selected flag 1
 * through drawRowMarkerOutlineCommu0107_801D36FC, then computes the slot origin
 * of that list row kind at the ordinal byte through computeKindSlotOrigin and, for
 * a nonzero ordinal byte, draws the marker triangle at that origin advanced by 4
 * on x with the selected flag 0 through drawMarkerTriangleCommu0107_801D3984 and
 * the record slot cursor box of the stride-8 active record slot at that ordinal
 * byte less one through drawRecordSlotChannelBox at 0x10 right of that origin on x and 8
 * less than it on y, while a zero ordinal byte draws only the marker triangle at
 * the origin itself with the selected flag 0. The 0x28-byte frame holds s0, s1
 * and ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleCommuRequestRowInputThenDrawCommu0107_801D1364(void) {
  u16 pad;
  s32 direction;
  u8 count;
  u8 ordinal;
  u8 slot;
  /* Signed view of the request ordinal byte at its two tests. */
  s8 ord;
  /* Value the confirm path advances the request step byte to. */
  u8 step;
  u8 *row;
  s16 x;
  s16 y;

  pad = D_80145AA8;
  if (pad & D_80145AC4) {
    dispatchSoundCue(0x106);
    D_801D4E0C = 0xB5;
    D_801F294C = D_801F294C - 1;
  } else if (pad & D_80145AC2) {
    dispatchSoundCue(0x103);
    ord = D_801D4E24;
    if (ord != 0) {
      D_801D4E0C = 0x44;
      step = D_801F294D + 1;
      D_801D4E28 = D_801448ED;
      D_801F294D = step;
    } else {
      slot = D_801457A8[D_801448ED - 1][0];
      if (slot == 0) {
        D_801D4E28 = 0;
      } else {
        D_801D4E28 = slot - 4;
      }
      D_801D4E1C = 0xFF;
      D_801D4E20 = 0xFF;
      D_801D4E2C = 0;
      D_801D4E30 = 0;
      D_801F294D = 2;
    }
  } else {
    direction = func_8014E640(pad & 0xA000);
    ordinal = D_801D4E24;
    count = countActiveRecordsByKind(D_801448ED);
    if (direction & 0x8000) {
      D_801D4E24 = D_801D4E24 - 1;
      if ((s8)D_801D4E24 < 0) {
        D_801D4E24 = count;
      }
    } else if (direction & 0x2000) {
      D_801D4E24 = D_801D4E24 + 1;
      if (count < (s8)D_801D4E24) {
        D_801D4E24 = 0;
      }
    }
    if (ordinal != D_801D4E24) {
      dispatchSoundCue(0x100);
    }
  }
  drawCommuEntryListScreen();
  row = &D_801448ED;
  drawRowMarkerOutlineCommu0107_801D36FC(*row, 1);
  computeKindSlotOrigin(&x, &y, *row, D_801D4E24);
  ord = D_801D4E24;
  if (ord == 0) {
    drawMarkerTriangleCommu0107_801D3984(x, y, 0);
  } else {
    drawMarkerTriangleCommu0107_801D3984(x + 4, y, 0);
    drawRecordSlotChannelBox(
        x + 0x10, y - 8, findActiveRecordSlotByKind(*row, D_801D4E24 - 1), 0);
  }
}
