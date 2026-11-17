#ifndef BOF3_PLCHAR_COUNTDOWN_PROJECTED_CELL_CLAIM_H
#define BOF3_PLCHAR_COUNTDOWN_PROJECTED_CELL_CLAIM_H

#include "base/types.h"

/* Pointer-cell inputs are intentionally re-read; helpers are direct names.
 * Countdown expiry marks a found record and tries projected cells.
 */
#define PLCHAR_DEFINE_COUNTDOWN_PROJECTED_CELL_CLAIM( \
    function_name, work_cell, x_steps, y_steps, record_flags, \
    find_record, claim_cell, poll_ready) \
void function_name(void) \
{ \
  u8* work; \
  u8 route; \
  s32 x; \
  s32 y; \
  s8 slot; \
  u8 claimed; \
  s16 cx; \
  s16 cy; \
  s32 off; \
  s32 y2; \
  (work_cell)[0x0A] -= 1; \
  work = (work_cell); \
  if (work[0x0A] == 0) { \
    route = work[0x08]; \
    x = *(s32*)(work + 0x34) + (x_steps)[route * 2] * 2; \
    y = *(s32*)(work + 0x38) + (y_steps)[route * 2] * 2; \
    y2 = y; \
    slot = (find_record)(x, y, 0); \
    if (slot != -1) { \
      off = slot * 0x98; \
      (record_flags)[off] |= 1; \
    } \
    cx = x >> 16; \
    cy = y >> 16; \
    claimed = (claim_cell)(cx, cy); \
    if ((claimed & 0xFF) == 0) { \
      if ((x & 0xFFFF) != 0) { \
        claimed = (claim_cell)(cx + 1, cy); \
      } \
      if ((claimed & 0xFF) == 0 && (y2 & 0xFFFF) != 0) { \
        (claim_cell)(cx, cy + 1); \
      } \
    } \
    (work_cell)[0x02] += 1; \
  } \
  (poll_ready)(); \
}

#endif
