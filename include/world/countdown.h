#ifndef BOF3_WORLD_COUNTDOWN_H
#define BOF3_WORLD_COUNTDOWN_H

#include "base/types.h"
#include "memory/scratchpad.h"

/* The mode argument is an integer constant; the branch tests the old byte value. */
#define WORLD_DEFINE_COUNTDOWN_TO_MODE(function_name, when_spent_mode) \
void function_name(void) { \
  u8** slots; \
  u8* work; \
  u8 value; \
 \
  slots = SPAD_PTR_TABLE(u8); \
  work = slots[0x11]; \
  value = work[0x09]; \
  work[0x09] = value + 0xFF; \
  if (value == 0) { \
    slots = SPAD_PTR_TABLE(u8); \
    slots[0x11][0x01] = when_spent_mode; \
  } \
}

#endif
