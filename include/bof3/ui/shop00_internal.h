#ifndef EMI_SHOP_00_INTERNAL_H
#define EMI_SHOP_00_INTERNAL_H

#include "bof3/context.h"
#include "bof3/core.h"
#include "base/compiler.h"
#include "battle/ability.h"
#include "panel/task.h"
#include "gpu/prim.h"

typedef struct ShopPanelWords {
  u32 w[7];
} ShopPanelWords;

/* One 6-byte entry of the shop selection-cursor geometry table D_801E5BC8:
 * the two halfwords are the position pair forwarded to the panel helper
 * func_801D8E2C and the flag byte at +4 selects the sprite/palette constants
 * of that call. The second raw byte at +5 is never read; it sets the stride. */
typedef struct ShopSelectionCell {
  u16 x;
  u16 y;
  u8 unk_04;
  u8 unk_05;
} ShopSelectionCell;

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
 * func_801D7E1C and the byte at +5 is the counter handed to
 * tickPhaseTimerThenAdvanceWorkCounter (read there with lbu and incremented). The phase handler func_801D4614 reads
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
/* Shop selection cursor of the visible/off-screen id lists: the kind byte
 * selects the list (0 = D_80144F5A, nonzero = D_801E6244) and the byte at
 * +1 is the slot index inside it. The selection swap swapSelectionIds
 * exchanges this cursor pair with the saved pair D_8014865A/D_8014865D and
 * loads all four cells signed (lb), which is why the three cells below are
 * declared signed; the unlifted confirm handler func_801DE4C8 writes the 0x7F
 * "no saved slot" sentinel into the saved pair and also reads D_80148658 and
 * D_80148659 zero-extended at two sites. Not owned by the shared map; the
 * generated undefined-symbol list carries the addresses, so no target-map row
 * is added.
 * @source 0x80148658
 * @kind unknown */
extern s8 D_80148658;
/* @source 0x80148659 @kind unknown */
extern s8 D_80148659;
/* @source 0x8014865A @kind unknown */
extern s8 D_8014865A;
/* Work-block byte at +5 of the block D_80148656, re-armed with 1 by the phase
 * step func_801D3AE4 when its frame timer wraps and read/incremented by the
 * byte-counter step tickPhaseTimerThenAdvanceWorkCounter, which those steps
 * hand the same address.
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

/* Live save-data block (0x10B0 bytes) that the save-data prepare routine
 * prepareSaveDataImageAndPublishPanelRecord fills from the scenario state and
 * copies into the 0x1E00-byte memory-card image buffer at 0x800C1A00. The save
 * format puts the flag bank D_80144F28 at byte +0x650, the u16 checksum that
 * the routine clears and re-sums at +0x70 and the first party record at +0x90;
 * the shop keeps its save-slot panel record at +0xCA0. Every access is
 * byte-offset arithmetic from this base, so it stays a raw byte array. The
 * address is owned by the shared map, so no target-map row is added.
 * @source 0x801448D8 @kind table */
extern u8 D_801448D8[];

/* Story-progress bytes of the save header, copied to save-data offsets
 * 0x20..0x23 by the prepare routine: the shared scenario-progress byte at
 * 0x80146864 and the three following story bytes. The first is owned by the
 * shared map; the three raw bytes are declared locally and auto-bind by their
 * D_<address> names.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Save-header bytes written by the prepare routine, each named separately
 * because the original materializes every address with its own `lui`/`sb`
 * pair instead of indexing one base register: the story bytes at 0x20..0x23,
 * the world-id halfword at +0x24 (D_801448FC), the game-mode record byte at
 * +0x27 (D_801448FF), the player position words at +0x28/+0x2C and the
 * cleared checksum halfword at +0x70 (D_80144948).
 * @source 0x801448F8 @kind unknown */
extern u8 D_801448F8;
/* @source 0x801448F9 @kind unknown */
extern u8 D_801448F9;
/* @source 0x801448FA @kind unknown */
extern u8 D_801448FA;
/* @source 0x801448FB @kind unknown */
extern u8 D_801448FB;
/* @source 0x801448FC @kind unknown */
extern u16 D_801448FC;
/* @source 0x801448FF @kind unknown */
extern u8 D_801448FF;
/* @source 0x80144900 @kind unknown */
extern u32 D_80144900;
/* @source 0x80144904 @kind unknown */
extern u32 D_80144904;
/* @source 0x80144948 @kind unknown */
extern u16 D_80144948;

