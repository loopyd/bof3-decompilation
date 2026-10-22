#include "bof3/battle/battle03_internal.h"

/* @source 0x801DA69C
 * @behavior Reports whether the enemy work record selected by the low byte of
 * arg0 may reuse a UI owner slot: it scans the eight UI owner slots from slot 7
 * down for the one whose kind byte at +0x3A equals arg0 + 3, then walks the
 * slots above it and rejects any visible slot (a slot whose mode byte at +0x33
 * is 2 or 3 is hidden) whose kind byte minus 3 selects the same enemy kind byte
 * at +0x7C. Returns 0 for the rejected case and 1 otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801DA69C(u32 arg0) {
  s8 owner_index;
  u8 slot_kind;
  s8 scan_index;
  u32 ui_kind;
  u32 kind_index;

  arg0 &= 0xffu;
  slot_kind = D_801EB6AC[arg0 * 0x118u];
  owner_index = 7;
  while (owner_index >= 0) {
    u32 owner_offset;

    owner_offset = (owner_index + 5) * 36;
    if (D_8014833A[owner_offset] == arg0 + 3u) {
      break;
    }
    owner_index -= 1;
  }

  scan_index = 7;
  if (owner_index < 7) {
    while (scan_index > owner_index) {
      u32 scan_offset;

      scan_offset = (scan_index + 5) * 36;
      if (D_80148333[scan_offset] != 2u) {
        if (D_80148333[scan_offset] != 3u) {
          ui_kind = D_8014833A[scan_offset];
          kind_index = (ui_kind - 3u) * 0x118u;
          if (D_801EB6AC[kind_index] == slot_kind) {
            return 0u;
          }
        }
      }
      scan_index -= 1;
    }
  }

  return 1u;
}
