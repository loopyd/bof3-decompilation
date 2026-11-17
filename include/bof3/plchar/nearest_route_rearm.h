#ifndef BOF3_PLCHAR_NEAREST_ROUTE_REARM_H
#define BOF3_PLCHAR_NEAREST_ROUTE_REARM_H

#include "base/types.h"

/* Clear record byte 0x12B for scenario 0xF; otherwise choose route 3 or 5.
 * Pointer-cell inputs are intentionally re-read; claim_route is a direct name.
 */
#define PLCHAR_DEFINE_NEAREST_ROUTE_REARM( \
    function_name, scenario, record_cell, work_cell, claim_route) \
void function_name(void) \
{ \
  s32 fromThree; \
  s32 fromFive; \
  if ((scenario) == 0xF) { \
    (record_cell)[0x12B] = 0; \
  } else { \
    fromThree = (work_cell)[8] - 3; \
    if (fromThree < 0) { \
      fromThree = -fromThree; \
    } \
    fromFive = (work_cell)[8] - 5; \
    if (fromFive < 0) { \
      fromFive = -fromFive; \
    } \
    if (fromFive < fromThree) { \
      (work_cell)[0xB] = (claim_route)(5); \
      (work_cell)[3] = 5; \
    } else { \
      (work_cell)[0xB] = (claim_route)(3); \
      (work_cell)[3] = 3; \
    } \
    (work_cell)[9] = 2; \
    (work_cell)[2] = (work_cell)[2] + 1; \
  } \
}

#endif
