#ifndef EMI_GAME_00_INTERNAL_H
#define EMI_GAME_00_INTERNAL_H

#include "bof3/bof3.h"
#include "panel/task.h"
#include "battle/ability.h"
#include "frontend/state.h"
#include "frontend/selection.h"
#include "gpu/palette.h"

typedef void (*GameEntry0StateHandler)(void);

/* @source 0x801454F4 @kind unknown */
extern u8 D_801454F4[3][3];

/* @source 0x80195ED4 @kind table */
extern GameEntry0StateHandler workStateHandlerTable[5];

/* @source 0x801C84A4 @kind table */
extern GameEntry0StateHandler stateHandlerTable[];

/* @source 0x801448EA @kind unknown */
extern s8 D_801448EA;
/* @source 0x801448EB @kind unknown */
extern s8 D_801448EB;
/* @source 0x801C84AC @kind table */
extern u8 D_801C84AC[];
/* @source 0x801C84BC @kind table */
extern GameEntry0StateHandler D_801C84BC[];

/* @source 0x80146260 @kind unknown */
extern u8 D_80146260;
/* @source 0x80146261 @kind unknown */
extern u8 D_80146261;
/* @behavior two-word direction record table used by the panel position
 * helpers func_801C090C / func_801C0758: each eight-byte record holds an x word
 * at +0 and a y word at +4. */
/* @source 0x801CD47C @kind table */
extern u8 D_801CD47C[];
/* @behavior one-byte record-index table indexed by D_801462EC; func_801BEE5C
 * compares a work record's route byte at 0x08 against its entry. */
/* @source 0x801CD478 @kind table */
extern u8 D_801CD478[];
/* @source 0x801CD480 @kind table */
extern u8 D_801CD480[];
/* @behavior three-byte list of record types accepted by the entry-0 record
 * gate func_801B55C0. */
/* @source 0x801CD1C8 @kind table */
extern u8 D_801CD1C8[];
/* @source 0x801CD49C @kind table */
extern GameEntry0StateHandler D_801CD49C[];

/* @source 0x801C80C8 @kind table */
extern GameEntry0StateHandler D_801C80C8[];
/* @source 0x801CD308 @kind table */
extern GameEntry0StateHandler D_801CD308[];
typedef struct GameEntry0HandlerSet {
  GameEntry0StateHandler handlers[5];
} GameEntry0HandlerSet;
typedef struct GameEntry0DispatchSet {
  GameEntry0StateHandler handlers[6];
} GameEntry0DispatchSet;

/* @source 0x80195F10 @kind table */
extern GameEntry0HandlerSet D_80195F10;
/* @source 0x80195F44 @kind table */
extern GameEntry0DispatchSet D_80195F44;

/* Main-RAM bitmap geometry referenced by this overlay (main exe data
 * segment, ref'd by overlay). */
/* @source 0x80104000 @kind unknown */
extern u8 D_80104000;
/* @source 0x80104001 @kind unknown */
extern u8 D_80104001;

/* Shared front-end record tables referenced by this overlay (main exe data
 * segment, ref'd by overlay): eight stride-8 local record slots at
 * 0x801457A8 (kind byte at +0, argument byte at +1, flags byte at +3, per-slot
 * counter word at +4) and the 60-entry stride-8 active-record table at
 * 0x801455C8 (presence byte at +0, owning slot index + 1 at +1). The front-end
 * state block that contains the slot table starts at 0x801448D8, so the slot
 * table is its 0xED0-byte offset. */
/* @source 0x801448D8 @kind unknown */
extern u8 D_801448D8[];
/* @source 0x801455C8 @kind unknown */
extern u8 D_801455C8[];
/* @source 0x801455C9 @kind unknown */
extern u8 D_801455C9[];
/* @source 0x801457A8 @kind unknown */
extern u8 D_801457A8[];
/* @source 0x801457A9 @kind unknown */
extern u8 D_801457A9[];
/* @source 0x801457AB @kind unknown */
extern u8 D_801457AB[];
/* @source 0x801457AC @kind unknown */
extern u8 D_801457AC[];

struct GameWorkArea;

/* Shared readiness helper of the main executable, referenced by this overlay. */
u8 func_8014DAEC(void);
/* NOTE: the shared main-executable service func_8014D290 is deliberately NOT
 * declared here. Its callers disagree on the signature - eight sources call it
 * with no arguments (and include/bof3/battle/battle03_internal.h declares
 * `void func_8014D290(void)`), while the pre-existing exact lift
 * src/bof3/ui/func_801A0514.c declares it with a struct GameWorkArea* parameter
 * and materialises that argument. An empty-parameter-list declaration here
 * conflicts with that file's local prototype and breaks the target build, so the
 * one declaration is left file-local to each caller until the callee's own bytes
 * settle the signature (parent-owned type task). */

/* Shared services of the main executable, referenced by this overlay through
 * fixed addresses: func_80154F78 is the route-parameterised position query whose
 * 16-bit result the route helpers compare against the 0x40 bound, and
 * func_8015B5A8 clears one flag bit of the shared main-RAM flag bank at
 * 0x80144F28. */
s32 func_80154F78(s32 arg0, s32 arg1, u8 arg2);
void func_8015B5A8(void* flag_bank, u8 bit_index);

/* Routines of the concurrently resident world/area overlay (the emi/worldNN/areaNNN/II
 * family, load address 0x801F2C00), referenced by this overlay through fixed
 * addresses. */
void func_801F53D4(void);
void func_801F5918(void);
void func_801F45BC(void);
void func_801F4B00(void);

int func_801C0F9C(s32 arg0);
void countActiveRecordsForSlot(u8 arg0, s32* counter);
void func_801C1F4C(u8 arg0);
void func_801C2438(void);
void func_801C2538(void);
void func_801C2710(void);
/* Shared tint-fade step of this overlay: the three signed tint channels of the
 * scratchpad work record (0x5D-0x5F) are ramped by the signed byte argument,
 * clamped at -0x40, and a non-zero byte result reports the completed fade, at
 * which point the palette row selected by the work record's field_05 byte is
 * restored and the tint is cleared. */
u8 func_801C27E0(s8 arg0);
int func_801C5474(void);
void func_801C4D1C(void);
void func_801C4D58(void);
u8 func_801C4E14(void);
u8 func_801C4EC4(void);
u8 func_801C4F88(void);
u8 func_801C4FE0(void);
void func_801C51B0(void);
u8 func_801C520C(void);
/* Reports success as a byte flag: the callee's own bodies return only 0 or 1 and
 * both of its callers mask the result with 0xFF before testing it. */
u8 func_801C539C(void);
s16 func_801BDCF8(void);
u8 func_801BDD58(s32 x, s32 y, s16 reference, s32 range,
                 struct GameWorkArea* work);

void func_801ACF2C(void);
void func_801AD0EC(void);
void func_801AD184(void);
void func_801AD218(void);
void func_801AD2CC(void);
void func_801AD3B4(void);

