#include "internal.h"

/* does: updates the local battle selection grid cursor, refreshes the preview
 * resource ring, and commits or cancels the current kind when the routed
 * buttons fire.
 * @source: 0x8009704c FUN_8009704c
 */
void func_8009704c(void) {
  volatile u8* active_selection_slot;
  u32          selection_input;
  u32          active_selection_flags;
  u8           previous_cursor_index;
  u8*          selection_kind_table;
  u8           selected_kind;
  u8           ring_slot_index;

  if (BOF3_BATTLE_SELECTION_LOCKED != 0u) {
    return;
  }

  BOF3_BATTLE_SELECTION_CURSOR_DIRTY = 1u;
  BOF3_BATTLE_SELECTION_CURSOR_X = BOF3_BATTLE_SELECTION_CURSOR_BASE_X + 7u;
  BOF3_BATTLE_SELECTION_CURSOR_Y =
      (u16)(BOF3_BATTLE_SELECTION_CURSOR_BASE_Y +
            (u16)((BOF3_BATTLE_SELECTION_CURSOR_INDEX -
                   BOF3_BATTLE_SELECTION_SCROLL_BASE) *
                  0xdu) +
            0x1au);

  selection_input =
      battle_decode_repeatable_input(BOF3_BATTLE_INPUT_HELD_MASK & 0xf00cu);
  active_selection_slot = BOF3_BATTLE_ACTIVE_SELECTION_SLOT_PTR;
  active_selection_flags = *(volatile u32*)(active_selection_slot + 0x10);

  if ((active_selection_flags & 0x20002u) != 2u) {
    if ((selection_input & 0x8000u) != 0u) {
      battle_queue_frontend_cue(0x101u);
      BOF3_BATTLE_SELECTION_MOVE_SFX = 0x32u;
      BOF3_BATTLE_SELECTION_GROUP_INDEX -= 1u;
      if ((s8)BOF3_BATTLE_SELECTION_GROUP_INDEX < 0) {
        BOF3_BATTLE_SELECTION_GROUP_INDEX = 3u;
      }
    } else if ((selection_input & 0x2000u) != 0u) {
      battle_queue_frontend_cue(0x101u);
      BOF3_BATTLE_SELECTION_MOVE_SFX = 0x31u;
      BOF3_BATTLE_SELECTION_GROUP_INDEX += 1u;
      if (BOF3_BATTLE_SELECTION_GROUP_INDEX > 3u) {
        BOF3_BATTLE_SELECTION_GROUP_INDEX = 0u;
      }
    }
  }

  previous_cursor_index = BOF3_BATTLE_SELECTION_CURSOR_INDEX;
  if ((selection_input & 0x1000u) != 0u) {
    if (BOF3_BATTLE_SELECTION_CURSOR_INDEX != 0u) {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX -= 1u;
    }
    if ((s16)BOF3_BATTLE_SELECTION_CURSOR_INDEX <
        BOF3_BATTLE_SELECTION_SCROLL_BASE) {
      BOF3_BATTLE_SELECTION_SCROLL_DELTA = (s16)0xfff0u;
    }
  } else if ((selection_input & 0x4000u) != 0u) {
    if (BOF3_BATTLE_SELECTION_CURSOR_INDEX < 9u) {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX += 1u;
    }
    if ((s16)(BOF3_BATTLE_SELECTION_SCROLL_BASE + 7) <=
        (s16)BOF3_BATTLE_SELECTION_CURSOR_INDEX) {
      BOF3_BATTLE_SELECTION_SCROLL_DELTA = 0x10;
    }
  } else if ((selection_input & 4u) != 0u) {
    if (BOF3_BATTLE_SELECTION_SCROLL_BASE == 0) {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX = 0u;
    } else if (BOF3_BATTLE_SELECTION_SCROLL_BASE < 7) {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX =
          (u8)(BOF3_BATTLE_SELECTION_CURSOR_INDEX -
               (u8)BOF3_BATTLE_SELECTION_SCROLL_BASE);
      BOF3_BATTLE_SELECTION_SCROLL_BASE = 0;
    } else {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX -= 7u;
      BOF3_BATTLE_SELECTION_SCROLL_BASE -= 7;
    }
  } else if ((selection_input & 8u) != 0u) {
    if (BOF3_BATTLE_SELECTION_SCROLL_BASE == 3) {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX = 9u;
    } else if (BOF3_BATTLE_SELECTION_SCROLL_BASE < -3) {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX += 7u;
      BOF3_BATTLE_SELECTION_SCROLL_BASE += 7;
    } else {
      BOF3_BATTLE_SELECTION_CURSOR_INDEX =
          (u8)(BOF3_BATTLE_SELECTION_CURSOR_INDEX -
               (u8)(BOF3_BATTLE_SELECTION_SCROLL_BASE - 3));
      BOF3_BATTLE_SELECTION_SCROLL_BASE = 3;
    }
  }

  if (previous_cursor_index != BOF3_BATTLE_SELECTION_CURSOR_INDEX) {
    battle_queue_frontend_cue(0x100u);
  }

  selection_kind_table = battle_resolve_selection_kind_table(
      BOF3_BATTLE_SELECTION_SOURCE_SLOT, BOF3_BATTLE_SELECTION_GROUP_INDEX, 1u);
  selected_kind = selection_kind_table[BOF3_BATTLE_SELECTION_CURSOR_INDEX];
  ring_slot_index = (u8)((BOF3_BATTLE_PANEL_ICON_RING_HEAD - 1u) & 0xfu);
  BOF3_BATTLE_SELECTION_RING_HANDLE(ring_slot_index) =
      battle_resolve_frontend_resource(
          BOF3_BATTLE_SELECTION_KIND_NAME_ID(selected_kind));

  if (BOF3_BATTLE_SELECTION_SCROLL_DELTA != 0) {
    return;
  }

  if ((BOF3_BATTLE_INPUT_CANCEL_MASK & BOF3_BATTLE_INPUT_HELD_MASK) != 0u) {
    BOF3_BATTLE_SELECTION_LOCKED = 1u;
    BOF3_BATTLE_SELECTION_CURSOR_DIRTY = 0u;
    BOF3_BATTLE_SELECTION_RING_FLAG(ring_slot_index) = 1u;
    BOF3_BATTLE_SELECTION_SAVED_GROUP(BOF3_BATTLE_SELECTION_RING_INDEX) =
        BOF3_BATTLE_SELECTION_GROUP_INDEX;
    BOF3_BATTLE_SELECTION_SAVED_SCROLL(BOF3_BATTLE_SELECTION_RING_INDEX) =
        (u8)BOF3_BATTLE_SELECTION_SCROLL_BASE;
    BOF3_BATTLE_SELECTION_SAVED_CURSOR(BOF3_BATTLE_SELECTION_RING_INDEX) =
        BOF3_BATTLE_SELECTION_CURSOR_INDEX;
    battle_queue_frontend_cue(0x106u);
    BOF3_BATTLE_SELECTION_ROOT_STATE += 1u;
    return;
  }

  if ((BOF3_BATTLE_INPUT_CONFIRM_MASK & BOF3_BATTLE_INPUT_HELD_MASK) == 0u) {
    return;
  }

  if (battle_selection_kind_is_blocked() != 0u) {
    battle_queue_frontend_cue(0x107u);
    return;
  }

  battle_queue_frontend_cue(0x103u);
  BOF3_BATTLE_SELECTION_SAVED_GROUP(BOF3_BATTLE_SELECTION_RING_INDEX) =
      BOF3_BATTLE_SELECTION_GROUP_INDEX;
  BOF3_BATTLE_SELECTION_SAVED_SCROLL(BOF3_BATTLE_SELECTION_RING_INDEX) =
      (u8)BOF3_BATTLE_SELECTION_SCROLL_BASE;
  BOF3_BATTLE_SELECTION_SAVED_CURSOR(BOF3_BATTLE_SELECTION_RING_INDEX) =
      BOF3_BATTLE_SELECTION_CURSOR_INDEX;
  *(volatile u16*)(active_selection_slot + 2) = selected_kind;
  BOF3_BATTLE_SELECTION_LOCKED = 1u;
  BOF3_BATTLE_SELECTION_CURSOR_DIRTY = 0u;

  if (*(volatile u16*)(active_selection_slot + 2) == 0x97u) {
    BOF3_BATTLE_PANEL_STATE_KIND = 2u;
    active_selection_slot[0] =
        ((volatile u8*)BOF3_BATTLE_ACTIVE_MESSAGE_SLOT_PTR)[5];
    BOF3_BATTLE_SELECTION_OWNER_STATE = 7u;
    BOF3_BATTLE_SELECTION_ROOT_STATE = 0u;
    return;
  }

  BOF3_BATTLE_SELECTION_ROOT_STATE = 3u;
}
