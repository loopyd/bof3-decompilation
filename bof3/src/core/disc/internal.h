#ifndef BOF3_SRC_CORE_DISC_INTERNAL_H
#define BOF3_SRC_CORE_DISC_INTERNAL_H

#include <stddef.h>

#include "bof3/core/disc/slot_table.h"

typedef enum RuntimePathKind {
  BOF3_RUNTIME_PATH_EMI = 0,
  BOF3_RUNTIME_PATH_STR = 1,
  BOF3_RUNTIME_PATH_PSX_EXE = 2,
  BOF3_RUNTIME_PATH_OTHER = 3,
} RuntimePathKind;

struct SlotTableEntry {
  u32             slot_id;
  u32             disc_lba;
  const char*     relative_path;
  RuntimePathKind kind;
};

extern const struct SlotTableEntry g_slot_table[];
extern const size_t                g_slot_table_count;

#endif
