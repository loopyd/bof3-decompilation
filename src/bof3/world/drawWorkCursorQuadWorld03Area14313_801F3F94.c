#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */
extern void func_801F3FDC(s32 *point); /* @source 0x801F3FDC @kind function */

/* @source 0x801F3F94
 * @behavior Copies the current work object's three cursor words at offsets
 * 0x34, 0x38 and 0x3C into a local point and passes it to the overlay renderer
 * at 0x801F3FDC. That renderer projects the point and emits translucent Gouraud
 * quads. The work pointer comes from scratchpad cell 0x1F800044; the handler
 * takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkCursorQuadWorld03Area14313_801F3F94(void) {
  s32 point[3];
  u8 *work;

  work = D_1F800044;
  point[0] = *(s32 *)(work + 0x34);
  point[1] = *(s32 *)(work + 0x38);
  point[2] = *(s32 *)(work + 0x3C);
  func_801F3FDC(point);
}
