#include "internal.h"

/* @behavior copies each active local/enemy battler into a queued slot entry when
 * its `0x8` flag is clear, then initializes the queued-slot state bytes.
 * @source 0x801DE1D4
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
      if (((*(volatile u8*)(0x80145e90u + work_offset) & 1u) != 0u) &&
          ((*(volatile u32*)(0x80145fb8u + work_offset) & 8u) == 0u)) {
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
        src = (const volatile u32*)(0x80145e90u + work_offset);
        end = (const volatile u32*)(0x80145f00u + work_offset);
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
        work_ptr = 0x80145e90u + work_offset;
        queued_slot->unk_74 = work_ptr;
        (*(volatile u32*)0x1f800044u) = work_ptr;
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
          ((*(volatile u32*)(0x801eb734u + (work_index * 0x118u)) & 8u) ==
           0u)) {
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
        src = (const volatile u32*)(0x801eb630u + work_offset);
        end = (const volatile u32*)(0x801eb6a0u + work_offset);
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
        work_ptr = 0x801eb2e8u + ((u32)index * 0x118u);
        queued_slot->unk_74 = work_ptr;
        (*(volatile u32*)0x1f800044u) = work_ptr;
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
