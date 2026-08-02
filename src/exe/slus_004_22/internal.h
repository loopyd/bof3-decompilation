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

extern const ItemObject      ITEM_OBJECTS[];
extern const WeaponObject    WEAPON_OBJECTS[];
extern const ArmorObject     ARMOR_OBJECTS[];
extern const AccessoryObject ACCESSORY_OBJECTS[];
extern const KeyItemObject   KEY_ITEM_OBJECTS[];

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
