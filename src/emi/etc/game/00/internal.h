#ifndef EMI_GAME_00_INTERNAL_H
#define EMI_GAME_00_INTERNAL_H

#include "bof3/bof3.h"
#include "bof3/ui/panel_task.h"
#include "panel/task.h"
#include "battle/ability.h"
#include "frontend/state.h"
#include "frontend/selection.h"
#include "gpu/palette.h"

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
  u8  pad_76[0x22];   /* 0x76-0x97 (work area slot is 0x98 bytes) */
};

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

extern PanelTask*               D_80148648;
#define D_80148648 g_PanelTaskRoot
extern GameScenarioState            GAME_SCENARIO_STATE;
extern const volatile AbilityObject ABILITY_OBJECTS[];

/* Work area pointer in scratchpad slot 0x1F800044 (offset 0x44).
 * Declared as a weak extern so the linker emits %hi/%lo relocations,
 * matching the original binary's codegen. */
extern struct GameWorkArea* volatile g_game_work;

/* @behavior entry-0 main state machine index */
extern volatile u16 D_80143B90;
extern u16          D_80143C40;
/* @behavior entry-0 sub-state within current state */
extern u16 D_80143B92;
/* @behavior world/phase index for entry-0 world dispatch */
extern u8 D_80143BB0;
/* @behavior current world state ID for world/front routing */
extern volatile u16 D_80143F00;
/* @behavior world/front flags: bit0=scenario pending, bit3=alt front mode */
extern volatile u8 D_80143F02;
/* @behavior context selection seed passed to entry-0 ctx init */
extern volatile u16 D_80143F10;
/* @behavior context bundle word A — world/route identifier */
extern volatile u32 D_80143F14;
/* @behavior context bundle word B — secondary selector data */
extern volatile u32 D_80143F18;
/* @behavior context kind byte — dispatch type discriminator */
extern volatile u8 D_80143F1C;
/* @behavior pending request kind — selects next front operation */
extern volatile u8 D_80143F1D;
/* @behavior pending mode after request resolution */
extern u8 D_80143F1E;
/* @behavior selection seed for the entry-0 front callback bank */
extern u8 D_80143F1F;
/* @behavior active selection id from front-end picker */
extern volatile u32 D_80144FC0;
/* @behavior front-end selection index for menu routing */
extern u8  D_80145029;
extern u8  D_80145024;
extern u32 D_8014502C;
/* @behavior palette stage serial for GPU upload sequencing */
extern volatile u8 D_80145988;
/* @behavior world flags — bit0=pending scenario, bit6=force-reset.
 * UNKNOWN: roles of observed bits 5 and 11. */
extern u16         D_8014625A;
extern volatile u8 D_80146256;
/* @behavior flag byte cleared on request 0xFE.
 * UNKNOWN: the flag's owning subsystem. */
extern volatile u8 D_8014832E;
extern u8          D_801462E0;
extern u8          D_801462E1;
extern u8          D_801462E2;
extern u8          D_80145E9B;
extern u8          D_80145FDB;
extern u8          D_8014611B;
extern u8          D_8014626C;
extern u8          D_8014626D;
extern u8          D_8014626E;
extern u8          D_8014626F;
extern u8          D_80146270;
extern u8          D_80148650;
extern u8          D_80148651;
extern u8          D_80148652;
extern s8          D_8014865C;
extern u8          D_80149332;
extern const u8    D_80181EBA[];
extern const u8    D_80181EBB[];
/* @behavior signed world-coord X argument for scenario entry */
extern s16 D_8014930A;
/* @behavior signed world-coord Y argument for scenario entry */
extern s16                 D_8014930E;
extern s16                 D_801492D8;
extern s16                 D_801492DC;
extern s16                 D_8014932E;
extern u8                  D_80146329;
extern u8                  D_801462E3;
extern u8                  D_801462E4;
extern u8                  D_801462F0;
extern u8                  D_801462EC;
extern u8                  D_80146325;
extern u32                 D_80149318;
extern s16                 D_80149330;
extern u8                  D_80149333;
extern u8                  D_8014933E;
extern struct GameWorkArea D_80146888[30];
extern u8                  D_8014933F;
extern u32                 D_801CD954;
extern const s8            D_801C7B74[];
extern volatile u16        D_801490A4;

/* INFERRED: palette work records use the observed 12-byte and 16-byte strides;
 * confirm field meanings against their setup paths. */
extern volatile u16     D_80037800[];
extern GamePaletteEntry D_80145BD4[];
extern u8               D_80145D54[][16];
extern GamePaletteSlot  D_80145D94[];
extern const u8         D_801C7AC0[];
extern const u8         D_801C7AC8[];
extern const u8         D_801C7AD0[];
extern const u8         D_801C7AD8[];
extern const u8         D_801C7AE0[];
extern const u8         D_801C7AE8[];

