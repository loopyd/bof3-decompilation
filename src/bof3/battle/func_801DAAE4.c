#include "bof3/battle/battle03_internal.h"

extern int rand(void);
/* @behavior builds and sorts the mixed local/enemy ranking list used by the owner
 * selection bytes at `0x8014630c`, then resets the current owner cursor.
 * @source 0x801DAAE4
 * @status partial
 * @match 94.27
 * @residual 329 matching instructions out of 349 current and 349 original;
 * both ranges are 1396 bytes with a 48-byte frame. Bytes differ; the first
 * aligned difference is saved-register setup at +0x0C. Prefix differences
 * include maximum register and constant/reset setup, the eliminated unused
 * sum, conditional halfword addressing, second-mode allocation and affected
 * branch destinations.
 * Kind has an independent word local for the signed decision tree. Rank holds
 * five, then the helper result. An ordinary byte holds the separate second
 * mode sample, then the fresh helper argument after the optional store.
 * Both rank results are truncated to their low byte only at table consumers.
 * The unsigned halfword maximum becomes the existing held one after its last
 * maximum use. A separate halfword phase local serves fill and outer index;
 * low-byte consumers and guards keep increments below 255. The enemy index is
 * bounded to 0..7. Row and entry offsets have genuine load/store consumers.
 * The local row offset precedes the zero store and first mode read. Quotient
 * storage precedes the fresh local halfword sample. Enemy bonus and halfword
 * reads precede ranking publication.
 * The held count admits the first outer sort iteration; later guards retain
 * their distinct volatile entry/latch observations. Count is reset before
 * each clear/copy phase. The copy entry tests for nonzero and its latch uses
 * a fresh count sample. The publication store uses a plain byte view; the
 * shared declaration and all five later count reads remain volatile.
 */
