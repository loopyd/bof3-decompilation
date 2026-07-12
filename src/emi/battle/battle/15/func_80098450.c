#include "internal.h"

/* @behavior updates the secondary battle selection grid cursor, refreshes the
 * preview ring handle, handles confirm/cancel, and routes into either the
 * prompt branch or the next shared owner state.
 * @source 0x80098450 FUN_80098450
 */
void func_80098450(void) {
  volatile u8* active_message_slot;
  volatile u8* active_selection_slot;
  volatile u8* secondary_group_table;
  u32          selection_input;
  u32          preview_ring_index;
  u32          source_slot;
  u32          choice_kind;
  u8           previous_cursor_index;

  if (BATTLE_SELECTION_LOCKED != 0u) {
    return;
  }

  source_slot = ((volatile u8*)BATTLE_ACTIVE_MESSAGE_SLOT_PTR)[5];
  BATTLE_SELECTION_PANEL_FLAGS(source_slot) &= 0xffffbfffu;

  if (BATTLE_SELECTION_LOCKED == 0u) {
    BATTLE_SELECTION_CURSOR_DIRTY = 1u;
    BATTLE_SELECTION_CURSOR_X = BATTLE_SELECTION_CURSOR_BASE_X + 7u;
    BATTLE_SELECTION_CURSOR_Y = (u16)(BATTLE_SELECTION_CURSOR_BASE_Y +
                                      (u16)((BATTLE_SECONDARY_CURSOR_INDEX -
                                             BATTLE_SECONDARY_PAGE_BASE) *
                                            0xdu) +
                                      0x1au);
  }

  selection_input =
      battle_decode_repeatable_input(BATTLE_INPUT_HELD_MASK & 0xf00cu);

  if ((selection_input & 0x8000u) != 0u) {
    battle_queue_frontend_cue(0x101u);
    BATTLE_SECONDARY_MOVE_SFX = 0x32u;
    BATTLE_SECONDARY_SOURCE_GROUP -= 1u;
    if ((s8)BATTLE_SECONDARY_SOURCE_GROUP < 0) {
      BATTLE_SECONDARY_SOURCE_GROUP = 3u;
    }
  } else if ((selection_input & 0x2000u) != 0u) {
    BATTLE_SECONDARY_MOVE_SFX = 0x31u;
    battle_queue_frontend_cue(0x101u);
    BATTLE_SECONDARY_SOURCE_GROUP += 1u;
    if (BATTLE_SECONDARY_SOURCE_GROUP > 3u) {
      BATTLE_SECONDARY_SOURCE_GROUP = 0u;
    }
  }

  previous_cursor_index = BATTLE_SECONDARY_CURSOR_INDEX;
  if ((selection_input & 0x1000u) != 0u) {
    if (BATTLE_SECONDARY_CURSOR_INDEX != 0u) {
      BATTLE_SECONDARY_CURSOR_INDEX -= 1u;
    }
    if (BATTLE_SECONDARY_CURSOR_INDEX < BATTLE_SECONDARY_PAGE_BASE) {
      BATTLE_SELECTION_SCROLL_DELTA = (s16)0xfff0u;
    }
  } else if ((selection_input & 0x4000u) != 0u) {
    if (BATTLE_SECONDARY_CURSOR_INDEX < 0x7fu) {
      BATTLE_SECONDARY_CURSOR_INDEX += 1u;
    }
    if ((u8)(BATTLE_SECONDARY_PAGE_BASE + 7u) <=
        BATTLE_SECONDARY_CURSOR_INDEX) {
      BATTLE_SELECTION_SCROLL_DELTA = 0x10;
    }
  } else if ((selection_input & 4u) != 0u) {
    if (BATTLE_SECONDARY_PAGE_BASE == 0u) {
      BATTLE_SECONDARY_CURSOR_INDEX = 0u;
    } else if (BATTLE_SECONDARY_PAGE_BASE < 7u) {
      BATTLE_SECONDARY_CURSOR_INDEX -= BATTLE_SECONDARY_PAGE_BASE;
      BATTLE_SECONDARY_PAGE_BASE = 0u;
    } else {
      BATTLE_SECONDARY_PAGE_BASE -= 7u;
      BATTLE_SECONDARY_CURSOR_INDEX -= 7u;
    }
  } else if ((selection_input & 8u) != 0u) {
    if (BATTLE_SECONDARY_PAGE_BASE == 0x79u) {
      BATTLE_SECONDARY_CURSOR_INDEX = 0x7fu;
    } else if (BATTLE_SECONDARY_PAGE_BASE < 0x73u) {
      BATTLE_SECONDARY_PAGE_BASE += 7u;
      BATTLE_SECONDARY_CURSOR_INDEX += 7u;
    } else {
      BATTLE_SECONDARY_CURSOR_INDEX = (u8)(BATTLE_SECONDARY_CURSOR_INDEX +
                                           0x79u - BATTLE_SECONDARY_PAGE_BASE);
      BATTLE_SECONDARY_PAGE_BASE = 0x79u;
    }
  }

  if (previous_cursor_index != BATTLE_SECONDARY_CURSOR_INDEX) {
    battle_queue_frontend_cue(0x100u);
  }

  secondary_group_table =
      BATTLE_SECONDARY_GROUP_TABLE(BATTLE_SECONDARY_SOURCE_GROUP);
  choice_kind = secondary_group_table[BATTLE_SECONDARY_CURSOR_INDEX];
  preview_ring_index = (BATTLE_PANEL_ICON_RING_HEAD - 1u) & 0xfu;
  BATTLE_SELECTION_RING_HANDLE(preview_ring_index) =
      battle_resolve_frontend_resource(battle_resolve_secondary_choice_resource(
          BATTLE_SECONDARY_SOURCE_GROUP, choice_kind));

  if (BATTLE_SELECTION_SCROLL_DELTA != 0) {
    return;
  }

  if (((selection_input & 0x1000u) != 0u) && (previous_cursor_index == 0u)) {
    battle_queue_frontend_cue(0x100u);
    BATTLE_SECONDARY_PROMPT_ROWS = 8u;
    BATTLE_SECONDARY_PROMPT_KIND = 5u;
    BATTLE_SECONDARY_PROMPT_ACTIVE = 1u;
    BATTLE_SECONDARY_PROMPT_CURSOR_LIMIT = 0xffu;
    BATTLE_SELECTION_ROOT_STATE = 6u;
    BATTLE_SECONDARY_PROMPT_MODE = 0u;
    BATTLE_PANEL_PROMPT_STATE = 0u;
    BATTLE_SECONDARY_PROMPT_X = BATTLE_SELECTION_CURSOR_BASE_X + 0x20u;
    BATTLE_SECONDARY_PROMPT_Y = BATTLE_SELECTION_CURSOR_BASE_Y - 0x16u;
    return;
  }

  if ((BATTLE_INPUT_CANCEL_MASK & BATTLE_INPUT_HELD_MASK) != 0u) {
    active_message_slot = (volatile u8*)BATTLE_ACTIVE_MESSAGE_SLOT_PTR;
    BATTLE_SELECTION_LOCKED = 1u;
    BATTLE_SELECTION_CURSOR_DIRTY = 0u;
    BATTLE_SELECTION_RING_FLAG(preview_ring_index) = 1u;
    BATTLE_SECONDARY_SAVED_GROUP = BATTLE_SECONDARY_SOURCE_GROUP;
    BATTLE_SECONDARY_SAVED_PAGE_BASE = BATTLE_SECONDARY_PAGE_BASE;
    BATTLE_SECONDARY_SAVED_CURSOR = BATTLE_SECONDARY_CURSOR_INDEX;
    active_message_slot[0x119] = 0u;
    battle_queue_frontend_cue(0x106u);
    BATTLE_SELECTION_ROOT_STATE += 1u;
    return;
  }

  if ((BATTLE_INPUT_CONFIRM_MASK & BATTLE_INPUT_HELD_MASK) == 0u) {
    return;
  }

  active_selection_slot = BATTLE_ACTIVE_SELECTION_SLOT_PTR;
  choice_kind = secondary_group_table[BATTLE_SECONDARY_CURSOR_INDEX] +
                ((u32)BATTLE_SECONDARY_SOURCE_GROUP << 8);
  *(volatile u16*)(active_selection_slot + 2) = (u16)choice_kind;
  if (battle_try_commit_secondary_choice(BATTLE_SECONDARY_PANEL_KIND, 0u,
                                         BATTLE_SECONDARY_SOURCE_GROUP,
                                         choice_kind & 0xffu) == 0u) {
    battle_queue_frontend_cue(0x107u);
    return;
  }

  BATTLE_SECONDARY_SAVED_GROUP = BATTLE_SECONDARY_SOURCE_GROUP;
  BATTLE_SECONDARY_SAVED_PAGE_BASE = BATTLE_SECONDARY_PAGE_BASE;
  BATTLE_SECONDARY_SAVED_CURSOR = BATTLE_SECONDARY_CURSOR_INDEX;
  battle_queue_frontend_cue(0x103u);
  BATTLE_SELECTION_LOCKED = 1u;
  BATTLE_SELECTION_CURSOR_DIRTY = 0u;
  BATTLE_SELECTION_ROOT_STATE = 3u;
}
