#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u16 D_801DB0C8; /* @source 0x801DB0C8 @kind data */
extern u8 D_80144952;  /* @source 0x80144952 @kind unknown */

/* @source 0x801D8A40 */
extern void func_801D8A40(s32 arg0, s32 arg1, s32 arg2, s32 arg3, u8 arg4);

/* @source 0x801D812C
 * @behavior draws the 0x118 by 0x13 panel strip whose origin is (0x14, 0x10)
 *           through func_801D8A40 with the main-RAM CLUT-bank byte D_80144952
 *           as the fifth argument; unless the overlay image selector at
 *           0x801DB0C8 holds the 0xFFFF "no image" sentinel it then submits the
 *           texture whose handle is the 0x80010000 main-RAM base plus the
 *           0x80010000 offset-table entry that selector names, at (0x1B, 0x13)
 *           with colour 0 and flag 0xFF through func_8014F800. Both handlers
 *           that reach this function publish 0xF0 or 0xF1 into the selector
 *           before drawing. No state is written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawPanelStripThenSubmitWorkImage(void) {
  u16 image;

  func_801D8A40(0x14, 0x10, 0x118, 0x13, D_80144952);
  image = D_801DB0C8;
  if (image != 0xFFFF) {
    func_8014F800(0x1B, 0x13, 0, 0xFF,
                0x80010000u + PSX_PTR(u16, 0x80010000u)[image]);
  }
}
