#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F7484
 * @behavior Seeds the shared scratchpad work object for the scene fade: stores
 * the fixed position pair 0xB0000/0xF8000 in the words at offsets 0x34 and
 * 0x38, stores the func_8015477C distance of that pair in the halfword at
 * offset 0x3E, sets the primitive index byte at offset 0x29 to 5 and advances
 * the dispatch byte at offset 0x01 to 1. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectPositionAndAdvance(void) {
  *(s32 *)(D_1F800044 + 0x34) = 0xB0000;
  *(s32 *)(D_1F800044 + 0x38) = 0xF8000;
  *(u16 *)(D_1F800044 + 0x3E) =
      (u16)func_8015477C(*(s32 *)(D_1F800044 + 0x34), 0xF8000);
  *(u8 *)(D_1F800044 + 0x29) = 5;
  *(u8 *)(D_1F800044 + 0x01) = 1;
}
