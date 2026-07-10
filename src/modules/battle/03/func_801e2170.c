#include "internal.h"

/* does: walks the eight enemy work records, makes each active one current, and
 * dispatches through one of two enemy-side handler tables.
 * @source: 0x801e2170 FUN_801e2170
 */
void func_801e2170(void) {
  Battle03Handler const volatile* table_b =
      (Battle03Handler const volatile*)(0x801f0000u - 0x4d68u);
  Battle03Handler const volatile* table_a =
      (Battle03Handler const volatile*)(0x801f0000u - 0x4d6cu);
  u8  index = 0u;
  u32 base;

  base = 0x801f0000u;
  do {
    volatile Battle03EnemyWork* battle_work;
    Battle03Handler             handler;
    u32                         offset;

    offset = (u32)index * 0x118u;
    if (*(volatile u8*)((0x801f0000u - 0x49d0u) + offset) != 0u) {
      battle_work = (volatile Battle03EnemyWork*)((base - 0x49d0u) + offset);
      *(volatile Battle03EnemyWork**)0x1f800044u = battle_work;
      *(volatile Battle03EnemyWork**)0x801eb4e8u = battle_work;
      if (BATTLE_GLOBAL_BYTE_62EA == 0u) {
        handler = table_a[battle_work->unk_f0];
      } else {
        if (battle_work->unk_01 != 0u) {
          battle_work->unk_e4(2);
        }
        handler =
            table_b[(*(volatile Battle03EnemyWork**)(base - 0x4b18u))->unk_f0];
      }
      handler();
    }
    index += 1u;
  } while (index < 8u);
}
