#ifndef SLUS_004_22_INTERNAL_H
#define SLUS_004_22_INTERNAL_H

#include "bof3/bof3.h"

typedef struct KeyItemObject {
  u8 name[0x0c];
  u8 unknown_0c[4];
} KeyItemObject;

typedef struct ItemObject {
  u8  name[0x0c];
  u8  flags;
  u8  unknown_0d[3];
  u16 price;
} ItemObject;

typedef struct WeaponObject {
  u8  name[0x0c];
  u8  equipability;
  u8  unknown_0d[2];
  u8  element;
  u8  weight;
  u8  unknown_11;
  u8  power;
  u8  unknown_13[3];
  u16 price;
} WeaponObject;

typedef struct ArmorObject {
  u8  name[0x0c];
  u8  equipability;
  u8  unknown_0d;
  u8  equip_type;
  u8  weight;
  u8  power;
  u8  unknown_11[3];
  u16 price;
} ArmorObject;

typedef struct AccessoryObject {
  u8  name[0x0c];
  u8  equipability;
  u8  unknown_0d[2];
  u8  weight;
  u8  unknown_10[2];
  u16 price;
} AccessoryObject;

extern const ItemObject      ITEM_OBJECTS[];
extern const WeaponObject    WEAPON_OBJECTS[];
extern const ArmorObject     ARMOR_OBJECTS[];
extern const AccessoryObject ACCESSORY_OBJECTS[];
extern const KeyItemObject   KEY_ITEM_OBJECTS[];

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

#define GAME_CALLBACK_FORCE_SWITCH ((s32)0xff000000u)
#define GAME_CALLBACK_SLOTS        VPTR(GameCallbackSlot, 0x80143b40u)
#define GAME_CALLBACK_CURSOR       VPTR(GameCallbackSlot*, 0x80143d40u)
#define GAME_CALLBACK_END          VPTR(GameCallbackSlot, 0x80143d40u)

typedef struct EmiActiveEntry {
  u32  active_lba;
  u32  remaining_size;
  u32  load_arg;
  u32  first_word;
  u16  type;
  bool header_mode;
} EmiActiveEntry;

typedef struct EmiLoaderEntry {
  u32 unknown_00;
  u32 source;
  u32 destination;
  u32 alternate_destination;
  s16 resource_id;
  u16 flags;
} EmiLoaderEntry;

bool emi_header_is_valid(const void* header, size_t size);
u32  emi_next_payload_offset(u32 current_offset, u32 current_size);
void emi_build_entry_lbas(u32 base_lba, const EmiTocEntry* entries,
                          size_t entry_count, u32* entry_lbas);
u32  emi_slot_to_lba(const u32* slot_lba_table, size_t slot_count, u32 slot_id);
#define GAME_FRONT_EFFECT_BUSY VPTR(u16, 0x80143c40u)
#define GAME_FRONT_LOCAL_MODE  VPTR(u16, 0x80143c90u)

void game_front_local_mode_callback_loop(void);

#endif
