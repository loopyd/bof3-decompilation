#ifndef PANEL_TASK_H
#define PANEL_TASK_H

#include "base/types.h"
#include "memory/access.h"

typedef struct PanelTask {
  u8  unk_00[3];
  u8  state;
  u16 x;
  u16 field_06;
} PanelTask;

#define g_PanelTaskRoot PSX_REF(PanelTask*, 0x80148648u)

#endif
