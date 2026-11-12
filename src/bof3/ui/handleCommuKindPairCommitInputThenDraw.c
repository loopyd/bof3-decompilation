#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Shared pressed-button masks paired with the pad-state word: 0x80145AC2 is the
 * confirm mask and 0x80145AC4 the cancel mask in this family's pad tests. */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Byte views of the shared stride-8 active records at 0x801455C8: byte 1 is the
 * record kind, bytes 2 and 3 its state bytes and the word at +4 its
 * progress-anchor word. */
extern u8 D_801455C9[]; /* @source 0x801455C9 @kind data */
extern u8 D_801455CA[]; /* @source 0x801455CA @kind data */
extern u8 D_801455CB[]; /* @source 0x801455CB @kind data */
extern u8 D_801455CC[]; /* @source 0x801455CC @kind data */

/* Shared elapsed battle counter, stored as the progress anchor of the record
 * this step commits and of the slot run it keeps in step with the request. */
extern u32 D_8014502C; /* @source 0x8014502C @kind data */

/* Stride-8 selected-slot run the commu01/07 request flow edits: byte 0 is the
 * request row plus four, byte 1 the kind grid line, byte 2 the cursor box row,
 * byte 3 a byte this step clears and the word at +4 the battle counter. */
extern u8 D_801457A8[][8]; /* @source 0x801457A8 @kind data */

/* Progress-anchor word of each slot of that run, as the stride-8 run of word
 * pairs the commit stores the battle counter into. */
extern u32 D_801457AC[][2]; /* @source 0x801457AC @kind data */

/* Current commu request bytes and their snapshot, which the commit path copies
 * back over them. */
extern u8 D_801D4DF4; /* @source 0x801D4DF4 @kind data */
extern u8 D_801D4DF8; /* @source 0x801D4DF8 @kind data */
extern u8 D_801D4DFC; /* @source 0x801D4DFC @kind data */
extern u8 D_801D4E00; /* @source 0x801D4E00 @kind data */
extern u8 D_801D4E04; /* @source 0x801D4E04 @kind data */
extern u8 D_801D4E08; /* @source 0x801D4E08 @kind data */

/* Result halfword of the current commu request, and the saved step byte and
 * saved result halfword the save at 0x801D4B58 leaves for this step to restore. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */
extern u8 D_801D4E10; /* @source 0x801D4E10 @kind data */
extern u16 D_801D4E14; /* @source 0x801D4E14 @kind data */

/* Sub-mode toggle byte the 0xA000 direction bits flip; while it is set the
 * confirm mask restores the saved step instead of committing the pair. */
extern u8 D_801D4E18; /* @source 0x801D4E18 @kind data */

/* Kind bytes of the kind grid and of its paired grid; 0xFF marks a grid that is
 * not drawn. */
extern u8 D_801D4E1C; /* @source 0x801D4E1C @kind data */
extern u8 D_801D4E20; /* @source 0x801D4E20 @kind data */

/* Request row byte, kind grid line byte and cursor box row byte: the slot
 * comparison reads them signed and the slot stores write them unsigned. */
extern u8 D_801D4E28; /* @source 0x801D4E28 @kind data */
extern u8 D_801D4E2C; /* @source 0x801D4E2C @kind data */
extern u8 D_801D4E30; /* @source 0x801D4E30 @kind data */

/* Commu request step byte of the enclosing handler run, restored from the saved
 * step byte above. */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

/* Overlay step byte the commit path rolls back by one. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

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
/* The marker emitter narrows only its y argument with a signed halfword, so x
 * is declared wide here. */
extern void drawMarkerTriangleCommu0107_801D3984(s32 x, s16 y, u8 selected);
extern s16 computeEntryListOriginY(void);
extern void drawEntryListPanel(s16 x, s16 y, u8 mode);
extern s16 drawKindGridPanel(s16 x, s16 y, u8 kind, u8 mode);
extern void func_801647C4(u16 x, u16 y, s32 mode);

