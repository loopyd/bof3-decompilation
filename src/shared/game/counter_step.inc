/* Counter-step template: writes a fixed mode byte then steps a u16 counter.
 * Byte-identical across area008/area026 (see LESSONS.md).
 *
 * Usage:
 *   #include "shared/game/counter_step.inc"
 *   COUNTER_ADD(func_801F4578, counter_extern, flag_extern)
 *   COUNTER_SUB(func_801F45A0, counter_extern, flag_extern)
 */
#define COUNTER_ADD(func, counter, flag) \
  void func(void) { \
    u16 count_; \
    count_ = (counter); \
    (flag) = 2; \
    (counter) = (u16)(count_ + 0x14); \
  }

#define COUNTER_SUB(func, counter, flag) \
  void func(void) { \
    u16 count_; \
    count_ = (counter); \
    (flag) = 2; \
    (counter) = (u16)(count_ - 0x14); \
  }
