#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The original re-materializes the cell address (lui $vN,0x1F80 /
 * lw $vN,68($vN)) once per access group and once for each of the five byte
 * clears in the reset arm, so the cell is declared volatile like the exact
 * sibling addWorkMotionPairAndIncrementBytes9AThenAdvanceWorkByte2At10
 * BmagicMagic04303_801EF55C. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * keeps its row in the generated undefined-symbol map because the overlay's
 * remaining assembly references the same address. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF64C
 * @behavior Per-frame motion step of the overlay handler that is entry 4 of the
 *           five-word run at 0x801EFC88 (0x801EF3A4, 0x801EF42C, 0x801EF55C,
 *           0x801EF5E0, 0x801EF64C), which 0x801EF360 dispatches on work-object
 *           byte 1, and entry 3 of the four-word run at 0x801EFC8C that
 *           0x801EF3A4 dispatches on work-object byte 2. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once, adds work
 *           word 0xC into word 0x34 and work word 0x10 into word 0x38 with two
 *           addu/sw pairs in the original order, and increments work byte 9 by
 *           one with a byte-width add (lbu/addiu 1/sb) in between. It then
 *           re-reads the cell and subtracts two from work byte 0xA
 *           (lbu/addiu -2/sb), re-reads the cell again and returns while that
 *           byte is nonzero. When byte 0xA reached zero it decrements byte 0xB
 *           of the shared battle work record read through 0x801EB4E0 and clears
 *           work bytes 0 through 4 with sb $zero, re-materializing the pointer
 *           cell before each clear because a store through the loaded pointer
 *           may alias it. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkMotionPairIncrementByte9DecrementByteABy2ThenClearWorkBytes0To4AndDecrementGlobalWorkByteBBmagicMagic04303_801EF64C(void) {
  u32 *words;
  u8 *step;

  words = (u32 *)g_battle_work;
  words[13] += words[3];
  ((u8 *)words)[9]++;
  words[14] += words[4];
  step = g_battle_work;
  step[0xA] -= 2;
  if (g_battle_work[0xA] == 0) {
    D_801EB4E0[0xB]--;
    g_battle_work[0] = 0;
    g_battle_work[1] = 0;
    g_battle_work[2] = 0;
    g_battle_work[3] = 0;
    g_battle_work[4] = 0;
  }
}
