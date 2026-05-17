#ifndef BOF3_SRC_CORE_CALLBACK_SCHEDULER_INTERNAL_H
#define BOF3_SRC_CORE_CALLBACK_SCHEDULER_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*Bof3CallbackEntry)(void);

typedef struct GameCallbackSlot {
  u16               state;
  u16               countdown;
  Bof3CallbackEntry callback;
  s32               thread_id;
  u32               unk_0c;
  u32               open_arg;
  u8                pad_14[0x30];
  u32               open_arg_2;
  u8                pad_48[0x38];
} GameCallbackSlot;

enum {
  GAME_CALLBACK_SLOT_STATE_EMPTY = 0,
  GAME_CALLBACK_SLOT_STATE_YIELD = 1,
  GAME_CALLBACK_SLOT_STATE_OPEN = 2,
  GAME_CALLBACK_SLOT_STATE_SWITCH = 4,
  GAME_CALLBACK_SLOT_STATE_IDLE = 0x7f,
};

#define GAME_CALLBACK_FORCE_SWITCH ((s32)0xff000000u)
#define GAME_CALLBACK_SLOTS VPTR(GameCallbackSlot, 0x80143b40u)
#define GAME_CALLBACK_CURSOR \
  VPTR(GameCallbackSlot*, 0x80143d40u)
#define GAME_CALLBACK_END VPTR(GameCallbackSlot, 0x80143d40u)

s32 func_8017ed9c(Bof3CallbackEntry callback, u32 open_arg, u32 open_arg_2);
s32 func_8017edac(s32 thread_id);

#endif
