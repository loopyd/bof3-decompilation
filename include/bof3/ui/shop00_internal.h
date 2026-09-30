#ifndef EMI_SHOP_00_INTERNAL_H
#define EMI_SHOP_00_INTERNAL_H

#include "bof3/context.h"
#include "bof3/core.h"
#include "base/compiler.h"
#include "battle/ability.h"
#include "panel/task.h"
#include "gpu/prim.h"

/* @source 0x80148648
 * @kind unknown */
extern PanelTask* D_80148648;
#define D_80148648 g_PanelTaskRoot

/* Main-RAM byte tested by the sub-step handler func_801D5D98 before it clears
 * the halfword cells D_80145AC8/D_80145AC6 and arms sub-step 0x14; it has no
 * other reference in this target, is not owned by the shared map and its role
 * is not recovered, so it is kept raw at the original access width (lbu).
 * @source 0x80143BB4 @kind unknown */
extern u8 D_80143BB4;

/* @source 0x80143C40 @kind unknown */
extern volatile u16 D_80143C40;
extern u8 D_80148330[];
extern u8 D_80148331;
extern u8 D_80148332;
extern u8 D_80148333;
extern s16 D_80148334;
extern s16 D_80148336;
extern u8 D_8014833A;
extern s16 D_80148340;
extern u8 D_80148354;
extern u8 D_80148355;
extern u8 D_80148356;
extern u8 D_80148357;
extern s16 D_80148358;
extern s16 D_8014835A;
extern u8 D_8014835C;
extern u8 D_8014835E;
extern u8 D_8014835F;
extern u8 D_80148360;
extern u8 D_80148361;
extern u8 D_80148378[];
extern u8 D_80148379;
extern u8 D_8014837A;
extern u8 D_8014837B;
extern s16 D_8014837C;
extern s16 D_8014837E;
extern u8 D_80148382;
extern u8 D_801483E4;
extern u8 D_801483E5;
extern u8 D_801483E6;
extern s16 D_801483E8;
extern s16 D_801483EA;
extern u8 D_801483EE;
extern u8 D_801483EF;
extern u8 D_80148408;
extern u8 D_80148409;
extern u8 D_8014840A;
extern u8 D_8014840B;
extern u16 D_8014840C;
extern u16 D_8014840E;
extern u8 D_8014839D;
extern u8 D_8014839E;
extern u8 D_8014839F;
extern s16 D_801483A0;
extern s16 D_801483A2;
extern u8 D_80148624;
extern u8 D_80148625;
extern u8 D_80148626;
extern u8 D_8014862E;

/* Absolute-address globals (byte-width counters/flags). */
/* Write-only in this target (7 stores, no loads); role unproven.
 * @source 0x80148650
 * @kind unknown */
extern volatile u8  D_80148650;
/* UI phase byte: reset with the timer on phase changes and stepped (+1/+2)
 * by the phase handlers. Kept raw: the shared map owns this address and
 * emi/etc/game/00 consumes it as D_80148651 (declared non-volatile there too).
 * The plain view is byte-required: the phase step func_801DE45C's original
 * schedule hoists the phaseTimer re-arm constant above the phase-byte load
 * (li in the branch delay slot); a volatile view makes gcc treat the load as a
 * scheduling barrier and materializes the constant after it instead.
 * @source 0x80148651
 * @kind unknown */
extern u8 D_80148651;
/* Sub-step counter: advanced when phaseTimer wraps and by the step
 * handler, cleared by the UI-state reset. Kept raw (shared-map address).
 * @source 0x80148652
 * @kind unknown */
extern u8 D_80148652;
/* Shop work-block base read at entry by the phase steps func_801D7B74,
 * func_801D7CC8 and func_801D7D48: its first byte is the index passed to
 * func_801D7E1C and the byte at +5 is the counter handed to func_801D81B4
 * (read there with lbu and incremented). The phase handler func_801D4614 reads
 * that same +5 byte with a signed byte load (lb), so it views it through an
 * explicit signed pointer; no target-map row exists for the byte itself.
 * Kept raw: the block's field roles are
 * not recovered, and the address is not owned by the shared map.
 * @source 0x80148656
 * @kind unknown */
extern u8 D_80148656[];
/* Main-RAM byte read twice by the phase step func_801D7B74: as the fifth
 * (stack) argument of func_801D7E1C, loaded zero-extended with lbu, and as the
 * shift amount of the field-record bit that step publishes, loaded signed with
 * lb. No target-map row existed for this address before the lift and its role
 * is not recovered, so it is kept raw at both original access widths.
 * @source 0x8014865D
 * @kind unknown */
extern u8 D_8014865D;
/* Work-block byte at +5 of the block D_80148656, re-armed with 1 by the phase
 * step func_801D3AE4 when its frame timer wraps and read/incremented by the
 * byte-counter check func_801D81B4, which those steps hand the same address.
 * Written with a folded `sb %lo(D_8014865B)(at)` in the original, so it is
 * declared as a scalar byte rather than through the D_80148656 array view. Not
 * owned by the shared map; declared locally with a target-map row.
 * @source 0x8014865B @kind unknown */
extern u8 D_8014865B;
/* @source 0x8014865F @kind unknown */
extern volatile u8  D_8014865F;
/* Saved-state byte: archived into D_8014865F by the phase step, then set to
 * the -2 sentinel that forces the next phase. Read as a byte for the archive
 * and as a signed byte for the sentinel test.
 * @source 0x8014865C
 * @kind unknown */
extern s8 D_8014865C;
/* @source 0x80148654
 * @kind bss — per-frame phase timer; decremented each tick, zeroed on phase
 * changes, advances D_80148652 on wrap. */
extern volatile u8  phaseTimer;
/* Byte directly after the frame timer phaseTimer, cleared with the timer by
 * the countdown phase step func_801D7380 when its countdown wraps. Stored only
 * there (no load in this target), so its role is not recovered; it is the only
 * reference to this address, which the shared map does not own. Declared
 * locally with a target-map row. The same function also reaches the block base
 * byte 0x80148656 through its address (`&D_80148655 + 1`), because the
 * address-keyed map carries only one name per address (D_80148656 is the
 * work-block array view).
 * @source 0x80148655
 * @kind unknown */
extern u8 D_80148655;
/* @source 0x801490A4
 * @kind unknown */
extern volatile u16 D_801490A4;

/* Panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E29C8, which calls the selected handler with no
 * arguments (framed jalr) and then draws the panel at the same task root's
 * signed x/field-6 coordinates, bytes 0xA/0xB and the constant 0 through
 * func_801DA77C. The three pointer entries present in the shipped payload are
 * panelNoop (0x801E2800), retreatPanelField6ToNeg20B (0x801E2A30) and
 * advancePanelField6To38 (0x801E2A70); the table ends 0xC bytes in, where the
 * pointer table D_801E5DCC starts.
 * @source 0x801E5DC0
 * @kind table */
extern void (*D_801E5DC0[])(void);

/* Panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E2AB0, which calls the selected handler with no
 * arguments (framed jalr) and then draws the panel at the same task root's
 * x/field-6 coordinates through func_801DA2F4. The three pointer entries
 * present in the shipped payload are panelNoop (0x801E2800),
 * retreatPanelField6ToNeg20C (0x801E2B14) and advancePanelField6To38B
 * (0x801E2B54); the table ends 0xC bytes in, where the u16 table D_801E5DD8
 * starts.
 * @source 0x801E5DCC
 * @kind table */
extern void (*D_801E5DCC[])(void);

/* u16 table read by func_801E2BC4 with the panel task's byte 0xA as index; the
 * single shipped entry is 0x70.
 * @source 0x801E5DD8
 * @kind table */
