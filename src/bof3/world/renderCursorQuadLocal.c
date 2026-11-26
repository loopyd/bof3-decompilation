#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

void renderCursorQuadPrim(s32 *point); /* @source 0x801F2D60 @kind function */

/* @source 0x801F2D18
 * @behavior Overlay step handler for state 1 of the local dispatch table based
 * at 0x801F407C: it loads the work-object pointer published at the scratchpad
 * cell 0x1F800044 and copies that object's three cursor words at +0x34, +0x38
 * and +0x3C into a local twelve-byte point, then passes the address of that
 * point to the overlay quad renderer at 0x801F2D60, renderCursorQuadPrim. The cell
 * pointer is loaded before the stack frame because nothing between it and the
 * local stores needs the frame, and the callee emits one semi-transparent G4
 * through SetPolyG4 after projecting the three words as a position, so the
 * copy is local: the callee receives the address of this frame slot, not the
 * work-object field. It takes no arguments, returns nothing and reads no other
 * state. Name evidence: the exact-shape sibling
 * drawWorkCursorQuadWorld03Area14514_801F3938 at 0x801F3938 differs from this
 * span in the jal immediate alone, each word shifting by exactly the delta
 * between the two callees (0xC20), so its statement order, operand forms and
 * declaration style are transcribed and this body's own callee and payload
 * address are written in.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderCursorQuadLocal(void) {
  s32 point[3];
  u8 *work;

  work = D_1F800044;
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  renderCursorQuadPrim(point);
}
