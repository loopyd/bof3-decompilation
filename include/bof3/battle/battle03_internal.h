#ifndef EMI_BATTLE_03_INTERNAL_H
#define EMI_BATTLE_03_INTERNAL_H

#include "bof3/bof3.h"
#include "battle/ability.h"
#include "battle/ram.h"

typedef void (*Battle03Handler)(void);
typedef void (*Battle03EnemyModeHandler)(s32 arg0);
typedef void (*Battle03ForwardingHandler)(s32 arg0, s32 arg1, s32 arg2,
                                          s32 arg3, s32 arg4, s32 arg5,
                                          s32 arg6, u8* selector);
typedef struct Battle03DispatchTable {
  Battle03Handler handlers[3];
} Battle03DispatchTable;
typedef struct Battle03FourDispatchTable {
  Battle03Handler handlers[4];
} Battle03FourDispatchTable;
typedef struct Battle03FiveDispatchTable {
  Battle03Handler handlers[5];
} Battle03FiveDispatchTable;
typedef struct Battle03EightDispatchTable {
  Battle03Handler handlers[8];
} Battle03EightDispatchTable;
typedef struct Battle03NineteenDispatchTable {
  Battle03Handler handlers[19];
} Battle03NineteenDispatchTable;
typedef struct Battle03SeventyDispatchTable {
  Battle03Handler handlers[110];
} Battle03SeventyDispatchTable;
/* Message view of the 12-byte battleDispatchSlots entries selected by state
 * byte +0x0A: a queued message stream pointer at +0x00 (consumed by
 * countMessageTokens) and a panel byte at +0x06 (the style-0x12 draw byte of
 * func_801E9538). */
typedef struct Battle03SlotMessageView {
  u8* unk_00;
  u8  pad_04[2];
  u8  unk_06;
  u8  pad_07[5];
} Battle03SlotMessageView;

typedef struct Battle03LocalWork {
  u8  flags_00;
  u8  unk_01;
  u8  unk_02;
  u8  unk_03;
  u8  unk_04;
  u8  unk_05;
  u8  pad_06[2];
  u8  unk_08;
  u8  pad_09[3];
  u32 unk_0c;
  s32 unk_10;
  u32 unk_14;
  u32 unk_18;
  s32 unk_1c;
  u32 unk_20;
  u8  pad_24[5];
  u8  unk_29;
  u8  pad_2a;
  u8  unk_2b;
  u8  pad_2c[8];
  s32 unk_34;
  s32 unk_38;
  u8  pad_3c[2];
  s16 unk_3e;
  u32 unk_40;
  s32 unk_44;
  u8  unk_48;
  u8  pad_49[0x14];
  /* +0x5D..+0x5F triple stepped by 0x10 in the scratch task tick at
   * 0x801E50C0 */
  u8  unk_5d;
  u8  unk_5e;
  u8  unk_5f;
  u8  pad_60[0x19];
  u8  unk_79;
  u8  pad_7a[6];
  u16 unk_80;
  u8  pad_82[6];
  u16 unk_88;
  u16 unk_8a;
  u8  unk_8c;
  u8  pad_8d[3];
  u16 unk_90;
  u16 unk_92;
  u8  pad_94[0xa];
  u8  unk_9e;
  u8  pad_9f[5];
  u8  unk_a4;
  u8  pad_a5[0x73];
  u8  unk_118;
  u8  unk_119;
  u8  pad_11a[2];
  /* unk_11c/unk_11e/unk_120 proven by the func_801D5BC0 reset path */
  u16 unk_11c;
  u16 unk_11e;
  u8  unk_120;
  u8  unk_121;
  u8  pad_122;
  u8  unk_123;
  u32 unk_124;
  u32 unk_128;
  u8  pad_12c[8];
  u16 unk_134;
  u8  unk_136;
  u8  unk_137;
  u8  unk_138;
  u8  unk_139;
  u8  pad_13a[6];
} Battle03LocalWork;

typedef struct Battle03FlagRecord {
  u32 flags_00;
  u8  pad_04[0x13c];
} Battle03FlagRecord;

typedef struct Battle03SelectionKind {
  u16 mask_00;
  u8  pad_02[0x12];
} Battle03SelectionKind;

typedef struct Battle03LocalHalfRecord {
  u16 half_00;
  u8  pad_02[0x13e];
} Battle03LocalHalfRecord;

typedef struct Battle03EnemyHalfRecord {
  u16 half_00;
  u8  pad_02[0x116];
} Battle03EnemyHalfRecord;

typedef struct Battle03EnemyWordRecord {
  u32 word_00;
  u8  pad_04[0x114];
} Battle03EnemyWordRecord;

typedef struct Battle03TemplateRecord {
  u32 words_00[41];
} Battle03TemplateRecord;

typedef struct Battle03EnemyWork {
  u8                       unk_00;
  u8                       unk_01;
  /* unk_02/unk_03/unk_04 proven by the func_801D5BC0 reset path */
  u8                       unk_02;
  u8                       unk_03;
  u8                       unk_04;
  u8                       pad_05[0x7d];
  /* unk_82/unk_fd proven by the func_801D57AC countdown path */
  u16                      unk_82;
  u8                       pad_84[4];
  /* unk_88 proven by the func_801DD978 slot-selection scan */
  u16                      unk_88;
  u8                       pad_8a[0xa];
  /* unk_94 proven by the initDeferredBattlerCountdown path */
  u16                      unk_94;
  u8                       pad_96[0xa];
  /* unk_a0 proven by the func_801E97A8 ratio path */
  u16                      unk_a0;
  u8                       pad_a2[0x12];
  u8                       unk_b4;
  u8                       pad_b5[0x2f];
  Battle03EnemyModeHandler unk_e4;
  u8                       pad_e8[8];
  u8                       unk_f0;
  u8                       pad_f1[4];
  u8                       unk_f5;
  u8                       pad_f6[2];
  /* unk_f8/unk_fa/unk_fc proven by the func_801D5BC0 reset path */
  u16                      unk_f8;
  u16                      unk_fa;
  u8                       unk_fc;
  u8                       unk_fd;
  u8                       pad_fe[2];
  u32                      unk_100;
  u32                      unk_104;
  u8                       pad_108[0xa];
  u8                       unk_112;
  u8                       pad_113;
  u8                       unk_114;
  u8                       unk_115;
  u8                       pad_116[2];
} Battle03EnemyWork;

typedef struct Battle03QueuedSlot {
  u8  unk_00;
  u8  unk_01;
  u8  pad_02[3];
  u8  unk_05;
  u8  unk_06;
  u8  pad_07[2];
  u8  unk_09;
  u8  pad_0a[2];
  s32 unk_0c;
  s32 unk_10;
  u8  pad_14[4];
  s32 unk_18;
  s32 unk_1c;
  u8  pad_20[0x14];
  s32 unk_34;
  s32 unk_38;
  s16 unk_3a;
  u8  pad_3c[0x38];
  u32 unk_74;
} Battle03QueuedSlot;

/* The 0x74-byte record image copied from a D_80145E90 work record into a
 * D_801EC330 slot-store entry by copySelectedWorkToQueuedSlot; the original
 * moves it as one block. */
typedef struct Battle03SlotBody {
  u32 words_00[29];
} Battle03SlotBody;

typedef struct Battle03SlotStore {
  /* unk_01..unk_06 and unk_0b are initialized by copySelectedWorkToQueuedSlot
   * after the 0x74-byte body copy. */
  u8  unk_00;
  u8  unk_01;
  u8  unk_02;
  u8  unk_03;
  u8  unk_04;
  u8  unk_05;
  u8  unk_06;
  u8  pad_07[2];
  u8  flag_09;
  u8  pad_0a;
  u8  unk_0b;
  u8  pad_0c[0x22];
  /* unk_2e/unk_30 proven by the func_801E1BA8 position stores. */
  u16 unk_2e;
  u16 unk_30;
  u8  pad_32[2];
  u32 unk_34;
  u32 unk_38;
  u32 unk_3c;
  u8  pad_40[0x34];
  u32 ptr_74;
} Battle03SlotStore;

typedef struct Battle03SpritePrimitive {
  u8  pad_00[4];
  u8  r0;
  u8  g0;
  u8  b0;
  u8  pad_07;
  s16 x0;
  s16 y0;
  s16 unk_0c;
  s16 unk_0e;
} Battle03SpritePrimitive;

typedef struct Battle03UiRingEntry {
  u8  unk_00;
  u8  unk_01;
  u8  pad_02[2];
  u32 unk_04;
} Battle03UiRingEntry;

extern volatile AbilityObject       ABILITY_OBJECTS[]; /* @source 0x801CA70C @kind unknown */
extern volatile u8                  D_801462F0; /* @source 0x801462F0 @kind unknown */
extern Battle03TemplateRecord       D_80144968[]; /* @source 0x80144968 @kind unknown */
extern volatile Battle03FlagRecord D_80145FB8[]; /* @source 0x80145FB8 @kind unknown */
extern volatile Battle03LocalHalfRecord D_80145F28[]; /* @source 0x80145F28 @kind unknown */
extern volatile Battle03EnemyHalfRecord D_801EB6D8[]; /* @source 0x801EB6D8 @kind unknown */
extern volatile Battle03EnemyWordRecord D_801EB734[]; /* @source 0x801EB734 @kind unknown */
extern s32                          D_801EAFA0[]; /* @source 0x801EAFA0 @kind unknown */
extern s16                          D_801EAFC0[]; /* @source 0x801EAFC0 @kind unknown */
extern volatile u16                 D_1F800000; /* @source 0x1F800000 @kind unknown */
extern u16                          D_80143F04; /* @source 0x80143F04 @kind unknown */
/* @source 0x80143F00 @kind unknown — shared u16 added to 0x2AB to select the
 * EMI stream slot started by startEmiStreamSlotWhenLoaderReady. */