/* Main-RAM player position words (Q16 tile) published into save-data offsets
 * 0x28/0x2C by the prepare routine.
 * @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;

/* First byte of the main-RAM game-mode record table, copied to save-data
 * offset 0x27 by the prepare routine at the width of the original `lbu`.
 * @source 0x80145E98 @kind table */
extern u8 D_80145E98[];

/* Main-RAM block whose nine halfwords the prepare routine copies to save-data
 * offset 0xCBC. The block sits just before the shared text buffer D_80145AD4
 * and its role is not recovered, so it keeps its address name.
 * @source 0x80145AB4 @kind table */
extern u16 D_80145AB4[];

/* u16 checksum cell of the memory-card image: the prepare routine stores the
 * byte-sum over the 0x10B0 live save bytes there after copying them into the
 * image buffer at the bare constant address 0x800C1A00 (the original
 * materializes that base with `lui`/`ori`, i.e. as an integer, while this cell
 * is a map symbol and keeps its own `lui`/`sh` pair).
 * @source 0x800C1A70 @kind unknown */
extern u16 D_800C1A70;

/* Main-RAM byte that the prepare routine republishes from the shared world-flag
 * halfword D_80146258 (stored as its low byte).
 * @source 0x80145025 @kind unknown */
extern u8 D_80145025;

/* Shared world-flag halfword republished by the prepare routine into
 * D_80145025; owned by the shared map, so no target-map row is added.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Overlay-local panel record table filled by func_801D6CE4: seven 32-bit words per
 * record (stride 0x1C bytes) taken from byte offset 0xCA0 of the shop state
 * block. Zero-filled in the shipped payload, so the record contents are not
 * observable there.
 * @source 0x801E6098
 * @kind table */
extern ShopPanelWords D_801E6098[];

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

/* Overlay-local entry count of the save-slot collector
 * collectSaveSlotsThenCountFreeBlocks: it stores the count returned by the
 * main-exe directory reader func_801EFCA8 there and re-reads it for the loop
 * test of every entry. The list navigation handlers func_801D4E74 and
 * func_801D69E0 step the same byte as the selected row of that list, and
 * func_801D4BBC clears it once the list has been published. Zero-filled in the
 * shipped payload, so its value there is not observable; kept as a raw byte at
 * the width of the original loads and stores. Not owned by the shared map, so
 * this target declares it locally and adds a target-map row.
 * @source 0x801E6074 @kind unknown */
extern u8 D_801E6074;

/* Overlay-local remaining-capacity byte of the save list: the collector
 * collectSaveSlotsThenCountFreeBlocks publishes 0xF minus the blocks used by the
 * BOF3 entries it kept, so a nonzero value means the list has room for one more
 * entry. func_801D4E74 widens its cursor bound by one when this byte is nonzero
 * and D_801E6078 is not 3, and func_801D4BBC selects the id 5 instead of 4.
 * Zero-filled in the shipped payload, so its value there is not observable;
 * kept as a raw byte at the width of the original loads and stores. Not owned
 * by the shared map, so this target declares it locally and adds a target-map
 * row.
 * @source 0x801E60FC @kind unknown */
extern u8 D_801E60FC;
/* EMI-local off-screen id list and its count byte: the sub-step handler
 * rebuildShopSlotLists clears the count, then appends the id of every enabled
 * 164-byte field record that the visible list D_80144F5A does not already
 * hold. The selection swap swapSelectionIds indexes the list with the saved
 * slot index and rewrites every entry equal to the saved id. Both cells sit
 * inside this EMI's tail segment, so no undefined or target-map row exists for
 * them; the names are exact address labels.
 * @source 0x801E6244 @kind table */
extern u8 D_801E6244[];
/* @source 0x801E6250 @kind unknown */
extern u8 D_801E6250;

