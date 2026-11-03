#include "bof3/world/area03004_internal.h"

/* @behavior slides the AREA030 panel timer at work halfword 0x2E down by 30 and,
 * once the signed remainder falls below 161, arms countdown byte 9 with 30, resets
 * that halfword to 160 and advances the dispatch index; the shared func_8014D4E0
 * helper runs last and unconditionally.
 * @source 0x801D34A0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D34A0(void) {
  u8* cursor;
  u8* next;
  s32 timer;

  cursor = (u8*)D_1F800044;
  timer = *(volatile u16*)(cursor + 0x2Eu) - 30;
  *(volatile s16*)(cursor + 0x2Eu) = (s16)timer;
  if ((s16)timer < 161) {
    cursor[9u] = 30u;
    next = (u8*)D_1F800044;
    *(volatile s16*)(cursor + 0x2Eu) = 160;
    next[1u] = (u8)(next[1u] + 1u);
  }
  func_8014D4E0();
}
