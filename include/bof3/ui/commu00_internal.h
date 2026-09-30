#ifndef EMI_COMMU00_00_INTERNAL_H
#define EMI_COMMU00_00_INTERNAL_H

#include "bof3/bof3.h"

typedef struct Commu00TaskSlot {
  u8  unk_00;
  u8  mode;
  u8  state;
  u8  unk_03[2];
  u8  source_index;
  u8  active;
  u8  unk_07;
  u8  resource_id;
  u8  unk_09[0xf];
  u32 variant_arg_0;
  u32 variant_arg_1;
  u8  unk_20[0x14];
  s16 field_34;
  s16 field_36;
  s16 field_38;
  s16 field_3a;
  u8  unk_3c[2];
  s16 field_3e;
  u8  unk_40[8];
  u8  reset_flag;
  u8  unk_49[0x30];
  u8  variant_state;
  u8  unk_7a[2];
  u16 label_id;
  u8  unk_7e[0x1a];
} Commu00TaskSlot;

typedef struct Commu00ActiveRecord {
  u8  active;
  u8  kind;
  u8  record_state;
  u8  unk_03;
  u32 progress_anchor;
} Commu00ActiveRecord;

/* Three u16 columns per fairy level; the level is the high nibble of the
 * stride-8 active-record state byte. */
typedef struct Commu00FairyLabelRecord {
  u16 unk_00;
  u16 unk_02;
  u16 unk_04;
} Commu00FairyLabelRecord;

/* One four-byte band entry of the container-local band table at 0x801EEC48:
 * the halfword bound the elapsed battle count is tested against, then the two
 * bytes the local label helper consumes as its (arg0, arg1) arguments (the
 * +3 byte is arg0, the +2 byte is arg1). */
typedef struct Commu00BandRecord {
  u16 threshold;
  u8  unk_02;
  u8  unk_03;
} Commu00BandRecord;

/* @source 0x80145E30 @kind unknown: removal queue bytes; the random-slot
 * removal scan appends the slot index it clears here, indexed by the append
 * counter at 0x80145E44.
 */
extern u8           D_80145E30[];
/* @source 0x80145E44 @kind unknown: notification gate byte; the settle path
 * requests UI mode 6 only while this byte, the pending queue count, and the
 * queue append index are all clear.
 */
extern u8           D_80145E44;
/* @source 0x80145E5C @kind unknown */
extern volatile u8  COMMU00_PENDING_QUEUE_COUNT;
/* @source 0x80145E5D @kind unknown: append index of the pending queue, advanced once
 * per notification stored by the type-45 notification scan.
 */
extern volatile u8  D_80145E5D;
/* @source 0x80145E60 @kind unknown: pending notification queue bytes, indexed in
 * pairs by the append index.
 */
extern volatile u8  D_80145E60[];
/* @source 0x8014688E @kind bss */
extern u8           D_8014688E[];
/* @source 0x80146888 @kind bss: 0x98-byte commu00 task slots; the scratchpad
 * task cursor at 0x1F800044 points into this array.
 */
extern Commu00TaskSlot D_80146888[];
/* @source 0x80146904 @kind bss */
extern u16          taskLabelWords[1];
/* @source 0x801448EB @kind bss: commu00 UI mode byte; requested values 0, 14, and 23. */
extern u8           uiMode;
/* @source 0x801448EC @kind bss: fairy progress byte; cleared on selection reset, advanced by
 * message selection, and indexes the progress handler tables.
 */
extern u8           fairyProgress[1];
/* @source 0x801448EC @kind bss: scalar view of the fairy progress byte; the
 * 0x801F1824 original translation unit reads and writes this byte as a plain
 * scalar, which folds each access at the symbol address instead of
 * materializing the array base into a register.
 */
extern u8           D_801448EC;
/* @source 0x801448ED @kind bss: fairy slot index byte; indexes the slot handler table. */
extern u8           fairySlotIndex;
/* @source 0x801448F2 @kind unknown: byte stored as 1 once the commu00 mode-0x17 UI
 * request has been issued.
 */
extern u8           D_801448F2;
/* @source 0x80144F28 @kind unknown: shared EXE flag-byte bank; the commu00
 * variant-1 arm probes entry 0x92 of this bank through the EXE-side flag helper.
 */