/* EMI-local slot pair lists written by the sub-step handler
 * rebuildShopSlotLists: for slot index i below the visible-list length it sets
 * D_801E6248[2 * i] to 1 and then clears every remaining pair byte up to the
 * third slot to 0, and for every slot the off-screen list D_801E6244 fills it
 * sets the sibling byte D_801E6249[2 * i] to 1. Both are written with `sb` at
 * the two-byte slot stride, so each is declared as a byte array indexed by
 * twice the slot. The pair byte D_801E6254 is re-armed with 0xFF by the same
 * handler and is otherwise only a store target in this target, so its role is
 * not recovered. They sit inside this EMI's tail segment, so no undefined or
 * target-map row exists for them; the names are exact address labels.
 * @source 0x801E6248 @kind table */
extern u8 D_801E6248[];
/* @source 0x801E6249 @kind table */
extern u8 D_801E6249[];
/* @source 0x801E6254 @kind unknown */
extern u8 D_801E6254;

/* Overlay-local kept-entry cursor of the save-slot collector
 * collectSaveSlotsThenCountFreeBlocks: it clears this byte before the scan,
 * uses it as the destination index of every kept entry, increments it through
 * its own address, and finally publishes it as D_801E6078. Zero-filled in the
 * shipped payload, so its value there is not observable; kept as a raw byte at
 * the width of the original loads and stores. Not owned by the shared map, so
 * this target declares it locally and adds a target-map row.
 * @source 0x801E610C @kind unknown */
extern u8 D_801E610C;

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

/* Overlay-local run of four signed halfwords (eight bytes) that ends where
 * D_801E6108, the slot byte of the shop header row, begins. Zero-filled in the
 * shipped payload, so its contents are not observable there and its role is
 * not recovered. Two handlers walk it with the same 0..3 bound and the same
 * nonzero test: the sub-step handler
 * emitPanelStripThenRebuildShopStateOnClearGate indexes it with a signed
 * halfword loop counter and passes that counter to func_8016BB6C, and
 * func_801D62DC indexes it with an unsigned byte counter and passes the same
 * counter. Signed halfword element width is the original's `lh` load at both
 * sites. The D_<address> name binds to its own address, so no target-map row
 * is added.
 * @source 0x801E6100 @kind unknown */
extern s16 D_801E6100[];

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

/* Main-exe save-directory reader (executable space, unlifted) called by the
 * save-slot collector collectSaveSlotsThenCountFreeBlocks with the wildcard
 * pattern D_801D0C94 and the 0x10 entry limit: the same call site stores the
 * returned count as a byte into D_801E6074 and tests that byte, so the return is
 * read only as a byte here. The record size it fills the table D_8018E990 with
 * is not part of that call site's evidence; the collector's own 0x18 stride is
 * what that table's entry type records. The widths below are the widths observed
 * at that call site, not a recovered callee contract.
 * @source 0x801EFCA8 */
s32 func_801EFCA8(char* pattern, s32 limit);

/* Main-exe front-end text emitter (executable space, unlifted) that draws the
 * shop header row buffer D_80145AD4 at the alignment and row constants the
 * save-data prepare routine passes. The widths below are the widths observed at
 * that call site, not a recovered callee contract; the address had no
 * prototype in this target before this lift and is bound with a target-map
 * row.
 * @source 0x801EF27C */
void func_801EF27C(s32 arg0, s32 arg1, u8* arg2);

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
/* Shop byte-counter step in this EMI (lifted: src/bof3/ui/
 * tickPhaseTimerThenAdvanceWorkCounter.c) called by the sibling phase steps
 * func_801D7B74/func_801D7CC8/func_801D7D48 with the address of the work-block
 * byte D_8014865B and a limit byte: it reads that byte with lbu, submits the
 * texture row 0x1C at row 0x14 through func_8014F800 with func_801502D0(byte +
 * 0x63) as the fifth argument, counts phaseTimer down, re-arms the timer with
 * 0x3C once the countdown reaches zero or the pad state D_80145AA8 is nonzero,
 * increments the byte and reports whether it now equals the limit by returning
 * 0 or 1 in a full word. The return is declared word-wide because the callers
 * mask it explicitly with andi 0xFF.
 * @source 0x801D81B4 */
