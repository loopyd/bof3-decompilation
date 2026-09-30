#include "bof3/world/area00813_internal.h"

/* @source 0x801F3AE8
 * @behavior draws both local status frames, decrements the scratch countdown
 *           byte 0x5d and restarts it at 0x1D when it goes negative, then
 *           decrements the scratch countdown byte 0x5e and, when that also
 *           goes negative, clears both countdowns and advances the area mode.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3AE8(void) {
  drawScratchStatus();
  drawFlagStatus();
  ((World00Area008Scratch*)g_areaWork)->field_5d -= 1;
  if (((World00Area008Scratch*)g_areaWork)->field_5d < 0) {
    ((World00Area008Scratch*)g_areaWork)->field_5d = 0x1D;
    ((World00Area008Scratch*)g_areaWork)->field_5e -= 1;
  }
  if (((World00Area008Scratch*)g_areaWork)->field_5e < 0) {
    ((World00Area008Scratch*)g_areaWork)->field_5d = 0;
    ((World00Area008Scratch*)g_areaWork)->field_5e = 0;
    g_areaWork->mode += 1;
  }
}
