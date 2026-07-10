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

  if (BATTLE_SELECTION_LOCKED != 0u) {
    return;
  }

  BATTLE_SELECTION_CURSOR_DIRTY = 1u;
  BATTLE_SELECTION_CURSOR_X = BATTLE_SELECTION_CURSOR_BASE_X + 7u;
  BATTLE_SELECTION_CURSOR_Y = (u16)(BATTLE_SELECTION_CURSOR_BASE_Y +
                                    (u16)((BATTLE_SELECTION_CURSOR_INDEX -
                                           BATTLE_SELECTION_SCROLL_BASE) *
                                          0xdu) +
                                    0x1au);

  selection_input =
      battle_decode_repeatable_input(BATTLE_INPUT_HELD_MASK & 0xf00cu);
  active_selection_slot = BATTLE_ACTIVE_SELECTION_SLOT_PTR;
  active_selection_flags = *(volatile u32*)(active_selection_slot + 0x10);

  if ((active_selection_flags & 0x20002u) != 2u) {
    if ((selection_input & 0x8000u) != 0u) {
      battle_queue_frontend_cue(0x101u);
      BATTLE_SELECTION_MOVE_SFX = 0x32u;
      BATTLE_SELECTION_GROUP_INDEX -= 1u;
      if ((s8)BATTLE_SELECTION_GROUP_INDEX < 0) {
        BATTLE_SELECTION_GROUP_INDEX = 3u;
      }
    } else if ((selection_input & 0x2000u) != 0u) {
      battle_queue_frontend_cue(0x101u);
      BATTLE_SELECTION_MOVE_SFX = 0x31u;
      BATTLE_SELECTION_GROUP_INDEX += 1u;
      if (BATTLE_SELECTION_GROUP_INDEX > 3u) {
        BATTLE_SELECTION_GROUP_INDEX = 0u;
      }
    }
  }

  previous_cursor_index = BATTLE_SELECTION_CURSOR_INDEX;
  if ((selection_input & 0x1000u) != 0u) {
    if (BATTLE_SELECTION_CURSOR_INDEX != 0u) {
      BATTLE_SELECTION_CURSOR_INDEX -= 1u;
    }
    if ((s16)BATTLE_SELECTION_CURSOR_INDEX < BATTLE_SELECTION_SCROLL_BASE) {
      BATTLE_SELECTION_SCROLL_DELTA = (s16)0xfff0u;
    }
  } else if ((selection_input & 0x4000u) != 0u) {
    if (BATTLE_SELECTION_CURSOR_INDEX < 9u) {
      BATTLE_SELECTION_CURSOR_INDEX += 1u;
    }
    if ((s16)(BATTLE_SELECTION_SCROLL_BASE + 7) <=
        (s16)BATTLE_SELECTION_CURSOR_INDEX) {
      BATTLE_SELECTION_SCROLL_DELTA = 0x10;
    }
  } else if ((selection_input & 4u) != 0u) {
    if (BATTLE_SELECTION_SCROLL_BASE == 0) {
      BATTLE_SELECTION_CURSOR_INDEX = 0u;
    } else if (BATTLE_SELECTION_SCROLL_BASE < 7) {
      BATTLE_SELECTION_CURSOR_INDEX = (u8)(BATTLE_SELECTION_CURSOR_INDEX -
                                           (u8)BATTLE_SELECTION_SCROLL_BASE);
      BATTLE_SELECTION_SCROLL_BASE = 0;
    } else {
      BATTLE_SELECTION_CURSOR_INDEX -= 7u;
      BATTLE_SELECTION_SCROLL_BASE -= 7;
    }
  } else if ((selection_input & 8u) != 0u) {
    if (BATTLE_SELECTION_SCROLL_BASE == 3) {
      BATTLE_SELECTION_CURSOR_INDEX = 9u;
    } else if (BATTLE_SELECTION_SCROLL_BASE < -3) {
      BATTLE_SELECTION_CURSOR_INDEX += 7u;
      BATTLE_SELECTION_SCROLL_BASE += 7;
    } else {
      BATTLE_SELECTION_CURSOR_INDEX =
          (u8)(BATTLE_SELECTION_CURSOR_INDEX -
               (u8)(BATTLE_SELECTION_SCROLL_BASE - 3));
      BATTLE_SELECTION_SCROLL_BASE = 3;
    }
  }

  if (previous_cursor_index != BATTLE_SELECTION_CURSOR_INDEX) {
    battle_queue_frontend_cue(0x100u);
  }

  selection_kind_table = battle_resolve_selection_kind_table(
      BATTLE_SELECTION_SOURCE_SLOT, BATTLE_SELECTION_GROUP_INDEX, 1u);
  selected_kind = selection_kind_table[BATTLE_SELECTION_CURSOR_INDEX];
  ring_slot_index = (u8)((BATTLE_PANEL_ICON_RING_HEAD - 1u) & 0xfu);
  BATTLE_SELECTION_RING_HANDLE(ring_slot_index) =
      battle_resolve_frontend_resource(
          BATTLE_SELECTION_KIND_NAME_ID(selected_kind));

  if (BATTLE_SELECTION_SCROLL_DELTA != 0) {
    return;
  }

  if ((BATTLE_INPUT_CANCEL_MASK & BATTLE_INPUT_HELD_MASK) != 0u) {
    BATTLE_SELECTION_LOCKED = 1u;
    BATTLE_SELECTION_CURSOR_DIRTY = 0u;
    BATTLE_SELECTION_RING_FLAG(ring_slot_index) = 1u;
    BATTLE_SELECTION_SAVED_GROUP(BATTLE_SELECTION_RING_INDEX) =
        BATTLE_SELECTION_GROUP_INDEX;
    BATTLE_SELECTION_SAVED_SCROLL(BATTLE_SELECTION_RING_INDEX) =
        (u8)BATTLE_SELECTION_SCROLL_BASE;
    BATTLE_SELECTION_SAVED_CURSOR(BATTLE_SELECTION_RING_INDEX) =
        BATTLE_SELECTION_CURSOR_INDEX;
    battle_queue_frontend_cue(0x106u);
    BATTLE_SELECTION_ROOT_STATE += 1u;
    return;
  }

  if ((BATTLE_INPUT_CONFIRM_MASK & BATTLE_INPUT_HELD_MASK) == 0u) {
    return;
  }

  if (battle_selection_kind_is_blocked() != 0u) {
    battle_queue_frontend_cue(0x107u);
    return;
  }

  battle_queue_frontend_cue(0x103u);
  BATTLE_SELECTION_SAVED_GROUP(BATTLE_SELECTION_RING_INDEX) =
      BATTLE_SELECTION_GROUP_INDEX;
  BATTLE_SELECTION_SAVED_SCROLL(BATTLE_SELECTION_RING_INDEX) =
      (u8)BATTLE_SELECTION_SCROLL_BASE;
  BATTLE_SELECTION_SAVED_CURSOR(BATTLE_SELECTION_RING_INDEX) =
      BATTLE_SELECTION_CURSOR_INDEX;
  *(volatile u16*)(active_selection_slot + 2) = selected_kind;
  BATTLE_SELECTION_LOCKED = 1u;
  BATTLE_SELECTION_CURSOR_DIRTY = 0u;

  if (*(volatile u16*)(active_selection_slot + 2) == 0x97u) {
    BATTLE_PANEL_STATE_KIND = 2u;
    active_selection_slot[0] =
        ((volatile u8*)BATTLE_ACTIVE_MESSAGE_SLOT_PTR)[5];
    BATTLE_SELECTION_OWNER_STATE = 7u;
    BATTLE_SELECTION_ROOT_STATE = 0u;
    return;
  }

  BATTLE_SELECTION_ROOT_STATE = 3u;
}