extern u16 D_801E5DD8[];

/* Panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell) read as an unsigned byte
 * and scaled by 4; dispatched by the sibling dispatcher func_801E2C18, which
 * calls the selected handler with no arguments (framed jalr) and then forwards
 * the same task root's x and field-6 halfwords, the field-record index
 * D_80181B10[D_80144F5A[task byte 0xA]], task byte 0xB and task byte 0xC to the
 * field-record emitter func_801DB9F0. The five pointer entries present in the
 * shipped payload are panelNoop (0x801E2800), retreatPanelXToNeg150
 * (0x801E2C9C), advancePanelXTo17 (0x801E2CDC), retreatPanelField6To62
 * (0x801E2D1C) and func_801E2D5C; the table starts one word past the u16
 * D_801E5DD8 and ends where the pointer table D_801E5DF0 starts.
 * @source 0x801E5DDC
 * @kind table */
extern void (*D_801E5DDC[])(void);

/* Panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E2DB4, which calls the selected handler with no
 * arguments (framed jalr) and then draws the shop panel through func_801D8A0C
 * with the same task root's x and field-6 halfwords and the byte
 * D_80181B10[D_80144F5A[task byte 0xA]]. The three pointer entries present in
 * the shipped payload are panelNoop (0x801E2800), retreatPanelXToNeg150B
 * (0x801E2E30) and advancePanelXTo15 (0x801E2E70); the table ends 0xC bytes
 * in, where the pointer table D_801E5DFC starts.
 * @source 0x801E5DF0
 * @kind table */
extern void (*D_801E5DF0[])(void);

/* Panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by
 * func_801E2EB0, which calls the selected handler with no arguments and then
 * steps the same task through func_801E2F04. The four entries present in the
 * shipped image are panelNoop, advancePanelXTo320, retreatPanelXTo132 and
 * retreatPanelXTo70.
 * @source 0x801E5DFC
 * @kind table */
extern void (*D_801E5DFC[])(void);

/* Second panel handler table indexed with the panel task root's state byte
 * (D_80148648->state); dispatched by the sibling dispatcher func_801E3720,
 * which calls the selected handler with no arguments and then forwards the
 * same task root to func_801DC6FC. The three pointer entries present in the
 * shipped payload are panelNoop (0x801E2800), advancePanelXTo320B
 * (0x801E3774) and retreatPanelXTo75 (0x801E37B4).
 * @source 0x801E5E2C
 * @kind table */
extern void (*D_801E5E2C[])(void);

/* Third panel handler table indexed with the panel task root's state byte
 * (D_80148648->state); dispatched by the sibling dispatcher func_801E3BA4,
 * which calls the selected handler with no arguments and then forwards the
 * same task root to the shop panel step func_801E112C. The five pointer
 * entries present in the shipped payload are panelNoop (0x801E2800),
 * advancePanelXTo320C (0x801E3BF8), retreatPanelXTo150 (0x801E3C38),
 * advancePanelXTo40 (0x801E3C78) and retreatPanelXToNeg170 (0x801E3CB8);
 * the payload's code-pointer run continues in the adjacent table below, and
 * each per-panel table starts with the panelNoop idle handler.
 * @source 0x801E5E48
 * @kind table */
extern void (*D_801E5E48[])(void);

/* Fourth panel handler table indexed with the panel task root's state byte
 * (D_80148648->state); dispatched by the sibling dispatcher func_801E3CF8,
 * which calls the selected handler with no arguments (framed jalr) and then
 * forwards the same task root to the shop panel step func_801E1710. The five
 * pointer entries present in the shipped payload are panelNoop (0x801E2800),
 * advancePanelXTo320D (0x801E3D4C), retreatPanelXTo150B (0x801E3D8C),
 * retreatPanelXTo17 (0x801E3DCC) and advancePanelXTo150 (0x801E3E0C).
 * @source 0x801E5E5C
 * @kind table */
extern void (*D_801E5E5C[])(void);

/* Panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E3E4C, which calls the selected handler with no
 * arguments (framed jalr) and then forwards the same task root's x and field-6
 * halfwords, byte 0xA and the constants 0 and 0 to the shop field-list emitter
 * func_801D826C. The three pointer entries present in the shipped payload are
 * panelNoop (0x801E2800), retreatPanelXToNeg150C (0x801E3EB4) and
 * advancePanelXTo17B (0x801E3EF4); the table ends 0xC bytes in, where the
 * following pointer-free word starts.
 * @source 0x801E5E70
 * @kind table */
extern void (*D_801E5E70[])(void);

/* Fifth panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E4224, which calls the selected handler with no
 * arguments (framed jalr) and then forwards the same task root to the shop
 * panel step func_801E45C0. The thirteen pointer entries present in the
 * shipped payload alternate the panelNoop (0x801E2800) idle handler with the
 * shop panel tweens retreatPanelXToNeg120, advancePanelXTo100,
 * retreatPanelXTo17B, advancePanelXTo320E, retreatPanelXTo140,
 * advancePanelField6To240, retreatPanelField6To128, advancePanelXTo320F and
 * retreatPanelXTo210; the table ends where the following pointer-free word
 * starts.
 * @source 0x801E5F14
 * @kind table */
extern void (*D_801E5F14[])(void);

/* Sixth panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E4338, which calls the selected handler with no
 * arguments (framed jalr) and then forwards the same task root to the shop
 * panel emitter func_801E4DF4. The nine pointer entries present in the shipped
 * payload are panelNoop (0x801E2800), advancePanelXTo320E (0x801E438C),
 * retreatPanelXTo140 (0x801E43CC), panelNoop, advancePanelField6To240
 * (0x801E4460), retreatPanelField6To128 (0x801E44A0), panelNoop,
 * advancePanelXTo320F (0x801E4540) and retreatPanelXTo210 (0x801E4580); the
 * window starts 0x10 bytes into the pointer run declared by D_801E5F14 and
 * ends where the following pointer-free word starts.
 * @source 0x801E5F24
 * @kind table */
extern void (*D_801E5F24[])(void);

/* Seventh panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E440C, which calls the selected handler with no
 * arguments (framed jalr) and then forwards the same task root to the shop
 * panel emitter func_801E4E9C. The six pointer entries present in the shipped
 * payload are panelNoop (0x801E2800), advancePanelField6To240 (0x801E4460),
 * retreatPanelField6To128 (0x801E44A0), panelNoop, advancePanelXTo320F
 * (0x801E4540) and retreatPanelXTo210 (0x801E4580); the window starts 0x1C
 * bytes into the pointer run declared by D_801E5F14 and ends where the
 * following pointer-free word starts.
 * @source 0x801E5F30
 * @kind table */
extern void (*D_801E5F30[])(void);

/* Eighth panel handler table indexed with the panel task root's state byte
 * (D_80148648->state, the shared g_PanelTaskRoot cell); dispatched by the
 * sibling dispatcher func_801E44E0, which calls the selected handler with no
 * arguments (framed jalr) and then draws the panel at the same task root's
 * x/field-6 coordinates through func_801E1E80. The three pointer entries
 * present in the shipped payload are panelNoop (0x801E2800),
 * advancePanelXTo320F (0x801E4540) and retreatPanelXTo210 (0x801E4580); the
 * window starts 0x28 bytes into the pointer run declared by D_801E5F14 and
 * ends where the following pointer-free word starts.
 * @source 0x801E5F3C
 * @kind table */
extern void (*D_801E5F3C[])(void);