extern u16                          D_80143F00;
extern s32                          D_80143F24; /* @source 0x80143F24 @kind unknown */
extern s32                          D_80143F28; /* @source 0x80143F28 @kind unknown */
extern u8                           D_80146328; /* @source 0x80146328 @kind unknown */
extern u8                           D_80146329; /* @source 0x80146329 @kind unknown */
/* @source 0x80146854 @kind unknown — shared EMI loader state byte whose 0x20
 * bit startEmiStreamSlotWhenLoaderReady raises before starting a stream slot;
 * the same bit gates the staged transfer slots in the EXE-side
 * stageEmiTransferSlot. */
extern u8                           D_80146854;
/* Shared front-selector primary state byte (0x80146874) and secondary
 * sub-state byte (0x80146875) of the main-RAM front-end state machine; written
 * by this target's battle hook requestFrontSelectorState1Substate19.
 * @source 0x80146874 @kind unknown
 */
extern u8                           D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8                           D_80146875;
extern u8                           D_80144955; /* @source 0x80144955 @kind unknown */
extern const u8                     D_801D0C98[]; /* @source 0x801D0C98 @kind unknown */
extern volatile u32                 D_801463D0; /* @source 0x801463D0 @kind unknown */
extern u8                           D_801462F3; /* @source 0x801462F3 @kind unknown */
extern u8                           D_80146384; /* @source 0x80146384 @kind unknown */
extern u8                           D_801462F4; /* @source 0x801462F4 @kind unknown */
/* Seven per-actor countdown bytes 0x801462FC..0x80146302; the group-queue cursor
 * at 0x80146303 abuts the array's end. */
extern u8                           D_801462FC[]; /* @source 0x801462FC @kind unknown */
extern Battle03Handler              D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Battle03Handler              D_801463A8; /* @source 0x801463A8 @kind unknown */
extern u8                           groupQueueTable[]; /* @source 0x801462F6 @kind unknown */
extern u8                           groupQueueCursor; /* @source 0x80146303 @kind unknown */
extern Battle03LocalWork           *D_1F800044; /* @source 0x1F800044 @kind unknown */
extern Battle03Handler              D_801EB210[]; /* @source 0x801EB210 @kind unknown */
extern Battle03Handler              D_801EB27C[]; /* @source 0x801EB27C @kind unknown */
/* @source 0x801EB294 @kind unknown */
extern const volatile Battle03Handler D_801EB294[];
/* @source 0x801EB298 @kind unknown */
extern const volatile Battle03Handler D_801EB298[];
extern Battle03Handler              D_801EB258[]; /* @source 0x801EB258 @kind unknown */
extern u16                          D_801EB09C[]; /* @source 0x801EB09C @kind unknown */
extern Battle03Handler              D_801EB46C[]; /* @source 0x801EB46C @kind unknown */
extern u8                           D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */
extern u8                           D_801EB288[]; /* @source 0x801EB288 @kind unknown */
extern Battle03LocalWork            D_80145E90[]; /* @source 0x80145E90 @kind unknown */
/* Fields of the D_80145E90 local work record selected by panel byte +0x0A in
 * func_801E9FDC: byte +0x08, halfword +0x2E, halfword +0x30 and byte +0x79. The
 * original reaches each with its own 32-bit displacement (`lui $at,%hi(X)` +
 * `addu at,at,offset` + load `%lo(X)(at)`), so they are bound as flat symbols
 * the same way the D_801EB638 family binds the D_801EB630 record bytes. */
extern u8                           D_80145E98[]; /* @source 0x80145E98 @kind unknown */
extern u8                           D_80145EBE[]; /* @source 0x80145EBE @kind unknown */
extern u8                           D_80145EC0[]; /* @source 0x80145EC0 @kind unknown */
extern u8                           D_80145F09[]; /* @source 0x80145F09 @kind unknown */
extern volatile u8 D_80144974[];
extern volatile u8 D_8014497C[];
extern volatile u8 D_8014497E[];
extern volatile u8 D_80144980[];
extern Battle03DispatchTable D_801D0FE0; /* @source 0x801D0FE0 @kind table */
extern volatile u8 D_80145F10[];
extern volatile u8 D_80145F18[];
extern volatile u8 D_80145F1A[];
extern volatile u8 D_80145F1C[];
extern volatile u8 D_80145F20[]; /* @source 0x80145F20 @kind unknown */
extern u8 D_80145FCC[];
extern volatile u16           D_80143C40; /* @source 0x80143C40 @kind unknown */
extern volatile u16           D_80145AA8; /* @source 0x80145AA8 @kind unknown */
extern u8                     D_801462E0; /* @source 0x801462E0 @kind unknown */
extern volatile u8            D_8014832E; /* @source 0x8014832E @kind unknown */
extern Battle03LocalWork* volatile D_80146250; /* @source 0x80146250 @kind unknown */
extern u8                           D_80146254; /* @source 0x80146254 @kind unknown */
extern const volatile u16 D_801EAFD0[];
extern Battle03LocalWork*          D_1F800044; /* @source 0x1F800044 @kind unknown */
extern volatile u8* battleWork; /* @source 0x1F800044 @kind unknown */
extern u8*          g_PrimCursor; /* @source 0x8014598C @kind unknown */
extern u8           D_80145AD4[]; /* @source 0x80145AD4 @kind unknown */
extern const char   D_801D0C70[]; /* @source 0x801D0C70 @kind unknown */
extern const char   D_801D0C74[]; /* @source 0x801D0C74 @kind unknown */
extern Battle03FiveDispatchTable D_801D0C84; /* @source 0x801D0C84 @kind table */
extern u8           D_801462E1[]; /* @source 0x801462E1 @kind unknown */
extern const u8     D_801EAF88[][6]; /* @source 0x801EAF88 */
extern Battle03NineteenDispatchTable D_801D0CD0; /* @source 0x801D0CD0 @kind unknown */
extern Battle03QueuedSlot* volatile D_801EC2E0; /* @source 0x801EC2E0 @kind unknown */
extern Battle03FourDispatchTable    D_801D0CC0; /* @source 0x801D0CC0 @kind table */
extern Battle03SlotStore            D_801EC330[]; /* @source 0x801EC330 @kind unknown */
/* 0x140-stride battle status records paired with the local work records; bit 0
 * of word 0 is cleared for the first three records by queueEligibleWorkRecord
 * and tested by func_801E7B34. The raw-address macro BATTLE_LOCAL_STATUS_ARRAY
 * in this header still binds the same address for that older lift. */
extern Battle03FlagRecord            battleLocalStatusArray[]; /* @source 0x801EC048 @kind unknown */
/* @kind: bss (map symbol: uiRingHead) — UI ring consumer index. */
extern u8                            uiRingHead; /* @source 0x801EBF04 */
extern u8                            D_801EC2E4; /* @source 0x801EC2E4 */
/* @kind: bss (map symbol: uiRingEntries) — 16-entry UI ring of (byte, byte, word) triples. */
extern volatile Battle03UiRingEntry uiRingEntries[]; /* @source 0x801EB5B0 */
/* @kind: bss (map symbol: uiRingTail) — UI ring producer index. */
extern volatile u8                   uiRingTail; /* @source 0x801EC328 */
extern u8*                           D_80148648; /* @source 0x80148648 */
extern u8                            D_801462E5; /* @source 0x801462E5 */
extern u8                            D_801462EE; /* @source 0x801462EE @kind unknown */
extern volatile u8                   D_801462EF; /* @source 0x801462EF @kind unknown */
extern u8                            D_8014630C; /* @source 0x8014630C @kind unknown */
extern u8                            D_8014864C; /* @source 0x8014864C @kind unknown */
extern u8                            D_80181B10[]; /* @source 0x80181B10 @kind unknown */
/* The pair is bound as two symbols because the original func_801E1BA8 reads
 * both bytes at one 2-byte stride and reaches them as separate symbol
 * relocations; a single array with a `+ 1` addend keeps one shared base-address
 * pseudo that forces an 8-byte stack frame (asm-diff vars= 8 versus vars= 0). */
extern u8                            D_80181D98[]; /* @source 0x80181D98 @kind unknown */
extern u8                            D_80181D99[]; /* @source 0x80181D99 @kind unknown */
extern Battle03FlagRecord            D_80145FB4[]; /* @source 0x80145FB4 @kind unknown */
extern u8*                           D_801EB4D8; /* @source 0x801EB4D8 @kind unknown */
extern volatile u8*                  D_801EB4DC; /* @source 0x801EB4DC @kind unknown */
extern volatile Battle03LocalWork*   D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern Battle03EightDispatchTable    D_801D0ED4; /* @source 0x801D0ED4 @kind unknown */
extern Battle03FiveDispatchTable     D_801D0EF4; /* @source 0x801D0EF4 @kind table */
extern u8                           D_801462EC; /* @source 0x801462EC @kind unknown */
extern Battle03SeventyDispatchTable D_801D0D1C; /* @source 0x801D0D1C @kind unknown */

