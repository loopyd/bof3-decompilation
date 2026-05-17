#ifndef BOF3_SRC_MODULES_GAME_00_INTERNAL_H
#define BOF3_SRC_MODULES_GAME_00_INTERNAL_H

#include "bof3/bof3.h"


typedef void (*GameEntry0StateHandler)(void);

/* Work area struct accessed via scratchpad pointer (0x1F800044) */
struct GameWorkArea {
 u8 flags_00; /* 0x00 - entity flags */
 u8 unk_01; /* 0x01 */
 u8 flags_02; /* 0x02 */
 u8 pad_03[0x02]; /* 0x03-0x04 */
 u8 field_05; /* 0x05 - region/scene index */
 u8 unk_06; /* 0x06 */
 u8 unk_07; /* 0x07 */
 u8 route_index_08; /* 0x08 */
 u8 pad_09[0x02]; /* 0x09-0x0A */
 u8 field_0B; /* 0x0B */
 u8 pad_0C[0x0C]; /* 0x0C-0x17 */
 s32 unk_18; /* 0x18 */
 u8 pad_1C[0x0D]; /* 0x1C-0x28 */
 u8 unk_29; /* 0x29 */
 u8 unk_2A; /* 0x2A */
 u8 pad_2B[0x09]; /* 0x2B-0x33 */
 s32 coord_x_34; /* 0x34 */
 s32 coord_y_38; /* 0x38 */
 u16 counter_3E; /* 0x3E */
 u8 pad_40[0x09]; /* 0x40-0x48 */
 u32 unk_49; /* 0x49 */
 u8 pad_4D[0x0F]; /* 0x4D-0x5B */
 u8 flags_5C; /* 0x5C */
 u8 unk_5D; /* 0x5D */
 u8 unk_5E; /* 0x5E */
 u8 unk_5F; /* 0x5F */
 u8 pad_60[0x10]; /* 0x60-0x6F */
 u8 speed_70; /* 0x70 */
 u8 pad_71[0x03]; /* 0x71-0x73 */
 u16 anim_state_74; /* 0x74 */
};

/* Scratchpad work pointer (PS1 hardware register at 0x1F800044) */
#define SCRATCH_WORK VPPTR(struct GameWorkArea, 0x1F800044u)

/* Global work pointer in main exe data section */
#define GLOBAL_WORK_PTR VPPTR(u8, 0x80146250u)

/* Movement/position offset tables in main exe data section */
#define MOVEMENT_OFFSET_0(i) VS32(0x80181B94u + (i) * 8)
#define MOVEMENT_OFFSET_1(i) VS32(0x80181B98u + (i) * 8)
#define MOVEMENT_THRESHOLD(i) VS16(0x80181B70u + (i) * 2)

#define GAME_ENTRY0_STATE VU16(0x80143b90u)
#define GAME_ENTRY0_SUBSTATE VU16(0x80143b92u)
#define GAME_ENTRY0_WORLD_PHASE VU8(0x80143bb0u)
#define BOF3_GAME_WORLD_STATE VU16(0x80143f00u)
#define GAME_ENTRY0_SELECTION_SEED VU8(0x80143f1fu)
#define GAME_ENTRY0_ACTIVE_SELECTION VU32(0x80144fc0u)
#define GAME_FRONT_SELECTION VU8(0x80145029u)
#define BOF3_GAME_PALETTE_STAGE_SERIAL VU8(0x80145988u)
#define BOF3_GAME_ALT_FRONT_CALLBACK_TABLE \
 CVPTR(GameEntry0StateHandler, 0x801c7b08u)
#define BOF3_GAME_SELECTION_CALLBACK_TABLE \
 CVPTR(GameEntry0StateHandler, 0x801c7b14u)

/* does: clears one local GAME entry-0 record slot by index.
 * @source: 0x801960c0
 */
void func_801960c0(u8 record_index);

/* does: seeds the shared callback/frame dispatch prologue before the entry-0
 * callback tables begin running.
 * @source: 0x8014ba04
 */
void func_8014ba04(void);

/* does: begins one shared front-end frame/update slice.
 * @source: 0x80158e50
 */
void func_80158e50(void);

/* does: finalizes one shared front-end frame/update slice.
 * @source: 0x80158c80
 */
void func_80158c80(void);

/* does: runs one selection-side post-dispatch update slice.
 * @source: 0x80198cac
 */
void func_80198cac(void);

/* does: resets the entry-0 front script/runtime bank for the requested mode.
 * @source: 0x801c1400
 */
void func_801c1400(u32 mode);

/* does: copies the active front selector/context bundle into the entry-0 local
 * runtime state.
 * @source: 0x8019fa28
 */
void func_8019fa28(u16 selection_seed, u32 context_a, u32 context_b,
 u8 context_kind);

/* does: copies the shared CPU-side palette bank before the corresponding VRAM
 * upload path.
 * @source: 0x8014e284
 */
void func_8014e284(void);

/* does: starts streaming one archive slot through the EXE-side EMI loader.
 * @source: 0x80161fdc
 */
void emi_stream_init_slot(u32 slot_id);

/* alias for exact assembly matching via normalization */
void func_80161fdc(u32 slot_id);

/* does: begins streaming the currently selected SCENA pack for the seeded
 * scenario state.
 * @source: 0x801a7804
 */
void func_801a7804(void);

/* does: enters the loaded scenario-local dispatch path after the SCENA loader
 * completes.
 * @source: 0x801a782c
 */
void func_801a782c(void);

/* does: ticks the shared world/front waiting path while the scenario loader is
 * still pending.
 * @source: 0x801992b8
 */
void func_801992b8(void);

/* does: returns a pointer into one of two sprite-rect tables indexed by
 * sprite_id * 4, with the table chosen by flags & 1.
 * @source: 0x801af270
 */
u8* func_801af270(u8 sprite_id, u8 flags);

/* does: draws one sprite by filling a GT quad primitive from a rect-table
 * entry, selecting CLUT by flags & 2, then appending to the OT.
 * @source: 0x801af2a0
 */
void func_801af2a0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* does: iterates a packed sprite-record table and draws each sprite via
 * func_801af2a0 with signed offsets shifted by 3 applied to base coords.
 * @source: 0x801af390
 */
void func_801af390(s16 base_x, s16 base_y, const u8* record_table, u8 flags);

/* does: computes a screen-space position from an entity's offset-adjusted
 * coordinates; returns the result as a signed 16-bit value.
 * @source: 0x80154f28
 */
s16 func_80154f28(s32 x, s32 y);

/* does: ??? placeholder for externally-defined work; called with no args.
 * @source: 0x8014d978
 */
u8 func_8014d978(void);

/* from old bof3/include/bof3/modules/game/00.h */
void func_8019611c(void);
void func_80196f78(void);
void func_80196ffc(void);
void func_80197068(void);
void func_801970ec(void);
void func_801a7704(u8 scenario_index);

#endif
