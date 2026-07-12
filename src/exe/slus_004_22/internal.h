#ifndef BOF3_SLUS_004_22_INTERNAL_H
#define BOF3_SLUS_004_22_INTERNAL_H

#include "bof3/bof3.h"

/* LOGO.EXE is loaded independently; this call targets its reviewed entry
 * address rather than linking LOGO.EXE implementation into SLUS_004.22. */
DEFINE_FUNC_AT(void, func_801ce758, 0x801ce758, (void));

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
#define GAME_CALLBACK_SLOTS        VPTR(GameCallbackSlot, 0x80143b40u)
#define GAME_CALLBACK_CURSOR       VPTR(GameCallbackSlot*, 0x80143d40u)
#define GAME_CALLBACK_END          VPTR(GameCallbackSlot, 0x80143d40u)

s32 func_8017ed9c(Bof3CallbackEntry callback, u32 open_arg, u32 open_arg_2);
s32 func_8017edac(s32 thread_id);

typedef enum RuntimePathKind {
  RUNTIME_PATH_EMI = 0,
  RUNTIME_PATH_STR = 1,
  RUNTIME_PATH_PSX_EXE = 2,
  RUNTIME_PATH_OTHER = 3,
} RuntimePathKind;

struct SlotTableEntry {
  u32             slot_id;
  u32             disc_lba;
  const char*     relative_path;
  RuntimePathKind kind;
};

extern const struct SlotTableEntry g_slot_table[];
extern const size_t                g_slot_table_count;

#define EMI_SECTOR_SIZE  0x800u
#define EMI_MAGIC_OFFSET 0x08u
#define EMI_MAGIC_SIZE   8u

typedef struct EmiTocEntry {
  u32 size;
  u32 load_arg;
  u32 first_word;
  u16 type;
  u16 unk;
} EmiTocEntry;

typedef struct EmiActiveEntry {
  u32  active_lba;
  u32  remaining_size;
  u32  load_arg;
  u32  first_word;
  u16  type;
  bool header_mode;
} EmiActiveEntry;

bool emi_header_is_valid(const void* header, size_t size);
u32  emi_next_payload_offset(u32 current_offset, u32 current_size);
void emi_build_entry_lbas(u32 base_lba, const EmiTocEntry* entries,
                          size_t entry_count, u32* entry_lbas);
void emi_stream_init_slot(u32 slot_id);
u32  emi_slot_to_lba(const u32* slot_lba_table, size_t slot_count, u32 slot_id);
void func_8016728c(u8 index, u8 family);

#define GAME_FRONT_EFFECT_BUSY VPTR(u16, 0x80143c40u)
#define GAME_FRONT_LOCAL_MODE  VPTR(u16, 0x80143c90u)

void game_front_local_mode_callback_loop(void);

#endif
