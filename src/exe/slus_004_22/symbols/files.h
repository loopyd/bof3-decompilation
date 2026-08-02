#ifndef SLUS_004_22_SYMBOLS_FILES_H
#define SLUS_004_22_SYMBOLS_FILES_H

#include "bof3/bof3.h"

typedef enum RuntimePathKind {
  RUNTIME_PATH_EMI = 0,
  RUNTIME_PATH_STR = 1,
  RUNTIME_PATH_PSX_EXE = 2,
  RUNTIME_PATH_OTHER = 3,
} RuntimePathKind;

struct SlotTableEntry {
  EmiLoaderSlotId slot_id;
  DiscLba         disc_lba;
  const char*     relative_path;
  RuntimePathKind kind;
};

extern const char                  s__LOGO_LOGO_EXE_1_80149800[];

#endif
