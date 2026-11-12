#include "bof3/world/area03004_internal.h"

/* @behavior decrements the AREA030 work-record countdown byte at scratch offset 9
 * and dispatches on the remaining count: a zero count runs the shared func_80196070
 * helper, otherwise the three-byte row of 0x801E1F14 selected by the span byte
 * of the current 12-byte entry is submitted through drawMessageTokenCellQuad
 * with a width derived from the remaining count, bit 7 set, and the selector
 * 29.
 * @source 0x801D41E4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D41E4(void) {
  u8* first;
  u8* next;
  s32 count;
  u8 span;
  s32 offset;

  first = (u8*)D_1F800044;
  first[9u] = (u8)(first[9u] - 1u);

  next = (u8*)D_1F800044;
  count = next[9u];
  if (count != 0) {
    span = D_801E1E78[next[0x4Bu]].byte_08;
    offset = span * 3;
    drawMessageTokenCellQuad(D_801E1F14[offset + 2], &D_801E1F14[offset],
                  (u8)(((8 - ((4 - count) << 1)) & 0xFE) | 0x80), 29);
  } else {
    func_80196070();
  }
}
