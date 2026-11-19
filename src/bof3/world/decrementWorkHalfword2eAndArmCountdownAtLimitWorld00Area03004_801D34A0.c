#include "bof3/world/area03004_internal.h"

/* @behavior Decrements work halfword +0x2E by 30. When the narrowed signed
 * result is below 161, seeds countdown byte 9 with 30, resets the
 * halfword to 160 and advances handler byte 1. Always calls the
 * bounded work-pointer queue appender.
 * @source 0x801D34A0
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkHalfword2eAndArmCountdownAtLimitWorld00Area03004(void) {
  u8* cursor;
  u8* handlerWork;
  s32 halfwordValue;

  cursor = (u8*)D_1F800044;
  halfwordValue = *(volatile u16*)(cursor + 0x2Eu) - 30;
  *(volatile s16*)(cursor + 0x2Eu) = (s16)halfwordValue;
  if ((s16)halfwordValue < 161) {
    cursor[9u] = 30u;
    handlerWork = (u8*)D_1F800044;
    *(volatile s16*)(cursor + 0x2Eu) = 160;
    handlerWork[1u] = (u8)(handlerWork[1u] + 1u);
  }
  func_8014D4E0();
}
