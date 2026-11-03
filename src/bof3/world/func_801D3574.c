#include "bof3/world/area03004_internal.h"

/* @behavior decrements the scratch halfword at +0x2E by 30; when the signed result
 * falls below -69 it resets the halfword to 160 and clears the dispatch index,
 * otherwise it runs the shared ready helper.
 * @source 0x801D3574
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3574(void) {
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
