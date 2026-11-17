#ifndef BOF3_PLCHAR_WORK_BYTE_B_STEP_H
#define BOF3_PLCHAR_WORK_BYTE_B_STEP_H

#include "base/types.h"

/* Pointer-cell arguments are intentionally re-read. Helpers are direct names.
 * Each generated function owns its u8 step and done locals.
 */
#define PLCHAR_DEFINE_WORK_BYTE_B_STEP( \
    function_name, work_cell, record_cell, state_byte, ready, request, update) \
void function_name(void) \
{ \
  u8 step; \
  u8 done; \
  step = (work_cell)[0xB]; \
  done = 0; \
  if (step != 0) { \
    if (step == 1) { \
      if ((ready)()) { \
        (request)((work_cell)[8]); \
        (work_cell)[0xB] = 2; \
      } \
    } else { \
      (update)(); \
      if ((state_byte) == 0) { \
        done = 1; \
      } \
    } \
  } else { \
    if ((ready)()) { \
      done = 1; \
    } \
  } \
  if (done) { \
    (work_cell)[0x2B] = 0; \
    (record_cell)[0x12B] = 0; \
  } \
}

#endif