void func_801DAAE4(void) {
  u8 count;
  u16 phase_value;
  u16 maximum_one;

  {
    u32 total;

    total = 0u;
    maximum_one = 0u;
    count = 0u;
    while (count < 3u) {
      if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
          ((D_80145FB8[count].flags_00 & 0x8000u) != 0u)) {
        if (func_801DB9E4(count) != 0u) {
          u16 value;

          value = D_80145F28[count].half_00;
          total += value;
          if (maximum_one < value) {
            maximum_one = value;
          }
        }
      }
      count += 1u;
    }
  }

  count = 0u;
  {
    maximum_one = 1u;
    phase_value = 0u;
    while ((u8)phase_value < 3u) {
      if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
          ((D_80145FB8[(u8)phase_value].flags_00 & 0x8000u) != 0u)) {
        if (func_801DB9E4((u8)phase_value) != 0u) {
          u32 local_row_offset;
          u32 rank;
          u32 kind;
          u8 mode_or_argument;
          u32 rank_entry_offset;
          u32 rank_table_offset;

          D_801EB550[count].index = (u8)phase_value;
          do {
            local_row_offset = (u32)(u8)phase_value * 0x140u;
            D_801EB550[count].value = 0;
          } while (0);
          if (FIELD_REF(volatile u8, (u8*)D_80145FA9 + local_row_offset, 0u) == 4u) {
            kind = D_801CA718[((u32)FIELD_REF(volatile u16, &D_80145FA9[(u8)phase_value], 1u) * 0x14u) + 1u];
            if (kind == maximum_one) {
              goto store_kind_two;
            }
            if ((s32)kind >= 2) {
              goto check_kind_three;
            }
            if (kind == 0u) {
              goto store_kind_four;
            }
            goto kind_done;
          check_kind_three:
            if (kind == 3u) {
              goto store_kind_two;
            }
            goto kind_done;
          store_kind_four:
            D_801EB550[count].value = 4;
            goto kind_done;
          store_kind_two:
            D_801EB550[count].value = 2;
          }
        kind_done:
          rank = 5u;
          mode_or_argument = D_80145FA9[(u8)phase_value].byte_00;
          if (mode_or_argument == rank) {
            D_801EB550[count].value = maximum_one;
          }
          mode_or_argument = D_80145F0A[(u8)phase_value].byte_00;
          rank = findThresholdRank(mode_or_argument, 1u);
          rank_entry_offset = (u32)count * 4u;
          rank_table_offset = (u32)(u8)rank * 2u;
          do {
            FIELD_REF(s16, D_801EB550, rank_entry_offset) =
                (s16)(FIELD_REF(s16, D_801EB550, rank_entry_offset) *
                      FIELD_REF(u16, D_801EAF3C, rank_table_offset)) / 100;
          } while (0);
          FIELD_REF(s16, D_801EB550, rank_entry_offset) =
              (s16)(FIELD_REF(s16, D_801EB550, rank_entry_offset) +
                    D_80145F28[(u8)phase_value].half_00);
          count += 1u;
        }
      }
      phase_value += 1u;
    }
  }

  {
    u32 admitted_enemy;
    const s8 *bonus_base;

    phase_value = 3u;
    bonus_base = D_801EAF48;
    while ((u8)phase_value < 0x0bu) {
      if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
          ((D_801EB734[(u8)phase_value - 3u].word_00 & 0x8000u) != 0u)) {
        if (func_801DB9E4(admitted_enemy = (u8)phase_value) != 0u) {
          u32 rank;
          u32 enemy_index;
          u32 enemy_row_offset;
          u32 enemy_rank_entry_offset;
          u32 random_value;
          u32 scaled_offset;
          u32 row_address;
          s8  random_bonus;
          u16 enemy_value;

          enemy_index = admitted_enemy - 3u;
          enemy_row_offset = (u32)enemy_index * 0x118u;
          rank = findThresholdRank(FIELD_REF(volatile u8, D_801EB6D8, enemy_row_offset - 0x20u), 0u);
          random_value = rand();
          enemy_rank_entry_offset = (u32)count * 4u;
          scaled_offset = (u8)rank * 0x10u;
          row_address = scaled_offset + (u32)bonus_base;
          random_bonus = PSX_PTR(const s8, row_address)[random_value & 0xfu];
          enemy_value = FIELD_REF(volatile u16, D_801EB6D8, enemy_row_offset);
          FIELD_REF(u16, D_801EB550, enemy_rank_entry_offset + 2u) = (u8)phase_value;
          FIELD_REF(s16, D_801EB550, enemy_rank_entry_offset) =
              (s16)(enemy_value + random_bonus);
          count += 1u;
        }
      }
      phase_value += 1u;
    }
  }

  {
  u8 *published_count;

  published_count = (u8 *)&BATTLE_GLOBAL_BYTE_6323;
  *published_count = count;
  if (count > 1u) {
    s32 sampled_count;
    volatile u8 *entry_count;

    sampled_count = (s32)count;
    phase_value = 0u;
    while ((u8)phase_value < sampled_count - 1) {
      entry_count = &BATTLE_GLOBAL_BYTE_6323;
      {
      u32 entry_index;

      count = (u8)phase_value + 1u;
      entry_index = count;
      if (entry_index < *entry_count) {
        volatile u8 *latch_count;

        do {
        u32 left_entry_offset;
        Battle03RankEntry *left_swap;

        do {
          left_entry_offset = (u32)(u8)phase_value * 4u;
          left_swap = FIELD_ADDR(Battle03RankEntry, D_801EB550, left_entry_offset);
        } while (0);
        latch_count = &BATTLE_GLOBAL_BYTE_6323;
        if (FIELD_REF(s16, D_801EB550, left_entry_offset) < D_801EB550[entry_index].value) {
          Battle03RankEntry temp;

          temp = *left_swap;
          *left_swap = D_801EB550[entry_index];
          D_801EB550[entry_index] = temp;
        }
        count += 1u;
        entry_index = count;
        } while (entry_index < *latch_count);
      }
      }
      do {
        phase_value += 1u;
      } while (0);
      sampled_count = BATTLE_GLOBAL_BYTE_6323;
    }
  }

  count = 0u;
  while (count < 0x0bu) {
    FIELD_REF(volatile u8, &D_8014630C, count) = 0xffu;
    count += 1u;
  }
  count = 0u;

  {
  volatile u8 *copy_count;

  copy_count = &BATTLE_GLOBAL_BYTE_6323;
  if (*copy_count != 0u) {
    do {
      FIELD_REF(volatile u8, &D_8014630C, count) = (u8)D_801EB550[count].index;
      count += 1u;
    } while (count < *copy_count);
  }

  }
  }
  BATTLE_GLOBAL_BYTE_6322 = 0u;
}
