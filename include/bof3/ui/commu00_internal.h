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
  u8  unk_49[0x2f];
  u8  variant_state;
  u8  unk_79[3];
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

/* @source 0x80145E5C @kind unknown */
extern volatile u8  COMMU00_PENDING_QUEUE_COUNT;
/* @source 0x80146904 @kind bss: per-task label/status words: label_id field view of the
 * Commu00TaskSlot array (base 0x80146888 + 0x7c), written with 0xC003,
 * 0xC00A, and variant-derived label words.
 */
extern u16          taskLabelWords[1];
/* @source 0x801448EB @kind bss: commu00 UI mode byte; requested values 0, 14, and 23. */
extern u8           uiMode;
/* @source 0x801448EC @kind bss: fairy progress byte; cleared on selection reset, advanced by
 * message selection, and indexes the progress handler tables.
 */
extern u8           fairyProgress[1];
/* @source 0x801448ED @kind bss: fairy slot index byte; indexes the slot handler table. */
extern u8           fairySlotIndex;
/* @source 0x801F2928 @kind rodata: variant rotation bytes backing COMMU00_VARIANT_ROTATION;
 * variant-derived labels are entries minus 0x3FFB.
 */
extern volatile u8  variantRotation[1];
/* @source 0x801F2930 @kind bss: three rows of three u16 task labels selected by battle-age band. */
extern u16          taskLabelBandTable[3][3];
extern u32          D_8014502C;
/* @source 0x801455C8 @kind unknown: typed view of the active-record region. */
extern volatile Commu00ActiveRecord activeRecordBytes[];
extern u8           D_801455C9[];
extern u8           D_801457AB[];
extern u8           D_801CA28C[];

/* Local commu00 dispatch tables (data blob T_801F24FC). */
/* @source 0x801F25EC @kind table: six function pointers indexed by the fairy progress byte
 * (plus a second group dispatched at offset +6).
 */
extern void (*progressHandlerTable[6])(void);
/* @source 0x801F2610 @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable2[])(void);
/* @source 0x801F2678 @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable3[])(void);
/* @source 0x801F269C @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable4[])(void);
/* @source 0x801F26B0 @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable5[])(void);
/* @source 0x801F26EC @kind table: function pointers indexed by the fairy progress byte. */
extern void (*progressHandlerTable6[])(void);
/* @source 0x801F268C @kind table: function pointers indexed by the fairy slot index byte. */
extern void (*slotHandlerTable[])(void);

void func_8015C058(void);
void func_8015C088(void);

void func_801EEDF8(void);
void func_801EEEF0(u32 row_index);
void func_801F00D4(void);
void func_801F01F4(void);
u8   countActiveRecords(void);
void func_801F0320(void);
void func_801F0C6C(u8 task_index, u8 record_kind_index);
void func_801F0D3C(u8 task_index, u8 record_kind_index);
void func_801F0E1C(u8 task_index, u8 record_kind_index);
void activateTaskStatusC003(u8 task_index);
void func_801F0F08(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801F0FBC(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801F1064(u8 task_index, u8 record_kind_index);
void func_801F1110(u8 task_index, u8 record_kind_index);
void setVariantTaskStatus(u8 task_index, u8 record_kind_index);
void activateTaskStatusC00A(u8 task_index);
void func_801F1F9C(void);
void func_801F2020(void);
void func_801F228C(void);

void func_801F0534(void);
void func_801F0718(u8 source_index, u8 task_index);
void func_801F08D8(u8 source_index, u8 task_index);
void func_801F0BF4(u8 task_index);

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
#define COMMU00_TYPE45_NOTIFICATION_TABLE                                      \
  PSX_PTR(const volatile u8, 0x801f2458u)
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
