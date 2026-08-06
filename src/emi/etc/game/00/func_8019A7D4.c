#include "internal.h"

/* @behavior clears work-area status bytes then clears work flags */
/* @source 0x8019A7D4 */
void func_8019A7D4(void) {
  struct GameWorkArea* work = g_game_work;
  work->unk_5F = 0;
  work->unk_5E = 0;
  work->unk_5D = 0;
  clearWorkFlags();
}