extern u8           D_80144F28[];
/* @source 0x80145029 @kind unknown: shared EXE selected-record index byte; the
 * 0x801F23A4 state-0 handler publishes it into the second byte of the
 * 0x801F2944 pair and indexes the shared 4-byte-stride record pair at
 * 0x80181EBA with it.
 */
extern u8           D_80145029;
/* @source 0x801F2928 @kind rodata: address-derived non-volatile alias of the
 * variant rotation bytes; the 0x801F1110 translation unit reads the byte once,
 * so its read must not go through the volatile view used by the two-read
 * variant scanners.
 */
extern u8           D_801F2928[1];
/* @source 0x801F2928 @kind rodata: variant rotation bytes backing COMMU00_VARIANT_ROTATION;
 * variant-derived labels are entries minus 0x3FFB.
 */
extern volatile u8  variantRotation[1];
/* @source 0x801F2930 @kind bss: three rows of three u16 task labels selected by battle-age band. */
extern u16          taskLabelBandTable[3][3];
/* @source 0x1F800044 @kind bss: scratchpad current task pointer cell. */
extern Commu00TaskSlot *commu00ScratchTask;
/* @source 0x801F2948 @kind bss: current commu00 task pointer. */
extern Commu00TaskSlot *currentCommu00Task;
/* @source 0x80143BB0 @kind unknown: shared frontend mode byte; func_801F1AFC
 * settles the UI selection state while it is clear.
 */
extern u8           D_80143BB0;
/* @source 0x801455B4 @kind unknown: shared counter snapshot word; func_801EF110
 * compares it against the shared counter at 0x8014502C and republishes the
 * counter there after advancing the 0x801455C2 byte.
 */
extern u32          D_801455B4;
/* @source 0x801455B8 @kind unknown: shared counter snapshot word of the
 * village-expansion tier path; func_801EF214 compares the counter at
 * 0x8014502C against it and advances it by the tier total consumed.
 */
extern u32          D_801455B8;
/* @source 0x801455C0 @kind unknown: u16 notification scan count latched to the
 * scan limit by the type-45 notification scan.
 */
extern u16          D_801455C0;
/* @source 0x801455C2 @kind unknown: signed counter byte that func_801EF110
 * clamps to 0..99; the filler init path seeds it with 10 and func_801EEEF0
 * reads it as the signed entry bound it scales the row index against.
 */
extern s8           D_801455C2;
/* @source 0x80143F00 @kind unknown: u16 notification scan limit (pending queue
 * capacity) bounding the type-45 notification scan.
 */
extern u16          D_80143F00;
/* @source 0x80144FC0 @kind unknown: shared byte that the commu00 counter-formatting
 * handler at 0x801F1318 prints into the first D_801490D8 text slot.
 */
extern u8           D_80144FC0;
/* @source 0x80144FC1 @kind unknown: shared byte that the commu00 counter-formatting
 * handler at 0x801F1318 prints into the second D_801490D8 text slot.
 */
extern u8           D_80144FC1;
extern u32          D_8014502C;
/* @source 0x80145030 @kind unknown: shared counter word that the commu00
 * counter-formatting handler at 0x801F1318 prints into the fourth D_801490D8
 * text slot.
 */
extern u32          D_80145030;
/* @source 0x80145034 @kind unknown: shared counter word that the commu00
 * counter-formatting handlers print into a D_801490D8 text slot.
 */
extern u32          D_80145034;
/* @source 0x80145038 @kind unknown: shared counter word that the commu00
 * counter-formatting handlers print into a D_801490D8 text slot.
 */
extern u32          D_80145038;
/* @source 0x8014503C @kind unknown: shared counter word that the commu00
 * counter-formatting handlers print into a D_801490D8 text slot.
 */
extern u32          D_8014503C;
/* @source 0x80145024 @kind unknown: stream index hint byte; the low seven bits
 * select the EMI stream slot index (family family 0 here).
 */
extern u8           D_80145024;
/* @source 0x80146854 @kind unknown: shared EMI loader state byte; the 0x20 bit
 * gates the staged transfer slots in stageEmiTransferSlot.
 */
extern u8           D_80146854;
/* @source 0x801490A8 @kind unknown: shared u16 flag word that the world resets
 * publish as 0xFFFF; the commu00 progress advance takes its 0xF9 arm when this
 * word holds 0xF9.
 */
