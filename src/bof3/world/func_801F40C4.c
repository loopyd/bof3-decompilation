#include "bof3/world/area01613_internal.h"

/* @behavior Calls two target-local marker helpers and one shared primitive
 * helper when the shared display-enable mask is active.
 * @source 0x801F40C4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F40C4(s16 arg0, s16 arg1) {
  s16 y;

  if ((D_8014832E & 0x1bu) != 0u) {
    y = arg1;
    emitSemiTransparentSprite(arg0, y, 4u);
    emitSemiTransparentSprite((s16)(arg0 + 0x80), y, 5u);
    func_8014F800((s16)(arg0 + 4), (s16)(arg1 + 4), 0, 0xffu,
                  0x80010000u + (u32)D_80010008);
  }
}
