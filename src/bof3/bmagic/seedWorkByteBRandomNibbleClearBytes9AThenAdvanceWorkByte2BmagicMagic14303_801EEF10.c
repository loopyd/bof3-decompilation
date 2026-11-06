#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by every exact sibling of the bmagic overlay family. It stays a named cell
 * rather than the constant-address memory accessor because the original
 * re-materialises the base at each access (lui $vN,0x1F80 / lw $vN,68($vN)): the
 * accessor's constant address is folded once by CSE into a full-address register
 * and reused, which loads the cell through that one base instead of reloading it
 * after the preceding store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEF10
 * @behavior Overlay work-step handler of the record published at the scratchpad
 *           pointer cell 0x1F800044. It calls rand() and stores the low four
 *           bits of the result (andi 0xF) into the record's unsigned byte 0xB,
 *           the work byte the overlay's step handlers add to; it then clears the
 *           record's byte 9 and byte 0xA and increments its byte 2 by one, the
 *           step index the overlay dispatcher at 0x801EEDB4 reads. Every group
 *           re-reads the cell, because the preceding store through the loaded
 *           pointer may alias it, so the original keeps four separate lui/lw
 *           pairs and a load-delay nop follows each lw that feeds a store. The
 *           0x18-byte frame exists only to keep $ra across the rand() call; the
 *           trailing byte-2 store is placed after the $ra reload and the return
 *           delay slot holds the frame restore. The function takes no arguments,
 *           returns nothing and reads or writes no other state. The word at
 *           0x801EFEEC (payload offset 0x2EC) holds 0x801EEF10, so it is entry 1
 *           of the five-word in-image handler run at 0x801EFEE8 that the step
 *           dispatcher dispatchStateByte1Tablefee8BmagicMagic14303_801EED70
 *           indexes with work byte 1, and entry 0 of the run that func_801EEDB4
 *           indexes at 0x801EFEEC with work byte 2; the entry is dispatched, so
 *           it has no jal caller. Statement order, operand forms and declaration
 *           style follow the exact siblings
 *           clearWorkByte9SetWorkByteATo88AndIncrementWorkByte2BmagicMagic06703_801EFF8C
 *           (same named cell, byte 9 and byte 0xA writes and byte 2 increment) and
 *           seedWorkTripletTintRandomWordCAndCountdownThenAdvanceWorkByte2BmagicMagic15103_801F0BA8
 *           (rand()-derived value stored through the same cell, `extern int
 *           rand(void)`, 0x18-byte frame with $ra only). This body's own literals
 *           differ from both, and no byte-identical copy of the span exists in
 *           any other bmagic payload: the store offsets 0xB, 9, 0xA and 2 with
 *           the 0xF mask appear only in this target's binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByteBRandomNibbleClearBytes9AThenAdvanceWorkByte2BmagicMagic14303_801EEF10(void) {
  g_battle_work[0xB] = (u8)(rand() & 0xF);
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0;
  g_battle_work[2]++;
}
