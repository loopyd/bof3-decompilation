#ifndef EMI_SISYOU_00_INTERNAL_H
#define EMI_SISYOU_00_INTERNAL_H

#include "bof3/context.h"
#include "gpu/prim.h"

/* One 8-byte entry of the shared 0x801CB8DC value table; only the leading u16
 * is consumed by this overlay. */
typedef struct SisyouValueRecord {
  u16 value;
  u8 unk_02[6];
} SisyouValueRecord;

/* Word/halfword views of the 0x0C status halfword pair of the 164-byte records
 * at 0x80144968: the badge test reads the whole word, the marker and limit
 * tests read single status bits through the halfword view. */
typedef union SisyouEntryStatus {
  u32 word;
  u16 half;
} SisyouEntryStatus;

/* One 164-byte entry of the 0x80144968 record table; only the fields consumed
 * by func_801D177C are named. */
typedef struct SisyouEntry {
  u8 unk_00[5];
  u8 unk_05;                /* icon kind selected by func_801D23A0 */
  u8 unk_06;                /* value byte and func_801D1B28 argument */
  u8 unk_07;
  u32 unk_08;               /* func_801D1B28 numeric row base */
  SisyouEntryStatus status; /* +0x0C */
  u16 unk_10;
  u16 unk_12;
  u16 unk_14;               /* first limit, compared against 2 */
  u16 unk_16;               /* second value, compared against 0 */
  u8 unk_18[3];
  u8 unk_1B;                /* badge byte compared against 0xFF by func_801D1444 */
  u16 unk_1C;               /* first value formatted after the first limit */
  u16 unk_1E;               /* second limit, shifted right by 2 */
  u16 unk_20;               /* Pwr value row formatted by func_801D1444 */
  u16 unk_22;               /* Def value row formatted by func_801D1444 */
  u16 unk_24;               /* Agl value row formatted by func_801D1444 */
  u16 unk_26;               /* Int value row formatted by func_801D1444 */
  u8 unk_28[0x7C];          /* record tail, stride is 0xA4 */
} SisyouEntry;

/* @source 0x8014598C @kind unknown */
/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* g_PrimCursor;

/* Main-RAM globals owned by the loaded image. */
/* @source 0x80010000 @kind table */
/* Offset table over the 0x80010000 main-RAM base: func_801D10AC adds the entry
 * selected by its argument to that base to obtain the texture handle passed to
 * func_8014F800. */
extern u16 D_80010000[];
/* @source 0x80144952 @kind unknown */
/* Byte read as the emitted panel strip's fifth argument. */
extern u8 D_80144952;
/* @source 0x80145AA8 @kind unknown */
/* Shared pad-state word; func_801D10AC reads it through the array form so its
 * address stays materialized in a register across the func_8014E640 call. */
extern u16 D_80145AA8[];
/* @source 0x80145AC2 @kind unknown */
/* Shared pressed-button mask matched against D_80145AA8 by func_801D10AC. */
extern u16 D_80145AC2;
/* @source 0x80145AC4 @kind unknown */
/* Shared pressed-button mask matched against D_80145AA8 by func_801D10AC. */
extern u16 D_80145AC4;
/* @source 0x80145FCC @kind table */
/* First byte of each D_80146254-entry, 0x140-byte-stride record drawn by
 * func_801D12FC. */
extern u8 D_80145FCC[];
/* @source 0x80144968 @kind unknown */
extern u8 D_80144968[];
/* @source 0x80144983 @kind table */
/* Owner byte of each 164-byte record at 0x80144968 (record offset +0x1B);
 * compared against masterIndex when scanning the visible records. */
extern u8 D_80144983[];
/* @source 0x80144F59 @kind unknown */
/* Confirmation bits 3..5 of the master states 0xB, 0xD and 0xE set by
 * func_801D3A24 once their record's +6/+0x84 byte difference reaches three. */
