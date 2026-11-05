#include "bof3/bof3.h"

extern void func_801D875C(s16 x, s16 y, u8 piece);

/* @source 0x801D81A4
 * @behavior draws the small window frame whose top-left corner sits at (x, y)
 * from the same eight window tiles that occupy pieces 6 through 13 of the
 * six-byte sprite rect table at 0x801DAE88: top-left, top-right, bottom-left
 * and bottom-right corners, seven repeated top and bottom edge pieces on an
 * eight-pixel horizontal step, and four repeated side pieces on a six-pixel
 * vertical step, for a frame that spans 0x40 by 0x22. Every coordinate is
 * truncated to sixteen bits per primitive. No other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSmallWindowFrame(s16 x, s16 y) {
  s32 i;
  s32 offset;

  func_801D875C((s16)x, (s16)y, 6);
  for (i = 0; i < 7; i++) {
    offset = i * 8 + 8;
    func_801D875C((s16)(x + offset), (s16)y, 7);
    func_801D875C((s16)(x + offset), (s16)(y + 0x22), 12);
  }
  func_801D875C((s16)(x + 0x40), (s16)y, 8);
  for (i = 0; i < 4; i++) {
    offset = i * 6 + 8;
    func_801D875C((s16)x, (s16)(y + offset), 9);
    func_801D875C((s16)(x + 0x40), (s16)(y + offset), 10);
  }
  func_801D875C((s16)x, (s16)(y + 0x22), 11);
  func_801D875C((s16)(x + 0x40), (s16)(y + 0x22), 13);
}
