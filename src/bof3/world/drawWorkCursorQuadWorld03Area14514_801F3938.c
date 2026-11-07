#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so the load is
 * one lui plus offset load on the shared cell, which is how the original
 * materialises the pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

void func_801F3980(void *point);

/* @source 0x801F3938
 * @behavior Overlay per-frame step handler for the trail quad: it loads the
 * work-object pointer published at the scratchpad cell 0x1F800044 and copies
 * that object's three cursor words at +0x34, +0x38 and +0x3C into a local
 * twelve-byte point, then passes the address of that point to the overlay's
 * quad renderer at 0x801F3980. The cell pointer is loaded before the stack
 * frame because nothing between it and the local stores needs the frame, the
 * two constant seeds func_801F38E0 writes into the same words and the shared
 * work-object layout both place the cursor triple at +0x34, and the renderer
 * reads the three words back as a position plus depth, projecting them through
 * the shared 3D-to-screen projector func_801AFF04 before it emits a
 * semi-transparent G4 through SetPolyG4. The copy is local because the callee
 * receives the address of this frame slot, not the work-object field. It takes
 * no arguments, returns nothing and reads no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkCursorQuadWorld03Area14514_801F3938(void) {
  s32 point[3];
  u8 *work;

  work = D_1F800044;
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  func_801F3980(point);
}
