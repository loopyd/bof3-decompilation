#include "internal.h"

/* @behavior advances the local scratch vertical offset and routes the next AREA030
 * panel draw through the later helper at `0x801d9534`.
 * @source 0x801D3938
 */
void func_801D3938(void) {
  u8*  scratch;
  u16  y;

  scratch = (u8*)WORLD00_AREA030_SCRATCH_PTR;

  y = *(u16*)(scratch + 0x30u) - 8u;
  *(u16*)(scratch + 0x30u) = y;
  if ((s16)y < -0x19) {
    if (scratch[7] != 0u) {
      scratch[1] = 1u;
    } else {
      scratch[1] = 0u;
    }
  }

  func_801D9534(0x14, *(volatile u16*)(WORLD00_AREA030_SCRATCH_PTR + 0x30u),
                0x118, 0x13, 0);
}
