#ifndef BOF3_SRC_MODULES_GAME_00_INTERNAL_H
#define BOF3_SRC_MODULES_GAME_00_INTERNAL_H

#include "bof3/core/callback_scheduler.h"
#include "bof3/defines.h"
#include "bof3/modules/game/00.h"

typedef void (*GameEntry0StateHandler)(void);

#define BOF3_GAME_ENTRY0_STATE            (*(volatile u16*)0x80143b90u)
#define BOF3_GAME_ENTRY0_SUBSTATE         (*(volatile u16*)0x80143b92u)
#define BOF3_GAME_ENTRY0_WORLD_PHASE      (*(volatile u8*)0x80143bb0u)
#define BOF3_GAME_WORLD_STATE             (*(volatile u16*)0x80143f00u)
#define BOF3_GAME_ENTRY0_SELECTION_SEED   (*(volatile u8*)0x80143f1fu)
#define BOF3_GAME_ENTRY0_ACTIVE_SELECTION (*(volatile u32*)0x80144fc0u)
#define BOF3_GAME_FRONT_SELECTION         (*(volatile u8*)0x80145029u)
#define BOF3_GAME_PALETTE_STAGE_SERIAL    (*(volatile u8*)0x80145988u)
#define BOF3_GAME_ALT_FRONT_CALLBACK_TABLE \
  ((GameEntry0StateHandler const volatile*)0x801c7b08u)
#define BOF3_GAME_SELECTION_CALLBACK_TABLE \
  ((GameEntry0StateHandler const volatile*)0x801c7b14u)

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

#endif
