#include "internal.h"

/* does: advances the local scratch vertical offset and routes the next AREA030
 * panel draw through the later helper at `0x801d9534`.
 * @source: 0x801d3938 FUN_801d3938
 */
void func_801d3938(void) {
  volatile u8* scratch;
  s16          y;

  scratch = BOF3_WORLD00_AREA030_SCRATCH_PTR;

  *(volatile s16*)(scratch + 0x30u) =
      (s16)(*(volatile s16*)(scratch + 0x30u) - 8);
  if (*(volatile s16*)(scratch + 0x30u) < -0x19) {
    if (scratch[7] == 0u) {
      scratch[1] = 0u;
    } else {
      scratch[1] = 1u;
    }
  }

  y = *(volatile s16*)(scratch + 0x30u);
  func_801d9534(0x14, y, 0x118, 0x13, 0);
}
