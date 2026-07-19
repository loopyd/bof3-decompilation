#include "internal.h"

/* @behavior derives queued-slot position offsets from either the local or enemy
 * source tables and writes them into the current queued object.
 * @source 0x801E8DD8
 */
void func_801E8DD8(void) {
  volatile u8* slot;
  s8           offset_x;
  u8           offset_y;

  slot = BATTLE_CURRENT_QUEUED_PTR_4B20;
  if (slot[5] < 3u) {
    offset_x = BATTLE_OFFSET_TABLE_0B10(
        slot[8], BATTLE_LOCAL_BYTE_79(&BATTLE_LOCAL_WORK_ARRAY[slot[5]]));
    *(volatile u16*)(BATTLE_LOCAL_SCRATCH_PTR + 0x2e) =
        *(volatile u16*)(slot + 0x2e) + offset_x;
    if (slot[8] < 2u) {
      offset_y = BATTLE_ENEMY_OFFSET_TABLE_0B08(
          BATTLE_LOCAL_BYTE_79(&BATTLE_LOCAL_WORK_ARRAY[slot[5]]));
    } else {
      offset_y = BATTLE_ENEMY_OFFSET_TABLE_0B09(
          BATTLE_LOCAL_BYTE_79(&BATTLE_LOCAL_WORK_ARRAY[slot[5]]));
    }
    *(volatile u16*)(BATTLE_LOCAL_SCRATCH_PTR + 0x30) =
        *(volatile u16*)(slot + 0x30) - offset_y - 8u;
    *(volatile u16*)(BATTLE_LOCAL_SCRATCH_PTR + 0x32) =
        *(volatile u16*)(slot + 0x32);
  } else {
    offset_x = BATTLE_ENEMY_OFFSET_S8_0B12((u32)slot[5] - 3u);
    *(volatile u16*)(BATTLE_LOCAL_SCRATCH_PTR + 0x2e) =
        *(volatile u16*)(slot + 0x2e) + offset_x;
    offset_y = BATTLE_CLASS_OFFSET_0C0CB(
        BATTLE_ENEMY_OFFSET_U8_0B10((u32)slot[5] - 3u));
    *(volatile u16*)(BATTLE_LOCAL_SCRATCH_PTR + 0x30) =
        *(volatile u16*)(slot + 0x30) - offset_y - 8u;
    *(volatile u16*)(BATTLE_LOCAL_SCRATCH_PTR + 0x32) =
        *(volatile u16*)(slot + 0x32);
  }
}
