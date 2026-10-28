#ifndef BOF3_WORLD_WORK_STATE_H
#define BOF3_WORLD_WORK_STATE_H

#include "base/types.h"
#include "memory/scratchpad.h"

#define WORLD_DEFINE_RESET_ADVANCE_STATE(function_name) \
void function_name(void) \
{ \
  u8** slots; \
  u8*  ptr; \
  slots = SPAD_PTR_TABLE(u8); \
  slots[0x11][0x09] = 0; \
  slots[0x11][0x0b] = 0; \
  ptr = slots[0x11]; \
  ptr[1] += 1; \
}

#define WORLD_DEFINE_WORK_HALFWORDS_A0_50(function_name) \
void function_name(void) { \
  SPAD_PTR_SLOT(u16, 0x44)[23] = 0xA0; \
  SPAD_PTR_SLOT(u16, 0x44)[24] = 0x50; \
}

#endif