/* Publishes handler index 2 into the work-area handler index at 0x03 once the
 * shared work-area service func_801C4E14 reports success. */
void advanceHandlerIndexTo2WhenReady(void);
/* Publishes handler index 3 into the work-area handler index at 0x03 once the
 * shared work-area service func_801C4EC4 reports success. */
void advanceHandlerIndexTo3WhenReady(void);
/* Publishes handler index 4 into the work-area handler index at 0x03 once the
 * shared work-area service func_801C4F88 reports success. */
void advanceHandlerIndexTo4WhenReady(void);
/* Publishes state 2 into the work-area byte at 0x02, the index of the
 * D_801CD130 dispatch table, and clears the work-area handler index at 0x03
 * once the shared work-area service func_801C4FE0 reports success; the closing
 * handler of the D_801CD140 pad_03 dispatch chain. */
void advanceStateToTwoAndResetHandlerIndexWhenReady_game00_801B343C(void);
/* Publishes handler index 2 into the work-area handler index at 0x03 once the
 * shared work-area service func_801C520C reports success; the second of the
 * two pad_03 dispatch chains (D_801CD154). */
void advanceHandlerIndexTo2WhenReady_game00_801B35B0(void);
/* Publishes handler index 2 into the work-area handler index at 0x04, the
 * index of the D_801CD330 work-state table, once the shared work-area service
 * func_801C4E14 reports success. */
void advanceWorkHandlerIndexTo2WhenReady(void);
/* Publishes handler index 3 into the work-area handler index at 0x04, the
 * index of the D_801CD330 work-state table, once the shared work-area service
 * func_801C4EC4 reports success. */
void advanceWorkHandlerIndexTo3WhenReady(void);
/* Publishes handler index 4 into the work-area handler index at 0x04, the
 * index of the D_801CD330 work-state table, once the shared work-area service
 * func_801C4F88 reports success. */
void advanceWorkHandlerIndexTo4WhenReady(void);
/* Publishes handler index 5 into the work-area handler index at 0x04, the
 * index of the D_801CD330 work-state table, once the shared work-area service
 * func_801C4FE0 reports success. */
void advanceWorkHandlerIndexTo5WhenReady(void);

/* Shared signed tint step of this overlay, the -0x80-floor counterpart of the
 * tint-fade ramp func_801C27E0: the signed byte argument is subtracted from
 * each of the three signed tint channels of the scratchpad work record (0x5D,
 * 0x5E, 0x5F), a channel already at the -0x80 floor is left untouched, every
 * updated channel is clamped to at least -0x80, and the byte result reports
 * whether all three channels are at that floor. Callers mask the result with
 * 0xFF before testing it. */
s32 func_801C29A0(s8 delta);

u8 func_801C3A18(s32 arg0, s32 arg1, u8 arg2, u8 arg3);
u8 func_801C3B78(s32 arg0, s32 arg1, u8 arg2, u8 arg3);
/* Route-step positioning attempt of the active work record, referenced by the
 * two gated wrappers requestRouteStepWithValue89 (value 0x89, step 2) and
 * requestRouteStepWithValue8A (value 0x8A, step 1): the first argument is the
 * tile-query value, the second
 * the step byte published into the work-record 0x0B byte, and the third enables
 * the eight-entry route-step scan. The body returns only 0 or 1 in the full
 * result word. */
s32 func_801C364C(s32 arg0, s32 arg1, s32 arg2);
/* Reference-scaled position checks of the same active-record range test as
 * func_801BDD58; the third argument is taken as a signed 16-bit reference. */
u8 func_801C4040(s32 x, s32 y, s16 reference);
u8 func_801C41D0(s32 x, s32 y, s16 reference);
u8 func_801C476C(s32 arg0, s32 arg1, u8 arg2, u8 arg3);
u8 func_801C48FC(s32 arg0, s32 arg1, u8 arg2, u8 arg3);
u8 func_801C6FD4(s16 arg0, s16 arg1);

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

/* Work area struct accessed via scratchpad pointer (0x1F800044) */
struct GameWorkArea {
  u8  flags_00;       /* 0x00 - entity flags */
  u8  unk_01;         /* 0x01 */
  u8  flags_02;       /* 0x02 */
  u8  pad_03;         /* 0x03 */
  u8  field_04;       /* 0x04 - handler index */
  u8  field_05;       /* 0x05 - region/scene index */
  u8  unk_06;         /* 0x06 */
  u8  unk_07;         /* 0x07 */
  u8  route_index_08; /* 0x08 */
  u8  pad_09[0x02];   /* 0x09-0x0A */
  u8  field_0B;       /* 0x0B */
  s32 field_0C;       /* 0x0C */
  s32 field_10;       /* 0x10 */
  s32 unk_14;         /* 0x14 - packed facing word copied to 0x80149330 */
  s32 unk_18;         /* 0x18 */
  u8  pad_1C[0x0D];   /* 0x1C-0x28 */
  u8  unk_29;         /* 0x29 */
  u8  unk_2A;         /* 0x2A */
  u8  pad_2B[0x05];   /* 0x2B-0x2F */
  u8  unk_30;         /* 0x30 */
  u8  pad_31[0x03];   /* 0x31-0x33 */
  s32 coord_x_34;     /* 0x34 */
  s32 coord_y_38;     /* 0x38 */
  u8  pad_3C[0x02];   /* 0x3C-0x3D */
  u16 counter_3E;     /* 0x3E */
  u8  pad_40[0x09];   /* 0x40-0x48 */
  u8  unk_49[0x04];   /* 0x49-0x4C (unaligned word) */
  u8  pad_4D[0x0F];   /* 0x4D-0x5B */
  u8  flags_5C;       /* 0x5C */
  s8  unk_5D;         /* 0x5D - signed tint channel */
  s8  unk_5E;         /* 0x5E - signed tint channel */
  s8  unk_5F;         /* 0x5F - signed tint channel */
  u8  pad_60[0x04];   /* 0x60-0x63 */
  s32 coord_64;       /* 0x64 */
  s32 coord_68;       /* 0x68 */
  s32 coord_6C;       /* 0x6C */
  u8  speed_70;       /* 0x70 */
  u8  pad_71[0x03];   /* 0x71-0x73 */
  u16 anim_state_74;  /* 0x74 */
  u8  pad_76[0x0C];   /* 0x76-0x81 */
  s16 unk_82;         /* 0x82 - X reference subtracted from the projected X */
  u8  pad_84[0x02];   /* 0x84-0x85 */
  s16 unk_86;         /* 0x86 - Y reference subtracted from the projected Y */
  u8  pad_88[0x04];   /* 0x88-0x8B */
  s16 unk_8C;         /* 0x8C - X range compared against the absolute X delta */
  s16 unk_8E;         /* 0x8E - Y range compared against the absolute Y delta */
  u8  pad_90[0x08];   /* 0x90-0x97 (work area slot is 0x98 bytes) */
};