/* @source 0x801D2110
 * @behavior Input and redraw step of the commu01/07 commu request flow: overlay
 * step five of the dispatch table at 0x801D4BEC, and the terminal step of the
 * kind-pair request that the save at 0x801D4B58 selects. Reads the shared
 * pad-state word 0x80145AA8 once and dispatches on its masks. While that word
 * shares a bit with the confirm mask 0x80145AC2 and the sub-mode toggle byte
 * 0x801D4E18 is set, it queues resident sound cue 0x106 and restores the request
 * result halfword 0x801D4E0C from 0x801D4E14 and the commu request step byte
 * 0x801F294D from 0x801D4E10. While the word shares a bit with that mask and the
 * toggle byte is clear, it first marks the request stale unless the request row
 * byte 0x801D4E28 plus four, the kind grid line byte 0x801D4E2C and the cursor
 * box row byte 0x801D4E30 already equal the three leading bytes of the stride-8
 * slot run 0x801457A8 at the list row 0x801448ED minus one; a stale request
 * rewrites every stride-8 active record at 0x801455C8 whose kind byte
 * 0x801455C9 equals the list row with zero in its state bytes at +2 and +3 and
 * the shared battle counter 0x8014502C as its progress-anchor word at +4, and
 * then clears that slot's fourth byte and stores the battle counter as the word
 * at slot +4. Either way it writes those three request bytes back into that
 * slot, copies the saved request bytes 0x801D4E00, 0x801D4E04 and 0x801D4E08
 * into the current request bytes 0x801D4DF4, 0x801D4DF8 and 0x801D4DFC, queues
 * resident cue 0x104, reports result code 0xB5 and rolls the overlay step byte
 * 0x801F294C back by one. While the word shares a bit with the cancel mask
 * 0x80145AC4 it takes the same cue 0x106 restore of the saved result and the
 * saved step. When neither mask matches it decodes the 0xA000 bits of the pad
 * word through the shared decoder at 0x8014E640 and, when one of those bits
 * comes back, queues resident cue 0x100 and flips the toggle byte 0x801D4E18.
 * Every path then runs drawCommuEntryListScreen, draws the list row marker of 0x801448ED
 * with selected flag 1 through drawRowMarkerOutlineCommu0107_801D36FC, computes
 * the slot origin of that row kind at ordinal 0 through computeKindSlotOrigin,
 * draws the marker triangle at that origin with selected flag 1 and stacks its
 * panels with an x of 52 times the quarter index of the list row plus a fixed
 * offset: while the kind grid byte 0x801D4E1C is 0xFF only the entry list panel
 * is drawn, in mode 0 at 0x42 on the origin computeEntryListOriginY returns, and
 * the cursor box is submitted through func_801647C4 at 0x44 rounded down to a
 * multiple of four on x and at that origin plus four plus sixteen per unit of
 * the request row byte; otherwise the entry list panel is drawn in mode 1 at
 * 0x42 and, while the paired grid byte 0x801D4E20 is 0xFF, the kind grid of
 * 0x801D4E1C is drawn in mode 0 at 0x4A twelve pixels below that panel plus
 * sixteen per unit of the request row byte with the cursor box at 0x4C below
 * that grid plus four plus sixteen per unit of the grid line byte, and while
 * 0x801D4E20 is not 0xFF that kind grid is drawn in mode 1 at 0x4A and the kind
 * grid of 0x801D4E20 in mode 0 at 0x52 twelve pixels below the first grid plus
 * sixteen per unit of the grid line byte, with the cursor box at 0x54 below the
 * second grid plus four plus sixteen per unit of the cursor box row byte, each
 * panel taking the y the previous call returned.
 * @status exact
 * @match 100.00
 * @residual none
 */

/* Representation notes from the two probes before the matching gate: one shared
 * pointer for the loop, the slot tail and the write-back, with the slot word
 * store folded through a byte view of the run, measured 365/395; giving each of
 * those three sites its own local took the same folded store to 374/395, and
 * indexing a word view of the run recovered the minus one the original expands.
 * The active-record anchor is read 0x73F bytes past that cursor byte inside the
 * loop, the pointer form the original bytes use there, while the same counter is
 * read through its own symbol after the loop. */
