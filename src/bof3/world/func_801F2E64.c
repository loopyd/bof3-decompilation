#include "bof3/world/area03213_internal.h"

/* @source 0x801F2E64
 * @behavior Slot 3 of the local handler table (T_801F3F6C, pointer word at
 * 0x801F3F78): emits the semi-transparent quad ring for the scratch cursor
 * record at 0x1F800044, then decrements its work byte at offset 9 and, once
 * that byte turns negative, advances its state byte at offset 1.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 32/32 instructions, 128 -> 128 bytes, exact on the first shape.
 */
void func_801F2E64(void) {
  func_801F2F04(*(s16*)(D_1F800044 + 0x2e), *(s16*)(D_1F800044 + 0x30),
                D_1F800044[9], 0x80, 0);
  D_1F800044[9]--;
  if ((s8)D_1F800044[9] < 0) {
    D_1F800044[1]++;
  }
}