/* Work-script interpreter state shared by the front-end script runner
 * (func_801A8F94 / func_801A8D38). Only the u16 byte cursor at +0x0A is proven
 * here: three readers load it as `lhu` and the runner stores the resolved
 * target back into it after func_801AC8F0. The remaining offsets are not
 * modelled yet. */
typedef struct GameWorkScriptState {
  u8  pad_00[0x0A];
  u16 cursor_0A; /* 0x0A - script byte cursor (bit 0x4000 = local label) */
} GameWorkScriptState;

/* @behavior Resolves a work-script jump operand: when the state cursor at
 * +0x0A has bit 0x4000 clear that 16-bit absolute target is returned unchanged,
 * otherwise the script is scanned from offset 0 for the 0x0A label definition
 * whose id byte equals label and the offset of that definition is returned.
 * @source 0x801AC8F0
 */
s32 func_801AC8F0(GameWorkScriptState* state, u8 label, const u8* script);

typedef struct GameResetRecord {
  u8 unk_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05;
  u8 unk_06;
  u8 unk_07;
  u8 unk_08[2];
  s16 unk_0A;
} GameResetRecord;

typedef struct GamePaletteEntry {
  u8  flags;
  u8  field_01;
  u8  red_offset;
  u8  green_offset;
  u8  blue_offset;
  u8  table_index;
  u8  step;
  u8  field_07;
  u8* target;
} GamePaletteEntry;

typedef struct GamePaletteSlot {
  u8  flags;
  u8  field_01;
  u8  field_02;
  u8  field_03;
  u8* source_table;
  u8* current_entry;
  u8* owner;
} GamePaletteSlot;

/* Entry table at 0x80143FC8 — 20 records × 0x74 bytes each.
 * Only the first 6 fields are known; the rest is padding. */
typedef struct RecordSlot {
  u8 flags_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05;    /* 0x05 - handler selector for the table at 0x801C7C70 */
  u8 pad[0x6E]; /* 0x74 - 6 */
} RecordSlot;

/* Work record passed to dispatchRecordCallbackByByte7A; only the handler selector byte at
 * 0x7A is proven. */
typedef struct GameIndexedWork {
  u8 pad_00[0x7A];
  u8 handler_index_7A; /* 0x7A */
} GameIndexedWork;

/* Indexed handler table dispatch record. */
typedef s32 (*GameIndexedHandler)(GameIndexedWork* work, u8* arg);

/* World-handler entry: pending world id key at +0, 3 pad bytes, handler at +4. */
typedef s8 (*GameWorldIdHandler)(u16 arg0, u16 arg1);
typedef struct GameWorldIdHandlerEntry {
  u8 key_00; /* 0x00 */
  u8 pad_01[3];
  GameWorldIdHandler handler_04; /* 0x04 */
} GameWorldIdHandlerEntry;

/* @source 0x80148648 @kind unknown */
/* @source 0x801C8090 @kind table */
extern GameEntry0StateHandler D_801C8090[];
/* @source 0x801C80E0 @kind table */
extern GameEntry0StateHandler D_801C80E0[];
/* @source 0x801C80EC @kind table */
extern GameEntry0StateHandler D_801C80EC[];
/* @source 0x801CD510 @kind table */
extern GameEntry0StateHandler* D_801CD510[];
/* @behavior four-byte-per-entry mode record table read by func_801C0F9C: the
 * bytes at +0..+3 of entry mode (byte offset mode * 4) are published into bytes
 * 1..4 of the mode's 0x140-byte-stride work record. */
/* @source 0x801CD4AC @kind table */
extern u8          D_801CD4AC[];
/* @source 0x801CD4C0 @kind table */
extern GameEntry0StateHandler* D_801CD4C0[];
/* @behavior four-entry work-flag handler table dispatched by func_801B2D90: the
 * byte at work+0x02 selects the handler, whose entries are the sibling update
 * routines 0x801B2DDC, 0x801B2FC8, 0x801B315C and 0x801B31F0. */
/* @source 0x801CD120 @kind table */
extern GameEntry0StateHandler D_801CD120[];
/* @behavior two-entry work-flag handler table dispatched by
 * dispatchWorkFlags02HandlerThenReadyUpdate: the byte at work+0x02 selects the
 * handler, whose entries are the sibling update routines 0x801B3768 and
 * 0x801B3A3C. */
/* @source 0x801CD1B0 @kind table */
extern GameEntry0StateHandler D_801CD1B0[];
/* @source 0x801CD130 @kind table */
extern GameEntry0StateHandler D_801CD130[];
/* @source 0x801CD330 @kind table */
extern GameEntry0StateHandler D_801CD330[];
/* @source 0x801CD310 @kind table */
extern GameEntry0StateHandler D_801CD310[];
/* @source 0x801CD2B0 @kind table */
extern GameEntry0StateHandler D_801CD2B0[];
/* @source 0x801CD37C @kind table */
extern GameEntry0StateHandler D_801CD37C[];
/* @source 0x801CD140 @kind table */
extern GameEntry0StateHandler D_801CD140[];
/* @source 0x801CD154 @kind table */
extern GameEntry0StateHandler D_801CD154[];
/* @source 0x801CD2F4 @kind table */
extern GameEntry0StateHandler D_801CD2F4[];
/* @source 0x801CD2EC @kind table */
extern GameEntry0StateHandler D_801CD2EC[];
/* @behavior 256-byte per-opcode width table used by the label scanner
 * func_801AC8F0 to step over each script instruction: the byte at the current
 * scan offset is added to that offset, except for the scanner's own 0xF8 and
 * 0x0E/0x0F overrides. Values are the scanner's initial lookup data, not proven
 * interpreter instruction lengths. */
/* @source 0x801C87FC @kind table */
extern u8 D_801C87FC[];
/* @source 0x801C88FC @kind table */
extern GameEntry0StateHandler D_801C88FC[];
/* @source 0x801C8904 @kind table */
extern GameEntry0StateHandler D_801C8904[];
/* @source 0x801C7BEC @kind table */
extern GameEntry0StateHandler D_801C7BEC[];
/* @behavior three-entry panel state callback table dispatched by
 * func_801995A4 with the panel task root state byte as the index. */
/* @source 0x801C7C00 @kind table */
extern GameEntry0StateHandler D_801C7C00[];
/* @behavior three-entry panel state callback table dispatched by
 * func_801996A8 with the panel task root state byte as the index. */
