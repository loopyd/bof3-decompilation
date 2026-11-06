#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* @source 0x801F440C
 * @behavior Arms the scratch work record published through the pointer cell at
 * 0x1F800044: clears flag bit 0x40 of record byte 0, sets flag bit 0x20, writes
 * 1 into the 0x5C marker byte, and then writes -0x80 into each of the signed
 * level bytes 0x5D, 0x5E and 0x5F in that order, each through a fresh load of
 * the cell. Takes one more load of the cell as a word view of the same record
 * to select handler byte 1 and to zero words 3, 4 and 5 and set word 8 to -8,
 * then consumes two units of the halfword at 0x0A of the record published
 * through the global pointer cell 0x80148210. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armScratchWorkMin80LevelTripleWorld04Area17213_801F440C(void) {
  u32* words;

  D_1F800044[0] &= 0xbf;
  D_1F800044[0] |= 0x20;
  D_1F800044[0x5c] = 1;
  *(s8*)(D_1F800044 + 0x5d) = -0x80;
  *(s8*)(D_1F800044 + 0x5e) = -0x80;
  *(s8*)(D_1F800044 + 0x5f) = -0x80;
  words = (u32*)D_1F800044;
  ((u8*)words)[4] = 1;
  words[3] = 0;
  words[4] = 0;
  words[5] = 0;
  words[8] = -8;
  D_80148210[5] -= 2;
}