u32 tickPhaseTimerThenAdvanceWorkCounter(u8* arg0, u8 arg1);
/* Literal variant of the byte-counter step above (lifted: src/bof3/ui/
 * tickPhaseTimerThenAdvanceWorkCounterB.c): it reads the counter byte through
 * its pointer argument, submits the texture row 0x1C at row 0x16 through
 * func_8014F800 with func_801502D0(byte + 0x85) as the fifth argument, counts
 * phaseTimer down, re-arms the timer with 0xA once the countdown reaches zero
 * or the pad state D_80145AA8 is nonzero, increments the byte and reports
 * whether it now equals the limit by returning 0 or 1 in a full word. No
 * assembled call site and no payload pointer names this entry, so the widths
 * below are the ones its body shows.
 * @source 0x801D66F8 */
u32 tickPhaseTimerThenAdvanceWorkCounterB(u8* arg0, u8 arg1);
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
/* Main-exe slot setter called by this target's setFieldRecordSlotValue
 * (src/bof3/ui/setFieldRecordSlotValue.c) with the helper group of the slot it
 * is about to write, the new byte value, 1 and 0, and by emi/world00/area030/04
 * with the same (group, value, 1, 0) shape; every reviewed call site drops the
 * result, so the widths below are the widths observed at those call sites, not
 * a recovered callee contract.
 * @source 0x801665A0 */
u8 func_801665A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Main-exe gate called by this target's setFieldRecordSlotValue with the same
 * helper group it just passed to func_801665A0 and the record byte that the
 * store has not replaced yet; the result is discarded here, and the reviewed
 * call sites of func_801D3A24 test only its low byte, so the widths below are
 * the widths observed at those call sites, not a recovered callee contract.
 * @source 0x801650B4 */
