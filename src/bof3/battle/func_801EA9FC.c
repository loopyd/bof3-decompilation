#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA9FC
 * @behavior Reads the panel state halfword +0x06 of the state object published at
 * 0x80148648: when it reads -0x16 it clears the D_801EC2E4 byte and panel state
 * byte +0x03, otherwise it rewrites that halfword 8 lower. It then forwards the
 * state halfwords +0x04/+0x06 to func_801D7EB0 and forwards those same halfwords
 * offset by +4/+3 as signed halfwords, the constant 0, the constant 0xFF, and the
 * +0x04 word of the 0x08-stride UI ring entry selected by the consumer index
 * uiRingHead - 1 to func_8014F800.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801EA9FC(void) {
  s16 state;

  state = *(s16*)&D_80148648[6];
  if (state == -0x16) {
    D_801EC2E4 = 0;
    D_80148648[3] = 0;
  } else {
    *(s16*)&D_80148648[6] = state - 8;
  }
  func_801D7EB0(*(s16*)&D_80148648[4], *(s16*)&D_80148648[6]);
  func_8014F800((s16)(*(u16*)&D_80148648[4] + 4u),
                (s16)(*(u16*)&D_80148648[6] + 3u), 0, 0xffu,
                uiRingEntries[(uiRingHead - 1u) & 0xFu].unk_04);
}
