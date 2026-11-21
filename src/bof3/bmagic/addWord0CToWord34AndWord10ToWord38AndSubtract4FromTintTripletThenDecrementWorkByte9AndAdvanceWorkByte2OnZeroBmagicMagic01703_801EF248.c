#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named fixed address, the
 * form the exact siblings of this overlay use, so the assembler folds the
 * constant into each access (lui $vN,0x1F80 / lw $vN,68($vN)); the original
 * re-materialises the cell before most access groups because a store through
 * the pointer it supplied may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF248
 * @behavior Work byte 2 state 1 of this overlay, index 1 of the ten-word
 *           handler table at 0x801EEC10 (0x801EF11C, 0x801EF248, 0x801EF300,
 *           0x801EF460, 0x801EF5FC, 0x801EF694, 0x801EF6EC, 0x801EF7E0,
 *           0x801EF8B4, 0x801EF970) whose word at 0x801EEC14 is the only
 *           occurrence of this address in the payload; the run dispatcher
 *           func_801EF058 copies that table into its frame and indexes it with
 *           scratchpad work-object byte 2. It adds work word 0xC into work word
 *           0x34 and work word 0x10 into work word 0x38, each read and written
 *           as a word at the work object; subtracts 4 from each of the three
 *           tint channels at bytes 0x5D, 0x5E and 0x5F with a byte-width
 *           read-modify-write (lbu / addiu -4 / sb), re-reading the pointer cell
 *           once per channel because a store through the pointer it supplied may
 *           alias the cell; then decrements work countdown byte 9 in place
 *           (lbu / addiu -1 / sb) and tests the stored byte truncated to eight
 *           bits (andi $v0,$v0,0xFF / bnez), so the last group runs only when
 *           that countdown reaches zero. On zero it re-reads the pointer cell
 *           and increments work byte 2 with a byte-width add, advancing the run
 *           to index 2 at 0x801EF300. Leaf with no frame and no calls; it takes
 *           no arguments, returns nothing, and reads or writes no other state.
 *           Statement order and cell re-materialisation grouping follow the
 *           exact sibling of the same run in this target,
 *           addWord0CToWord34AndWord10ToWord38And4ToTintTripletThenDecrement
 *           WorkByte9AndOnZeroDecrementGlobalWorkByteBOrClearFlag20ResetWork
 *           AdvanceWorkByte2BmagicMagic01703_801EF460, whose first 0x2C bytes are
 *           byte-identical to this body except for the tint delta sign (addiu
 *           a0,a0,4 there, addiu a0,a0,-4 here) and the frame reservation that
 *           sibling adds for its calls; the countdown group and the final
 *           byte-2 increment follow
 *           addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrementGlobal
 *           WorkByteBAndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4. The
 *           tint delta -4 is this function's own literal. Roles of work words
 *           0xC/0x10/0x34/0x38 and of the countdown stay structural here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWord0CToWord34AndWord10ToWord38AndSubtract4FromTintTripletThenDecrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF248(void) {
  s32 *words;

  words = (s32 *)D_1F800044;
  words[0xD] += words[3];
  ((u8 *)words)[0x5D] -= 4;
  words[0xE] += words[4];
  D_1F800044[0x5E] -= 4;
  D_1F800044[0x5F] -= 4;

  if (--D_1F800044[9] == 0) {
    D_1F800044[2]++;
  }
}
