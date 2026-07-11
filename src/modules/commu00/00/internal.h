#ifndef BOF3_SRC_MODULES_COMMU00_00_INTERNAL_H
#define BOF3_SRC_MODULES_COMMU00_00_INTERNAL_H

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

#define COMMU00_ACTIVE_RECORD_BASE   ((u32)0x801455c8u)
#define COMMU00_RECORD_KIND_TABLE    CVPTR(u8, 0x801457a8u)
#define COMMU00_ACTIVE_TEMPLATE_BASE ((u32)0x801457e8u)
extern vu16 COMMU00_LAST_NOTIFICATION_ROW;
extern s8   COMMU00_VISIBLE_SLOT_COUNT;
extern vu32 COMMU00_LAST_WINDOW_STEP_TICK;
extern vu32 COMMU00_WINDOW_ANCHOR_TICK;
#define COMMU00_REMOVAL_QUEUE VPTR(u8, 0x80145e30u)
extern vu8 COMMU00_REMOVAL_QUEUE_COUNT;
#define COMMU00_PENDING_QUEUE VPTR(u8, 0x80145e48u)
extern vu8 COMMU00_PENDING_QUEUE_COUNT;
extern vu8 COMMU00_NOTIFICATION_QUEUE_COUNT;
#define COMMU00_TYPE45_NOTIFICATION_TABLE CVPTR(u8, 0x801f2458u)
#define COMMU00_TASK_SLOT_BASE            ((u32)0x80146888u)
#define COMMU00_TASK_TEMPLATE_TABLE       CVPTR(u8, 0x801f2568u)
#define COMMU00_SLOT_TEMPLATE_TABLE       CVPTR(u8, 0x801f2700u)
#define COMMU00_TYPE8_LABEL_TABLE         CVPTR(u16, 0x801f2930u)
#define COMMU00_SLOT_PALETTE_TABLE        CVPTR(u16, 0x801f24fcu)
#define COMMU00_RECORD_VARIANTS           CVPTR(u8, 0x801f25a4u)
#define COMMU00_VARIANT_ROTATION          VPTR(u8, 0x801f2928u)
extern vu32 COMMU00_PROGRESS_COUNTER;
extern vu16 COMMU00_WORLD_STATE;
extern vu8  COMMU00_TYPE10_TOTAL;
extern vu8  COMMU00_TYPE11_TOTAL;
#define COMMU00_SCRATCH_SLOT VPPTR(Commu00TaskSlot, 0x1f800044u)

static inline volatile Commu00TaskSlot* commu00_task_slot(u8 task_index) {
  return VPTR(Commu00TaskSlot,
              COMMU00_TASK_SLOT_BASE + ((u32)task_index * 0x98u));
}

static inline const volatile Commu00ActiveRecord* commu00_active_record(
    u8 source_index) {
  return CVPTR(Commu00ActiveRecord,
               COMMU00_ACTIVE_RECORD_BASE + ((u32)source_index * 8u));
}

static inline volatile Commu00ActiveRecord* commu00_mutable_active_record(
    u8 source_index) {
  return VPTR(Commu00ActiveRecord,
              COMMU00_ACTIVE_RECORD_BASE + ((u32)source_index * 8u));
}

static inline volatile u8* commu00_active_template(u8 source_index) {
  return VPTR(u8, COMMU00_ACTIVE_TEMPLATE_BASE + ((u32)source_index * 5u));
}

static inline volatile u8* commu00_notification_queue_slot(u8 queue_index) {
  return VPTR(u8, 0x80145e60u + ((u32)queue_index * 2u));
}

u16  game_random_u16(void);
u16  commu00_pack_slot_anchor(s32 x, s32 y);
void commu00_apply_slot_palette(u16 palette_id);
void commu00_prime_slot_resource(u8 resource_id);
s32  commu00_check_selector_flag(const void* table_base, s32 selector_id);

void func_801eedf8(void);
void func_801eeef0(u32 row_index);
void func_801f00d4(void);
void func_801f01f4(void);
u8   func_801f02e4(void);
void func_801f0320(void);
void func_801f0c6c(u8 task_index, u8 record_kind_index);
void func_801f0d3c(u8 task_index, u8 record_kind_index);
void func_801f0e1c(u8 task_index, u8 record_kind_index);
void func_801f0ec8(u8 task_index);
void func_801f0f08(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801f0fbc(u8 source_index, u8 task_index, u8 record_kind_index);
void func_801f1064(u8 task_index, u8 record_kind_index);
void func_801f1110(u8 task_index, u8 record_kind_index);
void func_801f1204(u8 task_index, u8 record_kind_index);
void func_801f1254(u8 task_index);

void func_801f0534(void);
void func_801f0718(u8 source_index, u8 task_index);
void func_801f08d8(u8 source_index, u8 task_index);
void func_801f0bf4(u8 task_index);

#endif
