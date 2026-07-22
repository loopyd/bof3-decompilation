#include "internal.h"

void func_8019973C(void) {
  PanelTask* task = g_PanelTaskRoot;
  s16        val;

  val = task->x - 0x20;
  task->x = val;
  if (val < 0x98) {
    task->x = 0x98;
    task->state = 0;
  }
}
