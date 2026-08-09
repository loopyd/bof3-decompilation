#include "bof3/world/area00813_internal.h"

/* @behavior resets scratch-state fields 0x5d/0x5e and advances the mode byte.
 * @source 0x801F3AA8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F3AA8(void)
{
  ((World00Area008Scratch*)g_areaWork)->field_5d = 0;
  ((World00Area008Scratch*)g_areaWork)->field_5e = 0x1E;
  g_areaWork->mode += 1;
}
