#include "bof3/battle/battle03_internal.h"

/* @behavior copies each active local/enemy battler into a queued slot entry when
 * its `0x8` flag is clear, then initializes the queued-slot state bytes.
 * @source 0x801DE1D4
 * @status partial
 * @match 31.82
 * @residual non-exact live audit: 49/154 instructions; 616 original bytes versus 580 current.
 */
void func_801DE1D4(void) {
  u8                           index;
  volatile Battle03QueuedSlot* queued_slots;

  index = 0u;
  queued_slots = BATTLE_QUEUED_SLOT_ARRAY;
  while (index <= 10u) {
    if (index < 3u) {
      u32 work_offset;

      work_offset = ((u32)index * 5u) << 6;
      if (((BATTLE_LOCAL_ABS_BYTE_5E90(index) & 1u) != 0u) &&
          ((BATTLE_LOCAL_ABS_WORD_5FB8(index) & 8u) == 0u)) {
        u32                          slot;
        volatile Battle03QueuedSlot* queued_slot;
        volatile u32*                dst;
        const volatile u32*          src;
        const volatile u32*          end;
        u32                          work_ptr;
        volatile u8*                 queued_slot_bytes;

        slot = func_801E590C(0u, 7u) & 0xffu;
        queued_slot = &queued_slots[slot];
        dst = (volatile u32*)queued_slot;
        src =
            (const volatile u32*)((const volatile u8*)BATTLE_LOCAL_WORK_ARRAY +
                                  work_offset);
        end =
            (const volatile u32*)((const volatile u8*)BATTLE_LOCAL_WORK_ARRAY +
                                  work_offset + 0x70u);
        do {
          u32 word_01;
          u32 word_02;
          u32 word_03;

          word_01 = src[1];
          word_02 = src[2];
          word_03 = src[3];
          dst[0] = src[0];
          dst[1] = word_01;
          dst[2] = word_02;
          dst[3] = word_03;
          src += 4;
          dst += 4;
        } while (src != end);
        dst[0] = src[0];
        work_ptr = (u32)((volatile u8*)BATTLE_LOCAL_WORK_ARRAY + work_offset);
        queued_slot->unk_74 = work_ptr;
        BATTLE_SCRATCH_CELL_WORD = work_ptr;
        queued_slot_bytes = (volatile u8*)queued_slot;
        queued_slot_bytes[5] = 7u;
        queued_slot_bytes[6] = 0u;
        queued_slot_bytes[1] = 0u;
        queued_slot_bytes[2] = 0u;
        queued_slot_bytes[0x29] = 3u;
        queued_slot_bytes[0x5c] = 0u;
        queued_slot_bytes[0x5d] = 0u;
        queued_slot_bytes[0x5f] = 0u;
        queued_slot_bytes[0x5e] = 0u;
      }
    } else {
      u32 work_index;
      u32 work_offset;

      work_index = (u32)index - 3u;
      if ((func_801DB524(index) == 0u) &&
          ((BATTLE_ENEMY_ABS_WORD_734(work_index) & 8u) == 0u)) {
        u32                          slot;
        volatile Battle03QueuedSlot* queued_slot;
        volatile u32*                dst;
        const volatile u32*          src;
        const volatile u32*          end;
        u32                          work_ptr;
        volatile u8*                 queued_slot_bytes;

        work_offset = work_index * 0x118u;
        slot = func_801E590C(0u, 7u) & 0xffu;
        queued_slot = &queued_slots[slot];
        dst = (volatile u32*)queued_slot;
        src =
            (const volatile u32*)((const volatile u8*)BATTLE_ENEMY_WORK_ARRAY +
                                  work_offset);
        end =
            (const volatile u32*)((const volatile u8*)BATTLE_ENEMY_WORK_ARRAY +
                                  work_offset + 0x70u);
        do {
          u32 word_01;
          u32 word_02;
          u32 word_03;

          word_01 = src[1];
          word_02 = src[2];
          word_03 = src[3];
          dst[0] = src[0];
          dst[1] = word_01;
          dst[2] = word_02;
          dst[3] = word_03;
          src += 4;
          dst += 4;
        } while (src != end);
        dst[0] = src[0];
        work_ptr = (u32)&BATTLE_ENEMY_WORK_ARRAY[work_index];
        queued_slot->unk_74 = work_ptr;
        BATTLE_SCRATCH_CELL_WORD = work_ptr;
        queued_slot_bytes = (volatile u8*)queued_slot;
        queued_slot_bytes[5] = 7u;
        queued_slot_bytes[6] = 0u;
        queued_slot_bytes[1] = 0u;
        queued_slot_bytes[2] = 0u;
        queued_slot_bytes[0x29] = 3u;
        queued_slot_bytes[0x5c] = 0u;
        queued_slot_bytes[0x5d] = 0u;
        queued_slot_bytes[0x5f] = 0u;
        queued_slot_bytes[0x5e] = 0u;
      }
    }
    index += 1u;
  }
}
