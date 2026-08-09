#include "bof3/battle/battle03_internal.h"

/* Non-volatile view of the countdown byte: the original never reloads it
 * after the decrement store, which a volatile pointee would forbid. */
#define BATTLE_GLOBAL_BYTE_63CE_NV (*(u8*)&BATTLE_GLOBAL_BYTE_63CE)

/* @behavior advances several battler-local countdown bytes and, when the global
 * suppression countdown expires, clears the shared `0x10` flag across all
 * currently available battlers.
 * @source 0x801D4D44
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D4D44(void) {
  u8  index;
  u8* local_work;
  u8* enemy_work;

  index = 0u;
  local_work = (u8*)D_80145E90;
  do {
    if (func_801D64C4(index) == 0u) {
      if (((D_80145E90[index].unk_128 & 1u) != 0u) &&
          (D_80145E90[index].unk_136 < 6u)) {
        u8 next;

        next = D_80145E90[index].unk_136 + 1u;
        local_work[(u32)index * 0x140u + 0x136u] = next;
      }
      if (((D_80145E90[index].unk_80 & 0x0800u) != 0u) &&
          (D_80145E90[index].unk_137 < 6u)) {
        u8 next;

        next = D_80145E90[index].unk_137 + 1u;
        local_work[(u32)index * 0x140u + 0x137u] = next;
      }
      if (((D_80145E90[index].unk_128 & 0x4000u) != 0u) &&
          (D_80145E90[index].unk_136 < 3u)) {
        u8 next;

        next = D_80145E90[index].unk_136 + 1u;
        local_work[(u32)index * 0x140u + 0x136u] = next;
      }
    }
    index += 1u;
  } while (index < 3u);

  index = 3u;
  enemy_work = (u8*)D_801EB630;
  do {
    if (func_801D64C4(index) == 0u) {
      u32 enemy_index;

      enemy_index = index - 3u;
      if (((D_801EB630[enemy_index].unk_104 & 0x4000u) != 0u) &&
          (D_801EB630[enemy_index].unk_112 < 3u)) {
        u8 next;

        next = D_801EB630[enemy_index].unk_112 + 1u;
        enemy_work[enemy_index * 0x118u + 0x112u] = next;
      }
    }
    index += 1u;
  } while (index < 0x0bu);

  if (BATTLE_GLOBAL_BYTE_63CE_NV != 0u) {
    BATTLE_GLOBAL_BYTE_63CE_NV -= 1u;
    if (BATTLE_GLOBAL_BYTE_63CE_NV == 0u) {
      index = 0u;
      do {
        if (func_801DB524(index) == 0u) {
          D_80145E90[index].unk_128 &= 0xffffffefu;
        }
        index += 1u;
      } while (index < 3u);

      index = 3u;
      do {
        if (func_801DB524(index) == 0u) {
          D_801EB630[index - 3u].unk_104 &= 0xffffffefu;
        }
        index += 1u;
      } while (index < 0x0bu);
    }
  }
}
