#ifndef EMI_GAME_00_INTERNAL_H
#define EMI_GAME_00_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*GameEntry0StateHandler)(void);

typedef struct GameScenarioState {
  s8  scenario_id;
  u8  field_01;
  u8  field_02;
  u8  field_03;
  u8  field_04;
  u8  field_05;
  u16 field_06;
  u16 field_08;
} GameScenarioState;

extern GameScenarioState GAME_SCENARIO_STATE;

typedef struct AbilityObject {
  u8 name[0x0c];
  u8 targeting_flags;
  u8 skill_type;
  u8 cost;
  u8 power;
  u8 element;
  u8 ability_flags;
  u8 control_12[2];
} AbilityObject;

extern const volatile AbilityObject ABILITY_OBJECTS[];

/* Work area struct accessed via scratchpad pointer (0x1F800044) */
struct GameWorkArea {
  u8  flags_00;       /* 0x00 - entity flags */
  u8  unk_01;         /* 0x01 */
  u8  flags_02;       /* 0x02 */
  u8  pad_03[0x02];   /* 0x03-0x04 */
  u8  field_05;       /* 0x05 - region/scene index */
  u8  unk_06;         /* 0x06 */
  u8  unk_07;         /* 0x07 */
  u8  route_index_08; /* 0x08 */
  u8  pad_09[0x02];   /* 0x09-0x0A */
  u8  field_0B;       /* 0x0B */
  u8  pad_0C[0x0C];   /* 0x0C-0x17 */
  s32 unk_18;         /* 0x18 */
  u8  pad_1C[0x0D];   /* 0x1C-0x28 */
  u8  unk_29;         /* 0x29 */
  u8  unk_2A;         /* 0x2A */
  u8  pad_2B[0x09];   /* 0x2B-0x33 */
  s32 coord_x_34;     /* 0x34 */
  s32 coord_y_38;     /* 0x38 */
  u16 counter_3E;     /* 0x3E */
  u8  pad_40[0x09];   /* 0x40-0x48 */
  u32 unk_49;         /* 0x49 */
  u8  pad_4D[0x0F];   /* 0x4D-0x5B */
  u8  flags_5C;       /* 0x5C */
  u8  unk_5D;         /* 0x5D */
  u8  unk_5E;         /* 0x5E */
  u8  unk_5F;         /* 0x5F */
  u8  pad_60[0x10];   /* 0x60-0x6F */
  u8  speed_70;       /* 0x70 */
  u8  pad_71[0x03];   /* 0x71-0x73 */
  u16 anim_state_74;  /* 0x74 */
};

/* Scratchpad work pointer (PS1 hardware register at 0x1F800044) */
#define SCRATCH_WORK VPPTR(struct GameWorkArea, 0x1F800044u)

/* Global work pointer in main exe data section */
#define GLOBAL_WORK_PTR VPPTR(u8, 0x80146250u)

/* Movement/position offset tables in main exe data section */
#define MOVEMENT_OFFSET_0(i)  (*(volatile s32*)(0x80181B94u + (i) * 8))
#define MOVEMENT_OFFSET_1(i)  (*(volatile s32*)(0x80181B98u + (i) * 8))
#define MOVEMENT_THRESHOLD(i) (*(volatile s16*)(0x80181B70u + (i) * 2))

/* ---- RAM globals (DAT_ names match original game data patterns) ---- */