extern u16          D_801490A8;
/* @source 0x8014625A @kind unknown: shared front/world flag word; the commu00
 * settle path clears its low three bits.
 */
extern u16          D_8014625A;
/* @source 0x801455C3 @kind bss: village-expansion state byte (10 gate). */
extern u8           D_801455C3;
/* @source 0x801455C4 @kind bss: village-expansion state byte (7 gate). */
extern u8           D_801455C4;
/* @source 0x801455C5 @kind bss: byte total that func_801F0320 accumulates the
 * weight byte of every active kind-10 record into.
 */
extern u8           D_801455C5;
/* @source 0x801455C6 @kind bss: byte total that func_801F0320 accumulates the
 * weight byte of every active kind-11 record into.
 */
extern u8           D_801455C6;
/* @source 0x801455C8 @kind unknown: typed view of the active-record region. */
extern volatile Commu00ActiveRecord activeRecordBytes[];
extern u8           D_801455C9[];
/* @source 0x801455CA @kind unknown: state byte at +2 of the stride-8 active
 * record slots; the non-volatile byte view the fairy outcome draw
 * tests twice, so the compiler folds both tests into one load.
 */
extern u8           D_801455CA[];
/* @source 0x801457A8 @kind unknown: kind byte at +0 of the stride-8 local
 * record slots; the variant dispatcher selects its record-kind handler from it.
 */
extern u8           D_801457A8[];
extern u8           D_801457A9[];
extern u8           D_801457AA[];
extern u8           D_801457AB[];
extern u8           D_801CA28C[];
/* @source 0x80181EBA @kind table: shared EXE selection-effect record pair; the
 * 4-byte-stride entry selected by 0x80145029 carries the two effect indices at
 * bytes +0 and +1 that the frontend selection-effect helper consumes.
 */
extern const u8     D_80181EBA[];
/* @source 0x80145988 @kind unknown: shared byte published as 1 by func_801F17A4
 * once the EMI loader reports the streamed slot ready.
 */
extern u8           D_80145988;

/* Shared commu00 item-name buffer and its terminator byte. */
/* @source 0x801490D8 @kind unknown: twelve-byte item-name buffer filled from
 * the local label helper before the selected action starts.
 */
extern u8           D_801490D8[];
/* @source 0x801490E4 @kind unknown: terminator byte of the 0x801490D8 item-name
 * buffer.
 */
extern u8           D_801490E4[];
/* @source 0x801EEC44 @kind rodata: container-local decimal format string at
 * payload offset 0x44; the commu00 counter-formatting handlers pass its address
 * as the sprintf format for each D_801490D8 text slot.
 */
extern u8           D_801EEC44[];
/* @source 0x801EEC48 @kind table: container-local twenty-entry band table at
 * payload offset 0x48; the commu00 progress handler compares each entry's
 * halfword bound against the elapsed battle count of the fairy slot's active
 * record and passes the selected entry's +3/+2 bytes to the local label helper
 * in that order. INFERRED: the halfword column is the elapsed-count bound -- the
 * twenty payload halfwords are strictly increasing (1 to 0x1F4) and delimit the
 * bands the handler steps back through once the count passes an entry.
 */
extern Commu00BandRecord commu00BandTable[];
/* @source 0x801EEC98 @kind table: container-local twelve-byte weight table at
 * payload offset 0x98, three four-byte records; the fairy outcome draw
 * subtracts the first three bytes of the record selected by the local
 * record-kind slot byte from its clamped random value to pick a band.
 */
extern u8           D_801EEC98[];

/* Local commu00 mutable data at the tail of the T_801F24FC blob. */
/* @source 0x801F2944 @kind unknown: two bytes cleared by func_801F17A4; the
 * fairy progress state-0 handler reads byte 0 as a flag (effect group 10 when
 * set, otherwise 8) and stores a selected record index into byte 1.
 */
extern u8           D_801F2944[2];
/* @source 0x801F294C @kind unknown: four bytes cleared by func_801F17A4.
 */
extern u8           D_801F294C[4];

/* Local commu00 dispatch tables (data blob T_801F24FC). */
/* @source 0x801F24FC @kind unknown: u16 table read with the source index
 * reduced modulo six; the selected halfword seeds the slot table entry.
 */