/* @source 0x801C7C0C @kind table */
extern GameEntry0StateHandler D_801C7C0C[];
/* @source 0x801C7C70 @kind table */
extern GameEntry0StateHandler D_801C7C70[];
/* @source 0x801C80A4 @kind table */
extern GameEntry0StateHandler D_801C80A4[];
/* @source 0x801C80BC @kind table */
extern GameEntry0StateHandler D_801C80BC[];
/* @source 0x801C80F8 @kind table */
extern GameEntry0StateHandler D_801C80F8[];
/* @source 0x801C8120 @kind table */
extern GameEntry0StateHandler D_801C8120[];
/* @source 0x801C812C @kind table */
extern GameEntry0StateHandler D_801C812C[];
/* @source 0x801C8144 @kind table */
extern GameEntry0StateHandler D_801C8144[];
/* @source 0x801C8154 @kind table */
extern GameEntry0StateHandler D_801C8154[];
/* @source 0x801C8164 @kind table */
extern GameEntry0StateHandler D_801C8164[];
/* @source 0x801C817C @kind table */
extern GameEntry0StateHandler D_801C817C[];
/* @source 0x801C8198 @kind table */
extern GameEntry0StateHandler D_801C8198[];
/* @source 0x801C81A0 @kind table */
extern GameEntry0StateHandler D_801C81A0[];
/* @source 0x801C81B8 @kind table */
extern GameEntry0StateHandler D_801C81B8[];
/* @source 0x801C81B0 @kind table */
extern GameEntry0StateHandler D_801C81B0[];
/* @source 0x801C81C0 @kind table */
extern GameEntry0StateHandler D_801C81C0[];
/* @source 0x801C81CC @kind table */
extern GameEntry0StateHandler D_801C81CC[];
/* @source 0x801C81DC @kind table */
extern GameEntry0StateHandler D_801C81DC[];

extern PanelTask* D_80148648;
#define D_80148648 g_PanelTaskRoot
/* @source 0x80146870 @kind bss */
extern GameScenarioState            scenarioState;
/* @source 0x801CA70C @kind table */
extern const volatile AbilityObject abilityObjects[];

/* Work area pointer in scratchpad slot 0x1F800044 (offset 0x44).
 * Declared as a weak extern so the linker emits %hi/%lo relocations,
 * matching the original binary's codegen. */
/* @source 0x1F800044 @kind data */
extern struct GameWorkArea* g_game_work;
/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* 164-byte record table of the shared main executable data segment: this
 * overlay selects one record by the byte D_80181B10 and reads its status bytes
 * at 0x0E through 0x13. D_80144974 is the same table seen 0x0C bytes in. */
/* @source 0x80144968 @kind table */
extern u8 D_80144968[];
/* @behavior one-byte record-index selector table indexed by the entry byte. */
/* @source 0x80181B10 @kind table */
extern u8 D_80181B10[];
typedef struct GameInputRecord {
  volatile u16 flags;
  u8 pad_02[0xA2];
} GameInputRecord;
/* @source 0x80144974 @kind unknown */
extern GameInputRecord D_80144974[];
/* @source 0x801CD358 @kind table */
extern GameEntry0StateHandler D_801CD358[];
/* @source 0x801CD374 @kind table */
extern GameEntry0StateHandler D_801CD374[];

/* @behavior entry-0 main state machine index */
/* @source 0x80143B90 @kind unknown */
extern volatile u16 D_80143B90;
/* @source 0x8014932C @kind unknown */
extern volatile u16 D_8014932C;
/* @source 0x80143C40 @kind bss */
extern u16          effectBusy;
/* @source 0x80143F49 @kind unknown */
extern volatile u8  D_80143F49;
/* @source 0x80143F4A @kind unknown */
extern volatile u8  D_80143F4A;
/* @source 0x80143FBC @kind unknown */
extern u8           D_80143FBC;
/* @behavior entry-0 sub-state within current state */
/* @source 0x80143B92 @kind unknown */
extern u16 D_80143B92;
/* @behavior sub-state/flag byte selecting the direction the world y at
 * 0x801492DC is stepped toward its 512 target in func_801990D0 (zero = downward,
 * nonzero = upward). */
/* @source 0x80143BB1 @kind unknown */
extern u8 D_80143BB1;
/* @behavior world/phase index for entry-0 world dispatch */
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @behavior current world state ID for world/front routing */
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;
extern u8 *D_80146884;
extern void *D_8017F974[];
extern void (*D_801C890C[])(s32, s32);
/* @behavior one 0x1C-byte record of the eleven world-slot records read by the
 * record dispatchers 0x8019A210/0x8019A258/0x8019A2A0/0x8019A2E8/0x8019A370:
 * each opens with five handlers, of which the dispatcher calls handler 0..4 by
 * its own position, followed by the eight bytes holding the slot record pointer
 * (word view D_801C7F70) and the current world state ID key byte (byte view
 * D_801C7F74 at record+4 within that word view). */
typedef struct GameWorldSlotRecord {
  GameEntry0StateHandler handlers[5]; /* 0x00 */
  u8 pad_14[8];                       /* 0x14 record pointer + world ID key */
} GameWorldSlotRecord;
/* @source 0x801C7F5C @kind table */
extern GameWorldSlotRecord D_801C7F5C[11];
/* @behavior eleven 0x1C-byte world-slot records: word 0 holds the slot's record
 * pointer, the key byte holding the current world state ID sits at record+4
 * (byte view D_801C7F74), and func_8019A1D4 returns word 0 of the slot.
 * @source 0x801C7F70 @kind table */
extern s32 D_801C7F70[11][7];
/* @source 0x801C7F74 @kind table */
extern u8 D_801C7F74[11][0x1C];

u8 func_8019A194(void);
/* @behavior world/front flags: bit0=scenario pending, bit3=alt front mode */
/* @source 0x80143F02 @kind unknown */
extern volatile u8 D_80143F02;
/* @behavior pad state word whose bits 12-13 and 14-15 func_801BDBC4 swaps when
 * any visible input record carries flag bit 5. */
/* @source 0x80145AA4 @kind unknown */
extern u16 D_80145AA4;
/* @behavior gate mask ANDed with the pad state word above by func_80198F1C: a
 * shared bit selects the input-driven world-axis stepping path (the same word
 * carries 0x4000/0x1000 for the x axis and 0x2000/0x8000 for the y axis), no
 * shared bit takes the no-input settle path. */
/* @source 0x80145ABA @kind unknown */
extern u16 D_80145ABA;
/* Work gate pair ANDed together before the pending front-end request path may
 * run; a clear common bit blocks that path. */
/* @source 0x80145AA8 @kind unknown */
extern u16 D_80145AA8;
/* @source 0x80145AC0 @kind unknown */
extern u16 D_80145AC0;
/* @behavior context selection seed passed to entry-0 ctx init */
/* @source 0x80143F10 @kind unknown */
extern volatile u16 D_80143F10;
/* @behavior context bundle word A — world/route identifier */
/* @source 0x80143F14 @kind unknown */
extern volatile u32 D_80143F14;
/* @behavior context bundle word B — secondary selector data */
/* @source 0x80143F18 @kind unknown */
extern volatile u32 D_80143F18;
/* @behavior context kind byte — dispatch type discriminator */
/* @source 0x80143F1C @kind unknown */
extern volatile u8 D_80143F1C;
/* @behavior pending request kind — selects next front operation */
/* @source 0x80143F1D @kind unknown */
extern volatile u8 D_80143F1D;
/* @behavior pending mode after request resolution */
/* @source 0x80143F1E @kind unknown */
extern u8 D_80143F1E;
/* @behavior selection seed for the entry-0 front callback bank */
/* @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;
/* @behavior active selection id from front-end picker; the low three bytes are
 * the front-end clock advanced by func_80198CAC */
