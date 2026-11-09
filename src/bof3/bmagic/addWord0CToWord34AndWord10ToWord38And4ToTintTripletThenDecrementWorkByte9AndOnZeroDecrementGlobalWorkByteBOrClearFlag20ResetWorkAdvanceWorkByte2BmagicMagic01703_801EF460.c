#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image; Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * this lift needs no target-map row. The exact siblings of this overlay read
 * the same record: addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrement
 * GlobalWorkByteBAndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4 decrements
 * its byte 0xB, and invokeHelperThenDecrementWorkByte9ThenInvokeBattleHelpers
 * AndAdvanceWorkByte2OnZeroWhenGlobalWorkByteBIs1BmagicMagic01703_801EF5FC
 * reads that byte the same way. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named fixed address, the
 * form the exact siblings of this overlay use, so the assembler folds the
 * constant into each access (lui $vN,0x1F80 / lw $vN,68($vN)); the original
 * re-materialises the cell before most access groups because a store through
 * the pointer it supplied may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Acting battler byte of the main executable. Party slots are 0, 1 and 2 and
 * enemy panel slots are 3 and above, so the unsigned compare against 3 below
 * selects an enemy battler, which becomes the enemy work index this body hands
 * to func_801E257C. The exact sibling
 * decrementWorkByte9ThenInvokeActingEnemyScriptSetFlag20Mode1SeedByte2BAnd
 * WorkColorTripletToC0SetWorkByte9To8AndAdvanceWorkByte2Bmagic
 * Magic01703_801EF6EC reads the same byte with the same split. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Shared battle helper of the main executable, outside this overlay's image: it
 * publishes the 0x118-byte enemy work record at 0x801EB2E8 indexed by its first
 * argument as the current enemy work and runs the enemy script byte helper
 * func_801E2314 for the second argument. Splat binds the address through this
 * target's generated undefined_funcs_auto.txt, so no target-map row is needed. */
extern void func_801E257C(u8 index, u8 arg1);

/* Shared helper of the main executable, outside this overlay's image; the
 * bmagic call sites invoke it with no argument set and discard $v0, so it is
 * declared void with no arguments. */
extern void func_801E5988(void);

/* Shared helper of the main executable, outside this overlay's image; this call
 * site stores the result straight into the work object's countdown byte, so
 * only the byte width of the result matters here. The lift
 * src/bof3/battle/func_800AFF98_battle15.c defines the same unsigned byte
 * return. */
extern u8 func_800AFF98(void);

/* @source 0x801EF460
 * @behavior Step handler of this overlay's work-byte-2 handler run: the word at
 *           0x801EEC1C holds this function's address, index 3 of the ten words
 *           the run dispatcher func_801EF058 copies from 0x801EEC10 into its
 *           frame and indexes with scratchpad work-object byte 2, so the body
 *           runs while work byte 2 is 3. That dispatcher is the single entry of
 *           the run selected by
 *           dispatchWorkByte1HandlerBmagicMagic01703_801EF014, and no jal in
 *           this payload names this function, so the table word is its only
 *           incoming path. It first reads the scratchpad pointer cell at
 *           0x1F800044 once into a word-typed view of the work object and
 *           accumulates that object's two position words through it: word 0x34
 *           gains the step word 0xC and word 0x38 gains the step word 0x10,
 *           each read and written in place with lw / addu / sw. Between them it
 *           adds 4 to the object's three colour channels at bytes 0x5D, 0x5E
 *           and 0x5F with a byte-width read-modify-write (lbu / addiu 4 / sb);
 *           the first channel is reached through the byte view derived from
 *           the same word-typed pointer, so that one cell read serves the whole
 *           head group, and the last two channels re-read the pointer cell.
 *           Splitting the record view by access width is what the original
 *           codegen requires here: the word pairs keep their own word-typed
 *           pointer while each byte channel is derived from it. Then it
 *           decrements the object's unsigned countdown byte 9 in place (lbu /
 *           addiu -1 / sb) and tests the stored byte truncated to eight bits
 *           (andi $v0,$v0,0xFF / bnez), so the rest of the body runs only when
 *           that countdown reaches zero. On zero it tests the object's byte 0xB:
 *           when that byte is non-zero the handler leaves the object untouched
 *           and instead reads the shared battle work record pointer at
 *           0x801EB4E0, decrements that record's byte 0xB and invokes the shared
 *           helper func_801E5988, the store filling the call delay slot. When
 *           byte 0xB is zero it clears bit 0x20 of the object's byte 0 through
 *           the 0xDF mask, zeroes mode byte 0x5C, work byte 0x2B and the three
 *           colour channels at 0x5D, 0x5E and 0x5F, re-reading the pointer cell
 *           for each store, and then, only when the acting battler byte
 *           0x80146374 is 3 or above (an enemy panel slot, sltiu / bnez),
 *           invokes func_801E257C with that byte zero-extended as the enemy work
 *           index and 2 as the script byte selector. Finally it re-reads the
 *           pointer cell and stores the byte result of the shared helper
 *           func_800AFF98 into countdown byte 9, re-reads the cell once more and
 *           increments work byte 2 with a byte-width add, moving the run to
 *           index 4 at 0x801EF5FC. The 0x18-byte frame only keeps $ra at 0x10
 *           across the two calls; the function takes no arguments, returns
 *           nothing, and reads or writes no other state. Statement order and
 *           cell re-materialisation grouping follow the exact siblings of the
 *           same run in this target:
 *           addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrementGlobal
 *           WorkByteBAndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4 for the
 *           colour group, the in-place byte-9 countdown and the final byte-2
 *           increment, and
 *           decrementWorkByte9ThenInvokeActingEnemyScriptSetFlag20Mode1SeedByte2
 *           BAndWorkColorTripletToC0SetWorkByte9To8AndAdvanceWorkByte2Bmagic
 *           Magic01703_801EF6EC for the flag-0x20 clear and the enemy-slot
 *           selector. Roles of work byte 0xB, work byte 0x2B, mode byte 0x5C and
 *           of the shared helpers are not established here, so they stay
 *           address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWord0CToWord34AndWord10ToWord38And4ToTintTripletThenDecrementWorkByte9AndOnZeroDecrementGlobalWorkByteBOrClearFlag20ResetWorkAdvanceWorkByte2BmagicMagic01703_801EF460(void) {
  s32 *words;

  words = (s32 *)D_1F800044;
  words[0xD] += words[3];
  ((u8 *)words)[0x5D] += 4;
  words[0xE] += words[4];
  D_1F800044[0x5E] += 4;
  D_1F800044[0x5F] += 4;

  if (--D_1F800044[9] == 0) {
    if (D_1F800044[0xB] == 0) {
      D_1F800044[0] &= 0xDF;
      D_1F800044[0x5C] = 0;
      D_1F800044[0x2B] = 0;
      D_1F800044[0x5D] = 0;
      D_1F800044[0x5E] = 0;
      D_1F800044[0x5F] = 0;
      if (D_80146374 >= 3) {
        func_801E257C(D_80146374, 2);
      }
      D_1F800044[9] = func_800AFF98();
      D_1F800044[2]++;
    } else {
      D_801EB4E0[0xB]--;
      func_801E5988();
    }
  }
}
