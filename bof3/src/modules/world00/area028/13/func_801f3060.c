#include "internal.h"

/* does: refreshes the current active AREA028 slots, retiring any whose scale
 * falls below `0x80`, then spawns up to two new slots into the first free
 * entries.
 * @source: 0x801f3060 FUN_801f3060
 */
void func_801f3060(void) {
  u8 scratch[0x20];
  u8 i;

  func_801afe18(scratch);
  func_8017c2d8((void*)WORLD00_AREA028_PRIMITIVE_PTR, 0, 1,
                func_8017a620(0, 1, 0x3c0, 0), 0);
  func_8014e5a0(3u, 0x0cu);

  WORLD00_AREA028_WORK_PTR =
      (World00Area028Work*)WORLD00_AREA028_WORK_BASE;
  i = 0u;
  do {
    if (WORLD00_AREA028_WORK_PTR->unk_00[0] != 0u) {
      WORLD00_AREA028_WORK_PTR->field_08 =
          (s16)(WORLD00_AREA028_WORK_PTR->field_08 - 0x20);
      if (WORLD00_AREA028_WORK_PTR->field_08 < 0x80) {
        WORLD00_AREA028_WORK_PTR->unk_00[0] = 0u;
      }
      func_801f2d3c();
    }

    WORLD00_AREA028_WORK_PTR =
        (World00Area028Work*)((u8*)WORLD00_AREA028_WORK_PTR + 0x10u);
    i += 1u;
  } while (i < 0x20u);

  WORLD00_AREA028_WORK_PTR = (World00Area028Work*)func_801f3004();
  if (WORLD00_AREA028_WORK_PTR != 0) {
    func_801f2fb0((void*)WORLD00_AREA028_WORK_PTR);
  }

  WORLD00_AREA028_WORK_PTR = (World00Area028Work*)func_801f3004();
  if (WORLD00_AREA028_WORK_PTR != 0) {
    func_801f2fb0((void*)WORLD00_AREA028_WORK_PTR);
  }
}
