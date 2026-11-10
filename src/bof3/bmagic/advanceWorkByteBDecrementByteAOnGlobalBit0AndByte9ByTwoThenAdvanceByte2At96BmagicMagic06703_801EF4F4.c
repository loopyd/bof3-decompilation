#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* @source 0x801EF4F4
 * @behavior Adds one plus work byte 9 shifted right by six to work byte B.
 *           If bit 0 of the word at 0x80143E6C is set, decrements work byte A.
 *           Then subtracts two from work byte 9 and, if the stored byte is
 *           0x60, increments work byte 2. The work pointer cell at 0x1F800044
 *           is reloaded after each preceding byte-store group, preserving
 *           possible aliasing. Takes no arguments, returns nothing and has
 *           no stack frame. Published in the dispatch table at 0x801F04D0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByteBDecrementByteAOnGlobalBit0AndByte9ByTwoThenAdvanceByte2At96BmagicMagic06703_801EF4F4(void) {
  u8 *work;
  u32 next;

  work = g_battle_work;
  next = work[0xB] + 1;
  work[0xB] = next + (work[9] >> 6);
  if (D_80143E6C & 1) {
    g_battle_work[0xA]--;
  }
  if ((g_battle_work[9] -= 2) == 0x60) {
    g_battle_work[2]++;
  }
}
