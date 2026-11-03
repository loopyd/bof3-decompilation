#include "bof3/world/area03004_internal.h"

/* @behavior seeds the AREA030 work record for the next panel step from the five
 * shared parameter bytes at 0x80143FEC..0x80143FF0, stores 5 in byte 0x29, clears
 * byte 0x48, the word at 0x3C, byte 0x5C and the three bytes 0x5D..0x5F with 0x9C,
 * clears the four words 0x60..0x6C, runs the target-local func_801D2AE0 and finally
 * advances the dispatch index.
 * @source 0x801D29BC
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D29BC(void) {
  u8* step;
  u8* aux;
  u32* words;
  u8* next;

  D_1F800044[0x24u] = D_80143FEC[0u];
  D_1F800044[0x25u] = D_80143FEC[1u];
  D_1F800044[0x26u] = D_80143FEC[2u];
  D_1F800044[0x27u] = D_80143FEC[3u];
  D_1F800044[0x28u] = D_80143FEC[4u];
  D_1F800044[0x29u] = 5u;

  step = (u8*)D_1F800044;
  step[0x48u] = 0u;
  aux = (u8*)D_1F800044;
  *(volatile u32*)(step + 0x3Cu) = 0u;
  aux[0x5Cu] = 0u;
  ((s8*)D_1F800044)[0x5Du] = -100;
  ((s8*)D_1F800044)[0x5Eu] = -100;
  ((s8*)D_1F800044)[0x5Fu] = -100;

  words = (u32*)D_1F800044;
  words[24] = 0u;
  words[25] = 0u;
  words[26] = 0u;
  words[27] = 0u;

  func_801D2AE0();

  next = (u8*)D_1F800044;
  next[1u] = (u8)(next[1u] + 1u);
}