/* @behavior entry-0 main state machine index */
extern vu16 DAT_80143b90;
extern u16  DAT_80143c40;
/* @behavior entry-0 sub-state within current state */
extern u16 DAT_80143b92;
/* @behavior world/phase index for entry-0 world dispatch */
extern u8 DAT_80143bb0;
/* @behavior current world state ID for world/front routing */
extern vu16 DAT_80143f00;
/* @behavior world/front flags: bit0=scenario pending, bit3=alt front mode */
extern vu8 DAT_80143f02;
/* @behavior context selection seed passed to entry-0 ctx init */
extern vu16 DAT_80143f10;
/* @behavior context bundle word A — world/route identifier */
extern vu32 DAT_80143f14;
/* @behavior context bundle word B — secondary selector data */
extern vu32 DAT_80143f18;
/* @behavior context kind byte — dispatch type discriminator */
extern vu8 DAT_80143f1c;
/* @behavior pending request kind — selects next front operation */
extern vu8 DAT_80143f1d;
/* @behavior pending mode after request resolution */
extern u8 DAT_80143f1e;
/* @behavior selection seed for the entry-0 front callback bank */
extern u8 DAT_80143f1f;
/* @behavior active selection id from front-end picker */
extern vu32 DAT_80144fc0;
/* @behavior front-end selection index for menu routing */
extern u8  DAT_80145029;
extern u8  DAT_80145024;
extern u32 DAT_8014502c;
/* @behavior palette stage serial for GPU upload sequencing */
extern vu8 DAT_80145988;
/* @behavior world flags — bit0=pending scenario, bit6=force-reset.
 * UNKNOWN: roles of observed bits 5 and 11. */
extern u16 DAT_8014625a;
extern vu8 DAT_80146256;
/* @behavior flag byte cleared on request 0xFE.
 * UNKNOWN: the flag's owning subsystem. */
extern vu8      DAT_8014832e;
extern u8       DAT_801462e0;
extern u8       DAT_801462e1;
extern u8       DAT_801462e2;
extern u8       DAT_80145e9b;
extern u8       DAT_80145fdb;
extern u8       DAT_8014611b;
extern u8       DAT_8014626c;
extern u8       DAT_8014626d;
extern u8       DAT_8014626e;
extern u8       DAT_8014626f;
extern u8       DAT_80146270;
extern u8       DAT_80148650;
extern u8       DAT_80148651;
extern u8       DAT_80148652;
extern s8       DAT_8014865c;
extern u8       DAT_80149332;
extern const u8 DAT_80181eba[];
extern const u8 DAT_80181ebb[];
/* @behavior signed world-coord X argument for scenario entry */
extern s16 DAT_8014930a;
/* @behavior signed world-coord Y argument for scenario entry */
extern s16      DAT_8014930e;
extern s16      DAT_801492d8;
extern s16      DAT_801492dc;
extern s16      DAT_8014932e;
extern u8       DAT_80146329;
extern u8       DAT_801462e3;
extern u8       DAT_801462e4;
extern u8       DAT_801462f0;
extern u8       DAT_801462ec;
extern u8       DAT_80146325;
extern u32      DAT_80149318;
extern s16      DAT_80149330;
extern u8       DAT_80149333;
extern u8       DAT_8014933e;
extern u8       DAT_8014933f;
extern u32      DAT_801cd954;
extern const s8 DAT_801c7b74[];
extern vu16     DAT_801490a4;

extern const GameEntry0StateHandler DAT_801c7b08[];
extern const GameEntry0StateHandler DAT_801c7b14[];
#define GAME_ALT_FRONT_CALLBACK_TABLE DAT_801c7b08
#define GAME_SELECTION_CALLBACK_TABLE DAT_801c7b14
extern const GameEntry0StateHandler DAT_801c7b44[];
extern const GameEntry0StateHandler DAT_801c7b54[];
extern const GameEntry0StateHandler DAT_801c7b7c[];
extern const GameEntry0StateHandler DAT_801c7b88[];
extern const GameEntry0StateHandler DAT_801c7b98[];
extern const GameEntry0StateHandler DAT_801c7ba4[];
extern const GameEntry0StateHandler DAT_801c7bb0[];

/* @behavior clears one local GAME entry-0 record slot by index.
 * @source 0x801960c0
 */
void func_801960c0(u8 record_index);

/* @behavior seeds the shared callback/frame dispatch prologue before the entry-0
 * callback tables begin running.
 * @source 0x8014ba04
 */