/* @source 0x80144FC0 @kind unknown */
extern volatile u32 D_80144FC0;
/* @behavior front-end clock byte below D_80144FC1 */
/* @source 0x80144FC1 @kind unknown */
extern u8 D_80144FC1;
/* @behavior front-end clock byte below D_80144FC2 */
/* @source 0x80144FC2 @kind unknown */
extern u8 D_80144FC2;
/* @behavior front-end clock frame byte wrapping to zero at 30 in func_80198CAC */
/* @source 0x80144FC3 @kind unknown */
extern u8 D_80144FC3;
/* @behavior front-end countdown word decremented by func_80198CAC; D_80145554
 * is its lowest byte, D_80145557 the frame byte wrapping at 29 */
/* @source 0x80145554 @kind unknown */
extern u8 D_80145554;
/* @source 0x80145555 @kind unknown */
extern u8 D_80145555;
/* @source 0x80145556 @kind unknown */
extern u8 D_80145556;
/* @source 0x80145557 @kind unknown */
extern u8 D_80145557;
/* @behavior second front-end countdown word decremented by func_80198CAC;
 * D_80145558 is its lowest byte, D_8014555B the frame byte wrapping at 29 */
/* @source 0x80145558 @kind unknown */
extern u8 D_80145558;
/* @source 0x80145559 @kind unknown */
extern u8 D_80145559;
/* @source 0x8014555A @kind unknown */
extern u8 D_8014555A;
/* @source 0x8014555B @kind unknown */
extern u8 D_8014555B;
/* @behavior front-end selection index for menu routing */
/* @source 0x80145029 @kind bss */
extern u8  frontSelection;
/* @source 0x80145024 @kind unknown */
extern u8  D_80145024;
/* @source 0x8014502C @kind unknown */
extern u32 D_8014502C;
/* @source 0x801459F4 @kind unknown */
extern u8* D_801459F4;
/* @behavior palette stage serial for GPU upload sequencing */
/* @source 0x80145988 @kind bss */
extern volatile u8 paletteStageSerial;
/* @source 0x80039600 @kind unknown */
extern u16 D_80039600[];
/* @source 0x8003963E @kind unknown */
extern u16 D_8003963E[];
/* @behavior world flags — bit0=pending scenario, bit6=force-reset.
 * UNKNOWN: roles of observed bits 5 and 11. */
/* @source 0x80146258 @kind unknown */
extern u16         D_80146258;
/* @source 0x8014625A @kind unknown */
extern u16         D_8014625A;
/* @behavior published derivative of D_80145AA4 written by func_801BDBC4: the
 * same word with bits 12-13 and 14-15 swapped when a visible input record was
 * found, otherwise the raw word. */
/* @source 0x8014625C @kind unknown */
extern u16         D_8014625C;
/* @behavior count of live 0x140-byte-stride records walked by func_8019FF88
 * and the sibling request/route helpers. */
/* @source 0x80146254 @kind unknown */
extern u8          D_80146254;
/* @behavior low byte of the pending frontend request published by
 * func_8019FF88; the request argument pair is kept in the two words below. */
/* @source 0x80146262 @kind unknown */
extern u8          D_80146262;
/* @source 0x80146264 @kind unknown */
extern u32         D_80146264;
/* @source 0x80146268 @kind unknown */
extern u32         D_80146268;
/* @behavior per-record liveness byte at record offset 0x68 in each
 * D_80146254-entry, 0x140-byte-stride record; nonzero makes the panel position
 * helpers publish the direction-offset position for that record. */
/* @source 0x80145F00 @kind table */
extern u8          D_80145F00[];
/* @behavior leading byte of each D_80146254-entry, 0x140-byte-stride record;
 * func_8019FF88 copies the per-kind byte D_801C8380[kind] into it and sets
 * bit 1 of the record's second byte at D_80145FBC. */
/* @source 0x80145FBB @kind table */
extern u8          D_80145FBB[];
/* @source 0x80145FBC @kind table */
extern u8          D_80145FBC[];
/* @behavior leading byte of each D_80146254-entry, 0x140-byte-stride record;
 * selects the 164-byte game input record in D_80144974 whose flags bit 5 marks
 * the record as visible to func_801BDBC4. */
/* @source 0x80145FCC @kind table */
extern u8          D_80145FCC[];
/* @source 0x80146256 @kind unknown */
extern volatile u8 D_80146256;
/* @behavior flag byte cleared on request 0xFE.
 * UNKNOWN: the flag's owning subsystem. */
/* @source 0x8014832E @kind unknown */
extern volatile u8 D_8014832E;
/* @source 0x801462E0 @kind unknown */
extern u8          D_801462E0;
/* @source 0x801462E1 @kind unknown */
extern u8          D_801462E1;
/* @source 0x801462E2 @kind unknown */
extern u8          D_801462E2;
/* @source 0x80145E9B @kind unknown */
extern u8          D_80145E9B;
/* @source 0x80145FDB @kind unknown */
extern u8          D_80145FDB;
/* @source 0x80145FD1 @kind unknown */
extern u8          D_80145FD1;
/* @source 0x8014611B @kind unknown */
extern u8          D_8014611B;
/* @source 0x80146111 @kind unknown */
extern u8          D_80146111;
/* @source 0x8014626C @kind unknown */
extern u8          D_8014626C;
/* @source 0x8014626D @kind unknown */
extern u8          D_8014626D;
/* @source 0x8014626E @kind unknown */
extern u8          D_8014626E;
/* @source 0x8014626F @kind unknown */
extern u8          D_8014626F;
/* @source 0x80146270 @kind unknown */
extern u8          D_80146270;
/* @source 0x80148650 @kind unknown */
extern u8          D_80148650;
/* @source 0x80148651 @kind unknown */
extern u8          D_80148651;
/* @source 0x80148652 @kind unknown */
extern u8          D_80148652;
/* @source 0x8014865C @kind unknown */
extern s8          D_8014865C;
/* @source 0x80149332 @kind unknown */
extern u8          D_80149332;
/* @source 0x80145EC0 @kind unknown */
extern u8          D_80145EC0;
typedef struct GameModeRecord {
  u8 field_00;
  u8 pad_01[3];
  u32 field_04;
  u32 field_08;
  u32 field_0C;
  u8 pad_10[0x11];
  u8 field_21;
  u8 pad_22[0x0A];
  u32 field_2C;
  u32 field_30;
  u32 field_34;
  u8 pad_38[0x0B];
  u8 field_43;
  u8 pad_44[0x2D];
  u8 field_71;
  u8 pad_72[0x9E];
  u8 field_110;
  u8 field_111;
  u8 pad_112[0x12];
  u8 field_124;
  u8 pad_125[0x1B];
} GameModeRecord;

