#include "bof3/battle/battle15_internal.h"

/* @behavior draws three battle panel slots with labelled cursors. if the current
 * selection state targets a specific slot, that slot gets a redrawn preview
 * cursor using a dedicated palette row.
 * @source 0x800B0B0C
 * @status partial
 * @match 30.25
 * @residual non-exact live audit: 36/118 instructions; 472 original bytes versus 476 current.
 */
void func_800B0B0C(s16 base_x, s16 base_y) {
  volatile u16* palette_table;
  volatile u8*  panel_data;
  s16           slot_x;
  s16           slot_y;
  s16           cursor_y;
  u8            slot;
  u8            selection_byte;
  u8            active_slot;
  s32           slot_offset;
  s32           palette_index;

  palette_table = BATTLE_PALETTE_TABLE;
  cursor_y = base_y + 3;
  slot = 0u;

  do {
    slot_offset = (u32)slot * 0x30u;
    slot_x = base_x + (s16)slot_offset;
    slot_y = base_y;

    FUNCTION_AT(void (*)(s32, s32, s32, s32, s32, s32), 0x801ae3f0u)(
        (s32)(u16)slot_x, (s32)(u16)slot_y, 0x2du, 0x14u, 0u,
        (s32)PSX_REF(volatile u8, 0x80144952u));

    selection_byte = PSX_REF(volatile u8, 0x801462e5u);

    if (selection_byte & 0x80u) {
      active_slot = selection_byte & 0x7fu;
      if (slot == active_slot) {
        FUNCTION_AT(void (*)(s16, s16, s32, s32, volatile u16*), 0x8014f800u)(
            (s16)(slot_x + 4), cursor_y, 0u, 0x10u,
            palette_table + (slot * 6u));
        palette_index = (s32)BATTLE_PALETTE_ROW_28;
      } else {
        FUNCTION_AT(void (*)(s16, s16, s32, s32), 0x8014f800u)(
            (s16)(slot_x + 4), cursor_y, 7u, 0x10u);
        palette_index = (s32)BATTLE_PALETTE_ROW_20;
      }
    } else {
      FUNCTION_AT(void (*)(s16, s16, s32, s32), 0x8014f800u)(
          (s16)(slot_x + 4), cursor_y, 0u, 0x10u);
      palette_index = (s32)BATTLE_PALETTE_ROW_20;
    }

    FUNCTION_AT(void (*)(s16, s16, volatile void*, s32), 0x801af390u)(
        slot_x, base_y, (volatile void*)palette_index, 1u);

    slot += 1u;
  } while (slot < 3u);

  selection_byte = PSX_REF(volatile u8, 0x801462e5u);

  if (!(selection_byte & 0x80u)) {
    active_slot = selection_byte & 0x7fu;
    FUNCTION_AT(void (*)(s32, s32, s32), 0x801647c4u)(
        (s32)(u16)(base_x + (s16)((u32)active_slot * 0x30u)),
        (s32)(u16)(base_y + 4u), 0u);
  }
}
