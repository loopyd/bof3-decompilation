#include "bof3/world/area03213_internal.h"

/* @source 0x801F2D18
 * @behavior Emits the scratch record quad ring through the local primitive
 * helper, then advances the scratch record work byte at offset 9 by 0x0f and,
 * once that byte reaches 0x3d, parks it at 0xff and advances the state byte at
 * offset 1.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 37/37 instructions, 148 -> 148 bytes, exact on the first shape;
 * the sibling func_801F2E64 supplied the call/prologue idiom.
 */
void func_801F2D18(void) {
  func_801F2F04(*(s16*)(D_1F800044 + 0x2e), *(s16*)(D_1F800044 + 0x30),
                D_1F800044[9], 0x80, 0);
  D_1F800044[9] += 0xf;
  if (D_1F800044[9] >= 0x3d) {
    D_1F800044[9] = 0xff;
    D_1F800044[1]++;
  }
}
