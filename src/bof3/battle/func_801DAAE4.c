#include "bof3/battle/battle03_internal.h"

extern int rand(void);
/* @behavior builds and sorts the mixed local/enemy ranking list used by the owner
 * selection bytes at `0x8014630c`, then resets the current owner cursor.
 * @source 0x801DAAE4
 * @status partial
 * @match 41.26
 * @residual 144/349 instructions, 1396 original bytes versus 1296. The ranking
 * list is plain storage, so the sort swaps encode as the original's inline
 * `lwl`/`lwr`/`swl`/`swr` word moves and the value reload is a single `lh`. The
 * local arm keeps the original's four ordered value stores, the product is
 * truncated to 16 bits before the divide as the original does, and both fill arms
 * store the slot index before the value. The three late phase-2 record reads reach
 * the record through the flat byte-record symbols `D_80145FA9`/`D_80145F0A` and
 * `D_80145F28` indexed by `index*0x140`, which is the original's per-use
 * `lui $at,%hi` + `addu at,at,s0` + `%lo` load instead of one shared record base.
 * `D_801EAF3C`/`D_801EAF48` are bound as their own symbols, so the percent read
 * folds as `lui $at,0x801f` + `addu $at,$at,rank` + `lhu -20676($at)` and the
 * bonus base hoists into `$s4` as the original does. The first live difference is
 * the frame: 56 bytes here against 48, because the kind-table address `0x801ca719`
 * is materialised as `lui $s5,0x801d` + `addiu $s5,$s5,-22759` in a callee-saved
 * register the original does not hold, while the original reaches the same byte
 * as `lui $at,0x801d` + `addu $at,$at,index` + `lbu -22759($at)`; both compilers
 * keep the mode constant `4` in `$s4`. The original's dead `addu` running sum over
 * the local records is also missing, since this compiler's dead-store pass removes
 * it.
 */
void func_801DAAE4(void) {
  u16 max_local_value;
  u8                count;
  u8                index;

  max_local_value = 0u;
  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x8000u) != 0u)) {
      if ((func_801DB9E4(index) != 0u) &&
          (max_local_value < BATTLE_LOCAL_HALF_98(battle_work))) {
        max_local_value = BATTLE_LOCAL_HALF_98(battle_work);
      }
    }
    index += 1u;
  }

  count = 0u;
  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x8000u) != 0u)) {
      if (func_801DB9E4(index) != 0u) {
        D_801EB550[count].index = index;
        D_801EB550[count].value = 0;
        if (BATTLE_LOCAL_BYTE_119(battle_work) == 4u) {
          u8 kind_mode;

          kind_mode = D_801CA718[((u32)BATTLE_LOCAL_HALF_11A(battle_work) *
                                  0x14u) + 1u];
          if (kind_mode == 1u || kind_mode == 3u) {
            D_801EB550[count].value = 2;
          } else if (kind_mode == 0u) {
            D_801EB550[count].value = 4;
          }
        }
        if (D_80145FA9[index].byte_00 == 5u) {
          D_801EB550[count].value = 1;
        }
        D_801EB550[count].value =
            (s16)(D_801EB550[count].value *
                  D_801EAF3C[findThresholdRank(
                      D_80145F0A[index].byte_00, 1u)]) /
            100;
        D_801EB550[count].value =
            (s16)(D_801EB550[count].value + D_80145F28[index].half_00);
        count += 1u;
      }
    }
    index += 1u;
  }

  index = 3u;
  while (index < 0x0bu) {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_ENEMY_WORD_104(battle_work) & 0x8000u) != 0u)) {
      if (func_801DB9E4(index) != 0u) {
        u32 rank;
        s8  random_bonus;

        rank = findThresholdRank(BATTLE_ENEMY_BYTE_88(battle_work), 0u);
        random_bonus =
            D_801EAF48[(rank * 0x10u) + (rand() & 0xfu)];
        D_801EB550[count].index = index;
        D_801EB550[count].value =
            (s16)(BATTLE_ENEMY_HALF_A8(battle_work) + random_bonus);
        count += 1u;
      }
    }
    index += 1u;
  }

  BATTLE_GLOBAL_BYTE_6323 = count;
  if (count > 1u) {
    u8 left;

    left = 0u;
    while (left < count - 1) {
      u8 right;

      right = left + 1u;
      while (right < count) {
        if (D_801EB550[left].value < D_801EB550[right].value) {
          Battle03RankEntry temp;

          temp = D_801EB550[left];
          D_801EB550[left] = D_801EB550[right];
          D_801EB550[right] = temp;
        }
        right += 1u;
      }
      left += 1u;
    }
  }

  index = 0u;
  while (index < 0x0bu) {
    BATTLE_GLOBAL_BYTE_630C(index) = 0xffu;
    index += 1u;
  }

  index = 0u;
  while (index < BATTLE_GLOBAL_BYTE_6323) {
    BATTLE_GLOBAL_BYTE_630C(index) = (u8)D_801EB550[index].index;
    index += 1u;
  }

  BATTLE_GLOBAL_BYTE_6322 = 0u;
}
