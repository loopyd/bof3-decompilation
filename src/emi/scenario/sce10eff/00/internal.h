#ifndef EMI_SCE10EFF_00_INTERNAL_H
#define EMI_SCE10EFF_00_INTERNAL_H

#include "bof3/context.h"
#include "memory/scratchpad.h"

typedef struct ScenarioSce10effScratch {
  u8  pad_00[0x08];
  u8  flags_08;
  u8  pad_09[0x2b];
  s32 unk_34;
  s32 unk_38;
  u8  pad_3c[0x02];
  s16 unk_3e;
} ScenarioSce10effScratch;

void func_80178B78(void);
void func_80179558(s16* rotation, void* matrix, void* vector);
void func_80179738(s16* translation, void* object_work);
void func_80178CB8(void* scene, void* object_work);
void func_80178FD8(void* object_work);
void func_80179068(void* object_work);

#endif
