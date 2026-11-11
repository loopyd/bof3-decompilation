#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

void func_801F3EEC(s32 *point); /* @source 0x801F3EEC @kind function */

/* @source 0x801F3EA4
 * @behavior Overlay per-frame step handler for the trail quad, reached as entry
 *           1 of the state-byte-1 handler table based at 0x801F493C: entry 0,
 *           func_801F3E58, projects the cursor pair through func_8015477C,
 *           stores the result into the work object's +0x3C word and advances
 *           byte +1, which is this handler. It loads the work-object pointer
 *           published at the scratchpad cell 0x1F800044 and copies that
 *           object's three cursor words at +0x34, +0x38 and +0x3C into a local
 *           twelve-byte point, then passes the address of that point to this
 *           overlay's quad renderer func_801F3EEC, which projects the point
 *           through the shared 3D-to-screen projector func_801AFF04 and emits a
 *           semi-transparent G4 through SetPolyG4. The cell pointer is loaded
 *           before the stack frame because nothing between it and the local
 *           stores needs the frame, and the copy is local because the callee
 *           receives the address of this frame slot, not the work-object field.
 *           It takes no arguments, returns nothing and reads no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkCursorQuadWorld02Area11213_801F3EA4(void) {
  s32 point[3];
  u8 *work;

  work = D_1F800044;
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  func_801F3EEC(point);
}
