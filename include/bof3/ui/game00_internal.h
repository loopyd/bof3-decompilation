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

void func_801ACF2C(void);
void func_801AD0EC(void);
void func_801AD184(void);
void func_801AD218(void);
void func_801AD2CC(void);
void func_801AD3B4(void);

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
  u8  pad_0C[0x0C];   /* 0x0C-0x17 */
  s32 unk_18;         /* 0x18 */
  u8  pad_1C[0x0D];   /* 0x1C-0x28 */
  u8  unk_29;         /* 0x29 */
  u8  unk_2A;         /* 0x2A */
  u8  pad_2B[0x09];   /* 0x2B-0x33 */
  s32 coord_x_34;     /* 0x34 */
  s32 coord_y_38;     /* 0x38 */
  u8  pad_3C[0x02];   /* 0x3C-0x3D */
  u16 counter_3E;     /* 0x3E */
  u8  pad_40[0x09];   /* 0x40-0x48 */
  u8  unk_49[0x04];   /* 0x49-0x4C (unaligned word) */
  u8  pad_4D[0x0F];   /* 0x4D-0x5B */
  u8  flags_5C;       /* 0x5C */
  u8  unk_5D;         /* 0x5D */
  u8  unk_5E;         /* 0x5E */
  u8  unk_5F;         /* 0x5F */
  u8  pad_60[0x04];   /* 0x60-0x63 */
  s32 coord_64;       /* 0x64 */
  s32 coord_68;       /* 0x68 */
  s32 coord_6C;       /* 0x6C */
  u8  speed_70;       /* 0x70 */
  u8  pad_71[0x03];   /* 0x71-0x73 */
  u16 anim_state_74;  /* 0x74 */
  u8  pad_76[0x22];   /* 0x76-0x97 (work area slot is 0x98 bytes) */
};

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
 * Only the first 5 fields are known; the rest is padding. */
typedef struct RecordSlot {
  u8 flags_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 pad[0x6F]; /* 0x74 - 5 */
} RecordSlot;

/* Work record passed to dispatchRecordCallbackByByte7A; only the handler selector byte at
 * 0x7A is proven. */
typedef struct GameIndexedWork {
  u8 pad_00[0x7A];
  u8 handler_index_7A; /* 0x7A */
} GameIndexedWork;

/* Indexed handler table dispatch record. */
typedef s32 (*GameIndexedHandler)(GameIndexedWork* work, u8* arg);

/* @source 0x80148648 @kind unknown */
/* @source 0x801C8090 @kind table */
extern GameEntry0StateHandler D_801C8090[];
/* @source 0x801C80E0 @kind table */
extern GameEntry0StateHandler D_801C80E0[];
/* @source 0x801C80EC @kind table */
extern GameEntry0StateHandler D_801C80EC[];
/* @source 0x801CD510 @kind table */
extern GameEntry0StateHandler* D_801CD510[];
/* @source 0x801CD4C0 @kind table */
extern GameEntry0StateHandler* D_801CD4C0[];
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
/* @source 0x801C88FC @kind table */
extern GameEntry0StateHandler D_801C88FC[];
/* @source 0x801C8904 @kind table */
extern GameEntry0StateHandler D_801C8904[];
/* @source 0x801C7BEC @kind table */
extern GameEntry0StateHandler D_801C7BEC[];
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
/* @behavior world/phase index for entry-0 world dispatch */
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @behavior current world state ID for world/front routing */
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;
/* @source 0x801C7F74 @kind table */
extern u8 D_801C7F74[11][0x1C];

u8 func_8019A194(void);
/* @behavior world/front flags: bit0=scenario pending, bit3=alt front mode */
/* @source 0x80143F02 @kind unknown */
extern volatile u8 D_80143F02;
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
/* @behavior active selection id from front-end picker */
/* @source 0x80144FC0 @kind unknown */
extern volatile u32 D_80144FC0;
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
/* @behavior world flags — bit0=pending scenario, bit6=force-reset.
 * UNKNOWN: roles of observed bits 5 and 11. */
/* @source 0x8014625A @kind unknown */
extern u16         D_8014625A;
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
/* @source 0x8014611B @kind unknown */
extern u8          D_8014611B;
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
/* @source 0x80145EC4 @kind unknown */
extern u32         D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32         D_80145EC8;
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
extern struct GameWorkArea D_80146888[30];
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
/* @source 0x801C85F0 @kind table */
extern GameIndexedHandler  D_801C85F0[];

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
void func_801D0D9C(void);

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
void func_8014BA54(void);

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
void func_8016728C(u32 slot_id, u32 mode);
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
void startSelectionFx(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void stopSelectionFx(u8 arg0, u8 arg1);
void loadScenario(u8 scenario_index);
void func_801651DC(s32 ability_id, s32 character_id, s32 arg2, s32 arg3);
void func_80164A44(volatile void* character_state);

void func_8019FAA0(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void waitTransition(u32 arg0);
void func_801A0048(s16 a, s16 b);
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
