#include "internal.h"

/* @behavior submits one local battler template selected by byte `0x79` through the
 * common script/event helper rooted at `0x801490d8`.
 * @source 0x801de9a8 FUN_801de9a8
 */
void func_801de9a8(u32 arg0) {
  func_801501e4(
      (void*)0x801490d8u,
      0x80144968u +
          ((u32) *
           (volatile u8*)(0x80181b10u +
                          BATTLE_LOCAL_BYTE_79(
                              &BATTLE_LOCAL_WORK_ARRAY[arg0 & 0xffu])) *
           0xa4u),
      5u);
}
