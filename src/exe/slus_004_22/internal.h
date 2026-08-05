#ifndef SLUS_004_22_INTERNAL_H
#define SLUS_004_22_INTERNAL_H

#include "bof3/bof3.h"
#include "callback/scheduler.h"
#include "data/equipment.h"
#include "loader/emi.h"
#include "frontend/state.h"

#include "symbols/symbols.h"

typedef struct GameCallbackSlot {
  u16               state;
  u16               countdown;
  GameCallbackEntry callback;
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

extern s16 D_8018B308;
extern s16 D_8018B310;
extern s16 D_8018B314;
extern s16 D_8018B318;
extern s16 D_8018B31C;
extern s16 D_8018B320;
extern s16 D_8018B324;
extern s16 D_8018B328;
extern s16 D_8018B32C;
extern s16 D_8018B330;
extern s16 D_8018B338;
extern s16 D_8018B33C;
extern s16 D_8018B340;
extern s16 D_8018B344;
extern s16 D_8018B348;
extern s16 D_8018B34C;
extern s16 D_8018B350;
extern s16 D_8018B354;
extern s16 D_8018B358;
extern s16 D_8018B360;
extern s16 D_8018B364;
extern s16 D_8018B368;
extern s16 D_8018B36C;
extern s16 D_8018B370;
extern s16 D_8018B374;
extern s16 D_8018B378;
extern s16 D_8018B37C;
extern s16 D_8018B380;
extern s16 D_8018B388;
extern s16 D_8018B38C;
extern s16 D_8018B390;
extern s16 D_8018B394;
extern s16 D_8018B398;
extern s16 D_8018B39C;
extern s16 D_8018B3A0;
extern s16 D_8018B3A4;
extern s16 D_8018B30C;
extern s16 D_8018B3D8;
extern s16 D_8018B3DC;
extern s16 D_8018B3E4;
extern u16 D_8018B3E8;
extern u16 D_8018B3EC;
extern u16 D_8018B3F0;
extern s16 D_8018B3F4;
extern s16 D_8018B404;
extern s16 D_8018B408;
extern s8  D_8018232A;
extern void (*D_8018232C[])(u32);
extern s32 D_8018E140[];

extern const ItemObject      ITEM_OBJECTS[];
extern const WeaponObject    WEAPON_OBJECTS[];
extern const ArmorObject     ARMOR_OBJECTS[];
extern const AccessoryObject ACCESSORY_OBJECTS[];
extern const KeyItemObject   KEY_ITEM_OBJECTS[];

bool emi_header_is_valid(const void* header, size_t size);
u32  emi_next_payload_offset(u32 current_offset, u32 current_size);
void emi_build_entry_lbas(u32 base_lba, const EmiTocEntry* entries,
                          size_t entry_count, u32* entry_lbas);
u32  emi_slot_to_lba(const u32* slot_lba_table, size_t slot_count, u32 slot_id);

void game_front_local_mode_callback_loop(void);

#define GAME_CALLBACK_FORCE_SWITCH ((s32)0xff000000u)
#define GAME_CALLBACK_SLOTS   PSX_PTR(volatile GameCallbackSlot, 0x80143b40u)
#define GAME_CALLBACK_CURSOR  PSX_PTR(volatile GameCallbackSlot*, 0x80143d40u)
#define GAME_CALLBACK_END     PSX_PTR(volatile GameCallbackSlot, 0x80143d40u)
#define GAME_FRONT_LOCAL_MODE PSX_PTR(volatile u16, 0x80143c90u)

/* Fixed-address EMI stream data. The raw literals live only here; function
 * bodies reference these named accessors. */
#define EMI_STREAM_INDEX_HINT PSX_REF(volatile u8, 0x80145024u)
#define EMI_UNK_80190308      PSX_PTR(volatile s16, 0x80190308u)
#define EMI_UNK_8018E7EE      PSX_REF(s16, 0x8018e7eeu)
#define EMI_UNK_8018E264      PSX_REF(u8, 0x8018e264u)
#define EMI_UNK_8018DBFE      PSX_PTR(volatile u16, 0x8018dbfeu)
#define EMI_UNK_8018DBF8      PSX_PTR(volatile s16, 0x8018dbf8u)
#define EMI_UNK_8018E258      PSX_REF(s32, 0x8018e258u)
#define EMI_UNK_8018E250      PSX_REF(s32, 0x8018e250u)
#define EMI_UNK_8018DC02      PSX_PTR(volatile s16, 0x8018dc02u)
#define EMI_UNK_8018DC00      PSX_PTR(volatile s16, 0x8018dc00u)
#define EMI_UNK_8018DC04      PSX_PTR(volatile s16, 0x8018dc04u)
#define EMI_UNK_8018E25C      PSX_REF(s32, 0x8018e25cu)
#define EMI_UNK_8018E8C8      PSX_PTR(volatile s16, 0x8018e8c8u)
#define EMI_UNK_8018E8CA      PSX_PTR(volatile s16, 0x8018e8cau)
#define EMI_UNK_8018E0E8      PSX_PTR(volatile u8, 0x8018e0e8u)

#endif