extern const GameEntry0StateHandler D_801C7B08[];
extern const GameEntry0StateHandler D_801C7B14[];
extern const GameEntry0StateHandler D_801C7B44[];
extern const GameEntry0StateHandler D_801C7B54[];
extern const GameEntry0StateHandler D_801C7B7C[];
extern const GameEntry0StateHandler D_801C7B88[];
extern const GameEntry0StateHandler D_801C7B98[];
extern const GameEntry0StateHandler D_801C7BA4[];
extern const GameEntry0StateHandler D_801C7BB0[];

extern RecordSlot D_80143FC8[20];

/* @behavior per-mode 3-byte record table. Expected stride: mode * 3 bytes. */
extern u8 D_80144F5A[];

/* @behavior finds the first unused record slot by scanning the
 * entry table at D_80143FC8; returns its index (0‑19) or 0xFF
 * when all slots are occupied.
 * @source 0x8019601C
 */
u8 func_8019601C(u8 mode);

/* @behavior clears bytes 0‑4 of the record slot at record_index.
 * @source 0x801960C0
 */
void func_801960C0(u8 record_index);

void func_80196070(void);
void func_8019EAD4(void);

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
void func_8014E284(void);

/* @behavior begins streaming the currently selected SCENA pack for the seeded
 * scenario state.
 * @source 0x801A7804
 */
void func_801A7804(void);

/* @behavior enters the loaded scenario-local dispatch path after the SCENA loader
 * completes.
 * @source 0x801A782C
 */
void func_801A782C(void);

/* @behavior ticks the shared world/front waiting path while the scenario loader is
 * still pending.
 * @source 0x801992B8
 */
void func_801992B8(void);

/* @behavior returns a pointer into one of two sprite-rect tables indexed by
 * sprite_id * 4, with the table chosen by flags & 1.
 * @source 0x801AF270
 */
u8* func_801AF270(u8 sprite_id, u8 flags);

/* @behavior draws one sprite by filling a GT quad primitive from a rect-table
 * entry, selecting CLUT by flags & 2, then appending to the OT.
 * @source 0x801AF2A0
 */
void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* @behavior iterates a packed sprite-record table and draws each sprite via
 * func_801AF2A0 with signed offsets shifted by 3 applied to base coords.
 * @source 0x801AF390
 */
void func_801AF390(s16 base_x, s16 base_y, const u8* record_table, u8 flags);

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

void func_8019611C(void);
void func_8019625C(void);
u8   func_801968BC(u8 mode);
u8   func_80196B20(u8 value);
void func_80196B9C(void);
u8   func_80196CF0(u8* owner, u8* source_table);
void func_80196FFC(void);
void func_80197068(void);
void func_801970EC(void);
void func_801971E8(void);
void func_80197378(void);
void func_801975E4(void);
void func_801979D4(void);
void func_80197A24(void);
void func_80197A60(void);
void func_80197AA4(void);
void func_80197C1C(void);
void func_80197E54(void);
void func_80197EFC(void);
void func_80198170(void);
void func_801981B4(void);
void func_801981D4(void);
void func_80198234(void);
void func_801984AC(void);
void func_80198744(void);
void func_80198904(void);
void func_80198AC4(void);
void func_80199230(void);
void func_80198F1C(void);
void func_801990D0(void);
void func_801991B8(void);
void func_801BEDD0(void);
s32  func_801BEE5C(void);
void func_801A06D8(void);
void func_801B5BDC(u8 arg0);
u8   func_801BF11C(void);
void func_801BF8E0(void);
void func_801BFAC4(void);
u8   func_801BF78C(void);
u8   func_801BDB7C(u8 mode);
void func_801BB8E8(u8 arg_a, u8 arg_b, u8 arg_c);
void func_8016728C(u32 slot_id, u32 mode);
void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
void func_8015D404(u8 arg0, u8 arg1);
void func_801A7704(u8 scenario_index);
void func_801651DC(s32 ability_id, s32 character_id, s32 arg2, s32 arg3);
void func_80164A44(volatile void* character_state);

void func_8019FAA0(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_80198BC4(u32 arg0);
void func_801A0048(s16 a, s16 b);
/* @behavior Iterates active entity slots and dispatches per-type handlers.
 * @source 0x801A0514
 */
void func_801A0514(void);
void func_801B3CCC(u32 arg0);
void func_801996FC(void);
void func_801995F8(void);
void func_801997EC(void);
void func_8019982C(void);

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
#define GAME_SPAWN_GATE_BYTE(index) \
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