/* Main-RAM selection state (shared-map addresses). D_80143F02 is the shop
 * entry flag compared with 0x40; D_80143F00 is the world/selection id whose
 * values 0xBC/0x85/0xC1 select the shop phase step.
 * @source 0x80143F02 @kind unknown */
extern u8 D_80143F02;
/* @source 0x80143F00 @kind unknown */
extern u16 D_80143F00;

/* Shared main-RAM state byte read by the shop icon wrapper func_801D910C with
 * a signed byte load: when the wrapper's kind byte is 4 and this byte is at
 * least 8, the wrapper substitutes the kind 0xB. The address is the first byte
 * of the emi/etc/game/00 scenario state record (declared there as
 * scenarioState) and lies outside this EMI's payload, so this target declares
 * it locally as a raw signed byte at the original access width; no target-map
 * row existed for it before this lift.
 * @source 0x80146870
 * @kind unknown */
extern s8 D_80146870;

/* Main-RAM panel record table filled by func_801D6CE4: seven 32-bit words per
 * record (stride 0x1C bytes) taken from byte offset 0xCA0 of the shop state
 * block. Zero-filled in the shipped payload, so the record contents are not
 * observable there.
 * @source 0x801E6098
 * @kind table */
extern u32 D_801E6098[];

/* Overlay-local tail-data bytes shared by the D_801E530C phase-step family: the
 * sub-step handler func_801D472C clears D_801E6110 and D_801E6080, arms
 * D_801E60EC with 0x17 and archives the sub-step count in D_801E6094, while the
 * sibling steps func_801D47AC/func_801D4BBC read D_801E60EC and
 * func_801D4CF0/func_801D5D98 read D_801E6080/D_801E6110. All four are
 * zero-filled in the shipped payload, so their roles are not observable there;
 * each is kept as a raw byte at the width of the original access (sb/lbu).
 * @source 0x801E6080 @kind unknown */
extern u8 D_801E6080;
/* @source 0x801E6094 @kind unknown */
extern u8 D_801E6094;
/* @source 0x801E60EC @kind unknown */
extern u8 D_801E60EC;
/* @source 0x801E6110 @kind unknown */
extern u8 D_801E6110;

/* Overlay-local scan bound of rebuildPanelRecordTable: the bound of that
 * function's inner 0x18-byte-stride table scan, loaded with `lbu` once for the
 * loop entry test and re-loaded at every iteration because the loop body calls
 * func_801EEC30. Zero-filled in the shipped payload, so its value there is not
 * observable; kept as a raw byte at the width of the original load. The
 * address is not owned by the shared map, so this target declares it locally
 * and adds a target-map row.
 * @source 0x801E6078 @kind unknown */
extern u8 D_801E6078;

/* Overlay-local +0x15 slot byte of the first of the three 0x1C-byte records
 * of the panel record table D_801E6098: the table's record base is 0x801E6098,
 * so this byte is record 0's slot field at +0x15, and records 1 and 2 hold
 * theirs one and two 0x1C-byte strides further on. rebuildPanelRecordTable
 * clears all three slots to 0xFF through this address (records 2 and 1 through
 * the +0x38/+0x1C offsets, record 0 through the base) and later overwrites the
 * matched record's byte with the index of the table entry it copied, so the
 * field stays a raw byte with an evidenced offset rather than a named member.
 * The address is also the base of the seven-entry 0x38-byte-stride byte table
 * read by func_801D67B0, and it is zero-filled in the shipped payload. Not
 * owned by the shared map; declared locally with a target-map row.
 * @source 0x801E60AD @kind unknown */
extern u8 D_801E60AD;

/* Second overlay-local byte in the tail of this image (zero-filled in the
 * shipped payload); read as the row argument of the value-panel emitter
 * func_801DDFB0 by the phase step func_801D4CF0.
 * @source 0x801E60F8
 * @kind unknown */
extern u8 D_801E60F8;

/* This overlay's own flag byte (zero-filled in the shipped payload), cleared
 * by the phase step; read by other shop handlers.
 * @source 0x801E60F0
 * @kind unknown */
extern u8 D_801E60F0;

/* Second overlay-local gating byte (zero-filled in the shipped payload), read
 * twice by the phase step func_801D408C: it selects between resetting
 * phaseTimer and arming it with 0x2D when the advanced frame timer reaches 6,
 * and its value also biases the UI phase byte advance at that point.
 * @source 0x801E60F4
 * @kind unknown */
extern u8 D_801E60F4;

/* Second overlay-local byte in the tail of this image (zero-filled in the
 * shipped payload); cleared by the phase-advance handler func_801DF9B4.
 * @source 0x801E6260
 * @kind unknown */
extern u8 D_801E6260;

/* Overlay-local list-head counter read and post-incremented by the shop
 * list/state rebuild func_801E0F78 while it fills D_801E6264; byte after the
 * D_801E6260 flag. Zero-filled in the shipped payload, so its value there is
 * not observable. Not owned by the shared map; declared locally with a
 * target-map row.
 * @source 0x801E6258
 * @kind unknown */
extern u8 D_801E6258;

/* Overlay-local list of the field-record indices found by the shop list/state
 * rebuild func_801E0F78, appended at D_801E6258 and holding at most the eight
 * indices it scans. Zero-filled in the shipped payload, so the list contents
 * there are not observable. Not owned by the shared map; declared locally with
 * a target-map row.
 * @source 0x801E6264
 * @kind unknown */
extern u8 D_801E6264[];

/* Shared primitive cursor (PsyQ SDK, owned by the main exe).
 * @source 0x8014598C
 * @kind unknown */
extern u8* g_PrimCursor;

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * appendRenderPrim is a game primitive-append helper (lifted in exe/slus_004_22).
 * @source 0x8014E5A0 */
void appendRenderPrim(u32 ot_index, u32 primitive_size);
void func_801AEBA0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);
/* Main-exe shaded-panel rectangle helper (executable space, unlifted) called by
 * the shop panel emitter func_801DB938 with the task's x and field-6 halfwords,
 * the fixed 0x8B by 0x9F rectangle size, the low byte of that function's second
 * argument offset by 0xF0 and the main-RAM CLUT-bank byte D_80144952 as the
 * trailing stack argument. The AREA030 cell drawers and the sisyou00 panel
 * strip emitter submit the same rectangle through this address with the byte
 * global as the trailing argument; the widths below are the widths observed at
 * those reviewed call sites (u16 coordinates, word-size rectangle arguments),
 * not a recovered callee contract.
 * @source 0x801AE3F0 */
void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                   u32 arg5);

extern const char D_801D0E58[];
extern const char D_801D0E5C[];

/* Shop field-record emitter in this EMI (unlifted); the sibling dispatcher
 * func_801E2C18 calls it with the panel task's x and field-6 halfwords, the
 * field-record index D_80181B10[D_80144F5A[task byte 0xA]], task byte 0xB and
 * task byte 0xC. Its body reads both halfwords through 16-bit sign extensions
 * (coordinate arithmetic), masks the third argument to a byte and scales it by
 * the 164-byte record stride of the field table at D_80144968, compares the
 * fourth argument's low byte against 0xD (jump table) and reads the fifth from
 * the stack as a byte. The widths below are the widths observed at that call
 * site (lhu/lbu arguments, the fifth stored with a full word store), not a
 * recovered callee contract.
 * @source 0x801DB9F0 */
