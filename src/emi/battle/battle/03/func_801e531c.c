#include "internal.h"

/* @behavior consumes up to two pending halfword events from the current enemy work,
 * accumulating counts in the shared tables at `0x80146334/0x80146354`.
 * @source 0x801e531c FUN_801e531c
 */
void func_801e531c(void) {
  const u8* threshold_table;
  u8        slot;

  threshold_table = BATTLE_EVENT_PICK_TABLE_0CB8;
  slot = 0u;
  while (slot < 2u) {
    u16 value;
    u8  choice;
    u8  index;

    value = *(volatile u16*)((volatile u8*)BATTLE_CURRENT_ENEMY_PTR + 0x98u +
                             ((u32)slot * 4u));
    choice = *(volatile u8*)((volatile u8*)BATTLE_CURRENT_ENEMY_PTR + 0x9au +
                             ((u32)slot * 4u));
    if ((value != 0u) && (choice != 0u) &&
        ((func_8017e3d4() & 0xffu) <= threshold_table[choice])) {
      index = 0u;
      while (index < BATTLE_GLOBAL_BYTE_6327) {
        if (BATTLE_GLOBAL_HALF_6334(index) == value) {
          BATTLE_GLOBAL_BYTE_6354(index) += 1u;
          *(volatile u16*)((volatile u8*)BATTLE_CURRENT_ENEMY_PTR + 0x98u +
                           ((u32)slot * 4u)) = 0u;
          break;
        }
        index += 1u;
      }
      if (index == BATTLE_GLOBAL_BYTE_6327) {
        BATTLE_GLOBAL_HALF_6334(BATTLE_GLOBAL_BYTE_6327) = value;
        BATTLE_GLOBAL_BYTE_6354(BATTLE_GLOBAL_BYTE_6327) = 1u;
        if (BATTLE_GLOBAL_BYTE_6327 < 0x0fu) {
          BATTLE_GLOBAL_BYTE_6327 += 1u;
        }
        *(volatile u16*)((volatile u8*)BATTLE_CURRENT_ENEMY_PTR + 0x98u +
                         ((u32)slot * 4u)) = 0u;
      }
    }
    slot += 1u;
  }
}
