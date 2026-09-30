#include "bof3/boot/logo_internal.h"

/*
 * @source 0x801CE7F4
 * @behavior resets the console and video mode, builds both 320x240 display
 * environments of the LOGO double buffer as RGB24 surfaces, clears the 960x480
 * work area, then publishes the second environment and enables display output.
 * @status exact
 * @match 100.00
 * @residual none
 * live audit is instruction- and byte-exact.
 */
void func_801CE7F4(void) {
  RECT rect;

  ResetGraph(0);
  SetDispMask(0);
  SetVideoMode(MODE_NTSC);
  SetDefDispEnv(&D_801EB480[0], 0, 0, 0x140, 0xF0);
  SetDefDispEnv(&D_801EB480[1], 0, 0xF0, 0x140, 0xF0);
  D_801EB480[1].isrgb24 = 1;
  D_801EB480[0].isrgb24 = 1;
  rect.w = 0x3C0;
  D_801EB4A8 = 0;
  rect.x = 0;
  rect.y = 0;
  rect.h = 0x1E0;
  ClearImage(&rect, 0, 0, 0);
  DrawSync(0);
  VSync(0);
  PutDispEnv(&D_801EB480[1]);
  SetDispMask(1);
}