extern u8 D_80144F59;
/* @source 0x80145024 @kind unknown */
/* Stream index hint byte; its low seven bits select the alternate icon-kind
 * mapping used by func_801D23A0 when the requested kind is 4. */
extern u8 D_80145024;
/* @source 0x801490D8 @kind unknown */
extern u8 D_801490D8[];
/* @source 0x801490F8 @kind unknown */
/* Twelve-byte name work buffer filled from the gate-selected D_801CA70C ability
 * record by func_801D3A24; byte +0xC terminates it. */
extern u8 D_801490F8[];
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80143E6C @kind unknown */
/* Shared frame counter owned by the main exe; its low bits select the
 * highlight colour group used by the selected-record border. */
extern s32 D_80143E6C;
/* @source 0x801448EC @kind unknown */
extern u8 D_801448EC;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x80146889 @kind unknown */
extern u8 D_80146889[];
/* @source 0x80146254 @kind unknown */
extern u8 D_80146254;
/* @source 0x8014554C @kind unknown */
/* Base address passed with masterIndex to the EXE-side flag helper
 * func_8015B580 by func_801D26C4. */
extern u8 D_8014554C;
/* @source 0x8014554F @kind unknown */
/* Base address passed with masterIndex to the EXE-side flag helper
 * func_8015B580. */
extern u8 D_8014554F;
/* @source 0x801448ED @kind bss */
/* Index of the active master; selects the entry whose action base is read
 * from masterActionBaseTable. */
extern u8 masterIndex;
/* @source 0x801CB8DC @kind table */
/* Shared per-row value table; 99 records per row (0x318-byte row stride,
 * 8-byte record stride). Row summaries are read by func_801D1C44. */
extern SisyouValueRecord D_801CB8DC[][99];
/* @source 0x801CA70C @kind table */
/* Shared ability records; each 0x14-byte record holds the twelve name bytes
 * copied into D_801490F8 by func_801D3A24. */
extern u8 D_801CA70C[];
/* @source 0x80145AD4 @kind unknown */
/* Shared sprintf work buffer holding the formatted text row emitted by
 * func_80150098. */
extern u8 D_80145AD4[];

/* EMI-local data. */
/* @source 0x801D0C04 @kind rodata */
/* sprintf format strings of the numeric rows: "%3d", "%3d/   " and
 * "   /%3d". */
extern const char D_801D0C04[];
extern const char D_801D0C08[];
extern const char D_801D0C10[];
/* @source 0x801D3F58 @kind unknown */
/* Byte stream submitted as the sprite record table of func_801AF390. */
extern const u8 D_801D3F58[];
/* @source 0x801D3FE8 @kind unknown */
/* Sprite record stream submitted to func_801AF390 by func_801D1444 as the
 * panel's filled row. */
extern const u8 D_801D3FE8[];
/* @source 0x801D4014 @kind unknown */
/* Sixteen-cell sprite record stream drawn once per panel cell by
 * func_801D1444 through func_801AF390. */
extern const u8 D_801D4014[];
/* @source 0x801D4020 @kind unknown */
/* Panel background sprite record stream submitted to func_801AF390 by
 * func_801D1444. */
extern const u8 D_801D4020[];
/* @source 0x801D4038 @kind rodata */
/* Row-label strings "Pwr", "Def", "Int" and "Agl" passed to func_8014F800
 * as the row texture handle by func_801D1444. */
extern const u8 D_801D4038[];
extern const u8 D_801D403C[];
extern const u8 D_801D4040[];
extern const u8 D_801D4044[];
/* @source 0x801D4048 @kind rodata */
/* Highlight-marker strings "Pois" and "Conf" emitted through
 * func_8014FC90. */
extern const u8 D_801D4048[];
extern const u8 D_801D4050[];
/* @source 0x801D4058 @kind table */
/* Four-byte icon texture records (u, v and the CLUT VRAM coordinates) indexed
 * by the icon kind selected in func_801D23A0. */