void func_8014ba04(void);

/* @behavior begins one shared front-end frame/update slice.
 * @source 0x80158e50
 */
void func_80158e50(void);

/* @behavior finalizes one shared front-end frame/update slice.
 * @source 0x80158c80
 */
void func_80158c80(void);

/* @behavior runs one selection-side post-dispatch update slice.
 * @source 0x80198cac
 */
void func_80198cac(void);

/* @behavior resets the entry-0 front script/runtime bank for the requested mode.
 * @source 0x801c1400
 */
void func_801c1400(u32 mode);

/* @behavior copies the active front selector/context bundle into the entry-0 local
 * runtime state.
 * @source 0x8019fa28
 */
void func_8019fa28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @behavior copies the shared CPU-side palette bank before the corresponding VRAM
 * upload path.
 * @source 0x8014e284
 */
void func_8014e284(void);

/* @behavior begins streaming the currently selected SCENA pack for the seeded
 * scenario state.
 * @source 0x801a7804
 */
void func_801a7804(void);

/* @behavior enters the loaded scenario-local dispatch path after the SCENA loader
 * completes.
 * @source 0x801a782c
 */
void func_801a782c(void);

/* @behavior ticks the shared world/front waiting path while the scenario loader is
 * still pending.
 * @source 0x801992b8
 */
void func_801992b8(void);

/* @behavior returns a pointer into one of two sprite-rect tables indexed by
 * sprite_id * 4, with the table chosen by flags & 1.
 * @source 0x801af270
 */
u8* func_801af270(u8 sprite_id, u8 flags);

/* @behavior draws one sprite by filling a GT quad primitive from a rect-table
 * entry, selecting CLUT by flags & 2, then appending to the OT.
 * @source 0x801af2a0
 */
void func_801af2a0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* @behavior iterates a packed sprite-record table and draws each sprite via
 * func_801af2a0 with signed offsets shifted by 3 applied to base coords.
 * @source 0x801af390
 */
void func_801af390(s16 base_x, s16 base_y, const u8* record_table, u8 flags);

/* @behavior computes a screen-space position from an entity's offset-adjusted
 * coordinates; returns the result as a signed 16-bit value.
 * @source 0x80154f28
 */
s16 func_80154f28(s32 x, s32 y);

/* @behavior external no-argument update called after movement completion checks.
 * UNKNOWN: its owning subsystem and return-value meaning.
 * @source 0x8014d978
 */
u8 func_8014d978(void);

void func_8019611c(void);
void func_80196ffc(void);
void func_80197068(void);
void func_801970ec(void);
void func_801971e8(void);
void func_80197378(void);
void func_801975e4(void);
void func_801979d4(void);
void func_80197a24(void);
void func_80197a60(void);
void func_80197aa4(void);
void func_80197c1c(void);
void func_80197e54(void);
void func_80197efc(void);
void func_80198170(void);
void func_801981b4(void);
void func_801981d4(void);
void func_80198234(void);
void func_801984ac(void);
void func_80198744(void);
void func_80198904(void);
void func_80198ac4(void);
void func_80199230(void);
void func_80198f1c(void);
void func_801990d0(void);
void func_801991b8(void);
void func_801bedd0(void);
s32  func_801bee5c(void);
void func_801a06d8(void);
u8   func_801bf11c(void);
void func_801bf8e0(void);
void func_801bfac4(void);
u8   func_801bf78c(void);
u8   func_801bdb7c(u8 mode);
void func_8016728c(u32 slot_id, u32 mode);
void func_8015d4f8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void func_8015d404(u8 arg0, u8 arg1);
void func_801a7704(u8 scenario_index);
void func_801651dc(s32 ability_id, s32 character_id, s32 arg2, s32 arg3);
void func_80164a44(volatile void* character_state);

void func_8019faa0(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_80198bc4(u32 arg0);
void func_801a0048(s16 a, s16 b);
void func_801b3ccc(u32 arg0);

#endif
