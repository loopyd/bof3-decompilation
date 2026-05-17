#include "internal.h"

/* does: applies the type-8 record-specific task-slot variant.
 * @source: 0x801f0f08
 */
void func_801f0f08(u8 source_index, u8 task_index, u8 record_kind_index) {
  u32 delta;
  u32 label_band_index;
  u32 label_offset;
  u32 task_offset;

  COMMU00_SCRATCH_SLOT->active = 1u;
  delta = *(volatile u32*)(0x80140000u + 0x502cu) -
          *(volatile u32*)((0x80140000u + 0x55ccu) + ((u32)source_index << 3));

  if (delta < 0x14u) {
    label_band_index = 0u;
  } else {
    label_band_index = 2u;

    if (delta < 0x28u) {
      label_band_index = 1u;
    }
  }

  task_offset = (u32)task_index * 0x98u;
  label_offset = label_band_index * 3u +
                 (u32) * (volatile u8*)(0x801f2928u + (u32)record_kind_index);
  *(volatile u16*)((0x80140000u + 0x6904u) + task_offset) =
      *(volatile u16*)(0x801f2930u + (label_offset << 1));
}
