#include "bof3/battle/battle03_internal.h"

/* @source 0x801DE1D4
 * @behavior Copies each active local and enemy battler work record into a kind
 * 7 queued slot entry: local slots 0..2 whose record byte +0x00 carries bit 0x1
 * while their D_80145FB8 flag word lacks bit 0x8, and enemy slots 3..10 that
 * func_801DB524 reports available while their D_801EB734 flag word lacks bit
 * 0x8. For each such record it allocates the slot, copies the 0x74-byte record
 * image, publishes the record pointer into the entry +0x74 pointer and the
 * 0x1F800044 scratch cell, and clears the entry's queue bytes.
 * @status partial
 * @match 82.47
 * @residual non-exact live audit: 127/154 instructions at 600 versus 616 bytes;
 * first difference at +0x0028 in the slot-class branch target. Each arm already
 * builds the queued entry pointer the original uses for the +0x74 publish, so
 * that store takes the original's base-plus-displacement form and the queue-byte
 * clears re-derive the entry address. The trailing record word copy remains
 * per-arm, while the copy loop's source pointer advance is scheduled before
 * the four word stores. The 0x1F800044 cell is published through the named extern, whose
 * `lui $at` / `sw %lo` encoding matches the original.
 */
void func_801DE1D4(void) {
  u8 index;
  Battle03SlotStore* entry;

  index = 0u;
  while (index <= 10u) {
    u8  slot;
    u32 work_ptr;

    if (index < 3u) {
      u32 work_offset;

      work_offset = ((u32)index * 5u) << 6;
      if (((((u8*)D_80145E90)[work_offset]) & 1u) == 0u ||
          ((D_80145FB8[index].flags_00 & 8u) != 0u)) {
        goto advance;
      }
      {
        volatile u32* dst;
        const volatile u32* src;
        const volatile u32* end;

        slot = func_801E590C(0u, 7u);
        dst = (volatile u32*)&D_801EC330[slot];
        src = (const volatile u32*)((const volatile u8*)D_80145E90 + work_offset);
        end = (const volatile u32*)((const volatile u8*)src + 0x70u);
        do {
          u32 word_00;
          u32 word_01;
          u32 word_02;
          u32 word_03;

          word_00 = src[0];
          word_01 = src[1];
          word_02 = src[2];
          word_03 = src[3];
          dst[0] = word_00;
          dst[1] = word_01;
          dst[2] = word_02;
          dst[3] = word_03;
          src += 4;
          dst += 4;
        } while (src != end);
        dst[0] = src[0];
        entry = &D_801EC330[slot];
        work_ptr = (u32)&D_80145E90[index];
      }
    } else {
      u32 work_offset;

      if (func_801DB524(index) != 0u ||
          ((D_801EB734[(u32)index - 3u].word_00 & 8u) != 0u)) {
        goto advance;
      }
      {
        volatile u32* dst;
        const volatile u32* src;
        const volatile u32* end;

        work_offset = ((u32)index - 3u) * 0x118u;
        slot = func_801E590C(0u, 7u);
        dst = (volatile u32*)&D_801EC330[slot];
        src = (const volatile u32*)((const volatile u8*)D_801EB630 + work_offset);
        end = (const volatile u32*)((const volatile u8*)src + 0x70u);
        do {
          u32 word_00;
          u32 word_01;
          u32 word_02;
          u32 word_03;

          word_00 = src[0];
          word_01 = src[1];
          word_02 = src[2];
          word_03 = src[3];
          dst[0] = word_00;
          dst[1] = word_01;
          dst[2] = word_02;
          dst[3] = word_03;
          src += 4;
          dst += 4;
        } while (src != end);
        dst[0] = src[0];
        entry = &D_801EC330[slot];
        work_ptr = (u32)&D_801EB630[(u32)index - 3u];
      }
    }
    {
      volatile u8* queued_slot_bytes;

      entry->ptr_74 = work_ptr;
      D_1F800044 = (Battle03LocalWork*)work_ptr;
      queued_slot_bytes = (volatile u8*)&D_801EC330[slot];
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
  advance:
    index += 1u;
  }
}
