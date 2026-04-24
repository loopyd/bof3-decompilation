#ifndef BOF3_SRC_CORE_CALLBACK_SCHEDULER_INTERNAL_H
#define BOF3_SRC_CORE_CALLBACK_SCHEDULER_INTERNAL_H

#include "bof3/core/callback_scheduler.h"

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
  BOF3_GAME_CALLBACK_SLOT_STATE_EMPTY = 0,
  BOF3_GAME_CALLBACK_SLOT_STATE_YIELD = 1,
  BOF3_GAME_CALLBACK_SLOT_STATE_OPEN = 2,
  BOF3_GAME_CALLBACK_SLOT_STATE_SWITCH = 4,
  BOF3_GAME_CALLBACK_SLOT_STATE_IDLE = 0x7f,
};

#define BOF3_GAME_CALLBACK_FORCE_SWITCH ((s32)0xff000000u)
#define BOF3_GAME_CALLBACK_SLOTS ((volatile GameCallbackSlot*)0x80143b40u)
#define BOF3_GAME_CALLBACK_CURSOR \
  ((volatile GameCallbackSlot* volatile*)0x80143d40u)
#define BOF3_GAME_CALLBACK_END ((volatile GameCallbackSlot*)0x80143d40u)

#if defined(__GNUC__)
#define BOF3_NO_SIBLING_CALLS \
  __attribute__((optimize("no-optimize-sibling-calls")))
#else
#define BOF3_NO_SIBLING_CALLS
#endif

#include "bof3/original_symbols.h"

/* does: spawns one callback thread from the current slot metadata.
 * @source: 0x8017ed9c
 */
s32 func_8017ed9c(Bof3CallbackEntry callback, u32 open_arg, u32 open_arg_2);

/* does: closes one callback thread by id.
 * @source: 0x8017edac
 */
s32 func_8017edac(s32 thread_id);

/* does: yields or switches to the requested thread id.
 * @source: 0x8017edbc
 */
void func_8017edbc(s32 thread_id);

#endif