u8 func_801650B4(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Main-exe per-entry helper called by the two handlers that walk the four
 * halfwords D_801E6100: emitPanelStripThenRebuildShopStateOnClearGate passes
 * the constant 0 and its signed halfword loop counter, and func_801D62DC
 * passes the constant 0 and its byte counter, both as bare `addiu a0,zero,0`
 * plus an already-materialized second argument. Neither call site masks the
 * second argument to a byte, so the widths below are the widths observed at
 * those call sites, not a recovered callee contract.
 * @source 0x8016BB6C */
void func_8016BB6C(s32 arg0, s32 arg1);
u32 func_801502D0(u32 arg0);
/* Main-exe cue dispatcher called by this target's phase steps with a single
 * word cue id: func_801D3AE4 passes 0xD1,
 * emitPanelStripThenRebuildShopStateOnClearGate passes 0xD5 and func_801D5D98
 * passes 0xD0, always as a bare `addiu a0,zero,K`.
 * All three call sites drop the result, and the id range sits next to the cue
 * ids handed to func_8015DF18, so the width below is the width observed at
 * those call sites, not a recovered callee contract.
 * @source 0x80150284 */
void func_80150284(u32 arg0);
/* Byte-pair swap of this EMI (lifted: src/bof3/ui/swapBytesC.c) called by the
 * selection swap swapSelectionIds with two addresses inside the id list its
 * kind byte selects; it exchanges the two bytes in place.
 * @source 0x801DF464 */
void swapBytesC(u8* arg0, u8* arg1);
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

/* Shop panel prompt helper in this EMI (unlifted); the sub-step handler
 * redrawPanelStripThenConditionallyAdvanceUiSubstep calls it with no arguments
 * and tests the low byte of the result. Its body submits the texture row 0x1C
 * at row 0x16 through func_8014F800 with the value returned by
 * func_801502D0(0xF), masks a coordinate through func_801647C4, then tests the
 * input halfwords D_80145AC4 and D_80145AC2 against D_80145AA8, plays a cue
 * through func_8015DF18 on each accepted test and toggles the work-block byte
 * D_8014865B. Every return path loads $v0 with the constant 1 or 0, so the
 * result is a whole word.
 * @source 0x801DA678 */
s32 func_801DA678(void);

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
 * dispatchUiPhaseTable with the phase byte D_80148651 as index. */
extern void (*D_801E52C4[])(void);

/* @source 0x801E5360
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D6184 with the phase byte D_80148651 as index. */
extern void (*D_801E5360[])(void);

/* @source 0x801E545C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D7344 with the phase byte D_80148651 as index. */
extern void (*D_801E545C[])(void);

/* Shop selection-cursor geometry: 36 bytes of six 6-byte cells addressed as
 * [slot][column] by drawShopSlotListWithSelectionCursors, which multiplies the
 * saved/live slot cell by 12 and the kind cell by 6. The shipped payload holds
 * three rows of two entries: column 0 at x 0x11 with y 0x3E/0x74/0xAA and
 * column 1 at x 0x9D with y 0x41/0x75/0xA9, and the flag byte at +4 is 0 in
 * column 0 and 1 in column 1. Read-only payload data, so no target-map row is
 * added and the extracted rodata label carries the address.
 * @source 0x801E5BC8
 * @kind table */
extern ShopSelectionCell D_801E5BC8[3][2];

/* @source 0x801E5BEC
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DE188 with the phase byte D_80148651 as index. */
extern void (*D_801E5BEC[])(void);

/* @source 0x801E52D8
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * dispatchUiSubStepTable with the phase byte D_80148652 as index. */
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

/* Shop screen draw (lifted); called with no arguments by the phase step
 * func_801DE40C, the cursor/confirm handler func_801DE4C8 and the sub-step
 * handler func_801DE88C, all of which drop the result, so the call is
 * argument- and return-free at those sites.
 * @source 0x801DF6A8 */
void drawShopSlotListWithSelectionCursors(void);

/* Shop header-row builder (lifted: func_801D7080_shop00.c); the save-data
 * prepare routine calls it with no arguments before drawing the row through
 * func_801EF27C.
 * @source 0x801D7080 */
void func_801D7080(void);

/* Save-slot panel record copier (lifted: func_801D6CE4_shop00.c); the save-data
 * prepare routine hands it the selected save slot D_801E6074 and the save-data
 * block whose panel record at +0xCA0 is copied into that slot's record.
 * @source 0x801D6CE4 */
void func_801D6CE4(u8 arg0, u8* arg1);
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

/* Shop slot-list renderer in this EMI (lifted: drawNumberedShopSlotListThenPanelStrip,
 * 0x801DEE64); called by the phase step func_801DE45C with no arguments. It
 * reads phaseTimer and the value tables through func_801D826C/sprintf/
 * func_80150098, then emits the slidable off-screen slot region through
 * func_801DF00C and the shop panel strip through func_801DAB90.
 * @source 0x801DEE64 */
void drawNumberedShopSlotListThenPanelStrip(void);

/* Slidable shop slot region in this EMI (unlifted) drawn by
 * drawNumberedShopSlotListThenPanelStrip: it submits the shaded region through
 * func_801DC1F4 with the 0x12 by 0x15 size and then walks the off-screen count
 * byte D_801E6250, drawing one row per off-screen field record. The caller
 * passes the phase-derived x (0x96 + phaseTimer * 45 masked to 0xFFFF) and the
 * y constant 0x3B; both are re-sign-extended at the entry of the body, so the
 * widths below are the widths observed at that call site.
 * @source 0x801DF00C */
void func_801DF00C(u16 arg0, s32 arg1);

/* Main-exe front-end text emitter (unlifted, outside this EMI's payload) that
 * draws the shared text buffer D_80145AD4: drawNumberedShopSlotListThenPanelStrip
 * passes the panel-relative signed x, the row y as a sign-extended halfword,
 * the alignment constant 0 and the buffer, so the widths below are the widths
 * observed at that call site, not a recovered callee contract. The address has
 * no prototype in this target before this lift; it is bound by the generated
 * undefined-symbol list, so no target-map row is added.
 * @source 0x80150098 */
void func_80150098(s32 arg0, s16 arg1, s32 arg2, u8* arg3);

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

/* Second main-RAM id byte list of the visible slots, walked by the selection
 * swap swapSelectionIds in step with D_80144F5A: every slot whose byte equals
 * the current selection id is overwritten with the saved one. Not owned by the
 * shared map; the generated undefined-symbol list carries the address, so no
 * target-map row is added.
 * @source 0x80144F5D @kind unknown */
extern u8 D_80144F5D[];

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

/* Shared pad-state halfword: the sub-step handler
 * drawShopPanelsThenDecrementUiSubstep only tests it for a nonzero value and
 * steps the UI sub-step byte D_80148652 back by one. Other overlays AND it with
 * the pressed-button masks D_80145AC2/D_80145AC4 (this target's func_801DA678
 * does the same) and test single bits such as 0x1000, so it carries the current
 * button state bits. It is not owned by the shared map, so this target declares
 * it locally and adds a target-map row.
 * @source 0x80145AA8 @kind unknown */
extern u16 D_80145AA8;

/* Shared pressed-button mask tested against the same pad state D_80145AA8: the
 * entry-0 sibling func_801E2150 takes its cue-0x106 cancel arm when this mask
 * matches the pad state, and the pad helper func_801DA678 tests it next to
 * D_80145AC2 against that state. emi/etc/sisyou/00 and emi/world00/area030/04
 * declare the same address. The shared map does not own it; the
 * D_<address> name binds to its own address, so no target-map row is added.
 * @source 0x80145AC4 @kind unknown */
extern u16 D_80145AC4;

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
 * The +0x14 word is read whole (`lw`) by the save-slot collector
 * collectSaveSlotsThenCountFreeBlocks, which splits it into a 13-bit low half
 * and the bits above 13; that word is the only typed member of the entry.
 * @source 0x8018E990 @kind table */
typedef struct ShopSlotEntry {
  u8 unk_00[0x11];
  u8 unk_11;
  u8 unk_12[2];
  u32 unk_14;
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
 *
 * The record's +5 byte is its id: the sub-step handler rebuildShopSlotLists
 * reads it as its own byte list indexed by the record offset, and the id lists
 * it fills (D_80144F5A, D_801E6244) are the values D_80181B10 maps back to a
 * record index in the panel dispatchers. It is reached through this owning
 * table with the +5 offset rather than a second symbol at 0x8014496D, because
 * the original folds the offset into the same `%lo` immediate.
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
 * splits each load into tens and units. It is the +6 byte of the first 164-byte
 * field record of D_80144968; the phase step
 * drawShopSlotListThenChargeOnConfirm charges 10 per level of a record and
 * reaches that byte through the owning table (D_80144968[record_offset + 6]),
 * because the original folds the offset into the same `%lo` immediate. Not
 * owned by the shared map, so this target declares it locally and adds a
 * target-map row.
 * @source 0x8014496E @kind unknown */
extern u8 D_8014496E;

/* Shared sprintf work buffer of the front-end row formatters: the row builder
 * func_801D7080 clears its 0x40 bytes through `sb` and hands it to sprintf as
 * the destination. The address is owned by the shared map, so no target-map row
 * is added.
 * @source 0x80145AD4 @kind unknown */
extern u8 D_80145AD4[];

/* EMI-local decimal format string "%d" passed to sprintf by
 * drawNumberedShopSlotListThenPanelStrip: it formats the one-based ordinal of
 * the visible slot whose row is being drawn into the shared text buffer. The
 * payload holds it unmodified. Not owned by the shared map; declared locally
 * with a target-map row.
 * @source 0x801D0E60 @kind rodata */
extern const char D_801D0E60[];

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

/* EMI-local 16-character save-name prefix of this game ("BASLUS-00422BOF3", the
 * PSX save-file name prefix, stored without a terminator inside those 16 bytes)
 * that the save-slot collector collectSaveSlotsThenCountFreeBlocks compares
 * against the first 0x10 bytes of every directory entry through strncmp. The
 * payload holds it unmodified, so the contents above are original data. Not
 * owned by the shared map, so this target declares it locally and adds a
 * target-map row.
 * @source 0x801D0C20 @kind rodata */
extern const char D_801D0C20[];

/* EMI-local save-name pattern handed to the main-exe directory reader
 * func_801EFCA8 by the save-slot collector collectSaveSlotsThenCountFreeBlocks:
 * the payload holds the two bytes "*" and a terminator here, and the reader is
 * called with the 0x10 entry limit that the collected list never exceeds. Not
 * owned by the shared map, so this target declares it locally and adds a
 * target-map row.
 * @source 0x801D0C94 @kind rodata */
extern char D_801D0C94[];

/* Main-RAM ability record table: 0x14-byte records at 0x801CA70C whose cost
 * byte at +0x0E is the sort key of the skill-notes ordering command handler
 * sortSkillNotesByCostDescending, which reads the byte at 0x801CA71A as
 * abilityObjects[kind].cost. The record layout is owned by
 * include/battle/ability.h; the same address is declared as abilityObjects by
 * emi/etc/game/00 and as ABILITY_OBJECTS by emi/battle/battle/03.
 * @source 0x801CA70C @kind table */
extern AbilityObject abilityObjects[];

#endif