void func_801DB9F0(u16 arg0, u16 arg1, u8 arg2, u8 arg3, u8 arg4);
void func_801DBF18(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
/* Shop icon/sprite emitter in this EMI (unlifted) called by the wrapper
 * func_801D910C with two coordinates, a kind byte and a mode byte: its body
 * stores both coordinates as sprite halfwords, masks the third argument to a
 * byte, scales it by 4 and indexes a 4-byte-stride table with it, then picks
 * the sprite color from the fourth argument's low byte (0 -> 0x80, 1 -> 0x30,
 * otherwise 0x40). The call site widens all four arguments with
 * andi 0xFFFF/0xFFFF/0xFF/0xFF, so the widths below are the widths observed at
 * that call site, not a recovered callee contract.
 * @source 0x801D8FC0 */
void func_801D8FC0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Shop row emitter in this EMI (unlifted) called by func_801DB938 with the
 * task's x and field-6 halfwords, the leading byte of the 0x140-byte record
 * D_80145FCC selected by task byte 0xC and the low byte of that function's
 * second argument. Its body adds 2 to the first argument and 0x44 to the
 * second through 16-bit sign extensions (sll/sra pairs), masks the fourth
 * argument to a byte and turns it into a 7-or-0 offset; the widths below are
 * the widths observed at that call site (lhu/lbu arguments), not a recovered
 * callee contract.
 * @source 0x801D95F4 */
void func_801D95F4(u16 arg0, u16 arg1, u8 arg2, u8 arg3);
/* Shop row emitter in this EMI (unlifted) called by func_801DB938 with the same
 * arguments as func_801D95F4: the task's x and field-6 halfwords, the leading
 * byte of the 0x140-byte record D_80145FCC selected by task byte 0xC and the
 * low byte of that function's second argument. Its body adds 2 to both
 * coordinates through 16-bit sign extensions and masks the fourth argument to
 * a byte before turning it into a 7-or-0 offset; the widths below are the
 * widths observed at that call site (lhu/lbu arguments), not a recovered
 * callee contract.
 * @source 0x801D9254 */
void func_801D9254(u16 arg0, u16 arg1, u8 arg2, u8 arg3);
/* Shop panel draw helper in this EMI (unlifted); the sibling dispatcher
 * func_801E2DB4 draws the panel through it with the panel task root's x and
 * field-6 halfwords and the main-RAM byte D_80181B10[D_80144F5A[task byte
 * 0xA]]. The body sign-extends its first two arguments from 16 bits and masks
 * the third to a byte, and the call site loads them with `lh`/`lh`/`lbu`; the
 * widths below are the widths observed at that call site, not a recovered
 * callee contract.
 * @source 0x801D8A0C */
void func_801D8A0C(s16 arg0, s16 arg1, u8 arg2);
/* Shop helper (this EMI, unlifted); func_801E2BC4 passes the panel task's
 * x/field-6, its D_801E5DD8 table entry, 0x34, task byte 0xB and 6. The widths
 * below are the widths observed at that call site (lhu/lbu arguments), not a
 * recovered callee contract.
 * @source 0x801D8E2C */
void func_801D8E2C(u16 arg0, u16 arg1, u16 arg2, s32 arg3, s32 arg4, s32 arg5);
/* This-target record rebuild helper (unlifted); func_801D6668 calls it once
 * with the constant 0 when the global gate halfword D_80143C40 is clear. It
 * walks the eight 0xA4-byte records at D_80144968 (the loop index masked to a
 * byte, the argument masked to a byte twice inside the body) through
 * func_80164A44/func_801656EC; the width below is the width observed at that
 * call site, not a recovered callee contract.
 * @source 0x801D68D0 */
void func_801D68D0(u8 arg0);
/* Shop setup helper in this EMI (unlifted) called by the sibling phase steps
 * func_801D7B74/func_801D7CC8/func_801D7D48 with the work-block index byte
 * D_80148656[0], 0x98, 0x3F, 0 and a byte as the fifth (stack) argument. Its
 * body stores both halfword arguments from their zero-extended 16-bit forms
 * (andi 0xFFFF), masks the fourth and fifth arguments to bytes and reads the
 * fifth from the stack with lbu; the widths below are the widths observed at
 * that call site, not a recovered callee contract.
 * @source 0x801D7E1C */
void func_801D7E1C(u8 arg0, u16 arg1, u16 arg2, u8 arg3, u8 arg4);
/* Shop slot-list builder in this EMI (lifted partial:
 * src/bof3/ui/buildSlotList.c) called by the sibling phase steps
 * func_801D7B74/func_801D7CC8/func_801D7D48 with the constant 0; its body
 * masks the argument with andi 0xFF, so the parameter is a byte.
 * @source 0x801D8050 */
void buildSlotList(u8 arg0);
/* Shop byte-counter check in this EMI (unlifted) called by the sibling phase
 * steps func_801D7B74/func_801D7CC8/func_801D7D48 with the address of the
 * work-block byte D_8014865B and a limit byte: it reads that byte with lbu,
 * passes byte+0x63 to func_801502D0, re-arms phaseTimer while the byte is
 * nonzero, increments the byte and reports whether it now equals the limit by
 * returning 0 or 1 in a full word. The return is declared word-wide because
 * the callers mask it explicitly with andi 0xFF.
 * @source 0x801D81B4 */
u32 func_801D81B4(u8* arg0, u8 arg1);
void func_801644D8(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4, u32 arg5);
/* Main-exe byte-result query called by this target's shop list/state rebuild
 * func_801E0F78 and by the unlifted handlers func_801DA77C and func_801DF9EC
 * with the constant 0xF in `$a0`; every reviewed call site masks the result
 * with `andi v0,v0,0xFF` before testing it, so the return is declared
 * word-wide (the caller-side extension of a byte result) and the single
 * argument word-wide. The width below is the width observed at those call
 * sites, not a recovered callee contract.
 * @source 0x80166140 */
u32 func_80166140(u32 arg0);
/* Main-exe field-record rebuild helper called by the phase step func_801D7B74
 * with the address of the 164-byte field record it just updated, and by this
 * target's func_801D68D0 while it walks the same table. The same address is
 * declared as func_80164A44(volatile void*) by emi/etc/game/00 and as
 * func_80164A44(u32) by emi/battle/battle/03; the width below is the address
 * form observed at this call site, not a recovered callee contract.
 * @source 0x80164A44 */
void func_80164A44(void* arg0);
u32 func_801502D0(u32 arg0);
/* Main-exe cue dispatcher called by this target's phase steps with a single
 * word cue id: func_801D3AE4 passes 0xD1, and the unlifted steps func_801D446C
 * and func_801D5D98 pass 0xD5 and 0xD0, always as a bare `addiu a0,zero,K`.
 * All three call sites drop the result, and the id range sits next to the cue
 * ids handed to func_8015DF18, so the width below is the width observed at
 * those call sites, not a recovered callee contract.
 * @source 0x80150284 */
void func_80150284(u32 arg0);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
void func_801636A0(u32 arg0, u32 arg1);

/* Main-exe frontend-transition predicate polled by the shop UI phase handlers
 * tickPhaseTimerThenAdvanceUiSubstep (0x801D43F0) and its byte-identical phase
 * twin tickPhaseTimerThenAdvanceUiPhase (0x801D65EC) once the frame timer
 * phaseTimer has expired. Its body in the main EXE is one signed byte load and compare,
 * `return *(s8*)0x8014682D < 1`, so it reports nonzero while the signed
 * counter at 0x8014682D reads zero or below; the call site masks the result
 * with `andi v0,v0,0xFF` before branching. The width below is the
 * caller-side width, and emi/etc/commu00/00 declares the same address as
 * u32 func_80163EA0(void).
 * @source 0x80163EA0 */
u32 func_80163EA0(void);

/* Main-exe frontend-local mode setter (lifted as setFrontLocalMode in
 * src/bof3/ui/setFrontLocalMode.c) called by the shop sub-step handler
 * tickPhaseTimerThenAdvanceUiSubstep with the bare constant 1 after it
 * advances the UI sub-step byte D_80148652, and by the phase-step twin
 * tickPhaseTimerThenAdvanceUiPhase (0x801D65EC) with the same constant after
 * it advances the UI phase byte D_80148651; the result is dropped at both
 * sites. The lifted EXE source stores `mode & 0xFF` into the frontend-local
 * mode halfword at 0x80143C90 and installs callback slot 2, both only while
 * 0x80143C40 is clear; the width below is the width of that lifted source.
 * @source 0x8014ECAC */
void func_8014ECAC(u16 local_mode);

/* Main-exe sound cue dispatcher (lifted in src/bof3/audio/dispatchSoundCue.c)
 * called by the shop sub-step handler func_801E2084 with the cue id 0x102.
 * The battle, scenario and world targets declare this address as
 * func_8015DF18(u16), so the width below is that reviewed call-site width.
 * @source 0x8015DF18 */
void func_8015DF18(u16 arg0);

/* Main-exe front-end frame slice starter called by the phase handler
 * tickPhaseTimerThenBeginFrame when the per-frame phase timer phaseTimer wraps
 * to zero; the call site sets no argument registers and drops the result, and
 * the reviewed declaration in include/bof3/ui/game00_internal.h states that it
 * begins one shared front-end frame/update slice, so it is argument- and
 * return-free.
 * @source 0x80158E50 */
void func_80158E50(void);

/* Main-exe mode-slot scanner called by this target's buildSlotList with the
 * constant 0 as the only argument and used as the slot-loop bound after its
 * byte result is masked (the call site emits `andi v0,v0,0xFF` after the jal,
 * the caller-side extension of a byte return). The same address is lifted as
 * findModeFreeSlot(u8 mode) by emi/etc/game/00, which scans a mode-indexed
 * table for the 0xFF sentinel; the byte width below is the width observed at
 * this call site, not a recovered callee contract.
 * @source 0x801BDB7C */
u8 func_801BDB7C(u8 arg0);

/* Shop panel task draw helper in this EMI (unlifted); the sibling dispatcher
 * func_801E2AB0 draws the panel through it with the panel task root's x and
 * field-6 halfwords, the constant 0 and the main-RAM value D_80144F50. Both
 * task arguments are loaded with `lhu` (the u16 fields at the observed call
 * site), and the body uses its fourth argument as the value formatted by
 * sprintf into D_80145AD4 with the format string at D_801D0D08. The widths
 * below are the widths observed at that call site, not a recovered callee
 * contract.
 * @source 0x801DA2F4 */
void func_801DA2F4(u16 arg0, u16 arg1, s32 arg2, s32 arg3);
/* Shop panel draw helper in this EMI (unlifted); the sibling dispatcher
 * func_801E29C8 draws the panel through it with the panel task root's signed
 * x and field-6 halfwords, its bytes 0xA and 0xB and the constant 0. The first
 * two arguments are signed halfwords loaded with `lh`; the two byte arguments
 * are loaded with `lbu`. The widths below are the widths observed at that call
 * site, not a recovered callee contract.
 * @source 0x801DA77C */
void func_801DA77C(s16 arg0, s16 arg1, u8 arg2, u8 arg3, s32 arg4);
/* Shop panel helper (this EMI); the shop forwarders call it with the global
 * panel task root.
 * @source 0x801E3284 */
void func_801E3284(PanelTask* task);

/* Shop panel draw helper in this EMI (unlifted); the sibling dispatcher
 * func_801E44E0 draws the panel through it with the panel task root's x and
 * field-6 halfwords. Both arguments are signed halfwords: the call site loads
 * them with `lh` (the u16 fields promoted to the callee's s16 parameters), as
 * against the `lhu` of the u16 prototype calls.
 * @source 0x801E1E80 */
void func_801E1E80(s16 arg0, s16 arg1);

/* Shop panel task step in this EMI (unlifted); func_801E2EB0 calls it with the
 * global panel task root after the state dispatch. Its body reads the u16
 * x/field-6 halfwords at offsets 0x4/0x6 of its first argument, so the argument
 * is the panel task root (same layout as func_801E3284).
 * @source 0x801E2F04 */
void func_801E2F04(PanelTask* task);

/* Shop panel step in this EMI (unlifted); the sibling dispatcher func_801E3720
 * forwards the global panel task root to it after the state dispatch. Its body
 * reads the u16 at offsets 0x4/0x6 and bytes at 0x9/0xA/0xB/0x12 of its first
 * argument, so the argument is the panel task root (same layout as
 * func_801E3284).
 * @source 0x801DC6FC */
void func_801DC6FC(PanelTask* task);

/* Shop panel step in this EMI (unlifted); the sibling dispatcher func_801E3BA4
 * forwards the global panel task root to it after the state dispatch. Its body
 * reads the u16 x/field-6 halfwords at offsets 0x4/0x6 and the bytes at
 * 0xA/0xD of its first argument, so the argument is the panel task root (same
 * layout as func_801E3284).
 * @source 0x801E112C */
void func_801E112C(PanelTask* task);

/* Shop panel step in this EMI (unlifted); the sibling dispatcher func_801E3CF8
 * forwards the global panel task root to it after the state dispatch. Its body
 * reads the u16 x/field-6 halfwords at offsets 0x4/0x6 and the bytes at
 * 0xA/0xB/0xC/0xD of its first argument and writes the 0xD byte back, so the
 * argument is the panel task root (same layout as func_801E3284).
 * @source 0x801E1710 */
void func_801E1710(PanelTask* task);

/* Shop panel draw helper (this EMI); func_801E3B7C forwards the global panel
 * task root to it. It reads the same task layout as func_801E3284 (u16 x/y at
 * 0x4/0x6 and bytes at 0xA/0xB/0xD of the argument).
 * @source 0x801E37F4 */
void func_801E37F4(PanelTask* task);

/* Shop panel step in this EMI (unlifted); the sibling dispatcher func_801E4224
 * forwards the global panel task root to it after the state dispatch. Its body
 * copies its first argument into a callee-saved register and reads the u16
 * x/field-6 halfwords at offsets 0x4/0x6 and the bytes at 0xB/0xC/0xD of that
 * argument (writing byte 0xD back), so the argument is the panel task root
 * (same layout as func_801E3284).
 * @source 0x801E45C0 */
void func_801E45C0(PanelTask* task);

/* Shop panel emitter (this EMI); the dispatcher func_801E4338 forwards the
 * global panel task root to it. It reads the u16 x/y at 0x4/0x6 and the byte
 * at 0xA of the argument task (same layout as func_801E3284 and
 * func_801E45C0) and drives the primitive helpers func_801AE3F0,
 * func_801DD350, func_801502D0 and func_8014F800 with coordinates derived
 * from those fields.
 * @source 0x801E4DF4 */
void func_801E4DF4(PanelTask* task);

/* Shop panel emitter in this EMI (lifted: func_801E4E9C.c); the sibling
 * dispatcher func_801E440C forwards the global panel task root to it after the
 * state dispatch. Its body reads the u16 x/field-6 halfwords at offsets 0x4/0x6
 * of its first argument, so the argument is the panel task root (same layout as
 * func_801E3284).
 * @source 0x801E4E9C */
void func_801E4E9C(PanelTask* task);

/* Shop field-record emitter in this EMI; called only by func_801E4E9C, which
 * passes the task's x/field-6 halfwords offset by its running icon position and
 * the record's byte at +0. The widths below are the widths observed at that
 * call site (two `u16` truncated coordinates and one `lbu`), not a recovered
 * callee contract.
 * @source 0x801E4FC8 */
void func_801E4FC8(u16 arg0, u16 arg1, u8 arg2, s32 arg3);

/* Main-exe panel frame drawer called by this EMI's panel emitters
 * func_801D6D6C, func_801D7E1C, func_801E2F04, func_801E4DF4 and
 * func_801E4E9C with two sign-extended task halfwords (`lh`) and two small
 * constants. The widths below are the widths observed at those call sites, not
 * a recovered callee contract.
 * @source 0x801DD350 */
void func_801DD350(s16 arg0, s16 arg1, s32 arg2, s32 arg3);

/* Shop panel helper (this EMI); func_801E3F34 forwards the global panel task
 * root to it. It reads the u16 x/y at 0x4/0x6 of the argument task (same
 * layout as func_801E3284) and emits the panel primitives through
 * func_801AE3F0.
 * @source 0x801E3F5C */
void func_801E3F5C(PanelTask* task);

/* Shop field-list emitter in this EMI (unlifted); the sibling dispatcher
 * func_801E3E4C calls it with the panel task root's x and field-6 halfwords,
 * its byte 0xA and the constants 0 and 0. The byte argument indexes the same
 * 164-byte field records this target's func_801E4D4C scans (0x80144968 +
 * 0xA4 * index), and the fourth argument's low byte selects between the forced
 * highlight value 1 and a value derived from the record's 0xC halfword. The
 * widths below are the widths observed at that call site (lhu/lbu arguments),
 * not a recovered callee contract.
 * @source 0x801D826C */
void func_801D826C(u16 arg0, u16 arg1, u8 arg2, s32 arg3, s32 arg4);

/* Shop value-panel emitter in this EMI (lifted partial: func_801DDFB0); the
 * phase step func_801D4CF0 draws the sliding shop value panel through it with
 * the panel x derived from the frame timer phaseTimer, the y constant 0x4C,
 * the overlay flag byte D_801E60F8, the main-RAM byte D_8018E260 and the
 * variant constant 1. The three trailing arguments are loaded with `lbu` at
 * that call site (flags word, row byte, stack byte); the widths below are the
 * widths observed at that call site, not a recovered callee contract.
 * @source 0x801DDFB0 */
void func_801DDFB0(s16 arg0, s16 arg1, u32 arg2, u8 arg3, u8 arg4);

/* Shop panel strip emitter (this EMI); the phase step func_801D46E0 calls it
 * with the fixed panel arguments (0x14, 0x12, 0x118, 0x13) and the main-RAM
 * CLUT-bank byte D_80144952 as the fifth argument.
 * @source 0x801DAB90 */
void func_801DAB90(s32 arg0, s32 arg1, s32 arg2, s32 arg3, u8 arg4);

/* Shop panel row emitter in this EMI (unlifted); the phase step func_801D5390
 * calls it with the phase-relative row coordinate (phaseTimer * 80 + 0x20),
 * the constant 0x30 and the constant 0. The first argument is sign-extended
 * from 16 bits at that call site, so the parameter is a signed halfword; the
 * second and third are plain word moves of small constants, so no width is
 * proven for them here, and the widths below are the widths observed at that
 * call site, not a recovered callee contract. Its body adds 0x10 to the first
 * argument through a 16-bit sign extension and strides the seven-entry table
 * at D_801E60AD by index * 0x38 while adding index * 0x38 + 6 to the second.
 * @source 0x801D67B0 */
void func_801D67B0(s16 arg0, s32 arg1, s32 arg2);

/* Shop panel row draw helper in this EMI (unlifted); the phase step
 * func_801D408C draws the phase-relative row through it with
 * phaseTimer * 48 + 0x6E as the x coordinate and the y constant 0x4C. Both
 * arguments are plain word values at that call site (the x coordinate is
 * computed in a word register and the y constant is materialized as an
 * `li`), so no narrower width is proven; the widths below are the widths
 * observed at that call site, not a recovered callee contract.
 * @source 0x801D6D6C */
void func_801D6D6C(s32 arg0, s32 arg1);

/* @source 0x801E5D2C
 * @kind table — shop command handler pointers (this EMI, text blob
 * T_801E5144); dispatched by dispatchCommand with the command id
 * scaled by 4 as the handler argument. */
extern void (*commandHandlerTable[])(u32);

/* @source 0x801E52C4
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D39BC with the phase byte D_80148651 as index. */
extern void (*D_801E52C4[])(void);

/* @source 0x801E5360
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D6184 with the phase byte D_80148651 as index. */
extern void (*D_801E5360[])(void);

/* @source 0x801E545C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D7344 with the phase byte D_80148651 as index. */
extern void (*D_801E545C[])(void);

/* @source 0x801E5BEC
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DE188 with the phase byte D_80148651 as index. */
extern void (*D_801E5BEC[])(void);

/* @source 0x801E52D8
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D3AA8 with the phase byte D_80148652 as index. */
extern void (*D_801E52D8[])(void);

/* @source 0x801E5D10
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E092C with the phase byte D_80148652 as index. */
extern void (*D_801E5D10[])(void);

/* @source 0x801E52F0
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D41B0 with the phase byte D_80148652 as index. */
extern void (*D_801E52F0[])(void);

/* @source 0x801E530C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D46A4 with the phase byte D_80148652 as index. */
extern void (*D_801E530C[])(void);

/* @source 0x801E5BFC
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DE1C4 with the phase byte D_80148652 as index. */
extern void (*D_801E5BFC[])(void);

/* @source 0x801E5C08
 * @kind table */
extern void (*D_801E5C08[])(void);

/* @source 0x801E5CFC
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DFDB8 with the phase byte D_80148652 as index. */
extern void (*D_801E5CFC[])(void);

/* @source 0x801E5CE8
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DF978 with the phase byte D_80148651 as index. */
extern void (*D_801E5CE8[])(void);

/* @source 0x801E5D3C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E200C with the phase byte D_80148651 as index. */
extern void (*D_801E5D3C[])(void);

/* @source 0x801E5D48
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E2048 with the sub-step byte D_80148652 as index. */
extern void (*D_801E5D48[])(void);

/* @source 0x801E5D50
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E2114 with the sub-step byte D_80148652 as index. */
extern void (*D_801E5D50[])(void);

/* @source 0x801E5D5C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E2590 with the sub-step byte D_80148652 as index. */
extern void (*D_801E5D5C[])(void);

/* @source 0x801E5D68
 * @kind table — handler pointers (this EMI); dispatched by func_801E27BC
 * with panel task byte 2 (g_PanelTaskRoot->unk_00[2]) as index. */
extern void (*D_801E5D68[])(void);

/* This-EMI helper (unlifted) called by the phase handler func_801DE40C before
 * it tests the global gate halfword D_80143C40: the caller sets no argument
 * registers and drops the result, so the call is argument- and return-free at
 * that site. Its body starts by reading the main-RAM byte D_80144953.
 * @source 0x801DF6A8 */
void func_801DF6A8(void);
void appendFullscreenDimTile(void);
void appendFullscreenDimTileB(void);
void func_801DE8E8(void);

/* Shop UI state initializer in this EMI (lifted: initializeShopUiState,
 * 0x801E2650); called with no arguments by the sub-step handler
 * func_801E2084.
 * @source 0x801E2650 */
void initializeShopUiState(void);

/* Sprite-grid renderer in this EMI (unlifted) called by the phase step
 * func_801DE45C with the main-RAM index byte D_80144953: it masks the low byte
 * and uses it to index the D_801E5B1C table, which selects the per-row sprite
 * colour counts, then emits 5 bands of 2 semi-transparent rows by repeating
 * SetSprt/appendRenderPrim with the palette entry D_801E5A24[colour].
 * @source 0x801DBD24 */
void func_801DBD24(u8 arg0);

/* Byte-swap helper in this EMI (already lifted); called by the skill-notes
 * scanner func_801E1BCC with the two addresses of an adjacent zero/non-zero
 * byte pair inside the 128-slot main-RAM bank D_8014546C.
 * @source 0x801E1BB8 */
void swapBytes(u8* arg0, u8* arg1);

/* Skill-notes bank normalizer in this EMI (already lifted): it walks the
 * 128-slot main-RAM bank D_8014546C and swaps adjacent byte pairs whose first
 * byte is zero and second byte is not. It runs first inside the command
 * handlers sortSkillNotesByCostDescending and func_801E1D84.
 * @source 0x801E1BCC */
void func_801E1BCC(void);

/* Numeric list rebuilder in this EMI (unlifted) called by the phase step
 * func_801DE45C with no arguments; it reads phaseTimer and the value tables
 * through func_801D826C/sprintf/func_80150098 and finally emits the shop panel
 * strip through func_801DAB90.
 * @source 0x801DEE64 */
void func_801DEE64(void);

/* Shop list/state rebuild in this EMI (resets D_801E6258, refills the eight
 * D_801E6264 entries, then rewrites D_80148331/0x32/0x33); called by the phase
 * advance handler func_801DF9B4 with no arguments.
 * @source 0x801E0F78 */
void func_801E0F78(void);

/* Main-exe helper (unlifted, outside this EMI's payload) called once by
 * rebuildPanelRecordTable with the constant 1, the address of the
 * 0x18-byte-stride entry selected in the main-RAM table D_8018E990, the
 * constant 1, 0x1C and the constant 0xEA0 as the fifth (stack) argument. The
 * call site materializes every argument with a bare `addiu`/`addu` and drops
 * the result, so the widths below are the widths observed at that call site,
 * not a recovered callee contract. The address had no prototype in this target
 * before this lift; it is bound with a target-map row and WEAK_SYMBOL_AT.
 * @source 0x801EEC30 */
void func_801EEC30(u32 arg0, void* arg1, u32 arg2, u32 arg3, u32 arg4);

/* Shared flag-bank tester: the entry predicate func_801E4C2C passes the
 * main-RAM bank base D_80144F28 and the flag index 0x6C and treats a nonzero
 * return as "flag set". The two-argument (base, flag) prototype is the one
 * carried by emi/etc/commu00/00, emi/etc/sisyou/00, emi/scenario/scena00/00,
 * emi/scenario/scena16/00 and emi/world00/area027/13; the address has no other
 * reference in this target and is bound with a target-map row and
 * WEAK_SYMBOL_AT.
 * @source 0x8015B5D4 */
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Byte-list membership test lifted in src/bof3/ui/isSkillRecorded.c as a
 * 32-bit-returning function; the entry predicate func_801E4C2C reads only its
 * low byte, which is why that call site truncates the result with (u8).
 * @source 0x801E4D4C */
s32 func_801E4D4C(u8 arg0);

typedef struct ShopValueRecord {
  u16 value;
  u8 unk_02[6];
} ShopValueRecord;

extern ShopValueRecord D_801CB8DC[][99];

/* Main-RAM shop value tables: the u16 value at record offset 0 of the table
 * selected by func_801DA9C4's mode byte. Only the record stride is proven by
 * the original's index scaling (9/12/11/10 u16 = 18/24/22/20 bytes); the
 * record tails are not recovered (all four tables are zero-filled in the
 * shipped image, so no record content is observable).
 * @source 0x801C8974 @kind table */
extern u16 D_801C8974[];
/* @source 0x801C90F2 @kind table */
extern u16 D_801C90F2[];
/* @source 0x801C98B8 @kind table */
extern u16 D_801C98B8[];
/* @source 0x801C9E8E @kind table */
extern u16 D_801C9E8E[];

/* Main-RAM CLUT-bank byte read as the fifth argument of the shop panel strip
 * emitter func_801DAB90 by the phase step func_801D46E0; the same address is
 * declared as D_80144952 by emi/etc/sisyou/00.
 * @source 0x80144952 @kind unknown */
extern u8 D_80144952;

/* Main-RAM index byte read at the entry of the phase step func_801DE45C and
 * passed as the only argument of the sprite-grid renderer func_801DBD24, which
 * masks it to 0xFF and indexes the D_801E5B1C table with it. It is the byte
 * after the CLUT-bank byte D_80144952; the two are separate target-map rows.
 * @source 0x80144953 @kind unknown */
extern u8 D_80144953;

/* Main-RAM value drawn by the shop panel task helper func_801DA2F4, which
 * formats it through sprintf with the format string at D_801D0D08; read as the
 * fourth argument of the panel dispatcher func_801E2AB0.
 * @source 0x80144F50 @kind unknown */
extern s32 D_80144F50;

/* Main-RAM byte table read by the shop panel dispatcher func_801E2DB4 with the
 * panel task byte 0xA as index; its byte result indexes D_80181B10. The same
 * address is declared as D_80144F5A by emi/etc/game/00 (indexed there by the
 * mode byte) and it is owned by the shared map, so no target map row is added.
 * @source 0x80144F5A @kind unknown */
extern u8 D_80144F5A[];

/* Main-RAM flag byte whose bits 3, 4 and 5 are the shop requirement ids 0xB,
 * 0xD and 0xE tested by the entry predicate func_801E4C2C; the sisyou overlay
 * sets those same three bits in func_801D3A24_sisyou00. The address is owned by
 * the shared map, so no target map row is added.
 * @source 0x80144F59 @kind unknown */
extern u8 D_80144F59;

/* Main-RAM flag bank whose flag 0x6C is the shop requirement id 0xC read by
 * the entry predicate func_801E4C2C through the shared flag tester
 * func_8015B5D4, which takes this address as its base argument. The same bank
 * is declared by emi/etc/commu00/00, emi/etc/game/00, emi/world00/area027/13 and
 * emi/world03/area127/13; it is not owned by the shared map, so this target
 * declares it locally and adds a target-map row.
 * @source 0x80144F28 @kind unknown */
extern u8 D_80144F28;

/* Main-RAM halfword cells cleared together (`sh $zero`) by the sub-step handler
 * func_801D5D98 on its cue 0xD0 path, immediately before it arms sub-step
 * 0x14. The two stores are adjacent and hand-written to $at in the original;
 * the addresses have no other reference in this target and are not owned by
 * the shared map, so each keeps its own scalar declaration and target-map row.
 * D_80145AC8 is declared volatile u16 by emi/battle/battle/15.
 * @source 0x80145AC6 @kind unknown */
extern u16 D_80145AC6;
/* @source 0x80145AC8 @kind unknown */
extern u16 D_80145AC8;

/* Main-RAM leading byte of the 0x140-byte-stride record table walked as the
 * D_80146254 records at D_80145FCC by emi/etc/game/00, emi/etc/sisyou/00 and
 * emi/battle/battle/03. Read by this target's func_801DB938 to select the
 * record named by its task byte 0xC and passed as the third argument of the
 * shop row emitters func_801D95F4 and func_801D9254. The address is not owned
 * by the shared map, so this target declares it locally and adds a target-map
 * row.
 * @source 0x80145FCC
 * @kind table */
extern u8 D_80145FCC[];

/* Main-RAM byte read as the row argument of the value-panel emitter
 * func_801DDFB0 by the phase step func_801D4CF0, which loads it with `lbu` and
 * passes it as the emitter's fourth argument. Zero-filled in the shipped
 * payload, so its role is not observable there; kept raw at the access width.
 * @source 0x8018E260
 * @kind unknown */
extern u8 D_8018E260;

/* Main-RAM table of 0x18-byte entries scanned by rebuildPanelRecordTable:
 * for each of the three panel records it walks the entries in order, forms the
 * entry address 0x8018E990 + 0x18 * j and passes it to func_801EEC30 together
 * with the constants 1, 1, 0x1C and 0xEA0, and compares the entry's byte at
 * +0x11 minus 0x30 against the panel record index 0..2 to select that record's
 * entry. The table lies in main RAM outside this EMI's payload, is not owned by
 * the shared map and had no reference in this target before this lift, so it is
 * declared locally with a target-map row. Only the 0x18 stride and the +0x11
 * byte position are proven by the original's index scaling and by that byte's
 * separate `lbu`; the entry's other bytes are not recovered, so the leading and
 * trailing runs stay unnamed padding and the scanned byte keeps the raw
 * `unk_11` spelling. The named struct-array field is required: it is what makes
 * the compiler emit the original's symbol-relative
 * `lui $at, %hi(entry); addu $at, $at, scaled_index; lbu %lo(...)($at)` form
 * instead of materialising the table address into a callee-saved register.
 * @source 0x8018E990 @kind table */
typedef struct ShopSlotEntry {
  u8 unk_00[0x11];
  u8 unk_11;
  u8 unk_12[6];
} ShopSlotEntry;
extern ShopSlotEntry D_8018E990[];

/* Main-RAM state-byte table read by the shop panel dispatcher func_801E2DB4 as
 * the third argument of func_801D8A0C, indexed by D_80144F5A[task byte 0xA].
 * The same address is declared as D_80181B10 by emi/battle/battle/03, which
 * indexes it with a battle state byte; kept raw because the byte meanings are
 * not recovered.
 * @source 0x80181B10 @kind unknown */
extern u8 D_80181B10[];

/* Main-RAM field-record table: eight 164-byte (0xA4) records starting at
 * 0x80144968. The phase step func_801D7B74 rebuilds the record selected by
 * D_80145FCC through func_80164A44, this target's func_801D68D0 walks the same
 * eight records, and emi/etc/game/00 (status bytes +0x0E..+0x13) and
 * emi/battle/battle/03 (41-word records) declare the same table. The address is
 * not owned by the shared map, so this target declares it locally and adds a
 * target-map row.
 * @source 0x80144968 @kind table */
extern u8 D_80144968[];

/* Main-RAM byte inside each 164-byte field record, at offset +0x19: the phase
 * step func_801D7B74 writes 1 << (s8)D_8014865D into it for the record selected
 * by D_80145FCC, so the field is a bit mask byte whose contents are not
 * recovered. Declared as its own byte list at the original access width (the
 * sibling D_801449E2 is the same record's +0x7A list); no target-map row
 * existed for it before this lift.
 * @source 0x80144981 @kind unknown */
extern u8 D_80144981[];

/* Main-RAM bit-flag byte at offset +0x07 of each 164-byte field record of the
 * table D_80144968: the shop list/state rebuild func_801E0F78 scans the eight
 * records and keeps the index of every one whose low bit is set. Declared as
 * its own byte list at the original access width (the sibling D_80144981 is
 * the same record's +0x19 list); no target-map row existed for it before this
 * lift.
 * @source 0x8014496F @kind unknown */
extern u8 D_8014496F[];

/* Main-RAM skill lists scanned by func_801E4D4C. D_801449E2 is the ten-slot
 * list at offset +0x7A of the field records (record base 0x80144968, stride
 * 164); D_8014546C is the 128-slot global skill-notes bank.
 * @source 0x801449E2 @kind table */
extern u8 D_801449E2[];
/* @source 0x8014546C @kind table */
extern u8 D_8014546C[];

/* Front-end play-time clock bytes formatted as the two h/m digit fields of the
 * shop header row by func_801D7080: the byte at 0x80144FC0 is the hour cell and
 * the byte at 0x80144FC1 the minute cell, each read with `lbu` and split into
 * its tens and units halves through the unsigned divide-by-ten sequence. Both
 * addresses are owned by the shared map, so no target-map row is added. They are
 * plain `u8` here: the original folds one byte access per source expression with
 * no volatile reload side effects (emi/etc/game/00 reads the same cells as the
 * front-end clock and declares its own reviewed widths, including a volatile
 * view).
 * @source 0x80144FC0 @kind unknown */
extern u8 D_80144FC0;
/* @source 0x80144FC1 @kind unknown */
extern u8 D_80144FC1;

/* Main-RAM level byte formatted as the two digits of the `Lv.` field of the
 * same header row by func_801D7080, which loads it once per digit with `lbu` and
 * splits each load into tens and units. Not owned by the shared map, so this
 * target declares it locally and adds a target-map row.
 * @source 0x8014496E @kind unknown */
extern u8 D_8014496E;

/* Shared sprintf work buffer of the front-end row formatters: the row builder
 * func_801D7080 clears its 0x40 bytes through `sb` and hands it to sprintf as
 * the destination. The address is owned by the shared map, so no target-map row
 * is added.
 * @source 0x80145AD4 @kind unknown */
extern u8 D_80145AD4[];

/* EMI-local table of the ten full-width digit strings: the ten 4-byte char
 * pointers at 0x801E5434 point at the Shift-JIS "0".."9" strings of
 * 0x801E540C..0x801E5430 (one 2-byte glyph plus a terminator each), and
 * func_801D7080 passes two of them per formatted number as the `%s` arguments
 * of the header row (tens then units). Not owned by the shared map; declared
 * locally with a target-map row.
 * @source 0x801E5434 @kind table */
extern const char* D_801E5434[];

/* EMI-local table of requirement lists for the shop entry predicate
 * func_801E4C2C, which indexes it with the entry id byte and walks the
 * 0xFF-terminated byte list it selects: the 17 live entries 0..0x10 point into
 * the list block at 0x801E5F6C..0x801E5FBD (entry 0 -> {0xAA, 0x27, 0x03,
 * 0x14}, entry 0x10 -> {0x9E, 0xAB, 0x26}); entries 0xB..0xE (11..14) are empty
 * lists, which is why those four ids are answered by the flag tests in
 * func_801E4C2C instead. Not owned by the shared map; declared locally with a
 * target-map row.
 * @source 0x801E5FC0 @kind table */
extern u8* D_801E5FC0[];

/* EMI-local byte selecting the table entry formatted into the bracketed slot
 * of the header row built by func_801D7080. Zero-filled in the shipped payload,
 * so its role is not observable there; kept raw at the access width. Not owned
 * by the shared map; declared locally with a target-map row.
 * @source 0x801E6108 @kind unknown */
extern u8 D_801E6108;

/* EMI-local Shift-JIS format string of the shop header row (the
 * "(%s) %s%s h %s%s m Lv.%s%s" row) passed to sprintf by func_801D7080. Not
 * owned by the shared map; declared locally with a target-map row.
 * @source 0x801D0C98 @kind rodata */
extern const char D_801D0C98[];

/* Main-RAM ability record table: 0x14-byte records at 0x801CA70C whose cost
 * byte at +0x0E is the sort key of the skill-notes ordering command handler
 * sortSkillNotesByCostDescending, which reads the byte at 0x801CA71A as
 * abilityObjects[kind].cost. The record layout is owned by
 * include/battle/ability.h; the same address is declared as abilityObjects by
 * emi/etc/game/00 and as ABILITY_OBJECTS by emi/battle/battle/03.
 * @source 0x801CA70C @kind table */
extern AbilityObject abilityObjects[];

#endif
