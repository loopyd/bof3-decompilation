#include "bof3/bof3.h"

extern u8* D_1F800044;
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

typedef struct {
  s16 x;
  s16 y;
} Scena0300DrawAreaOrigin;

/* @source 0x801FCF40 @kind data */
extern s16 D_801FCF40[];
/* @source 0x801FCF42 @kind data */
extern u16 D_801FCF42[];
/* The destination draw-area origin is the record at 0x801FCF40 read through a
 * second declaration of that address. Spelling both the indexed source-rect
 * reads and these two record-0 reads through one object makes GCC materialise
 * the table base in a register, spill a slot and widen the frame from 0x20 to
 * 0x28. */
/* @source 0x801FCF40 @kind data */
extern Scena0300DrawAreaOrigin drawAreaOriginScenarioScena0300_801FCF40;

void func_8014E5A0(u8 arg0, u8 arg1);
void func_80196070(void);

/* @source 0x801F73BC
 * @behavior Ticks the countdown held in work-object byte 9 while it is nonzero.
 * At zero it draws one draw-area move packet for a 16 by 40 rect whose source
 * offsets come from the overlay-local record run at 0x801FCF40: the x is the
 * signed halfword of the record selected by unsigned work byte 1, read as a
 * quarter unit and biased by the draw-area origin 0x2C0, and the y is that
 * record's unsigned halfword at 0x801FCF42 biased by 0x100. The move packet is
 * emitted through SetDrawMove on the primitive published in g_PrimCursor, whose
 * pointer is read into a local before the rect is built, with the destination
 * draw area taken from record 0 of the same run. It then advances work byte 1,
 * clears work byte 0x29, reloads work byte 9 from the advanced byte 1, queues
 * the 0x18-byte packet through func_8014E5A0 with the just-cleared byte 0x29,
 * and clears the scratchpad work-area header through func_80196070 once work
 * byte 1 reaches 4. The work pointer is re-read from scratchpad cell 0x1F800044
 * after every store through it, so the byte and packet stores stay in the
 * original alias-sensitive order. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownThenDrawIndexedRectAndAdvanceWorkByte1ScenarioScena0300_801F73BC(void) {
  RECT rect;
  DR_MOVE* prim;

  if (D_1F800044[9] == 0) {
    prim = (DR_MOVE*)g_PrimCursor;
    rect.x = D_801FCF40[D_1F800044[1] * 2] / 4 + 0x2C0;
    rect.y = D_801FCF42[D_1F800044[1] * 2] + 0x100;
    rect.w = 0x10;
    rect.h = 0x28;
    SetDrawMove(prim, &rect, drawAreaOriginScenarioScena0300_801FCF40.x / 4 + 0x2C0,
                drawAreaOriginScenarioScena0300_801FCF40.y + 0x100);

    D_1F800044[1]++;
    D_1F800044[0x29] = 0;
    D_1F800044[9] = D_1F800044[1];
    func_8014E5A0(D_1F800044[0x29], 0x18);
    if (D_1F800044[1] == 4) {
      func_80196070();
    }
  } else {
    D_1F800044[9]--;
  }
}
