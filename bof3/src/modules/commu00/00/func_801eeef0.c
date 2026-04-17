#include "internal.h"

/* does: reconciles the visible COMMU00 slot window for one requested row by
 * rate-limiting random slot spawns or clears against the current window
 * counters.
 * @source: 0x801eeef0 FUN_801eeef0
 */
void func_801eeef0(u32 row_index) {
  s32 visible_slot_count;
  u32 current_tick;
  u32 step_budget;
  u32 max_steps;
  u32 step_index;

  row_index &= 0xffu;
  visible_slot_count = (s32)BOF3_COMMU00_VISIBLE_SLOT_COUNT;

  if ((s32)(row_index * 4u) < visible_slot_count) {
    current_tick = BOF3_COMMU00_PROGRESS_COUNTER;
    if ((current_tick - BOF3_COMMU00_WINDOW_ANCHOR_TICK) >= 0x15u) {
      BOF3_COMMU00_WINDOW_ANCHOR_TICK = current_tick - 0x14u;
    }

    if (row_index < 0x14u) {
      max_steps =
          (u32)(((visible_slot_count - (s32)(row_index * 4u)) >> 2) + 1);
      step_budget = (current_tick - BOF3_COMMU00_LAST_WINDOW_STEP_TICK) / 10u;
      if (max_steps < step_budget) {
        step_budget = max_steps;
      }

      step_index = 0u;
      while (step_index < step_budget) {
        if ((row_index + step_index) == 0x14u) {
          break;
        }

        func_801f00d4();
        step_index += 1u;
      }

      if (step_budget != 0u) {
        BOF3_COMMU00_LAST_WINDOW_STEP_TICK = current_tick;
      }
    }

    return;
  }

  current_tick = BOF3_COMMU00_PROGRESS_COUNTER;
  if (visible_slot_count == 0) {
    if ((current_tick - BOF3_COMMU00_LAST_WINDOW_STEP_TICK) >= 0x0bu) {
      BOF3_COMMU00_LAST_WINDOW_STEP_TICK = current_tick - 10u;
    }
    if (row_index < 2u) {
      return;
    }

    step_budget = (current_tick - BOF3_COMMU00_WINDOW_ANCHOR_TICK) / 0x14u;
    if (step_budget == 0u) {
      return;
    }

    step_index = 0u;
    while (step_index < step_budget) {
      if ((row_index - step_index) == 1u) {
        BOF3_COMMU00_WINDOW_ANCHOR_TICK = current_tick;
        return;
      }

      func_801f01f4();
      step_index += 1u;
    }

    BOF3_COMMU00_WINDOW_ANCHOR_TICK = current_tick;
    return;
  }

  if ((current_tick - BOF3_COMMU00_LAST_WINDOW_STEP_TICK) >= 0x0bu) {
    BOF3_COMMU00_LAST_WINDOW_STEP_TICK = current_tick - 10u;
  }
  if ((current_tick - BOF3_COMMU00_WINDOW_ANCHOR_TICK) >= 0x15u) {
    BOF3_COMMU00_WINDOW_ANCHOR_TICK = current_tick - 0x14u;
  }
}
