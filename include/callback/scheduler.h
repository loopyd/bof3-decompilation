#ifndef CALLBACK_SCHEDULER_H
#define CALLBACK_SCHEDULER_H

#include "base/types.h"
#include "memory/access.h"

typedef void (*CallbackEntry)(void);

typedef struct CallbackSlot {
  u16           state;
  u16           countdown;
  CallbackEntry callback;
  s32           thread_id;
  u32           unk_0c;
  u32           open_arg;
  u8            pad_14[0x30];
  u32           open_arg_2;
  u8            pad_48[0x38];
} CallbackSlot;

enum {
  CALLBACK_SLOT_STATE_EMPTY = 0,
  CALLBACK_SLOT_STATE_YIELD = 1,
  CALLBACK_SLOT_STATE_OPEN = 2,
  CALLBACK_SLOT_STATE_SWITCH = 4,
  CALLBACK_SLOT_STATE_IDLE = 0x7f,
};

#define CALLBACK_FORCE_SWITCH ((s32)0xff000000u)

#define g_CallbackSlots  PSX_PTR(volatile CallbackSlot, 0x80143b40u)
#define g_CallbackCursor PSX_PTR(volatile CallbackSlot*, 0x80143d40u)
#define g_CallbackEnd    PSX_PTR(volatile CallbackSlot, 0x80143d40u)

#define CbSchedInit     func_8014B73C
#define CbSchedRegister func_8014B854
#define CbSchedSetCount func_8014B87C
#define CbSchedTick     func_8014B8B0

#endif