extern u16          D_801F24FC[];
/* @source 0x801F2514 @kind table: u16 tier totals indexed by the
 * village-expansion state byte at 0x801455C4; func_801EF214 requires the
 * counter at 0x8014502C to have moved that far past its snapshot before the
 * tier byte may advance.
 */
extern u16          D_801F2514[];
/* @source 0x801F25A4 @kind unknown: three-byte variant records indexed by the
 * record index times nine plus the rotation byte times three; the bytes seed a
 * task slot's coordinate pair and resource id.
 */
extern u8           D_801F25A4[];
/* @source 0x801F25EC @kind table: six function pointers indexed by the fairy progress byte
 * (plus a second group dispatched at offset +6).
 */
extern void (*progressHandlerTable[6])(void);
/* @source 0x801F2610 @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable2[])(void);
/* @source 0x801F2618 @kind table: +0 byte column of the local forty-eight-entry
 * reward pair table; the fairy outcome draw indexes it with (*2) a band index
 * times sixteen plus a random nibble and passes the selected pair to the local
 * label helpers.
 */
extern u8           D_801F2618[];
/* @source 0x801F2619 @kind table: +1 byte column of the 0x801F2618 reward pair
 * table.
 */
extern u8           D_801F2619[];
/* @source 0x801F2678 @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable3[])(void);
/* @source 0x801F269C @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable4[])(void);
/* @source 0x801F26B0 @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable5[])(void);
/* @source 0x801F26EC @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable6[])(void);
/* @source 0x801F2705 @kind table: container-local byte column read at stride
 * nine in lockstep with the stride-8 active-record table by func_801EF110, which
 * adds three to each byte it reads; it is the byte immediately below the
 * 0x801F2706 column of the same sixty stride-9 records. INFERRED: the two
 * columns are the first two bytes of one stride-9 record (the byte read here
 * precedes the 0x801F2706 weight) -- both are walked at the same stride, so the
 * 0x801F2705 byte is a record field rather than the tail of its predecessor.
 */
extern u8           D_801F2705[];
/* @source 0x801F2706 @kind table: sixty stride-9 local weight records walked in
 * lockstep with the active-record table by func_801F0320, which reads the +0
 * byte of each record.
 */
extern u8           D_801F2706[];
/* @source 0x801F2708 @kind table: +3 byte column of the sixty stride-9 local
 * records; func_801EF214 sums it over the active records whose kind byte equals
 * the tier index plus one.
 */
extern u8           D_801F2708[];
/* @source 0x801F268C @kind table: function pointers indexed by the fairy slot index byte. */
extern void (*slotHandlerTable[])(void);
/* @source 0x801F26CC @kind table: 6-byte label records indexed by the high
 * nibble of the stride-8 active-record state byte.
 */
extern const Commu00FairyLabelRecord D_801F26CC[];

void func_8014E284(void);
void func_80150224(s32 action_id);
s32  func_8015B5D4(u32 arg0, s32 arg1);
void func_8015BAC4(u8 resource_id);
void func_8015C058(void);
void func_8015C088(void);
void func_8015D404(u32 arg0, s32 arg1);
void func_801636A0(u32 arg0, u32 arg1);
u32  func_80163EA0(void);
u8   func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
u8*  func_80165D48(s32 arg0, u8 arg1);
void func_8016728C(u8 index, u8 family);