void integrateMotionOrSet2(void);
extern Battle03EightDispatchTable   D_801D0F80; /* @source 0x801D0F80 @kind unknown */
extern Battle03FiveDispatchTable   D_801D0F44; /* @source 0x801D0F44 @kind unknown */
extern Battle03FiveDispatchTable    D_801D0F58; /* @source 0x801D0F58 @kind unknown */
extern Battle03DispatchTable        D_801D0F20; /* @source 0x801D0F20 @kind unknown */

Battle03LocalWork *findLocalWorkByKind(u8 arg0);

s16 scalePercentClamp999(s32 arg0, s32 arg1);
extern Battle03DispatchTable        D_801D0F2C; /* @source 0x801D0F2C @kind unknown */
extern Battle03DispatchTable        D_801D0F38; /* @source 0x801D0F38 @kind table */
extern Battle03DispatchTable        D_801D0FA0; /* @source 0x801D0FA0 @kind unknown */
extern Battle03DispatchTable        D_801D0FAC; /* @source 0x801D0FAC @kind unknown */
extern Battle03DispatchTable        D_801D0FB8; /* @source 0x801D0FB8 @kind unknown */
extern Battle03DispatchTable        D_801D0FC4; /* @source 0x801D0FC4 @kind unknown */
extern Battle03FourDispatchTable    D_801D0FD0; /* @source 0x801D0FD0 @kind table */
extern Battle03DispatchTable        D_801D0FF8; /* @source 0x801D0FF8 @kind table */
extern u8*            D_801C893C[]; /* @source 0x801C893C @kind unknown */
extern u32*           D_800B6178[]; /* @source 0x800B6178 @kind unknown */
extern u8*            D_801C8950[]; /* @source 0x801C8950 @kind unknown */
extern Battle03Handler D_801EB154[]; /* @source 0x801EB154 @kind unknown */
extern Battle03Handler D_801EB15C[]; /* @source 0x801EB15C @kind unknown */
extern Battle03Handler D_801EB218[]; /* @source 0x801EB218 @kind unknown */
extern Battle03Handler D_801EB3B0[]; /* @source 0x801EB3B0 @kind unknown */
extern Battle03EnemyWork          D_801EB630[]; /* @source 0x801EB630 @kind unknown */
/* Unknown bytes of the 0x118-byte D_801EB630 record read by func_801E99AC. The
 * original encodes each of them as its own 32-bit displacement (`lui $at,%hi(X)`
 * + `addu at,at,index*0x118` + load `%lo(X)(at)`), so they are bound as flat
 * symbols the same way battle15 binds the D_801EB712/D_801EB713 pair. */
extern u8  D_801EB638[]; /* @source 0x801EB638 @kind unknown */
extern u8  D_801EB65E[]; /* @source 0x801EB65E @kind unknown */
extern u8  D_801EB660[]; /* @source 0x801EB660 @kind unknown */
extern u8  D_801EB712[]; /* @source 0x801EB712 @kind unknown */
extern u8  D_801EB713[]; /* @source 0x801EB713 @kind unknown */
extern Battle03Handler D_801EB188; /* @source 0x801EB188 @kind unknown */
/* Kind-keyed byte table read by func_801DF914 with the current local work
 * record's kind byte +0x79 as its index. The original reaches it
 * symbol-relative (`lui $at,%hi(X)` + `addu $at,$at,index` + `lbu
 * %lo(X)($at)`), so it is bound as a symbol beside the D_801EB188 handler
 * rather than as a fixed-address macro. */
extern const u8 D_801EB18C[]; /* @source 0x801EB18C @kind unknown */
/* Actor-keyed byte table read by func_801DF914 with the global byte
 * BATTLE_GLOBAL_BYTE_63C9 as its index; it sits 0x0C bytes after D_801EB18C
 * in the same byte run and is reached the same symbol-relative way. */
extern const u8 D_801EB198[]; /* @source 0x801EB198 @kind unknown */
extern Battle03Handler D_801EACD4[]; /* @source 0x801EACD4 @kind unknown */
extern Battle03Handler D_801EACE8[]; /* @source 0x801EACE8 @kind unknown */
extern Battle03Handler D_801EACF4[]; /* @source 0x801EACF4 @kind unknown */
extern Battle03Handler D_801EAD00[]; /* @source 0x801EAD00 @kind unknown */
extern Battle03Handler D_801EAD0C[]; /* @source 0x801EAD0C @kind unknown */
extern Battle03Handler D_801EAD20[]; /* @source 0x801EAD20 @kind unknown */
extern Battle03Handler D_801EB1B4[]; /* @source 0x801EB1B4 @kind unknown */
extern Battle03Handler D_801EB1BC[]; /* @source 0x801EB1BC @kind unknown */
extern Battle03Handler D_801EB1D4[]; /* @source 0x801EB1D4 @kind unknown */
extern Battle03Handler D_801EB1F4[]; /* @source 0x801EB1F4 @kind unknown */
extern Battle03Handler D_801EB3E4[]; /* @source 0x801EB3E4 @kind unknown */
extern Battle03Handler D_801EB3EC[]; /* @source 0x801EB3EC @kind unknown */
extern Battle03Handler D_801EB3F4[]; /* @source 0x801EB3F4 @kind unknown */
/* Handler table for the scratch work byte +0x02 dispatch in func_801E3638; the
 * original reaches it symbol-relative (`lui $at,%hi(D_801EB3FC)` +
 * `lw %lo(D_801EB3FC)($at)`), so it is bound as its own table beside the byte
 * +0x03 table at 0x801EB3F4. */
extern Battle03Handler D_801EB3FC[]; /* @source 0x801EB3FC @kind unknown */
extern Battle03Handler D_801EB404[]; /* @source 0x801EB404 @kind unknown */
extern Battle03Handler D_801EB40C[]; /* @source 0x801EB40C @kind unknown */
extern Battle03Handler D_801EB424[]; /* @source 0x801EB424 @kind unknown */
extern Battle03Handler D_801EB430[]; /* @source 0x801EB430 @kind unknown */
extern Battle03Handler D_801EB444[]; /* @source 0x801EB444 @kind unknown */
extern Battle03Handler D_801EB460[]; /* @source 0x801EB460 @kind unknown */
extern Battle03Handler D_801EB478[]; /* @source 0x801EB478 @kind unknown */
extern void             D_801492B8; /* @source 0x801492B8 @kind unknown */
extern u8               D_801EB000[]; /* @source 0x801EB000 @kind unknown */
/* @source 0x801EB05B @kind unknown — battle message text at the 0xD-stride row 7
 * of the D_801EB000 table; consumed as the queued-event message argument. */
extern u8               D_801EB05B[];
/* @source 0x801EB068 @kind unknown — the 0xD-stride row 8 sibling of D_801EB05B. */
extern u8               D_801EB068[];
/* Panel correction bytes selected by the D_80145E90 local work record bytes
 * +0x08/+0x79 read by func_801E9FDC; the original reaches each with its own
 * displacement, so they are bound as two flat symbols the same way the
 * D_801EB638 family binds the D_801EB630 record bytes. */
extern u8               D_801EB0B0[]; /* @source 0x801EB0B0 @kind unknown */
extern u8               D_801EB0B1[]; /* @source 0x801EB0B1 @kind unknown */
extern volatile u8      battleDispatchSlots[]; /* @source 0x801EB4F0 @kind unknown */
/* @source 0x801EB4F4 @kind table — the message view of the 12-byte
 * battleDispatchSlots entries read by func_801E9538: the original reaches the
 * message pointer at the base and the panel byte at +0x06 of that base, so the
 * table is bound at its own address the same way the D_801EB638 family binds
 * the D_801EB630 record bytes. */
extern Battle03SlotMessageView D_801EB4F4[];
extern s8               D_801EB4F2; /* @source 0x801EB4F2 @kind unknown */
extern volatile u8  BATTLE_GLOBAL_BYTE_62E2; /* @source 0x801462E2 @kind unknown */
extern volatile u16 BATTLE_GLOBAL_HALF_62E8; /* @source 0x801462E8 @kind unknown */
extern volatile u8  BATTLE_GLOBAL_BYTE_62EA; /* @source 0x801462EA @kind unknown */
extern u8  D_801EC303; /* @source 0x801EC303 @kind unknown */
extern volatile u8  BATTLE_GLOBAL_BYTE_62F3;
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

void func_801E679C(void);
void clearQueuedSlotWhenReady(void);
extern volatile u8  BATTLE_GLOBAL_BYTE_6374; /* @source 0x80146374 @kind unknown */
extern volatile u8  D_801483C3; /* @source 0x801483C3 @kind unknown */
extern u8           D_80146374; /* @source 0x80146374 @kind unknown */
extern u8           D_80146375; /* @source 0x80146375 @kind unknown */
extern u16          D_801463C0; /* @source 0x801463C0 @kind unknown */
extern volatile u8  BATTLE_GLOBAL_BYTE_6375; /* @source 0x80146375 @kind unknown */
extern volatile u8  BATTLE_GLOBAL_BYTE_63BA; /* @source 0x801463BA @kind unknown */
extern volatile u16 BATTLE_GLOBAL_HALF_63C0; /* @source 0x801463C0 @kind unknown */
extern u8           BATTLE_GLOBAL_BYTE_63C8; /* @source 0x801463C8 @kind unknown */
extern u8           BATTLE_GLOBAL_BYTE_63C9; /* @source 0x801463C9 @kind unknown */
extern Battle03SelectionKind D_801CA71C[]; /* @source 0x801CA71C @kind unknown */
/* @source 0x801CA718 @kind unknown — 0x14-stride kind-keyed byte four bytes below
 * the D_801CA71C mask table; func_801E020C reaches it as its own displacement
 * (`lbu %lo(D_801CA718)($at)`), so it is bound as a flat symbol. */
