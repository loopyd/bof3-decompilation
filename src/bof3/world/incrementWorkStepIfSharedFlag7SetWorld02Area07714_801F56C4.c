#include "bof3/bof3.h"

/* @source 0x80144F28 @kind unknown */
extern s32 D_80144F28;

s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801F56C4
 * @behavior Tests flag 7 of the main-RAM flag bank 0x80144F28 through the
 *           shared flag tester func_8015B5D4; when it reports the flag set,
 *           loads the work record pointer from the scratchpad cell 0x1F800044
 *           and increments work byte +2, the dispatch step byte. Shares the
 *           guard-and-advance shape of the neighbouring panel handlers.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkStepIfSharedFlag7SetWorld02Area07714_801F56C4(void) {
  u8* work;

  if (func_8015B5D4((u32)&D_80144F28, 7) != 0) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[2]++;
  }
}
