#include "internal.h"

/* does: derives queued-slot position offsets from either the local or enemy
 * source tables and writes them into the current queued object.
 * @source: 0x801e8dd8 FUN_801e8dd8
 */
void func_801e8dd8(void) {
  volatile u8* slot;
  s8           offset_x;
  u8           offset_y;

  slot = BOF3_BATTLE_CURRENT_QUEUED_PTR_4B20;
  if (slot[5] < 3u) {
    offset_x = *(volatile s8*)(0x801eb0b0u + (slot[8] * 2u) +
                               (BOF3_BATTLE_LOCAL_BYTE_79(
                                    &BOF3_BATTLE_LOCAL_WORK_ARRAY[slot[5]]) *
                                8u));
    *(volatile u16*)(BOF3_BATTLE_LOCAL_SCRATCH_PTR + 0x2e) =
        *(volatile u16*)(slot + 0x2e) + offset_x;
    if (slot[8] < 2u) {
      offset_y = *(volatile u8*)(0x801eb108u +
                                 (BOF3_BATTLE_LOCAL_BYTE_79(
                                      &BOF3_BATTLE_LOCAL_WORK_ARRAY[slot[5]]) *
                                  2u));
    } else {
      offset_y = *(volatile u8*)(0x801eb109u +
                                 (BOF3_BATTLE_LOCAL_BYTE_79(
                                      &BOF3_BATTLE_LOCAL_WORK_ARRAY[slot[5]]) *
                                  2u));
    }
    *(volatile u16*)(BOF3_BATTLE_LOCAL_SCRATCH_PTR + 0x30) =
        *(volatile u16*)(slot + 0x30) - offset_y - 8u;
    *(volatile u16*)(BOF3_BATTLE_LOCAL_SCRATCH_PTR + 0x32) =
        *(volatile u16*)(slot + 0x32);
  } else {
    offset_x = *(volatile s8*)(0x801eb712u + (((u32)slot[5] - 3u) * 0x118u));
    *(volatile u16*)(BOF3_BATTLE_LOCAL_SCRATCH_PTR + 0x2e) =
        *(volatile u16*)(slot + 0x2e) + offset_x;
    offset_y =
        *(volatile u8*)(0x800e40cbu +
                        (*(volatile u8*)(0x801eb710u +
                                         (((u32)slot[5] - 3u) * 0x118u)) *
                         0x88u));
    *(volatile u16*)(BOF3_BATTLE_LOCAL_SCRATCH_PTR + 0x30) =
        *(volatile u16*)(slot + 0x30) - offset_y - 8u;
    *(volatile u16*)(BOF3_BATTLE_LOCAL_SCRATCH_PTR + 0x32) =
        *(volatile u16*)(slot + 0x32);
  }
}
