#include "bof3/world/area03004_internal.h"

/* @behavior issues the fixed five-argument panel request, then submits one shaded
 * panel request selected by the scratch byte at +0x4B through the 12-byte entry
 * table 0x801E1E78, with a fixed vertical argument of 20.
 * @source 0x801D3824
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3824(void) {
  u8* cursor;
  u8 index;

  func_801D9534(20, 18, 280, 19, 0);
  cursor = (u8*)D_1F800044;
  index = cursor[0x4Bu];
  func_8014f800(29, 20, 0, D_801E1E78[index].byte_00, D_801E1E78[index].word_04);
}
