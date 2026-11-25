#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

void func_801F3D70(s32 *point); /* @source 0x801F3D70 @kind function */

/* @source 0x801F3D28
 * @behavior Overlay step handler: entry 1 of the handler table based at
 * 0x801F4F68 that this target's dispatcher
 * callStateTable4World02 indexes with the work
 * record's state byte at +1, whose entry 0 is
 * stateScaledDistanceAdvance.
 * It loads the work-object pointer published at the scratchpad cell 0x1F800044
 * and copies that object's three cursor words at +0x34, +0x38 and +0x3C into a
 * local twelve-byte point, then passes the address of that point to this
 * overlay's quad renderer func_801F3D70, which projects the point through the
 * shared 3D-to-screen projector func_801AFF04 and emits one semi-transparent G4
 * through SetPolyG4. The cell pointer is loaded before the stack frame because
 * nothing between it and the local stores needs the frame, and the copy is
 * local: the callee receives the address of this frame slot, not the
 * work-object field. It takes no arguments, returns nothing and reads no other
 * state.
 * Shape evidence: the sibling renderCursorQuadLocal in
 * this same payload differed from this span in one word only, its own jal
 * immediate at 0x801F3D58, which shifts by exactly the delta between the two
 * callees (0x0C07CF5C - 0x0C07CB58 = 0x404 = (0x801F3D70 - 0x801F2D60) >> 2),
 * so its statement order, operand forms and declaration style are transcribed
 * here and this body's own callee is written in. The same twelve-byte copy
 * shape carries the family names drawWorkCursorQuadWorld02Area11213_801F3EA4,
 * drawWorkCursorQuadWorld03Area14313_801F3F94 and
 * drawWorkCursorQuadWorld03Area14514_801F3938.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderProjectedCursorQuad(void) {
  s32 point[3];
  u8 *work;

  work = D_1F800044;
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  func_801F3D70(point);
}
