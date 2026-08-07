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

/* @source 0x8018B308 @kind unknown */
extern s16 D_8018B308;
/* @source 0x8018B310 @kind unknown */
extern s16 D_8018B310;
/* @source 0x8018B314 @kind unknown */
extern s16 D_8018B314;
/* @source 0x8018B318 @kind unknown */
extern s16 D_8018B318;
/* @source 0x8018B31C @kind unknown */
extern s16 D_8018B31C;
/* @source 0x8018B320 @kind unknown */
extern s16 D_8018B320;
/* @source 0x8018B324 @kind unknown */
extern s16 D_8018B324;
/* @source 0x8018B328 @kind unknown */
extern s16 D_8018B328;
/* @source 0x8018B32C @kind unknown */
extern s16 D_8018B32C;
/* @source 0x8018B330 @kind unknown */
extern s16 D_8018B330;
/* @source 0x8018B338 @kind unknown */
extern s16 D_8018B338;
/* @source 0x8018B33C @kind unknown */
extern s16 D_8018B33C;
/* @source 0x8018B340 @kind unknown */
extern s16 D_8018B340;
/* @source 0x8018B344 @kind unknown */
extern s16 D_8018B344;
/* @source 0x8018B348 @kind unknown */
extern s16 D_8018B348;
/* @source 0x8018B34C @kind unknown */
extern s16 D_8018B34C;
/* @source 0x8018B350 @kind unknown */
extern s16 D_8018B350;
/* @source 0x8018B354 @kind unknown */
extern s16 D_8018B354;
/* @source 0x8018B358 @kind unknown */
extern s16 D_8018B358;
/* @source 0x8018B360 @kind unknown */
extern s16 D_8018B360;
/* @source 0x8018B364 @kind unknown */
extern s16 D_8018B364;
/* @source 0x8018B368 @kind unknown */
extern s16 D_8018B368;
/* @source 0x8018B36C @kind unknown */
extern s16 D_8018B36C;
/* @source 0x8018B370 @kind unknown */
extern s16 D_8018B370;
/* @source 0x8018B374 @kind unknown */
extern s16 D_8018B374;
/* @source 0x8018B378 @kind unknown */
extern s16 D_8018B378;
/* @source 0x8018B37C @kind unknown */
extern s16 D_8018B37C;
/* @source 0x8018B380 @kind unknown */
extern s16 D_8018B380;
/* @source 0x8018B388 @kind unknown */
extern s16 D_8018B388;
/* @source 0x8018B38C @kind unknown */
extern s16 D_8018B38C;
/* @source 0x8018B390 @kind unknown */
extern s16 D_8018B390;
/* @source 0x8018B394 @kind unknown */
extern s16 D_8018B394;
/* @source 0x8018B398 @kind unknown */
extern s16 D_8018B398;
/* @source 0x8018B39C @kind unknown */
extern s16 D_8018B39C;
/* @source 0x8018B3A0 @kind unknown */
extern s16 D_8018B3A0;
/* @source 0x8018B3A4 @kind unknown */
extern s16 D_8018B3A4;
/* @source 0x8018B30C @kind unknown */
extern s16 D_8018B30C;
/* @source 0x8018B3D8 @kind unknown */
extern s16 D_8018B3D8;
/* @source 0x8018B3DC @kind unknown */
extern s16 D_8018B3DC;
/* @source 0x8018B3E4 @kind unknown */
extern s16 D_8018B3E4;
/* @source 0x8018B3E8 @kind unknown */
extern u16 D_8018B3E8;
/* @source 0x8018B3EC @kind unknown */
extern u16 D_8018B3EC;
/* @source 0x8018B3F0 @kind unknown */
extern u16 D_8018B3F0;
/* @source 0x8018B3F4 @kind unknown */
extern s16 D_8018B3F4;
/* @source 0x8018B404 @kind unknown */
extern s16 D_8018B404;
/* @source 0x8018B408 @kind unknown */
extern s16 D_8018B408;
/* @source 0x8018232A @kind unknown */
extern s8  D_8018232A;
/* @source 0x8018232C @kind unknown */
extern void (*D_8018232C[])(u32);
/* @source 0x8018E140 @kind unknown */
extern s32 D_8018E140[];

/* @source 0x801C8964 @kind table */
extern const ItemObject      ITEM_OBJECTS[];
/* @source 0x801C90DC @kind table */
extern const WeaponObject    WEAPON_OBJECTS[];
/* @source 0x801C98A4 @kind table */
extern const ArmorObject     ARMOR_OBJECTS[];
/* @source 0x801C9E7C @kind table */
extern const AccessoryObject ACCESSORY_OBJECTS[];
/* @source 0x801C8FDC @kind table */
extern const KeyItemObject   KEY_ITEM_OBJECTS[];

bool isEmiHeaderValid(const void* header, size_t size);
u32  nextEmiPayloadOffset(u32 current_offset, u32 current_size);
void buildEmiEntryLbas(u32 base_lba, const EmiTocEntry* entries,
                          size_t entry_count, u32* entry_lbas);
u32  emiSlotToLba(const u32* slot_lba_table, size_t slot_count, u32 slot_id);

/* @source 0x8014ED6C @kind unknown */
void frontLocalModeCallbackLoop(void);

#define GAME_CALLBACK_FORCE_SWITCH ((s32)0xff000000u)

/* Fixed-address EMI stream data. The raw literals live only here; function
 * bodies reference these named accessors. */
#define EMI_STREAM_INDEX_HINT PSX_REF(volatile u8, 0x80145024u)

/* @source 0x1F800044 @kind unknown */
extern u8 g_game_work;

#endif