/* @behavior base of the D_80146254-entry, 0x140-byte-stride local work record
 * array read by the front-end request helpers; func_801C0E48 publishes the
 * record address at index D_80146262 here once its gate byte is set. */
/* @source 0x80145E90 @kind table */
extern u8          D_80145E90[];
/* @behavior gate byte at offset 1 of each 0x140-byte-stride local work record;
 * func_801C0E48 only advances the pending request while the record at
 * D_80146262 - 1 carries 1 here. */
/* @source 0x80145E91 @kind table */
extern u8          D_80145E91[];
/* @behavior byte at offset 1 of each 0x140-byte-stride local work record.
 * func_801B55C0 accepts a record whose leading byte (D_80145E91) reads 2 only
 * while this type byte appears in the three-byte list D_801CD1C8. */
/* @source 0x80145E92 @kind table */
extern u8          D_80145E92[];
/* @behavior work-record byte at offset 3 of each D_80146254-entry,
 * 0x140-byte-stride local work record; func_801C0F9C publishes bytes 2 and 3 of
 * the four-byte mode record D_801CD4AC + mode * 4 here. */
/* @source 0x80145E93 @kind table */
extern u8          D_80145E93[];
/* @behavior work-record byte at offset 4 of each D_80146254-entry,
 * 0x140-byte-stride local work record; see D_80145E93. */
/* @source 0x80145E94 @kind table */
extern u8          D_80145E94[];
/* @source 0x80145E98 @kind unknown */
extern GameModeRecord D_80145E98[];
/* @source 0x80145EC4 @kind unknown */
extern u32         D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32         D_80145EC8;
/* @source 0x80145ECE @kind unknown */
extern u16         D_80145ECE;
/* @source 0x80181AC0 @kind table */
extern s32         D_80181AC0[];
/* @source 0x80181AC4 @kind table */
extern s32         D_80181AC4[];
/* @behavior Eight packed direction steps shared by the front-end walkers:
 * entry i (two bytes) holds the signed first-coordinate step at D_80181B00 and
 * the signed second-coordinate step at D_80181B01, indexed by
 * route_index_08 * 2. */
/* @source 0x80181B00 @kind table */
extern u8          D_80181B00[];
/* @source 0x80181B01 @kind table */
extern u8          D_80181B01[];
/* @source 0x80181B94 @kind table */
extern s32         D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32         D_80181B98[];
/* @source 0x80181BD4 @kind table */
extern u8          D_80181BD4[];
/* @source 0x80181164 @kind table */
extern u8*         D_80181164[];
/* Packed world-id to context-seed mapping record: key at +0, mapped id at +2
 * (4-byte stride).  Ten records are read by func_801B6418. */
typedef struct GameWorldContextSeed {
  u16 world_id;
  u16 context_seed;
} GameWorldContextSeed;
/* @source 0x801CD224 @kind table */
extern GameWorldContextSeed D_801CD224[];
/* @behavior eleven u16 context seeds searched by func_801A02E8; a hit selects
 * the 0xB pending request kind, an exhausted search selects 1. */
/* @source 0x801C8384 @kind table */
extern const u16 D_801C8384[];
/* @behavior per-kind byte copied into the record leading byte by
 * func_8019FF88, indexed by the pending handler index (0-3). */
/* @source 0x801C8380 @kind table */
extern u8 D_801C8380[];
/* @source 0x80149308 @kind unknown */
extern u32         D_80149308;
extern u32         D_8014930C;
/* @source 0x80181EBA @kind unknown */
extern const u8    D_80181EBA[];
/* @source 0x80181EBB @kind unknown */
extern const u8    D_80181EBB[];
/* @behavior signed world-coord X argument for scenario entry */
/* @source 0x8014930A @kind unknown */
extern s16 D_8014930A;
/* @behavior signed world-coord Y argument for scenario entry */
/* @source 0x8014930E @kind unknown */
extern s16                 D_8014930E;
/* @behavior 16-bit companion that func_801990D0 keeps in step with the signed
 * front-end world x at 0x801492D8 by mirroring every per-frame delta. */
/* @source 0x801481D8 @kind unknown */
extern u16                 D_801481D8[];
/* @behavior 16-bit companion that func_801990D0 keeps in step with the signed
 * front-end world y at 0x801492DC by mirroring every per-frame delta. */
/* @source 0x801481DC @kind unknown */
extern u16                 D_801481DC[];
/* @behavior the three 16.16 panel-axis accumulators advanced by the paired
 * per-frame deltas at 0x801481F8 / 0x801481FC / 0x80148200; the integer part of
 * each sum is committed to the signed halfword panel coordinates below. */
/* @source 0x801481E8 @kind unknown */
extern s32                 D_801481E8;
/* @source 0x801481EC @kind unknown */
extern s32                 D_801481EC;
/* @source 0x801481F0 @kind unknown */
extern s32                 D_801481F0;
/* @behavior per-frame 16.16 deltas added to the matching accumulator above */
/* @source 0x801481F8 @kind unknown */
extern s32                 D_801481F8;
/* @source 0x801481FC @kind unknown */
extern s32                 D_801481FC;
/* @source 0x80148200 @kind unknown */
extern s32                 D_80148200;
/* @source 0x801492D8 @kind unknown */
extern s16                 D_801492D8;
/* @source 0x801492DA @kind unknown */
extern s16                 D_801492DA;
/* @source 0x801492DC @kind unknown */
extern s16                 D_801492DC;
/* @source 0x8014932E @kind unknown */
extern s16                 D_8014932E;
/* @source 0x80146329 @kind unknown */
extern u8                  D_80146329;
/* @source 0x801462E3 @kind unknown */
extern u8                  D_801462E3;
/* @source 0x801462E4 @kind unknown */
extern u8                  D_801462E4;
/* @source 0x801462F0 @kind unknown */
extern u8                  D_801462F0;
/* @source 0x801462EC @kind unknown */
extern u8                  D_801462EC;
/* @source 0x80146325 @kind unknown */
extern u8                  D_80146325;
/* @source 0x80149318 @kind unknown */
extern u32                 D_80149318;
/* @source 0x80149330 @kind unknown */
extern s16                 D_80149330;
/* @source 0x80149333 @kind unknown */
extern u8                  D_80149333;
/* @source 0x8014933E @kind unknown */
extern u8                  D_8014933E;
/* @source 0x80146888 @kind unknown */
extern struct GameWorkArea D_80146888[];
/* @source 0x8014933F @kind unknown */
extern u8                  D_8014933F;
/* @source 0x801CD954 @kind unknown */
extern u32                 D_801CD954;
/* @source 0x801C7B74 @kind unknown */
extern const s8            D_801C7B74[];
/* @source 0x80146864 @kind unknown */
extern volatile u8         g_ScenarioProgress;
/* @source 0x801490A4 @kind unknown */
extern volatile u16        D_801490A4;
/* @source 0x80144F28 @kind unknown */
extern u8                  D_80144F28[];
/* @behavior no-argument handler table selected by the current world state ID
 * resolved through func_8019A194; its 0x2C span ends where the
 * D_801C85F0 table begins. */
