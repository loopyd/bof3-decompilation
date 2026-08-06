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

extern volatile u16 COMMU00_LAST_NOTIFICATION_ROW;
extern s8           COMMU00_VISIBLE_SLOT_COUNT;
extern volatile u32 COMMU00_LAST_WINDOW_STEP_TICK;
extern volatile u32 COMMU00_WINDOW_ANCHOR_TICK;
extern volatile u8  COMMU00_REMOVAL_QUEUE_COUNT;
extern volatile u8  COMMU00_PENDING_QUEUE_COUNT;
extern volatile u8  COMMU00_NOTIFICATION_QUEUE_COUNT;
extern volatile u32 COMMU00_PROGRESS_COUNTER;
extern volatile u16 COMMU00_WORLD_STATE;
extern volatile u8  COMMU00_TYPE10_TOTAL;
extern volatile u8  COMMU00_TYPE11_TOTAL;
/* @kind: bss — per-task label/status words: label_id field view of the
 * Commu00TaskSlot array (base 0x80146888 + 0x7c), written with 0xC003,
 * 0xC00A, and variant-derived label words. */
extern u16          commu00_task_label_words[1];
/* @kind: bss — commu00 UI mode byte; requested values 0, 14, and 23. */
extern u8           commu00_ui_mode;
/* @kind: bss — fairy progress byte; cleared on selection reset, advanced by
 * message selection, and indexes the progress handler tables. */
extern u8           commu00_fairy_progress[1];
/* @kind: bss — fairy slot index byte; indexes the slot handler table. */
extern u8           commu00_fairy_slot_index;
/* @kind: rodata — variant rotation bytes backing COMMU00_VARIANT_ROTATION;
 * variant-derived labels are entries minus 0x3FFB. */
extern volatile u8  commu00_variant_rotation[1];

/* Local commu00 dispatch tables (data blob T_801F24FC). */
/* @kind: table — six function pointers indexed by the fairy progress byte
 * (plus a second group dispatched at offset +6). */
extern void (*commu00_progress_handlerTable[6])(void);
/* @kind: table — function pointers indexed by the fairy progress byte. */
extern void (*commu00_progress_handlerTable2[])(void);
/* @kind: table — function pointers indexed by the fairy progress byte. */
extern void (*commu00_progress_handlerTable3[])(void);
/* @kind: table — function pointers indexed by the fairy slot index byte. */
extern void (*commu00_slot_handlerTable[])(void);

u16  commu00_pack_slot_anchor(s32 x, s32 y);
void commu00_apply_slot_palette(u16 palette_id);
void commu00_prime_slot_resource(u8 resource_id);
s32  commu00_check_selector_flag(const void* table_base, s32 selector_id);

void func_8015C058(void);
void func_8015C088(void);

void func_801EEDF8(void);
void func_801EEEF0(u32 row_index);
void func_801F00D4(void);
void func_801F01F4(void);
u8   commu00_count_active_records(void);
void func_801F0320(void);
void func_801F0C6C(u8 task_index, u8 record_kind_index);
void func_801F0D3C(u8 task_index, u8 record_kind_index);
void func_801F0E1C(u8 task_index, u8 record_kind_index);
void commu00_activate_task_status_c003(u8 task_index);
void func_801F0F08(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801F0FBC(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801F1064(u8 task_index, u8 record_kind_index);
void func_801F1110(u8 task_index, u8 record_kind_index);
void commu00_set_variant_task_status(u8 task_index, u8 record_kind_index);
void commu00_activate_task_status_c00a(u8 task_index);

void func_801F0534(void);
void func_801F0718(u8 source_index, u8 task_index);
void func_801F08D8(u8 source_index, u8 task_index);
void func_801F0BF4(u8 task_index);

#define COMMU00_TASK_SLOTS PSX_PTR(volatile Commu00TaskSlot, 0x80146888u)

#define COMMU00_ACTIVE_RECORD_BASE   ((u32)0x801455c8u)
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

static inline volatile Commu00TaskSlot* commu00_task_slot(u8 task_index) {
  return PSX_PTR(volatile Commu00TaskSlot,
                 COMMU00_TASK_SLOT_BASE + ((u32)task_index * 0x98u));
}

static inline const volatile Commu00ActiveRecord* commu00_active_record(
    u8 source_index) {
  return PSX_PTR(const volatile Commu00ActiveRecord,
                 COMMU00_ACTIVE_RECORD_BASE + ((u32)source_index * 8u));
}

static inline volatile Commu00ActiveRecord* commu00_mutable_active_record(
    u8 source_index) {
  return PSX_PTR(volatile Commu00ActiveRecord,
                 COMMU00_ACTIVE_RECORD_BASE + ((u32)source_index * 8u));
}

static inline volatile u8* commu00_active_template(u8 source_index) {
  return PSX_PTR(volatile u8,
                 COMMU00_ACTIVE_TEMPLATE_BASE + ((u32)source_index * 5u));
}

static inline volatile u8* commu00_notification_queue_slot(u8 queue_index) {
  return PSX_PTR(volatile u8, 0x80145e60u + ((u32)queue_index * 2u));
}

#endif