extern u8 D_801D4058[];
/* @source 0x801D4088 @kind table */
/* Per-master gate descriptors of func_801D3A24: twelve bytes per master, six
 * (limit, gate bit) pairs; a limit of 99 with gate bit 0xFF disables a pair. */
extern u8 D_801D4088[];
/* @source 0x801D4089 @kind table */
/* Second byte of each D_801D4088 pair: the gate bit tested in the
 * masterIndex + 0x693 bit table. */
extern u8 D_801D4089[];
/* @source 0x801D4154 @kind unknown */
/* Six per-master bytes copied into the selected record's offsets 0x85..0x8A
 * by func_801D2CA4. */
extern u8 D_801D4154[];
/* @source 0x801D41BC @kind table */
/* Per-master u16 action-id base; adding 4 or 0x11 yields the action id
 * dispatched for the selected master. */
extern u16 masterActionBaseTable[];
/* @source 0x801D4270 @kind table */
extern u8 D_801D4270[];
/* @source 0x801D4284 @kind unknown */
/* Nonzero byte; selects the alternate entry path that starts the selected
 * master's +4 action instead of the record copy path. */
extern u8 D_801D4284;
/* @source 0x801D41E0 @kind table */
/* Per-mode handler table; indexed by modeIndex and tail-called. */
extern void (*D_801D41E0[])(void);
/* @source 0x801D41FC @kind table */
/* Handler table indexed by D_801D4286 and tail-called. */
extern void (*D_801D41FC[])(void);
/* @source 0x801D4204 @kind table */
/* Handler table indexed by D_801D4286 and tail-called. */
extern void (*D_801D4204[])(void);
/* @source 0x801D4240 @kind table */
/* Handler table indexed by D_801D4286 and called (not tail-called). */
extern void (*D_801D4240[])(void);
/* @source 0x801D4264 @kind table */
/* Handler table indexed by D_801D4286 and called (not tail-called). */
extern void (*D_801D4264[])(void);
/* @source 0x801D421C @kind table */
/* Handler table indexed by D_801D4286 and tail-called. */
extern void (*D_801D421C[])(void);
/* @source 0x801D4285 @kind bss */
/* Current mode index; selects the handler from the D_801D41E0 table and is
 * set to 6 after an entry action starts. */
extern u8 modeIndex;
/* @source 0x801D4286 @kind unknown */
/* Index selecting the handler from the D_801D41FC table. */
extern u8 D_801D4286;
/* @source 0x801D4287 @kind unknown */
/* Second byte of the three-byte gap between the handler index and the phase
 * byte, cleared by func_801D0C18. */
extern u8 D_801D4287;
/* @source 0x801D4288 @kind unknown */
/* Third byte of the three-byte gap between the handler index and the phase
 * byte, cleared by func_801D0C18. */
extern u8 D_801D4288;
/* @source 0x801D4289 @kind bss */
/* Phase byte written as 4 by the frontend cue-advance helper. */
extern u8 D_801D4289;
/* @source 0x801D428A @kind unknown */
extern u8 D_801D428A;

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004_22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

/* Main-exe functions called by this overlay. */
/* @behavior draws the shaded panel rectangle of one master-list entry: a
 * rect of the given width and height at the masked 16-bit coordinates, the
 * fifth argument being a value read from a byte global at every callsite and
 * the sixth the entry and icon bytes stored into the caller frame.
 * Body not lifted in this target.
 * @source 0x801AE3F0
 */
void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                   u32 arg5);
/* @behavior queues the inner fill rectangle bounded by the two submitted
 * corner coordinates; func_801D1E54 passes the panel's inner top-left corner
 * and the width and height reduced by the four-pixel strip border as its four
 * 16-bit arguments, and 0 as its fifth argument.
 * Body not lifted in this target.
 * @source 0x801AEBA0
 */
