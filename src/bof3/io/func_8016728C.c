#include "bof3/core/slus_internal.h"

/* Target-local binding for the shared stream index hint byte; the address row
 * lives in the composed Splat map and the manifest-owned slus_variables.c. */
extern volatile u8 D_80145024;

/*
 * @behavior records the stream index hint byte for one BOF3 content family (bit
 * 7 set only for family 0 and nothing written for family 3), then initializes
 * the EMI loader slot whose id is the family's slot base (0x26a, 0x1db, 0x1ee,
 * 0x27d) plus the index; an unknown family returns without touching the loader.
 * @source 0x8016728C
 * @status partial
 * @match 73.91
 * @residual Entry-copy, hint-store scheduling and switch-arm mask registers differ; both bodies are 184 bytes.
 */
void func_8016728C(u8 index, u8 family) {
  volatile u8* hint;
  u32 slot_id;
  u32 captured;

  captured = index;

  if (family != 3u) {
    hint = &D_80145024;
    if (family == 0u) {
      *hint = captured | 0x80u;
    } else {
      *hint = captured;
    }
  }

  switch (family) {
    case 0:
      slot_id = (u32)index + 0x26au;
      break;
    case 1:
      slot_id = (u32)index + 0x1dbu;
      break;
    case 2:
      slot_id = (u32)index + 0x1eeu;
      break;
    case 3:
      slot_id = (u32)index + 0x27du;
      break;
    default:
      return;
  }

  initStreamSlot(slot_id);
}