void handleCommuKindPairCommitInputThenDraw(void) {
  u16 pad;
  u8 *rec;
  u8 *sel;
  u8 *cursor;
  u8 *row;
  u32 anchor;
  s32 direction;
  s32 i;
  u8 stale;
  s16 x;
  s16 y;

  pad = D_80145AA8;
  stale = 0;
  if (pad & D_80145AC2) {
    if (D_801D4E18 != 0) {
      dispatchSoundCue(0x106);
      D_801D4E0C = D_801D4E14;
      D_801F294D = D_801D4E10;
    } else {
      if (D_801457A8[D_801448ED - 1][0] != (s8)D_801D4E28 + 4 ||
          D_801457A8[D_801448ED - 1][1] != (s8)D_801D4E2C ||
          D_801457A8[D_801448ED - 1][2] != (s8)D_801D4E30) {
        stale = 1;
      }
      if (stale) {
        rec = &D_801448ED;
        for (i = 0; i < 0x1E0; i += 8) {
          if (D_801455C9[i] == *rec) {
            anchor = *(u32 *)(rec + 0x73F);
            D_801455CA[i] = 0;
            D_801455CB[i] = 0;
            *(u32 *)(D_801455CC + i) = anchor;
          }
        }
        sel = &D_801448ED;
        D_801457A8[*sel - 1][3] = 0;
        D_801457AC[*sel - 1][0] = D_8014502C;
      }
      cursor = &D_801448ED;
      D_801457A8[*cursor - 1][0] = D_801D4E28 + 4;
      D_801457A8[*cursor - 1][1] = D_801D4E2C;
      D_801457A8[*cursor - 1][2] = D_801D4E30;
      D_801D4DF4 = D_801D4E00;
      D_801D4DF8 = D_801D4E04;
      D_801D4DFC = D_801D4E08;
      dispatchSoundCue(0x104);
      D_801D4E0C = 0xB5;
      D_801F294C = D_801F294C - 1;
    }
  } else if (pad & D_80145AC4) {
    dispatchSoundCue(0x106);
    D_801D4E0C = D_801D4E14;
    D_801F294D = D_801D4E10;
  } else {
    direction = func_8014E640(pad & 0xA000);
    if (direction & 0xA000) {
      dispatchSoundCue(0x100);
      D_801D4E18 = D_801D4E18 ^ 1;
    }
  }
  drawCommuEntryListScreen();
  row = &D_801448ED;
  drawRowMarkerOutlineCommu0107_801D36FC(*row, 1);
  computeKindSlotOrigin(&x, &y, *row, 0);
  drawMarkerTriangleCommu0107_801D3984(x, y, 1);
  y = computeEntryListOriginY();
  if (D_801D4E1C == 0xFF) {
    drawEntryListPanel((s16)(52 * ((*row - 1) % 4) + 0x42), y, 0);
    func_801647C4((u16)((52 * ((*row - 1) % 4) + 0x44) & 0xFFFC),
                  (u16)(y + 4 + (s8)D_801D4E28 * 16), 0);
  } else {
    drawEntryListPanel((s16)(52 * ((*row - 1) % 4) + 0x42), y, 1);
    y = y + (12 + 16 * (s8)D_801D4E28);
    if (D_801D4E20 == 0xFF) {
      y = drawKindGridPanel((s16)(52 * ((*row - 1) % 4) + 0x4A), y, D_801D4E1C,
                            0);
      func_801647C4((u16)((52 * ((*row - 1) % 4) + 0x4C) & 0xFFFC),
                    (u16)(y + 4 + (s8)D_801D4E2C * 16), 0);
    } else {
      y = drawKindGridPanel((s16)(52 * ((*row - 1) % 4) + 0x4A), y, D_801D4E1C,
                            1);
      y = y + (12 + 16 * (s8)D_801D4E2C);
      y = drawKindGridPanel((s16)(52 * ((*row - 1) % 4) + 0x52), y, D_801D4E20,
                            0);
      func_801647C4((u16)((52 * ((*row - 1) % 4) + 0x54) & 0xFFFC),
                    (u16)(y + 4 + (s8)D_801D4E30 * 16), 0);
    }
  }
}
