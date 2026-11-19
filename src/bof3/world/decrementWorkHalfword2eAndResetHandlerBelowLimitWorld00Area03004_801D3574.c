#include "bof3/world/area03004_internal.h"

/* @behavior Decrements work halfword +0x2E by 30. When the narrowed signed
 * result is below -69, resets the halfword to 160 and clears handler
 * byte 1; otherwise calls the bounded work-pointer queue appender.
 * @source 0x801D3574
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkHalfword2eAndResetHandlerBelowLimitWorld00Area03004(void) {
  u8* cursor;
  s16* half;

  cursor = (u8*)D_1F800044;
  half = (s16*)(cursor + 0x2Eu);
  *half = (s16)(*half - 30);
  if (*half < -69) {
    *half = 160;
    cursor[1u] = 0u;
  } else {
    func_8014d4e0();
  }
}
