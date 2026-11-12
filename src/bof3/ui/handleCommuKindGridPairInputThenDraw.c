#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Shared pressed-button masks paired with the pad-state word: 0x80145AC2 is the
 * confirm mask and 0x80145AC4 the cancel mask in this family's pad tests. */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */

/* Shared commu list cursor: the list row this overlay's handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Result halfword of the current commu request. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* Kind bytes of the two kind grids this step stacks under the entry list, and
 * the kind byte the cancel path clears; 0xFF marks a kind with no grid. */
extern u8 D_801D4E1C; /* @source 0x801D4E1C @kind data */
extern u8 D_801D4E20; /* @source 0x801D4E20 @kind data */

/* Row offsets that place each kind grid sixteen pixels per unit below the panel
 * above it; both are read as signed bytes. */
extern s8 D_801D4E28; /* @source 0x801D4E28 @kind data */
extern s8 D_801D4E2C; /* @source 0x801D4E2C @kind data */

/* Cursor box row byte, toggled between 0 and 1 by the pad direction bits and
 * scaled by sixteen pixels; read as a signed byte. */
extern s8 D_801D4E30; /* @source 0x801D4E30 @kind data */

/* Commu request step byte the cancel path rolls back by one. */
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
/* The marker emitter narrows only its y argument with a signed halfword, so x
 * is declared wide here. */
extern void drawMarkerTriangleCommu0107_801D3984(s32 x, s16 y, u8 selected);
extern s16 computeEntryListOriginY(void);
extern void drawEntryListPanel(s16 x, s16 y, u8 mode);
extern s16 drawKindGridPanel(s16 x, s16 y, u8 kind, u8 mode);
extern void func_801647C4(u16 x, u16 y, s32 mode);
extern void saveCommuRequestStateAndSelectStep5(void);

/* @source 0x801D1E48
 * @behavior Input and redraw step of the commu01/07 commu request flow, one of
 * the four in-image callers of drawEntryListPanel. Reads the shared pad-state
 * word 0x80145AA8 once and dispatches on its masks. While that word shares a bit
 * with the cancel mask 0x80145AC4 it queues resident sound cue 0x106, reports
 * result code 0x45 through the request result halfword 0x801D4E0C, clears the
 * second kind grid byte 0x801D4E20 to 0xFF and rolls the commu request step byte
 * 0x801F294D back by one. While it shares a bit with the confirm mask 0x80145AC2
 * it saves the request state and selects overlay step 5. Otherwise it decodes
 * the 0x5000 bits of the pad word through the shared decoder at 0x8014E640 and,
 * when one of those bits comes back, queues resident cue 0x100 and toggles the
 * cursor box row byte 0x801D4E30 between 0 and 1. Every path then runs
 * drawCommuEntryListScreen, draws the list row marker of 0x801448ED with selected flag 1,
 * computes the slot origin of that row kind at ordinal 0 through
 * computeKindSlotOrigin, draws the marker triangle at that origin with selected
 * flag 1, and stacks three panels whose x is 52 times the quarter index of the
 * list row plus a fixed offset: the entry list panel in mode 1 at 0x42 on the
 * origin computeEntryListOriginY returns, the kind grid of 0x801D4E1C in mode 1
 * at 0x4A twelve pixels below that panel plus sixteen per unit of 0x801D4E28,
 * and while 0x801D4E20 is not 0xFF the kind grid of that byte in mode 0 at 0x52
 * twelve pixels below the first grid plus sixteen per unit of 0x801D4E2C, each
 * panel taking the y the previous call returned. It then submits the cursor box
 * through func_801647C4 at 0x54 rounded down to a multiple of four on x and at
 * the last returned bottom plus four plus sixteen per unit of 0x801D4E30 on y.
 * No state other than the result halfword, the two kind bytes, the cursor row
 * byte, the request step byte and the callee cells is written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void handleCommuKindGridPairInputThenDraw(void) {
  u16 pad;
  u8 *row;
  s32 direction;
  s16 x;
  s16 y;

  pad = D_80145AA8;
  if (pad & D_80145AC4) {
    dispatchSoundCue(0x106);
    D_801D4E0C = 0x45;
    D_801D4E20 = 0xFF;
    D_801F294D = D_801F294D - 1;
  } else if (pad & D_80145AC2) {
    saveCommuRequestStateAndSelectStep5();
  } else {
    direction = func_8014E640(pad & 0x5000);
    if (direction & 0x5000) {
      dispatchSoundCue(0x100);
      D_801D4E30 = D_801D4E30 ^ 1;
    }
  }
  drawCommuEntryListScreen();
  row = &D_801448ED;
  drawRowMarkerOutlineCommu0107_801D36FC(*row, 1);
  computeKindSlotOrigin(&x, &y, *row, 0);
  drawMarkerTriangleCommu0107_801D3984(x, y, 1);
  y = computeEntryListOriginY();
  drawEntryListPanel((s16)(52 * ((*row - 1) % 4) + 0x42), y, 1);
  y = y + (12 + 16 * D_801D4E28);
  y = drawKindGridPanel((s16)(52 * ((*row - 1) % 4) + 0x4A), y, D_801D4E1C, 1);
  if (D_801D4E20 != 0xFF) {
    y = y + (12 + 16 * D_801D4E2C);
    y = drawKindGridPanel((s16)(52 * ((*row - 1) % 4) + 0x52), y, D_801D4E20, 0);
    func_801647C4((u16)((52 * ((*row - 1) % 4) + 0x54) & 0xFFFC),
                  (u16)(y + 4 + D_801D4E30 * 16), 0);
  }
}