extern u8           D_801CA718[];
extern volatile u8  BATTLE_GLOBAL_BYTE_6322; /* @source 0x80146322 @kind unknown */
extern volatile u8  BATTLE_GLOBAL_BYTE_6323; /* @source 0x80146323 @kind unknown */
/* @source 0x80146324 @kind unknown — actor-phase byte read by
 * isBattlerEligibleForActorPhase. */
extern u8           BATTLE_GLOBAL_BYTE_6324;
extern volatile u8  BATTLE_GLOBAL_BYTE_63CE; /* @source 0x801463CE @kind unknown */
extern volatile u8  BATTLE_RANDOM_TABLE_AC58_DATA[]; /* @source 0x801EAC58 @kind unknown */
extern volatile u8  BATTLE_RANDOM_TABLE_AC78_DATA[]; /* @source 0x801EAC78 @kind unknown */
extern u8           func_8017E3D4(void);

void func_8014D290(void);
void func_8014D4E0(void);
void dispatchByte1PairTable(void);
void dispatchModeFiveTable(void);
void func_801E72F4(void);
void func_8014D5F0(u8 arg0, u32 arg1, s32 arg2);
u16  func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A904(u32 arg0, s32 arg1);
void func_8017A9A4(u32 arg0);
void func_8017A9B8(u32 arg0);
u16  func_8017A6F0(s32 arg0, s32 arg1);
void func_8017AA6C(u32 arg0);
void func_8017AA80(u32 arg0);
void func_8017AA1C(void);
void func_8017E3F4(void* arg0, const void* arg1, ...);
void func_8017E364(void* arg0, const void* arg1);
void func_8017C2D8(u32 arg0, s32 arg1, s32 arg2, u16 arg3, s32 arg4);
void func_80158DB8(u8 arg0, u8 arg1);
void func_80158E20(void);
void func_80158E50(void);
u8   func_8014D978(void);
u8   func_8014DAEC(void);
void func_8014E5A0(u8 arg0, u8 arg1);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
u32  func_8014D8D4(u8 arg0);
u8   func_801DB524(u8 arg0);
s32  testFlag400WhenEligible(u8 arg0);
u8   func_800A9304(u8 arg0);
u8   func_800A94A8(void);
u8   func_800A955C(void);
void func_800A36F0(u32 arg0, u32 arg1);
u16  func_800A2AE0(u8 arg0);
u8   func_800A3DF8(u32 arg0, u32 arg1);
void func_800A31E0(u32 arg0, u32 arg1);
void func_800AD074(u8 arg0);
void func_800AAA74(void);
void func_800A4458(void);
void func_800A9BD8(u8 arg0);
/* Owned by the SLUS address space (boundary func_800A9820 in emi/battle/battle/15);
 * the first parameter is the zero-extended battle byte D_80146384. */
void func_800A9820(u8 arg0);
/* Shared front-end startup helper run by this target's battle hook
 * requestFrontSelectorState1Substate19 (and by the sibling scenario, ui and
 * world targets); the sibling targets declare the same prototype. */
void func_8015C088(void);
/* Frontend cue dispatcher reached from startScratchWorkTransform with the
 * constant 0x205; same family/shape as func_8015DF18. */
void func_8015D71C(u16 arg0);
void func_8015DF18(u16 arg0);
void func_801636A0(u32 arg0, u32 arg1);
void func_80164A44(u32 arg0);
s32  func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_80196718(void* arg0);
void func_801D7EB0(s32 arg0, s32 arg1);
u8   isBattlerBlockedOrUnavailable(u32 arg0);
u8   shouldTriggerBattlerCountdownRetry(u32 arg0);
void func_801644D8(u32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, u32 arg5);
extern Battle03Handler D_801EB120[]; /* @source 0x801EB120 @kind unknown */

void dispatchLocalStateTable(void);
void markPendingBit(u32 arg0);
void func_801DE560(u8 arg0, u8 arg1, u8 arg2, u8 arg3, u32 arg4);
void func_801DE9A8(u32 arg0);
extern u8 D_801490D8[]; /* @source 0x801490D8 @kind unknown */
extern u8 D_801EB35C[]; /* @source 0x801EB35C @kind unknown */

void submitEnemyScriptBlock(u32 arg0);
void initDeferredBattlerCountdown(u32 arg0);
u32  func_801502D0(u32 arg0);
void func_801501E4(void* arg0, u32 arg1, u32 arg2);
void func_80150098(s16 arg0, s16 arg1, u32 arg2, void* arg3);
u32  func_801E590C(u32 arg0, u32 arg1);
void copySelectedWorkToQueuedSlot(void);
u8   func_801E2E30(void);
s16  func_8015477C(s32 arg0, s32 arg1);

void setupAllLocalWork(void);
void runActiveLocalHandlers(void);
u8   localReadyOrHelper1(void);
u8   localReadyOrHelper2(void);
void setState8WhenReady(void);
void storeLocalReady2Result(void);
void func_801DEF0C(void);
void func_801DEFE4(void);
void forwardSelectedEffectId(void);
void resetReadyAdvanceByte1(void);
void callNextLocalHandler(void);
void func_801DF914(void);
void copyScratchToLocalWork(void);
void waitReadyAdvanceSubstate3(void);
extern Battle03Handler D_801EB1E0[]; /* @source 0x801EB1E0 @kind unknown */

void dispatchLocalSubstate3Table(void);
void dispatchLocalState4Table(void);
void dispatchAltState3Table(void);
extern Battle03Handler D_801EB224[]; /* @source 0x801EB224 @kind unknown */

void dispatchState2ClassTable(void);
extern Battle03Handler D_801EB26C[]; /* @source 0x801EB26C @kind unknown */
extern Battle03Handler D_801EB274[]; /* @source 0x801EB274 @kind unknown */

void dispatchState2EventTable(void);
void dispatchState2FollowupTable(void);
void dispatchDefaultClassTable(void);
void func_801E2170(void);
void func_801E25E0(u8 arg0);
void pickTargetStoreGlobal(s8 arg0);
void func_801D9304(u8 arg0);
u8   resolveKindResultMode(void);
void func_801D4850(void);
void initUiBundleSlot2(u8 arg0);
void initUiBundleSlot3(u8 arg0);
void initUiBundleSlot4(u8 arg0);
void initUiBundleSlot0(void);
u8   func_801D54F8(void);
u8   advanceLocalFlag40Countdown(void);
u8   func_801D57AC(void);
u8   advanceLocalFlag20Countdown(void);
u8   func_801D5A60(void);
u8   func_801D5BC0(void);
u8   func_801D5DCC(void);
void func_801D4D44(void);
void func_801D750C(s32 arg0, s32 arg1);
void func_801D7A40(s16 arg0, s16 arg1);
void func_801D7D10(u8 arg0, s16 arg1, s16 arg2, u16 arg3, u8 arg4, u8 arg5);
void func_801D8270(s32 arg0, s32 arg1);
void func_801D8450(u32 arg0);
void func_801D8690(s32 arg0, s32 arg1, s32 arg2);
void func_801D8AE4(s32 arg0, s32 arg1, s32 arg2);
void func_801D8DF8(s32 arg0, s32 arg1, u32 arg2);
u8   markBattlersMeetingOpposingThresholds(void);
void drawDecimalGlyphRun(s16 arg0, u16 arg1, u8 arg2, s16 arg3);
void func_801D9684(s16 arg0, u16 arg1, s32 arg2, u16 arg3);
extern s8  D_80144952; /* @source 0x80144952 @kind unknown */
extern u16 D_80033A08[]; /* @source 0x80033A08 @kind table */
extern u16 D_801EAD30[]; /* @source 0x801EAD30 @kind table */
extern u16 D_801EAE50[][2]; /* @source 0x801EAE50 @kind table */
/* Panel correction bytes selected by the D_801EB630 record byte +0x08 read by
 * func_801E99AC; bound as two symbols because the original reaches each with its
 * own displacement. */