void func_801AEBA0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);
/* @behavior draws the submitted text at the given signed coordinates with the
 * justification argument and the string passed as the last argument; the
 * body is not lifted in this target.
 * @source 0x8014FC90
 */
void func_8014FC90(s16 arg0, s16 arg1, s32 arg2, u32 arg3, const u8* arg4);
/* @behavior emits one 8x8 glyph sprite per non-space byte of the submitted
 * string; lifted as drawGlyphString8x8 in exe/slus_004_22.
 * @source 0x80150098
 */
void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8* arg3);
/* @behavior submits the byte stream of its third argument; lifted as
 * drawSpriteRecordTable (0x801AF390) in emi/etc/game/00.
 * @source 0x801AF390
 */
void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);
/* @behavior EXE-side helper; func_801D10AC passes the shared pad word masked
 * with 0xA000 and tests the low halfword of the result to decide whether to
 * toggle D_801D4284. Body not lifted in this target.
 * @source 0x8014E640
 */
s32 func_8014E640(u32 arg0);
/* @behavior panel coordinate helper called with the computed x (arg0), y
 * (arg1) and a third flag argument that every callsite in the repository
 * passes as 0.
 * @source 0x801647C4
 */
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
void func_80150224(s32 arg0);
/* @behavior returns the texture handle for the given colour index; body not
 * lifted in this target.
 * @source 0x801502D0
 */
u32 func_801502D0(u32 arg0);
/* @behavior EXE-side icon-record helper called with a zero first argument and
 * the fixed second argument 8 by func_801D1444; body not lifted in this
 * target.
 * @source 0x8016620C
 */
void func_8016620C(u32 arg0, u32 arg1, u32 arg2);
/* Byte-flag helper; arg0 carries the address of the byte flag, arg1 the index
 * reported as its second argument (the same helper family as func_8015B580). */
s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(void* arg0, u8 bit_index);
/* @behavior reports whether the ability record identified by the two byte
 * arguments passes its availability gate; the byte return is tested by
 * func_801D3A24.
 * @source 0x801651DC
 */
u8 func_801651DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* @behavior runs the three-argument-id ability gate used by func_801D3A24 as
 * func_801650B4(3, 0x16, 1, 0) and reports its result as a byte.
 * @source 0x801650B4
 */
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);

/* @behavior queues one frontend cue/event id through the EXE-side dispatcher.
 * @source 0x8015DF18
 */
void game_queue_frontend_cue(u32 cue_id);

/* EMI-local functions. */
/* a0 carries the action id selected from masterActionBaseTable; a1 is a 0/1
 * flag (two callsites pass 0 and 1). */
void func_801D0DD4(u16 action, s32 flag);
void func_801D10AC(u32 arg0);
/* Draws one D_80145FCC record at the masked 16-bit coordinates (arg0, arg1);
 * arg2 is the record's first byte and arg3 the record index. */
void func_801D177C(u16 arg0, u16 arg1, u8 arg2, u8 arg3);
/* Draws the D_80145FCC record selected by arg2 at (arg0, arg1). */
void func_801D1444(s32 arg0, u16 arg1, u8 arg2);
/* Emits the shared panel strip at (arg0, arg1) sized arg2 by arg3; arg4 is
 * read from a byte global by every callsite. */
void func_801D1E54(s32 arg0, s32 arg1, s32 arg2, s32 arg3, u8 arg4);
/* Submits the record selected by arg2 at the masked coordinates (arg0, arg1);
 * arg3 is the record value byte and arg4 the record numeric row base, both
 * read as bytes/words by the callee. */
void func_801D1B28(u16 arg0, u16 arg1, u8 arg2, u8 arg3, u32 arg4);
/* Emits the icon primitive at the masked coordinates (arg0, arg1) with the
 * kind arg2, the texture page arg3, the CLUT arg4 and the colour byte arg5. */
void emitPanelIconPrim(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u16 arg4,
                       u8 arg5);
void func_801D25D8(void);
void func_801D2BE8(void);

#endif
