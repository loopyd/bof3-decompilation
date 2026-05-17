#include "internal.h"

/* does: emits one fixed background strip primitive with constant dimensions and
 * constant grey rgb values.
 * @source: 0x801d9900 FUN_801d9900
 */
void func_801d9900(void) {
  u32 primitive;

  func_8017c2d8(BATTLE_GLOBAL_WORD_598C, 0, 0,
                func_8017a620(0, 2, 0x3c0, 0), 0);
  func_8014e5a0(1u, 0x0cu);
  primitive = BATTLE_GLOBAL_WORD_598C;
  func_8017aa6c(primitive);
  *(u16*)(primitive + 0xc) = 0x03c0u;
  *(u16*)(primitive + 0xe) = 0x00f0u;
  *(u16*)(primitive + 8) = 0u;
  *(u16*)(primitive + 10) = 0u;
  *(u8*)(primitive + 4) = 0x28u;
  *(u8*)(primitive + 5) = 0x28u;
  *(u8*)(primitive + 6) = 0x28u;
  func_8017a904(primitive, 1);
  func_8014e5a0(1u, 0x10u);
}