extern s8  D_801EAEA4; /* @source 0x801EAEA4 @kind unknown */
extern s8  D_801EAEA6; /* @source 0x801EAEA6 @kind unknown */
extern u8  D_801EAF27[]; /* @source 0x801EAF27 @kind unknown */
void func_801D9AB4(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801D9C80(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void func_801D9DBC(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4);
void func_801D9E9C(s16 arg0, s16 arg1, u16 arg2, u16 arg3, s8 arg4);
void func_801DA078(s16 arg0, s16 arg1, s32 arg2);
void drawFlatLinePrim(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
u8   func_801DA69C(u32 arg0);

u32  findThresholdRank(u8 arg0, u32 arg1);
u8   func_801DB844(u32 arg0);
u8   func_801DB9E4(u32 arg0);
u8   func_801DB2F8(u32 arg0);
u8   func_801DB3A0(u32 arg0, u32 arg1, u32 arg2);
u8   enemyBattlerMeetsThresholds(u32 arg0, s32 arg1, u32 arg2);
void clearRankingScratch(void);
void func_801DA7D4(void);
void func_801DAAE4(void);
u32  func_801DC73C(s16 arg0, u32 arg1, u32 arg2);
u32  func_801DCCB0(void);
u32  func_801DCD50(u32 arg0, u8 arg1, s32 arg2);
u32  func_801DC044(u8 arg0, u8 arg1, u16 arg2);
u32  func_801DCAD8(u8 arg0, u8 arg1, s8 arg2);
u32  func_801DC894(s16 arg0, u8 arg1, u32 arg2);
u32  func_801DBB78(u8 arg0, u8 arg1);
void copyLocalTemplates(void);
void clearBattlerActionFlags(u8 arg0);
void func_801DD29C(void);
void transformFirstPointPairByMode(s32 arg0);
void func_801DD3CC(s32 arg0);
u8   checkLocalQueuedBranch(void);
void submitCurrentWorkEffectVariant(u32 arg0);
void copyCurrentBattlerVisibleStateToTemplate(u32 arg0);
void submitPositionalEffectBit80(u32 arg0);
void copyCurrentBattlerToQueuedSlot(void);
void func_801DD800(void);
u8   advanceCounterStorePacked(u32 arg0, u32 arg1);
void initModeTuple530(void);
u8   scanTriggerTableSubmit(u16 arg0, u32 arg1);
void func_801DDFEC(u32 arg0);
void clearPendingBit(u32 arg0);
void func_801DE1D4(void);
void func_801DE60C(u32 arg0, u8 arg1, u8 arg2, u8 arg3, u8 arg4, u32 arg5);
void stubHandlerE690(void);
void stubHandlerE7fc(void);
void deactivateBattleDispatchSlots(void);
u8   func_801DE858(s8 arg0);
void pushUiRingTriple(s8 arg0, s8 arg1, u32 arg2);
u8   hasUiRingWork(void);
void func_801DEA64(s32 arg0);
void queueActiveBattlerSnapshots(void);
u8   func_801E0E0C(void);
void func_801E0B64(void);
void resetScratchWhenGlobalBit4(void);
void func_801E1DD4(void);
u8   enemyReadyOrHelper2(void);
u8   checkEnemyQueuedPredicate(void);
void func_801E2314(u32 arg0);
void resetEnemyScratchWhenBit4(void);
void func_801E531C(void);
void submitEffectUnlessNeg1(s16 arg0);
void applyGravityOrReset(void);
void func_801E54EC(void);
void resetEnemyScratchState(void);
void raiseBit80AllocSlot(void);
void clearQueuedSlotBytes(void);
void requestFrontSelectorState1Substate19(void);
void enterFrontendMode4(void);
void func_801E5A38(void);
void func_801E62BC(u8 arg0);
u8   func_801E7B34(void);
void func_801E8DD8(void);
u8   func_801E8FA8(void);
void func_801E97A8(void);
void func_801E9910(void);
void func_801DEAE0(void);
void queueScriptEvent(u32 arg0, u32 arg1);
void drawIconStrip24x8(s16 arg0, s16 arg1, s32 arg2, s32 arg3);
void drawFullscreenFadeTile(void);
void func_801D99AC(s16 arg0, s16 arg1, s32 arg2);
void drawSpritePrimTpage0(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
void drawSpritePrimTpage1(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6);
u8   pickRandomUnblockedId(u8 arg0);
s8   func_801E2A88(u8 arg0);
u8   func_801E2CA4(void);
u8   pickTargetByMode(s8 arg0);
u8   func_801E2D90(void);
u8   enemyReadyOrHelper1(void);
u8   pickTargetOrFf(s8 arg0);
void dispatchPresentationState1(void);
void dispatchPresentationByte3(void);
extern Battle03Handler D_801EB454[]; /* @source 0x801EB454 @kind unknown */

void dispatchResultSubstateTable(void);
void func_801E5AF4(void);
void runQueuedSlotHandlers(void);
void dispatchByte1FiveTable(void);
void func_801E7818(void);
void dispatchStateByte2TableF80(void);
void func_801E915C(void);
void forwardPanelHalfwords(void);
void callD8450WhenIdle(void);
void activatePanelTask(void);
void commitQueuedPanelTask(void);
void func_801EA1E0(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, u8* selector);
void dispatchResultAuxState(void);
void func_801E8684(void);
void func_801E8D04(void);
void func_801EA650(void);
void func_801EA774(void);
void func_801EA7DC(void);
void func_801EAAB8(void);
u8   advanceUiRingCheck(void);
u8   countMessageTokens(u8* arg0);

#define BATTLE_LOCAL_WORK_ARRAY PSX_PTR(volatile Battle03LocalWork, 0x80145e90u)
#define BATTLE_ENEMY_WORK_ARRAY PSX_PTR(volatile Battle03EnemyWork, 0x801eb630u)
#define BATTLE_QUEUED_SLOT_ARRAY                                               \
  PSX_PTR(volatile Battle03QueuedSlot, 0x801ec330u)
#define BATTLE_LOCAL_WORK_PTR                                                  \
  PSX_REF(volatile Battle03LocalWork*, 0x80146250u)
#define BATTLE_GLOBAL_BYTE_62FC(index)                                         \
  PSX_REF(volatile u8, 0x801462fcu + (u32)(index))
#define BATTLE_GLOBAL_BYTE_62F6(index)                                         \
  PSX_REF(volatile u8, 0x801462f6u + (u32)(index))
#define BATTLE_GLOBAL_BYTE_630C(index)                                         \
  PSX_REF(volatile u8, 0x8014630cu + (u32)(index))
#define BATTLE_UI_CHAR_BUFFER PSX_PTR(volatile u8, 0x80145ad4u)
#define BATTLE_GLOBAL_HALF_6334(index)                                         \
  PSX_REF(volatile u16, 0x80146334u + ((u32)(index) * 2u))
#define BATTLE_GLOBAL_BYTE_6354(index)                                         \
  PSX_REF(volatile u8, 0x80146354u + (u32)(index))
#define BATTLE_GLOBAL_PTR_6380 PSX_REF(volatile u8 *, 0x80146380u)
#define BATTLE_UI_BYTE_8333_INDEX(index)                                       \
  PSX_REF(volatile u8, 0x80148333u + ((u32)(index) * 0x24u))
#define BATTLE_UI_BYTE_833A(index)                                             \
  PSX_REF(volatile u8, 0x8014833au + ((u32)(index) * 0x24u))
#define BATTLE_LOCAL_SCRATCH_PTR                                               \
  PSX_REF(volatile Battle03LocalWork*, 0x1f800044u)
#define BATTLE_ENEMY_SCRATCH_PTR SPAD_PTR_SLOT(volatile Battle03EnemyWork, 0x44u)
/* @kind: bss (map symbol: battleCurrentEnemyWorkState) — current Battle03EnemyWork pointer cell. */
#define BATTLE_CURRENT_ENEMY_PTR                                               \
  PSX_REF(volatile Battle03EnemyWork*, 0x801eb4e8u)
#define BATTLE_CURRENT_QUEUED_SLOT_PTR                                         \
  SPAD_PTR_SLOT(volatile Battle03QueuedSlot, 0x801ec2e0u)
#define BATTLE_CURRENT_QUEUED_PTR_4B20 SPAD_PTR_SLOT(volatile u8, 0x801eb4e0u)
#define BATTLE_SLOT_STORE_FLAG(index)                                          \
  PSX_REF(volatile u8, 0x801ec339u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_PTR(index)                                           \
  PSX_REF(volatile u32, 0x801ec3a4u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_34(index)                                       \
  PSX_REF(volatile u32, 0x801ec364u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_38(index)                                       \
  PSX_REF(volatile u32, 0x801ec368u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_WORD_3C(index)                                       \
  PSX_REF(volatile u32, 0x801ec36cu + ((u32)(index) * 0x78u))
#define BATTLE_EVENT_SLOT_FLAG(index)                                          \
  PSX_REF(volatile u8, 0x801eb4f0u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_A(index)                                             \
  PSX_REF(volatile u8, 0x801eb4f1u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_B(index)                                             \
  PSX_REF(volatile u8, 0x801eb4f2u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_C(index)                                             \
  PSX_REF(volatile u8, 0x801eb4f3u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_KIND(index)                                          \
  PSX_REF(volatile u8, 0x801eb4f5u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_MODE(index)                                          \
  PSX_REF(volatile u8, 0x801eb4f6u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_WORD(index)                                          \
  PSX_REF(volatile u32, 0x801eb4f4u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_HALF(index)                                          \
  PSX_REF(volatile u16, 0x801eb4f8u + ((u32)(index) * 0x0cu))
#define BATTLE_EVENT_SLOT_BYTE(index)                                          \
  PSX_REF(volatile u8, 0x801eb4fau + ((u32)(index) * 0x0cu))
#define BATTLE_SLOT_STORE_BYTE_01(index)                                       \
  PSX_REF(volatile u8, 0x801ec331u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_02(index)                                       \
  PSX_REF(volatile u8, 0x801ec332u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_05(index)                                       \
  PSX_REF(volatile u8, 0x801ec335u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_06(index)                                       \
  PSX_REF(volatile u8, 0x801ec336u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_29(index)                                       \
  PSX_REF(volatile u8, 0x801ec359u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5C(index)                                       \
  PSX_REF(volatile u8, 0x801ec38cu + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5D(index)                                       \
  PSX_REF(volatile u8, 0x801ec38du + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5E(index)                                       \
  PSX_REF(volatile u8, 0x801ec38eu + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_5F(index)                                       \
  PSX_REF(volatile u8, 0x801ec38fu + ((u32)(index) * 0x78u))
#define BATTLE_LOCAL_STATE_TABLE                                               \
  PSX_PTR(const volatile Battle03Handler, 0x801eb120u)
#define BATTLE_LOCAL_FLAGS_80(work) PSX_REF(volatile u16, (u32)(work) + 0x80u)
#define BATTLE_LOCAL_BYTE_79(work)  PSX_REF(volatile u8, (u32)(work) + 0x79u)
#define BATTLE_LOCAL_BYTE_7A(work)  PSX_REF(volatile u8, (u32)(work) + 0x7au)
#define BATTLE_LOCAL_BYTE_82(work)  PSX_REF(volatile u8, (u32)(work) + 0x82u)
#define BATTLE_LOCAL_BYTE_85(work)  PSX_REF(volatile u8, (u32)(work) + 0x85u)
#define BATTLE_LOCAL_BYTE_86(work)  PSX_REF(volatile u8, (u32)(work) + 0x86u)
#define BATTLE_LOCAL_BYTE_87(work)  PSX_REF(volatile u8, (u32)(work) + 0x87u)
#define BATTLE_LOCAL_BYTE_4B(work)  PSX_REF(volatile u8, (u32)(work) + 0x4bu)
#define BATTLE_LOCAL_BYTE_09(work)  PSX_REF(volatile u8, (u32)(work) + 9u)
#define BATTLE_LOCAL_BYTE_0A(work)  PSX_REF(volatile u8, (u32)(work) + 10u)
#define BATTLE_LOCAL_BYTE_119(work) PSX_REF(volatile u8, (u32)(work) + 0x119u)
#define BATTLE_LOCAL_BYTE_118(work) PSX_REF(volatile u8, (u32)(work) + 0x118u)
#define BATTLE_LOCAL_HALF_11A(work) PSX_REF(volatile u16, (u32)(work) + 0x11au)
#define BATTLE_LOCAL_WORD_124(work) PSX_REF(volatile u32, (u32)(work) + 0x124u)
#define BATTLE_LOCAL_WORD_128(work) PSX_REF(volatile u32, (u32)(work) + 0x128u)
#define BATTLE_ABILITY_RECORD_TABLE ABILITY_OBJECTS
#define BATTLE_LOCAL_KIND_MASK(kind)                                           \
  (BATTLE_ABILITY_RECORD_TABLE[(kind)].tail_10.selection_mask)
#define BATTLE_PANEL_SLOT_MASK(kind)                                           \
  PSX_REF(volatile u8, 0x801d90ebu + ((u32)(kind) * 0x18u))
#define BATTLE_LOCAL_HALF_88(work)  PSX_REF(volatile u16, (u32)(work) + 0x88u)
#define BATTLE_LOCAL_HALF_8A(work)  PSX_REF(volatile u16, (u32)(work) + 0x8au)
#define BATTLE_LOCAL_HALF_90(work)  PSX_REF(volatile u16, (u32)(work) + 0x90u)
#define BATTLE_LOCAL_HALF_92(work)  PSX_REF(volatile u16, (u32)(work) + 0x92u)
#define BATTLE_LOCAL_HALF_96(work)  PSX_REF(volatile u16, (u32)(work) + 0x96u)
#define BATTLE_LOCAL_HALF_98(work)  PSX_REF(volatile u16, (u32)(work) + 0x98u)
#define BATTLE_LOCAL_BYTE_8C(work)  PSX_REF(volatile u8, (u32)(work) + 0x8cu)
#define BATTLE_LOCAL_BYTE_2A(work)  PSX_REF(volatile u8, (u32)(work) + 0x2au)
#define BATTLE_LOCAL_BYTE_9E(work)  PSX_REF(volatile u8, (u32)(work) + 0x9eu)
#define BATTLE_LOCAL_BYTE_A6(work)  PSX_REF(volatile u8, (u32)(work) + 0xa6u)
#define BATTLE_LOCAL_BYTE_A9(work)  PSX_REF(volatile u8, (u32)(work) + 0xa9u)
#define BATTLE_LOCAL_HALF_1E(work)  PSX_REF(volatile u16, (u32)(work) + 0x1eu)
#define BATTLE_LOCAL_BYTE_136(work) PSX_REF(volatile u8, (u32)(work) + 0x136u)
#define BATTLE_LOCAL_BYTE_137(work) PSX_REF(volatile u8, (u32)(work) + 0x137u)
#define BATTLE_LOCAL_BYTE_138(work) PSX_REF(volatile u8, (u32)(work) + 0x138u)
#define BATTLE_LOCAL_BYTE_139(work) PSX_REF(volatile u8, (u32)(work) + 0x139u)
#define BATTLE_LOCAL_WORD_134(work) PSX_REF(volatile u32, (u32)(work) + 0x134u)
#define BATTLE_LOCAL_BYTE_134(work) PSX_REF(volatile u8, (u32)(work) + 0x134u)
#define BATTLE_LOCAL_BYTE_120(work) PSX_REF(volatile u8, (u32)(work) + 0x120u)
#define BATTLE_LOCAL_BYTE_121(work) PSX_REF(volatile u8, (u32)(work) + 0x121u)
#define BATTLE_LOCAL_BYTE_122(work) PSX_REF(volatile u8, (u32)(work) + 0x122u)
#define BATTLE_LOCAL_BYTE_13C(work) PSX_REF(volatile u8, (u32)(work) + 0x13cu)
#define BATTLE_LOCAL_HALF_2C(work)  PSX_REF(volatile u16, (u32)(work) + 0x2cu)
#define BATTLE_LOCAL_STATE_TABLE_015C                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb15cu)
#define BATTLE_LOCAL_STATE_TABLE_0188                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb188u)
#define BATTLE_LOCAL_BYTE_TABLE_018C PSX_PTR(const volatile u8, 0x801eb18cu)
#define BATTLE_LOCAL_BYTE_TABLE_0198 PSX_PTR(const volatile u8, 0x801eb198u)
#define BATTLE_LOCAL_SUBSTATE3_TABLE                                           \
  PSX_PTR(const volatile Battle03Handler, 0x801eb1e0u)
#define BATTLE_LOCAL_STATE4_TABLE                                              \
  PSX_PTR(const volatile Battle03Handler, 0x801eb210u)
#define BATTLE_LOCAL_ALT_STATE3_TABLE                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801eb218u)
#define BATTLE_LOCAL_STATE2_CLASS_TABLE                                        \
  PSX_PTR(const volatile Battle03Handler, 0x801eb224u)
#define BATTLE_LOCAL_STATE2_EVENT_TABLE                                        \
  PSX_PTR(const volatile Battle03Handler, 0x801eb26cu)
#define BATTLE_LOCAL_STATE2_FOLLOWUP_TABLE                                     \
  PSX_PTR(const volatile Battle03Handler, 0x801eb274u)
#define BATTLE_LOCAL_DEFAULT_CLASS_TABLE                                       \
  PSX_PTR(const volatile Battle03Handler, 0x801eb27cu)
/* The 0x801EB294/0x801EB298 pair is bound as symbols because the original
 * func_801E2170 encoding is symbol-relative: both handler-table bases are
 * hoisted into callee-saved registers in the preheader (`lui`/`addiu`).
 * Spelled as fixed-address macros GCC instead materializes each base with
 * `lui`/`ori` inside its branch (asm-diff 32/69, first mismatch +0x000c). */
#define BATTLE_ENEMY_DISPATCH_TABLE_A D_801EB294
#define BATTLE_ENEMY_DISPATCH_TABLE_B D_801EB298
#define BATTLE_ENEMY_FLAGS_82(work) PSX_REF(volatile u16, (u32)(work) + 0x82u)
#define BATTLE_ENEMY_FLAGS_80(work) PSX_REF(volatile u16, (u32)(work) + 0x80u)
#define BATTLE_ENEMY_BYTE_7E(work)  PSX_REF(volatile u8, (u32)(work) + 0x7eu)
#define BATTLE_ENEMY_BYTE_7D(work)  PSX_REF(volatile u8, (u32)(work) + 0x7du)
#define BATTLE_ENEMY_BYTE_7C(work)  PSX_REF(volatile u8, (u32)(work) + 0x7cu)
#define BATTLE_ENEMY_BYTE_7F(work)  PSX_REF(volatile u8, (u32)(work) + 0x7fu)
#define BATTLE_ENEMY_BYTE_02(work)  PSX_REF(volatile u8, (u32)(work) + 2u)
#define BATTLE_ENEMY_BYTE_03(work)  PSX_REF(volatile u8, (u32)(work) + 3u)
#define BATTLE_ENEMY_BYTE_04(work)  PSX_REF(volatile u8, (u32)(work) + 4u)
#define BATTLE_ENEMY_BYTE_05(work)  PSX_REF(volatile u8, (u32)(work) + 5u)
#define BATTLE_ENEMY_HALF_AA(work)  PSX_REF(volatile u16, (u32)(work) + 0xaau)
#define BATTLE_ENEMY_HALF_94(work)  PSX_REF(volatile u16, (u32)(work) + 0x94u)
#define BATTLE_ENEMY_BYTE_88(work)  PSX_REF(volatile u8, (u32)(work) + 0x88u)
#define BATTLE_ENEMY_HALF_A8(work)  PSX_REF(volatile u16, (u32)(work) + 0xa8u)
#define BATTLE_ENEMY_HALF_FA(work)  PSX_REF(volatile u16, (u32)(work) + 0xfau)
#define BATTLE_ENEMY_BYTE_F5(work)  PSX_REF(volatile u8, (u32)(work) + 0xf5u)
#define BATTLE_ENEMY_BYTE_FC(work)  PSX_REF(volatile u8, (u32)(work) + 0xfcu)
#define BATTLE_ENEMY_BYTE_FD(work)  PSX_REF(volatile u8, (u32)(work) + 0xfdu)
#define BATTLE_ENEMY_HALF_F6(work)  PSX_REF(volatile u16, (u32)(work) + 0xf6u)
#define BATTLE_ENEMY_WORD_100(work) PSX_REF(volatile u32, (u32)(work) + 0x100u)
#define BATTLE_ENEMY_BYTE_100(work) PSX_REF(volatile u8, (u32)(work) + 0x100u)
#define BATTLE_ENEMY_WORD_104(work) PSX_REF(volatile u32, (u32)(work) + 0x104u)
#define BATTLE_ENEMY_BYTE_112(work) PSX_REF(volatile u8, (u32)(work) + 0x112u)
#define BATTLE_ENEMY_BYTE_114(work) PSX_REF(volatile u8, (u32)(work) + 0x114u)
#define BATTLE_ENEMY_BYTE_115(work) PSX_REF(volatile u8, (u32)(work) + 0x115u)
#define BATTLE_ENEMY_BYTE_E0(work)  PSX_REF(volatile u8, (u32)(work) + 0xe0u)
#define BATTLE_ENEMY_BYTE_E6(work)  PSX_REF(volatile u8, (u32)(work) + 0xe6u)
#define BATTLE_ENEMY_PTR_EC(work)                                              \
  SPAD_PTR_SLOT(volatile u8, (volatile u8*)(work) + 0xecu)
#define BATTLE_WEIGHT_TABLE_0394       PSX_PTR(const volatile u8, 0x801eb394u)
#define BATTLE_WEIGHT_TABLE_039C       PSX_PTR(const volatile u8, 0x801eb39cu)
#define BATTLE_RANDOM_TABLE_AC58       PSX_PTR(const volatile u8, 0x801eac58u)
#define BATTLE_RANDOM_TABLE_AC78       PSX_PTR(const volatile u8, 0x801eac78u)
/* @kind: rodata (map symbol: battleCommandEntryTable) — command entries: 12-byte params + 16-byte name. */
#define BATTLE_RETRY_TABLE_AFF4        PSX_PTR(const volatile u8, 0x801eaff4u)
#define BATTLE_COUNTER_TABLE_AFFC      PSX_PTR(const volatile u8, 0x801eaffcu)
#define BATTLE_PERCENT_TABLE_AF3C      PSX_PTR(const volatile u16, 0x801eaf3cu)
#define BATTLE_RANDOM_BONUS_TABLE_AF48 PSX_PTR(const volatile s8, 0x801eaf48u)
#define BATTLE_RANK_TABLE_AF88         PSX_PTR(const volatile u8, 0x801eaf88u)
#define BATTLE_VARIANCE_TABLE_AFA0     PSX_PTR(const volatile s32, 0x801eafa0u)
#define BATTLE_SCALE_TABLE_AFC0        PSX_PTR(const volatile s16, 0x801eafc0u)
#define BATTLE_DAMAGE_SCALE_TABLE_0C7C PSX_PTR(const volatile u8, 0x801d0c7cu)
#define BATTLE_EVENT_PICK_TABLE_0C98   PSX_PTR(const volatile u8, 0x801d0c98u)
#define BATTLE_EVENT_PICK_TABLE_0CB8   PSX_PTR(const volatile u8, 0x801d0cb8u)
#define BATTLE_VARIANCE_TABLE_AF94     PSX_PTR(const volatile u8, 0x801eaf94u)
#define BATTLE_TARGET_MODE_PACK(index)                                         \
  PSX_REF(volatile u8, 0x800b51f8u + (u32)(index))
#define BATTLE_ENEMY_SLOT_KIND(index)                                          \
  PSX_REF(volatile u8, 0x801eb6acu + ((u32)(index) * 0x118u))
#define BATTLE_KIND_BYTE_00(kind)                                              \
  PSX_REF(volatile u8, 0x801ca718u + ((u32)(kind) * 0x14u))
#define BATTLE_LOCAL_PRESENTATION_BYTE3_TABLE                                  \
  PSX_PTR(const volatile Battle03Handler, 0x801eb430u)
#define BATTLE_QUEUED_RESULT_SUBSTATE_TABLE                                    \
  PSX_PTR(const volatile Battle03Handler, 0x801eb454u)
#define BATTLE_ACTIVE_SLOT_TABLE_0                                             \
  PSX_PTR(const volatile Battle03Handler, 0x801d0cd0u)
#define BATTLE_QUEUED_SLOT_TABLE                                               \
  PSX_PTR(const volatile Battle03Handler, 0x801d0cc0u)
#define BATTLE_PANEL_TASK_ROOT_TABLE                                           \
  PSX_PTR(const volatile Battle03Handler, 0x801d0f80u)
#define BATTLE_PANEL_TASK_ARG_DISPATCH_TABLE                                   \
  PSX_PTR(const volatile Battle03ForwardingHandler, 0x801d0fecu)
#define BATTLE_PANEL_TASK_PTR SPAD_PTR_SLOT(volatile u8, 0x80148648u)
#define BATTLE_PANEL_TASK_HALF_04                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 4)
#define BATTLE_PANEL_TASK_HALF_06                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 6)
#define BATTLE_PANEL_TASK_BYTE_03                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 3)
#define BATTLE_PANEL_TASK_BYTE_0F                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0xf)
#define BATTLE_PANEL_TASK_BYTE_0A                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0x0au)
#define BATTLE_PANEL_TASK_BYTE_0B                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0x0bu)
#define BATTLE_PANEL_TASK_BYTE_0D                                              \
  PSX_REF(volatile u8, (u32)(BATTLE_PANEL_TASK_PTR) + 0x0du)
#define BATTLE_PANEL_TASK_HALF_10                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 0x10)
#define BATTLE_PANEL_TASK_HALF_12                                              \
  PSX_REF(volatile u16, (u32)(BATTLE_PANEL_TASK_PTR) + 0x12)
#define BATTLE_UI_RING_BYTE0(index)                                            \
  PSX_REF(volatile u8, 0x801eb5b0u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_BYTE1(index)                                            \
  PSX_REF(volatile u8, 0x801eb5b1u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_WORD2(index)                                            \
  PSX_REF(volatile u32, 0x801eb5b4u + ((u32)(index) * 8u))
#define BATTLE_UI_RING_BYTE(index)                                             \
  PSX_REF(volatile u8, 0x801eb4fau + ((u32)(index) * 0x0cu))
#define BATTLE_UI_RING_WORD(index)                                             \
  PSX_REF(volatile u32, 0x801eb4f4u + ((u32)(index) * 0x0cu))
#define BATTLE_UI_MODE_TABLE_AF27       PSX_PTR(const volatile u8, 0x801eaf27u)
#define BATTLE_QUAD_OFFSET_TABLE_AD30   PSX_PTR(const volatile s16, 0x801ead30u)
#define BATTLE_SPRITE_OFFSET_TABLE_AE50 PSX_PTR(const volatile s16, 0x801eae50u)
#define BATTLE_ICON_OFFSET_TABLE_AE94   PSX_PTR(const volatile u8, 0x801eae94u)
#define BATTLE_PANEL_FRAME_TABLE_AEE8   PSX_PTR(const volatile s16, 0x801eaee8u)
#define BATTLE_PANEL_ICON_TABLE_AEB0    PSX_PTR(const volatile u32, 0x801eaeb0u)
#define BATTLE_ICON_CLUT_TABLE_0C64     PSX_PTR(const volatile u8, 0x801d0c64u)
#define BATTLE_GLOBAL_PTR_BF08          SPAD_PTR_SLOT(volatile u8, 0x801ebf08u)
#define BATTLE_LOCAL_ALT_WORK_ARRAY     PSX_PTR(volatile u8, 0x801ebf20u)
#define BATTLE_LOCAL_STATUS_ARRAY       PSX_PTR(volatile u8, 0x801ec048u)
#define BATTLE_PANEL_TASK_ICON_TABLE                                           \
  PSX_PTR(const volatile Battle03Handler, 0x801d0ff8u)
#define BATTLE_RESULT_UI_AUX_HANDLER_0 ((Battle03Handler)0x801e8684u)
#define BATTLE_RESULT_UI_AUX_HANDLER_1 ((Battle03Handler)0x801e8d04u)
#define BATTLE_PREVIEW_SEQUENCE_TABLE                                          \
  PSX_PTR(const volatile Battle03Handler, 0x801d0f44u)
#define BATTLE_SAVED_PREVIEW_RESULT_TABLE                                      \
  PSX_PTR(const volatile Battle03Handler, 0x801d0f6cu)

/* Scratchpad flags at 0x1F800000 (u16). */
#define BATTLE_SCRATCH_FLAGS SPAD_REF(volatile u16, 0u)

/* Scratchpad pointer cell at 0x1F800044, read as several cell types. */
#define BATTLE_SCRATCH_CELL_U8PTR PSX_REF(volatile u8*, 0x1f800044u)
#define BATTLE_SCRATCH_CELL_WORD PSX_REF(volatile u32, 0x1f800044u)

/* Local-work-array (0x80145e90, stride 0x140) absolute-field accessors. */
#define BATTLE_LOCAL_ABS_BYTE_5E90(index)                                      \
  PSX_REF(volatile u8, 0x80145e90u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F10(index)                                      \
  PSX_REF(volatile u16, 0x80145f10u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F18(index)                                      \
  PSX_REF(volatile u16, 0x80145f18u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F1A(index)                                      \
  PSX_REF(volatile u16, 0x80145f1au + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_BYTE_5F1C(index)                                      \
  PSX_REF(volatile u8, 0x80145f1cu + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_BYTE_5FB0(index)                                      \
  PSX_REF(volatile u8, 0x80145fb0u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F26(index)                                      \
  PSX_REF(volatile u16, 0x80145f26u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_HALF_5F28(index)                                      \
  PSX_REF(volatile u16, 0x80145f28u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_BYTE_5FCC(index)                                      \
  PSX_REF(volatile u8, 0x80145fccu + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_WORD_5F04(index)                                      \
  PSX_PTR(volatile u32, 0x80145f04u + ((u32)(index) * 0x140u))
#define BATTLE_LOCAL_ABS_WORD_5FB8(index)                                      \
  PSX_REF(volatile u32, 0x80145fb8u + ((u32)(index) * 0x140u))

/* Ability template records (0x80144968, stride 0xa4) absolute-field accessors. */
#define BATTLE_TEMPLATE_ABS_WORD_4968(index)                                   \
  PSX_PTR(const volatile u32, 0x80144968u + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_HALF_497C(index)                                   \
  PSX_REF(volatile u16, 0x8014497cu + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_HALF_497E(index)                                   \
  PSX_REF(volatile u16, 0x8014497eu + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_HALF_4974(index)                                   \
  PSX_REF(volatile u16, 0x80144974u + ((u32)(index) * 0xa4u))
#define BATTLE_TEMPLATE_ABS_BYTE_4980(index)                                   \
  PSX_REF(volatile u8, 0x80144980u + ((u32)(index) * 0xa4u))

/* Enemy-work-array (0x801eb630, stride 0x118) absolute-field accessors. */
#define BATTLE_ENEMY_ABS_HALF_6B2(index)                                       \
  PSX_REF(volatile u16, 0x801eb6b2u + ((u32)(index) * 0x118u))
#define BATTLE_ENEMY_ABS_WORD_734(index)                                       \
  PSX_REF(volatile u32, 0x801eb734u + ((u32)(index) * 0x118u))

/* Queued-slot (0x801ec330, stride 0x78) missing byte-field accessors. */
#define BATTLE_SLOT_STORE_BYTE_00(index)                                       \
  PSX_REF(volatile u8, 0x801ec330u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_03(index)                                       \
  PSX_REF(volatile u8, 0x801ec333u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_04(index)                                       \
  PSX_REF(volatile u8, 0x801ec334u + ((u32)(index) * 0x78u))
#define BATTLE_SLOT_STORE_BYTE_48(index)                                       \
  PSX_REF(volatile u8, 0x801ec378u + ((u32)(index) * 0x78u))

/* UI ring head byte at 0x801ec328. */
#define BATTLE_UI_RING_HEAD PSX_REF(volatile u8, 0x801ec328u)

/* Local-state dispatch tables reached as (0x801f0000 - offset) + index*4. */
#define BATTLE_DISPATCH_STATE2_CLASS(index)                                    \
  PSX_PTR(const volatile Battle03Handler, 0x801eb124u + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_STATE2_EVENT(index)                                    \
  PSX_PTR(const volatile Battle03Handler, 0x801eb16cu + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_STATE2_FOLLOWUP(index)                                 \
  PSX_PTR(const volatile Battle03Handler, 0x801eb174u + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_PRESENTATION_BYTE3(index)                              \
  PSX_PTR(const volatile Battle03Handler, 0x801eb24cu + ((u32)(index) * 4u))
#define BATTLE_DISPATCH_QUEUED_RESULT(index)                                   \
  PSX_PTR(const volatile Battle03Handler, 0x801eb270u + ((u32)(index) * 4u))

/* Offset/position tables for queued-slot rendering (func_801E8DD8). */
#define BATTLE_OFFSET_TABLE_0B10(a, b)                                         \
  PSX_REF(volatile s8, 0x801eb0b0u + ((u32)(a) * 2u) + ((u32)(b) * 8u))
#define BATTLE_ENEMY_OFFSET_TABLE_0B08(b)                                      \
  PSX_REF(volatile u8, 0x801eb108u + ((u32)(b) * 2u))
#define BATTLE_ENEMY_OFFSET_TABLE_0B09(b)                                      \
  PSX_REF(volatile u8, 0x801eb109u + ((u32)(b) * 2u))
#define BATTLE_ENEMY_OFFSET_U8_0B10(index)                                     \
  PSX_REF(volatile u8, 0x801eb710u + ((u32)(index) * 0x118u))
#define BATTLE_ENEMY_OFFSET_S8_0B12(index)                                     \
  PSX_REF(volatile s8, 0x801eb712u + ((u32)(index) * 0x118u))
#define BATTLE_CLASS_OFFSET_0C0CB(b)                                           \
  PSX_REF(volatile u8, 0x800e40cbu + ((u32)(b) * 0x88u))
/* +0x03 byte of each 0x88-byte class record; non-volatile so GCC keeps the
 * constant as the load displacement (`lui $at; addu $at,idx,$at; lbu %lo`). */
#define BATTLE_CLASS_BYTE_0C0CE(b)                                             \
  PSX_REF(u8, 0x800e40ceu + ((u32)(b) * 0x88u))

/* Panel-task root pointer cell at 0x801485c8 (u8* volatile). */
#define BATTLE_PANEL_ROOT_CELL PSX_REF(u8* volatile, 0x801485c8u)
#define BATTLE_PANEL_ROOT_BASE PSX_PTR(u8* volatile, 0x80150000u)

/* Base-pointer accessors for fixed RAM regions (preserve lui+offset codegen). */
#define BATTLE_GLOBAL_RAM_U8  PSX_PTR(volatile u8, 0x80140000u)
#define BATTLE_GLOBAL_RAM_U16 PSX_PTR(volatile u16, 0x80140000u)
#define BATTLE_GLOBAL_RAM_U32 PSX_PTR(volatile u32, 0x80140000u)
#define BATTLE_HIGH_RAM_U8    PSX_PTR(volatile u8, 0x801f0000u)
#define BATTLE_HIGH_RAM_U16   PSX_PTR(volatile u16, 0x801f0000u)
#define BATTLE_HIGH_RAM_U32   PSX_PTR(volatile u32, 0x801f0000u)

/* Additional fixed-base tables and dispatch helpers. */
#define BATTLE_ENEMY_TABLE_6D8    PSX_PTR(volatile u16, 0x801eb6d8u)
#define BATTLE_MODE_TUPLE_62E0    PSX_PTR(volatile u8, 0x801462e0u)
#define BATTLE_TABLE_81B10        PSX_PTR(volatile u8, 0x80181b10u)
#define BATTLE_SCRIPT_TABLE_490D8 PSX_PTR(void, 0x801490d8u)
#define BATTLE_ROM_BASE_D0000     PSX_PTR(const u8, 0x801d0000u)
#define BATTLE_DISPATCH_SUBSTATE3(index)                                       \
  PSX_PTR(const volatile Battle03Handler, 0x801eb1e0u + ((u32)(index) * 4u))
#define BATTLE_SCRIPT_TABLE_492B8 PSX_PTR(void, 0x801492b8u)
#define BATTLE_SCRIPT_TABLE_0B00D PSX_PTR(const void, 0x801eb00du)

/* Single fixed-address globals (inline; no symbol binding required). */
#define BATTLE_GLOBAL_WORD_598C  PSX_REF(u32, 0x8014598cu)
#define BATTLE_GLOBAL_BYTE_62F0  PSX_REF(volatile u8, 0x801462f0u)
#define BATTLE_GLOBAL_BYTE_62F3  PSX_REF(u8, 0x801462f3u)
#define BATTLE_GLOBAL_HALF_EC2EE PSX_REF(volatile u16, 0x801ec2eeu)

/* @source 0x801EAFF4 @kind unknown */
extern u8 battleCommandEntryTable;

/* @source 0x801EB4E8 @kind unknown */
extern Battle03EnemyWork* battleCurrentEnemyWorkState;

#endif