/* @source 0x801C85C4 @kind table */
extern GameEntry0StateHandler D_801C85C4[];
/* @source 0x801C85F0 @kind table */
extern GameIndexedHandler  D_801C85F0[];
/* @source 0x801C86F8 @kind table */
extern GameWorldIdHandlerEntry worldIdHandlerTable[28];

/* INFERRED: palette work records use the observed 12-byte and 16-byte strides;
 * confirm field meanings against their setup paths. */
/* @source 0x80037800 @kind unknown */
extern volatile u16     D_80037800[];
/* @source 0x80145BD4 @kind unknown */
extern GamePaletteEntry D_80145BD4[];
/* @source 0x80145D54 @kind unknown */
extern u8               D_80145D54[][16];
/* Scratchpad base byte array (0x1F800000); extern forces lui/addiu pair. */
/* @source 0x1F800000 @kind unknown */
extern u8               D_1F800000[];
/* @source 0x80145D94 @kind unknown */
extern GamePaletteSlot  D_80145D94[];
/* @source 0x801C7AC0 @kind unknown */
extern const u8         D_801C7AC0[];
/* @source 0x801C7AC8 @kind unknown */
extern const u8         D_801C7AC8[];
/* @source 0x801C7AD0 @kind unknown */
extern const u8         D_801C7AD0[];
/* @source 0x801C7AD8 @kind unknown */
extern const u8         D_801C7AD8[];
/* @source 0x801C7AE0 @kind unknown */
extern const u8         D_801C7AE0[];
/* @source 0x801C7AE8 @kind unknown */
extern const u8         D_801C7AE8[];

/* @source 0x801CD568 @kind table */
extern const GameEntry0StateHandler scenarioSubstateHandlerTable[];
/* @source 0x801C7B08 @kind unknown */
extern const GameEntry0StateHandler D_801C7B08[];
/* @source 0x801C7B14 @kind unknown */
extern const GameEntry0StateHandler D_801C7B14[];
/* @source 0x801C7B44 @kind unknown */
extern const GameEntry0StateHandler D_801C7B44[];
/* @source 0x801C7B54 @kind unknown */
extern const GameEntry0StateHandler D_801C7B54[];
/* @source 0x801C7B7C @kind unknown */
extern const GameEntry0StateHandler D_801C7B7C[];
/* @source 0x801C7B88 @kind unknown */
extern const GameEntry0StateHandler D_801C7B88[];
/* @source 0x801C7B98 @kind unknown */
extern const GameEntry0StateHandler D_801C7B98[];
/* @source 0x801C7BA4 @kind unknown */
extern const GameEntry0StateHandler D_801C7BA4[];
/* @source 0x801C7BB0 @kind unknown */
extern const GameEntry0StateHandler D_801C7BB0[];

/* @source 0x80143FC8 @kind bss */
extern RecordSlot recordTable[20];

/* @behavior per-mode 3-byte record table. Expected stride: mode * 3 bytes. */
/* @source 0x80144F5A @kind unknown */
extern u8 D_80144F5A[];

/* @behavior finds the first unused record slot by scanning the
 * entry table at recordTable; returns its index (0‑19) or 0xFF
 * when all slots are occupied.
 * @source 0x8019601C
 */
u8 findFreeRecord(u8 mode);
void clearResetRecord(GameResetRecord* record);

/* @behavior clears bytes 0‑4 of the record slot at record_index.
 * @source 0x801960C0
 */
void clearRecord(u8 record_index);

void clearWorkFlags(void);
void resetWork2(void);
void func_801D0D80(void);
void func_801D0D9C(void);
void func_801D0EA0(void);
void func_801D1740(void);

/* @behavior seeds the shared callback/frame dispatch prologue before the entry-0
 * callback tables begin running.
 * @source 0x8014BA04
 */
void func_8014BA04(void);

/* @behavior begins one shared front-end frame/update slice.
 * @source 0x80158E50
 */
void func_80158E50(void);

/* @behavior finalizes one shared front-end frame/update slice.
 * @source 0x80158C80
 */
void func_80158C80(void);

/* @behavior runs one selection-side post-dispatch update slice.
 * @source 0x80198CAC
 */
void func_80198CAC(void);

/* @behavior resets the entry-0 front script/runtime bank for the requested mode.
 * @source 0x801C1400
 */
void func_801C1400(u32 mode);

/* @behavior copies the active front selector/context bundle into the entry-0 local
 * runtime state.
 * @source 0x8019FA28
 */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @behavior copies the shared CPU-side palette bank before the corresponding VRAM
 * upload path.
 * @source 0x8014E284
 */
void stageSharedPaletteBank(void);

/* @behavior begins streaming the currently selected SCENA pack for the seeded
 * scenario state.
 * @source 0x801A7804
 */
void requestScenarioOverlay(void);

/* @behavior enters the loaded scenario-local dispatch path after the SCENA loader
 * completes.
 * @source 0x801A782C
 */
/* @source 0x801C8454 @kind table */
extern GameEntry0StateHandler* D_801C8454[];

void dispatchScenarioHandlerAndState(void);
void dispatchStateHandler(void);

/* @behavior ticks the shared world/front waiting path while the scenario loader is
 * still pending.
 * @source 0x801992B8
 */
void func_801527E4(void);
void func_8015A758(void);
void func_801BDAB8(void);
void runFrameFinalizationServices(void);
void func_8019A0E4(void);
void func_8019AAFC(void);
void func_8014BA54(void);

/* @behavior runs the panel task update slice over the panel task root record
 * passed by the front-end panel state dispatchers.
 * @source 0x801999F8
 */
void func_801999F8(PanelTask* task);

/* @behavior runs the panel task update slice over the panel task root record
 * passed by the front-end panel state dispatchers.
 * @source 0x801D7AD0
 */
void func_801D7AD0(PanelTask* task);

/* @behavior returns a pointer into one of two sprite-rect tables indexed by
 * sprite_id * 4, with the table chosen by flags & 1.
 * @source 0x801AF270
 */
u8* getSpriteRectEntry(u8 sprite_id, u8 flags);

/* @behavior draws one sprite by filling a GT quad primitive from a rect-table
 * entry, selecting CLUT by flags & 2, then appending to the OT.
 * @source 0x801AF2A0
 */
void drawSprite(s16 x, s16 y, u8 sprite_id, u8 flags);

