#include "bof3/bof3.h"

extern u8 D_80143F03; /* @source 0x80143F03 @kind data */
extern u8 D_80144EA0; /* @source 0x80144EA0 @kind unknown */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F30D0
 * @behavior While the shared front-side gate byte D_80143F03 reads 1, probes
 * two flags of the main-RAM flag bank at 0x80144EA0 through the shared flag
 * tester func_8015B5D4: flag 0x1C set together with flag 0x1D clear queues the
 * shared UI element at (0x23, 0x4F) with size 0x51, and every other
 * combination of the pair queues the same element with size 0x50. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElement23x4FSize50Or51OnFlagPair1C1DWorld02Area09913_801F30D0(void) {
  if (D_80143F03 != 1) {
    return;
  }
  if (func_8015B5D4((u32)&D_80144EA0, 0x1c) != 0) {
    if (func_8015B5D4((u32)&D_80144EA0, 0x1d) == 0) {
      func_801A4BC0(0x23, 0x4f, 0x51);
      return;
    }
  }
  func_801A4BC0(0x23, 0x4f, 0x50);
}
