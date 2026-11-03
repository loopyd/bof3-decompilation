#include "bof3/world/area03004_internal.h"

/* @behavior clears the scratch flags byte at +7, seeds the scratch halfword at
 * +0x30 with -25 and advances the dispatch index.
 * @source 0x801D3650
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D3650(void) {
  u8* cursor;
  u8* next;

  cursor = (u8*)D_1F800044;
  cursor[7u] = 0u;
  next = (u8*)D_1F800044;
  *(s16*)(cursor + 0x30u) = -25;
  next[1u] = (u8)(next[1u] + 1u);
}