s16  func_8015477C(s32 coord_x, s32 coord_y);
void func_8014DD3C(u16 arg0);
void func_801EEDF8(void);
void func_801EEEF0(u32 row_index);
void func_801EF110(u8 active_count);
void func_801EF214(void);
void func_801EF398(void);
void func_801EF52C(void);
void func_801EF784(void);
void func_801EFA50(void);
void func_801EFEA8(void);
void func_801F00D4(void);
void func_801F01F4(void);
u8   countActiveRecords(void);
void func_801F0320(void);
void func_801F0C6C(u8 task_index, u8 record_kind_index);
void func_801F0D3C(u8 task_index, u8 record_kind_index);
void func_801F0E1C(u8 task_index, u8 record_kind_index);
void activateTaskStatusC003(u8 task_index);
void activateScratchTaskWithBandLabel(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801F0FBC(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801F1064(u8 task_index, u8 record_kind_index);
void func_801F1110(u8 task_index, u8 record_kind_index);
void setVariantTaskStatus(u8 task_index, u8 record_kind_index);
void activateTaskStatusC00A(u8 task_index);
void func_801F1F9C(void);
void func_801F2020(void);
void func_801F228C(void);

void clearUiSelectionState(void);
void dispatchCurrentTaskVariantResource(void);

void func_801F0534(void);
void func_801F0718(u8 source_index, u8 task_index);
void func_801F08D8(u8 source_index, u8 task_index);
void activateTaskWithRandomLabel(u8 task_index);

#define COMMU00_TASK_SLOTS PSX_PTR(volatile Commu00TaskSlot, 0x80146888u)

#define COMMU00_ACTIVE_RECORD_BASE   ((u32)0x801455c8u)
#define COMMU00_ACTIVE_RECORDS       activeRecordBytes
#define COMMU00_RECORD_KIND_TABLE    PSX_PTR(const volatile u8, 0x801457a8u)
#define COMMU00_ACTIVE_TEMPLATE_BASE ((u32)0x801457e8u)
#define COMMU00_FAIRY_PROGRESS       PSX_PTR(volatile u8, 0x801448ecu)
#define COMMU00_FAIRY_SLOT_INDEX     PSX_PTR(volatile u8, 0x801448edu)
#define COMMU00_BATTLE_COUNT         PSX_PTR(volatile u32, 0x8014502cu)
#define COMMU00_STATE                PSX_PTR(volatile u8, 0x80143bb0u)
#define COMMU00_ITEM_NAME            PSX_PTR(volatile u8, 0x801490d8u)
#define COMMU00_ACTIVE_UI            PSX_REF(u8*, 0x801f2948u)
#define COMMU00_REMOVAL_QUEUE        PSX_PTR(volatile u8, 0x80145e30u)
#define COMMU00_PENDING_QUEUE        PSX_PTR(volatile u8, 0x80145e48u)
/* @source 0x801F2458 @kind rodata: type-45 notification kinds, one byte per
 * pending-queue entry, scanned by func_801EEDF8.
 */
extern const u8   COMMU00_TYPE45_NOTIFICATION_TABLE[];
#define COMMU00_TASK_SLOT_BASE      ((u32)0x80146888u)
#define COMMU00_TASK_TEMPLATE_TABLE PSX_PTR(const volatile u8, 0x801f2568u)
#define COMMU00_SLOT_TEMPLATE_TABLE PSX_PTR(const volatile u8, 0x801f2700u)
#define COMMU00_TYPE8_LABEL_TABLE   PSX_PTR(const volatile u16, 0x801f2930u)
#define COMMU00_SLOT_PALETTE_TABLE  PSX_PTR(const volatile u16, 0x801f24fcu)
#define COMMU00_RECORD_VARIANTS     PSX_PTR(const volatile u8, 0x801f25a4u)
#define COMMU00_VARIANT_ROTATION    PSX_PTR(volatile u8, 0x801f2928u)
#define COMMU00_SCRATCH_SLOT PSX_REF(volatile Commu00TaskSlot*, 0x1f800044u)

static inline volatile Commu00TaskSlot* taskSlot(u8 task_index) {
  return PSX_PTR(volatile Commu00TaskSlot,
                 COMMU00_TASK_SLOT_BASE + ((u32)task_index * 0x98u));
}

static inline const volatile Commu00ActiveRecord* activeRecord(
    u8 source_index) {
  return PSX_PTR(const volatile Commu00ActiveRecord,
                 COMMU00_ACTIVE_RECORD_BASE + ((u32)source_index * 8u));
}

static inline volatile Commu00ActiveRecord* mutableActiveRecord(
    u8 source_index) {
  return PSX_PTR(volatile Commu00ActiveRecord,
                 COMMU00_ACTIVE_RECORD_BASE + ((u32)source_index * 8u));
}

static inline volatile u8* activeTemplate(u8 source_index) {
  return PSX_PTR(volatile u8,
                 COMMU00_ACTIVE_TEMPLATE_BASE + ((u32)source_index * 5u));
}

static inline volatile u8* notificationQueueSlot(u8 queue_index) {
  return PSX_PTR(volatile u8, 0x80145e60u + ((u32)queue_index * 2u));
}

#endif
