/* COUNTER_ADVANCE and COUNTER_RETREAT read a u16 counter, write mode 2,
 * then store the counter plus or minus 0x14.
 *
 * Usage:
 *   #include "internal.h"
 *   #include "game/counter_step.h"
 *   COUNTER_ADVANCE(advanceCounter2, counter_extern, flag_extern)
 *   COUNTER_RETREAT(retreatCounter2, counter_extern, flag_extern)
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