/* @behavior iterates a packed sprite-record table and draws each sprite via
 * drawSprite with signed offsets shifted by 3 applied to base coords.
 * @source 0x801AF390
 */
void drawSpriteRecordTable(s16 base_x, s16 base_y, const u8* record_table, u8 flags);

/* @behavior computes a screen-space position from an entity's offset-adjusted
 * coordinates; returns the result as a signed 16-bit value.
 * @source 0x80154F28
 */
s16 func_80154F28(s32 x, s32 y);

/* @behavior external no-argument update called after movement completion checks.
 * UNKNOWN: its owning subsystem and return-value meaning.
 * @source 0x8014D978
 */
u8 func_8014D978(void);

/* @behavior shared region upload helper: called with a work-record byte, a zero
 * argument, the fixed 0x800F0800 source base and a 0xA00 size.
 * UNKNOWN: its owning subsystem and the meaning of the two trailing constants.
 * @source 0x8014D664
 */
void func_8014D664(u8 arg0, u32 arg1, u32 arg2, u32 arg3);

/* @behavior main-executable status helper called by sixteen sites of this
 * overlay. Every caller compares the full word it returns against the small
 * constants 1 and 2 and never masks it to a byte (two callers store it as a
 * byte and as a halfword), so the returned word is a small status code.
 * UNKNOWN: its owning subsystem and the meaning of the returned status.
 * @source 0x8015C148
 */
s32 func_8015C148(void);

void clearAllRecords(void);
void func_8019625C(void);
u8   func_801968BC(u8 mode);
u8   locatePaletteColor(u8 value);
void func_80196B9C(void);
u8   allocPaletteSlot(u8* owner, u8* source_table);
void advanceEntrySerial(void);
void selectionMainLoop(void);
void resetSelectionState(void);
void applySelectionContext(void);
void updateStateMachine(void);
void dispatchSubstate1(void);
void enterState2OnInput(void);
void dispatchSubstate2(void);
void bank2Init(void);
void func_80197AA4(void);
void bank2AdvanceWhenReady(void);
void bank2CompleteOperation(void);
void bank2FinalUpdates(void);
void dispatchWorldUpdate(void);
void updateWorld(void);
void updateWorldPosition(void);
void dispatchSubstate3(void);
void dispatchSubstate4(void);
void dispatchSubstate5(void);
void dispatchSubstate6(void);
void dispatchSubstate7(void);
void func_80199230(void);
void func_80198F1C(void);
void func_801990D0(void);
void func_801991B8(void);
void func_801BEDD0(void);
s32  func_801BEE5C(void);
void func_801A06D8(void);
void applyRemapRequest(u8 arg0);
u8   func_801BF11C(void);
void func_801BF8E0(void);
void func_801BFAC4(void);
u8   func_801BF78C(void);
u8   findModeFreeSlot(u8 mode);
void updateWorkRouteIndex(u8 arg_a, u8 arg_b, u8 arg_c);
/* @behavior tests the 16.16 x/y position handed in by func_8019C1E4 against
 * the active work area's route/sub-state and the shared collision helpers,
 * returning a truthy acceptance result whose low byte the caller reads.
 * @source 0x8019C344
 */
s32 func_8019C344(s32 arg0, s32 arg1);
void func_8016728C(u32 slot_id, u32 mode);
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
void startSelectionFx(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void stopSelectionFx(u8 arg0, u8 arg1);
void loadScenario(u8 scenario_index);
void func_801651DC(s32 ability_id, s32 character_id, s32 arg2, s32 arg3);
void func_80164A44(volatile void* character_state);

void func_8019FAA0(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* @behavior Publishes the pending frontend request block for the given handler
 * kind and stamps every live 0x140-byte-stride record.
 * @source 0x8019FF88
 */
void func_8019FF88(u8 kind, u32 context_a, u32 context_b);
void waitTransition(u32 arg0);
void func_801A0048(s16 a, s16 b);
/* @behavior searches the eleven u16 context seeds of D_801C8384 for the current
 * context seed D_80143F10, storing pending request kind 0xB on a hit and 1
 * after the search is exhausted.
 * @source 0x801A02E8
 */
void func_801A02E8(void);
/* @behavior resolves the pending front-end request for the 16.16 context point
 * (x, y) and the stored context seed, publishing the resulting request byte at
 * D_80143F1F; when world-flags bit 0x80 is set it publishes the front selector
 * without resolving.
 * @source 0x801A0380
 */
void func_801A0380(s32 x, s32 y, u32 selector);
/* @behavior Iterates active entity slots and dispatches per-type handlers.
 * @source 0x801A0514
 */
void func_801A0514(void);
void func_801B3CCC(u32 arg0);
void advancePanelXTo320(void);
void retreatPanelXToNeg170(void);
void retreatPanelXToNeg170_2(void);
void advancePanelXTo17(void);
void retreatPanelField6ToNeg20(void);

/* Legacy alias for already‑matched functions that dereference 0x1F800044
 * via a literal‑address macro (lui+ori+lw 0(base) codegen). */
#define SCRATCH_WORK SPAD_PTR_SLOT(volatile struct GameWorkArea, 0x44u)

/* Scratchpad entity-spawn counters (0x1F800000 / 0x1F800002). */
#define GAME_ENTITY_COUNTER    SPAD_REF(s16, 0x0u)
#define GAME_ENTITY_ENTRY_DATA SPAD_REF(u16, 0x2u)

/* Work-area pointer cell in RAM at 0x80146884 (immediately before the
 * D_80146888 work-area array). */
#define GAME_WORK_AREA_PTR PSX_REF(volatile struct GameWorkArea*, 0x80146884u)

/* Per-record spawn-gate byte table at 0x80144FC4, indexed by byte. */
#define GAME_SPAWN_GATE_BYTE(index)                                            \
  PSX_REF(volatile u8, 0x80144FC4u + (u32)(index))

/* Movement/position offset tables in main exe data section */
#define MOVEMENT_OFFSET_0(i)  PSX_REF(volatile s32, 0x80181B94u + (i) * 8)
#define MOVEMENT_OFFSET_1(i)  PSX_REF(volatile s32, 0x80181B98u + (i) * 8)
#define MOVEMENT_THRESHOLD(i) PSX_REF(volatile s16, 0x80181B70u + (i) * 2)

/* Misc. single fixed-address globals (read-only status flags) */
#define GAME_UNK_80145558 PSX_REF(const volatile u32, 0x80145558u)
#define GAME_UNK_80145554 PSX_REF(const volatile u32, 0x80145554u)
#define GAME_UNK_801462EA PSX_REF(const volatile u8, 0x801462eau)

/* ---- RAM globals (D_ names match original game data patterns) ---- */

#define GAME_ALT_FRONT_CALLBACK_TABLE D_801C7B08
#define GAME_SELECTION_CALLBACK_TABLE D_801C7B14

#endif
