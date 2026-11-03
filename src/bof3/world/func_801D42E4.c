#include "bof3/world/area03004_internal.h"

/* @behavior resets the AREA030 work record published at the scratchpad cursor
 * 0x1F800044: clears bytes 8/6, seeds byte 7 with 0x80 and countdown byte 9 with
 * four, clears bytes 0x0A/0x0B, clears the step words at 0x34/0x38/0x3C and the
 * origin words at 0x0C/0x10, stores 0xFFFF in the halfword at 0x2C and finally
 * advances the dispatch index.
 * @source 0x801D42E4
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D42E4(void) {
  u32* words;
  u8* tail;
  u8 next;

  D_1F800044[8u] = 0u;
  D_1F800044[6u] = 0u;
  D_1F800044[7u] = 128u;
  D_1F800044[9u] = 4u;
  D_1F800044[10u] = 0u;

  words = (u32*)D_1F800044;
  ((u8*)words)[11u] = 0u;
  words[13] = 0u;
  words[14] = 0u;
  words[15] = 0u;
  words[3] = 0u;
  words[4] = 0u;

  tail = (u8*)D_1F800044;
  next = tail[1u];
  *(volatile u16*)(tail + 0x2Cu) = 0xFFFFu;
  tail[1u] = (u8)(next + 1u);
}
