#include "bof3/world/area03213_internal.h"

/* @source 0x801F2C48
 * @behavior Projects the camera position and azimuth at 0x80145EC4, 0x80145EC8
 * and 0x80145ECE into the scratchpad vertex at 0x1F800014, then stores the
 * projected depth and screen pair into the scratch record at 0x1F800044,
 * clearing its work byte 9 and advancing its state byte 1.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 52/52 instructions, 208 -> 208 bytes, exact after one retention
 * step. The statement order is measured, not stylistic: only with the projected
 * screen Y held in a register local and the cursor cell re-read before the 0x30
 * store does the original reload the cursor into $a0 with the 0x30 store in its
 * load-delay slot and the screen-X load delay left unfilled. The straightforward
 * order emitted the reload in $v1 with a trailing nop (96.15% before that step).
 */
void func_801F2C48(void) {
  SVECTOR* vtx;
  long*    sxy;
  long     depth;
  long     flag;
  s32      otz;
  u16      screen_y;
  u8*      record;
  u8*      cursor;

  vtx = (SVECTOR*)&D_1F800014;
  sxy = (long*)&D_1F800034;

  vtx->vx = (s16)(D_80145EC4 >> 9) - 0x4000;
  D_1F800016 = (s16)(D_80145EC8 >> 9) - 0x4000;
  D_1F800018 = (s16)(-D_80145ECE / 2);

  otz = RotTransPers(vtx, sxy, &depth, &flag);
  record = D_1F800044;
  *(s32*)(record + 0x60) = otz;
  *(u16*)(record + 0x2e) = *(u16*)sxy;
  screen_y = D_1F800036;
  *(u8*)(record + 0x09) = 0;
  cursor = D_1F800044;
  *(u16*)(record + 0x30) = screen_y;
  cursor[1]++;
}
