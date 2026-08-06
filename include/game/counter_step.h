/* Counter-step template: writes a fixed mode byte then steps a u16 counter.
 * Byte-identical across area008/area026 (see docs/agents/lessons.md).
 *
 * Usage:
 *   #include "internal.h"
 *   #include "game/counter_step.h"
 *   COUNTER_ADVANCE(world00_area008_counter2_advance, counter_extern, flag_extern)
 *   COUNTER_RETREAT(world00_area008_counter2_retreat, counter_extern, flag_extern)
 */

#ifndef GAME_COUNTER_STEP_H
#define GAME_COUNTER_STEP_H

#include "base/types.h"

#define COUNTER_ADVANCE(func, counter, flag)                                   \
  void func(void) {                                                            \
    u16 count_;                                                                \
    count_ = (counter);                                                        \
    (flag) = 2;                                                                \
    (counter) = (u16)(count_ + 0x14);                                          \
  }

#define COUNTER_RETREAT(func, counter, flag)                                   \
  void func(void) {                                                            \
    u16 count_;                                                                \
    count_ = (counter);                                                        \
    (flag) = 2;                                                                \
    (counter) = (u16)(count_ - 0x14);                                          \
  }

#endif
