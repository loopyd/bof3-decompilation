#ifndef BOF3_WORLD_GATE_STATUS_H
#define BOF3_WORLD_GATE_STATUS_H

#include "base/types.h"

/* Object arguments are scalar identifiers; branch arguments are integer constants. */
#define WORLD_DEFINE_GATE_STATUS(function_name, gate_byte, status_halfword, when_nonzero, when_zero) \
void function_name(void) { \
  s32 value; \
 \
  value = gate_byte; \
  if (value != 0) { \
    value = when_nonzero; \
  } else { \
    value = when_zero; \
  } \
  status_halfword = value; \
}

#endif
