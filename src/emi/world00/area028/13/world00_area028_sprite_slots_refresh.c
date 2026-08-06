#include "internal.h"

/* @behavior refreshes the current active AREA028 slots, retiring any whose scale
 * falls below `0x80`, then spawns up to two new slots into the first free
 * entries.
 * @source 0x801F3060
 */
void world00_area028_sprite_slots_refresh(void) {
  u8 scratch[0x20];
  u8 i;

  func_801AFE18(scratch);
  SetDrawMode((DR_MODE*)WORLD00_AREA028_PRIMITIVE_PTR, 0, 1,
              GetTPage(0, 1, 0x3c0, 0), 0);
  func_8014E5A0(3u, 0x0cu);

  WORLD00_AREA028_WORK_PTR = (World00Area028Work*)WORLD00_AREA028_WORK_BASE;
  i = 0u;
  do {
    if (WORLD00_AREA028_WORK_PTR->unk_00[0] != 0u) {
      WORLD00_AREA028_WORK_PTR->field_08 =
          (s16)(WORLD00_AREA028_WORK_PTR->field_08 - 0x20);
      if (WORLD00_AREA028_WORK_PTR->field_08 < 0x80) {
        WORLD00_AREA028_WORK_PTR->unk_00[0] = 0u;
      }
      func_801F2D3C();
    }

    WORLD00_AREA028_WORK_PTR =
        (World00Area028Work*)((u8*)WORLD00_AREA028_WORK_PTR + 0x10u);
    i += 1u;
  } while (i < 0x20u);

  WORLD00_AREA028_WORK_PTR = (World00Area028Work*)world00_area028_free_slot_scan();
  if (WORLD00_AREA028_WORK_PTR != 0) {
    world00_area028_sprite_slot_init((void*)WORLD00_AREA028_WORK_PTR);
  }

  WORLD00_AREA028_WORK_PTR = (World00Area028Work*)world00_area028_free_slot_scan();
  if (WORLD00_AREA028_WORK_PTR != 0) {
    world00_area028_sprite_slot_init((void*)WORLD00_AREA028_WORK_PTR);
  }
}
