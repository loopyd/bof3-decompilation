#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C,
 * invokeReadinessHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvance
 * WorkByte2OnZeroBmagicMagic00800_801F1384 and
 * setGlobalWorkByteBToFFAtWorkByte2State8ThenAdvanceWorkByte2WhenHelperReady
 * BmagicMagic00800_801F1880 declare. The named symbol lets the assembler fold
 * the constant address into each load (lui $v0,0x1F80 / lw $v0,68($v0)), and
 * the original re-reads the cell after every call and before every store
 * through the loaded pointer. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte in main RAM, outside this overlay's image. The original
 * reads it once for the battler-status helper call and once after it, spanning
 * the call, so the address is materialized once into $s0 (lui plus addiu) and
 * both reads use offset zero; the same value is the record selector this
 * function indexes the two battler record tables with. Bound through this
 * target's generated undefined_syms_auto.txt, so no target-map row is added
 * here. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Acting battler byte in main RAM, outside this overlay's image: party slots
 * are 0, 1 and 2 and enemy panel slots are 3 and above, the split the exact
 * lifts of this target use for the same compare against 3. Read once for that
 * compare and once as the enemy work index func_801E257C takes. */
extern u8 D_80146374; /* @source 0x80146374 @kind data */

/* Party record table in main RAM, outside this overlay's image: the
 * 0x140-byte-stride party rows the battle targets own, read here at the byte
 * the shared battle byte selects. Kept a byte array with the stride written at
 * the index, the spelling the exact sibling lift
 * seedWorkObjectFromBattleRecordThenClaimSlotCopyActingBattlerRecordHead
 * StampSlotBytesPublishWorkPointerRaiseFlag40AndInvokeHelperAt100Bmagic
 * Magic00800_801EED5C used for the same table. Splat binds the address through
 * this target's generated undefined_syms_auto.txt, which the remaining asm
 * rows reference. */
extern u8 D_80145E91[]; /* @source 0x80145E91 @kind table */

/* Enemy record table in main RAM, outside this overlay's image: the
 * 0x118-byte-stride enemy rows, read here at the byte of the row whose index
 * is the shared battle byte minus 3, and declared like the party table above. */
extern u8 D_801EB631[]; /* @source 0x801EB631 @kind table */

/* Shared battle work-record pointer in main RAM, outside this overlay's image:
 * the original loads it and stores 0xFF into its byte 0xB, the same signal the
 * sibling lifts setGlobalWorkByteBToFFAtWorkByte2State8ThenAdvanceWorkByte2
 * WhenHelperReadyBmagicMagic00800_801F1880 and
 * invokeBattlerStatusHelperThenSetGlobalWorkByteBToFFAndWorkByte2To6WhenNonZero
 * ElseWaitWhenRecordByte1IsSixElseClearQueuedSlotOrRunEnemyScriptThenSeedWork
 * Byte9AndAdvanceWorkByte2BmagicMagic00800_801F1474 write through the same
 * declaration. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Byte counter of the shared queued-slot store in main RAM, outside this
 * overlay's image: decremented in place and then used as the index of the three
 * parallel slot arrays below. Declared and indexed the way the sibling lift
 * invokeBattlerStatusHelperThenSetGlobalWorkByteBToFFAndWorkByte2To6WhenNonZero
 * ElseWaitWhenRecordByte1IsSixElseClearQueuedSlotOrRunEnemyScriptThenSeedWork
 * Byte9AndAdvanceWorkByte2BmagicMagic00800_801F1474 does it; no other lifted
 * source of this tree reads it, so its role beyond the counter and the three
 * cleared entries stays address-anchored. */
extern u8 D_80145A00; /* @source 0x80145A00 @kind data */

/* The three parallel queued-slot arrays the counter above indexes: word-stride
 * four for the first and halfword-stride two for the other two. */
extern u8 D_80145A04[]; /* @source 0x80145A04 @kind table */
extern u8 D_80145A54[]; /* @source 0x80145A54 @kind table */
extern u8 D_80145A7C[]; /* @source 0x80145A7C @kind table */

/* Shared battle-kind halfword in main RAM, outside this overlay's image: read
 * as a halfword and compared with 0xB, the width and signedness other lifts of
 * this tree bind the same cell at (isBattleStateFourWithKindOtherThan4COrB4B
 * BmagicMagic07603_801EF2E8 tests it against 0x2B). No role beyond that
 * equality is established, so the name stays address-anchored. */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* Shared battler-status helper of the main executable at 0x801DB524, outside
 * this overlay's image. The original passes the shared battle byte zero
 * extended and tests the answer truncated to eight bits (andi 0xFF / bnez), the
 * same byte-wide contract the sibling lift
 * invokeBattlerStatusHelperThenSetGlobalWorkByteBToFFAndWorkByte2To6WhenNonZero
 * ElseWaitWhenRecordByte1IsSixElseClearQueuedSlotOrRunEnemyScriptThenSeedWork
 * Byte9AndAdvanceWorkByte2BmagicMagic00800_801F1474 records. */
u8 func_801DB524(u8 arg0);

/* Resource request helper of the main executable at 0x8014D6B8, outside this
 * overlay's image. The original completes its single argument with andi 0xFF,
 * so the parameter is byte wide here. */
void func_8014D6B8(u8 arg0);

/* Enemy script helper of the main executable at 0x801E257C, outside this
 * overlay's image: it publishes the enemy work record indexed by its first
 * argument and runs the script byte helper for the second. Declared with the
 * two byte-wide parameters the sibling lift above uses at its own
 * func_801E257C(D_80146374, 2) site. */
void func_801E257C(u8 index, u8 scriptByte);

/* Shared seed helper of the main executable at 0x800AFF98, outside this
 * overlay's image: its returned byte seeds work byte 9 in this body and in the
 * sibling lift above. */
u8 func_800AFF98(void);

/* PsyQ libc random source at 0x8017E3D4, the address this body's jal word
 * encodes. The answer is truncated to its low three bits and tested against
 * zero, so the declaration carries the library's return type and casts at the
 * test. */
extern int rand(void);

/* @source 0x801F18F4
 * @behavior Entry 3 and entry 6 of the ten-entry work-state table at
 *           0x801EECC8 (0x801F17BC, 0x801F1804, 0x801F1880, 0x801F18F4,
 *           0x801F1804, 0x801F1880, 0x801F18F4, 0x801F1804, 0x801F1880,
 *           0x801F1BCC), which 0x801F1708 copies into its own 0x40-byte frame
 *           and dispatches on scratchpad work-object byte 2 before running the
 *           shared main-executable service func_8014D290 when work byte 0 is
 *           non-zero; no jal in this payload names this span and no payload
 *           word holds its address beyond those two table words, so those are
 *           its only incoming paths.
 *           Per frame it reads the shared battle byte 0x80146384 and passes it
 *           to the shared battler-status helper func_801DB524. When the byte
 *           answer is non-zero, the 0xFF constant filling that test's branch
 *           delay slot, it stores 0xFF into byte 0xB of the shared battle work
 *           record reached through the pointer at 0x801EB4E0 and sets work
 *           byte 2 to 9, which parks the run on the second 0x801F18F4 entry,
 *           and the frame ends. Otherwise it re-reads the same shared byte and,
 *           when it is below 3, reads the byte at 0x80145E91 of the
 *           0x140-byte party record that byte selects, or, when it is 3 or
 *           above, the byte at 0x801EB631 of the 0x118-byte enemy record that
 *           byte minus 3 selects; while that record byte reads 6 the frame ends
 *           with no state written.
 *           In every other case it branches on the acting battler byte
 *           0x80146374: below 3 (a party slot) it requests the resource for
 *           work byte 8 plus 0xC through func_8014D6B8, decrements the byte
 *           counter 0x80145A00 in place and zeroes the entry at the new count
 *           in the word array 0x80145A04 and in the two halfword arrays
 *           0x80145A54 and 0x80145A7C; for 3 and above (an enemy panel slot) it
 *           stores 0xFF into work byte 0x4B and runs func_801E257C with that
 *           byte as the enemy work index and 2 as the script byte. Both of
 *           those arms then seed work byte 9 from the byte func_800AFF98
 *           returns and add 1 to work byte 2 with a byte-width add, moving
 *           dispatch to the next entry of the run.
 *           The two selector arms carry that whole tail twice, so the helper
 *           call, the byte-9 seed and the increment appear in each, and only
 *           the final byte-2 store is shared, entered by a jump from either
 *           increment and by the fall out of the non-zero answer; the work
 *           pointer each of the three stores writes through is loaded by its
 *           own arm, so one pointer local carries the read, the increment and
 *           the store.
 *           After that store, and also on the record-byte-six path that skips
 *           it, a shared tail runs on every frame: it re-reads work byte 2 and
 *           ends when it is 9, ends when the shared halfword 0x801463C0 is not
 *           0xB, and ends unless the low three bits of rand() are clear; only
 *           then does it store 0xFF into byte 0xB of the same battle work
 *           record and park work byte 2 at 9.
 *           The 0x28-byte frame keeps $ra at 0x24 and $s0 at 0x20, $s0 holding
 *           the address of the shared battle byte across the helper call;
 *           nothing is returned. One comparison of the extracted payloads
 *           found no other copy of the 0x2D8-byte span: the sibling lift above
 *           differs in the terminal work byte its non-zero arm stores (6 there,
 *           9 here) and in the whole 0x64-byte gated tail this span adds after
 *           the shared store, so nothing here is cloned.
 * @status partial
 * @match 95.05
 * @residual The 0x2D8-byte span matches its original size and 173 of 182
 *           instructions. The one residual class is the register of the local
 *           that carries the value stored to work byte 2: the original keeps
 *           it in $v0 (li v0,255 in the helper test delay slot, addiu v0,v0,1
 *           in both arm seed tails, lbu v0,2(v1)/sb v0,2(v1) in the else arm
 *           and the shared store) while this body allocates $a0 for the same
 *           three sites. The original also keeps two byte-identical arm
 *           suffixes, and the owning cc1 cross-jumps identical arm bodies, so
 *           the second arm seeds byte 9 in two statements to keep both copies
 *           at no instruction cost; nine allocation spellings (u32 next,
 *           declaration order, swapping which arm carries the split, casts at
 *           the increment) did not move that pseudo. The 0x10-byte frame
 *           reservation is evidenced by this span's own frame and save offsets
 *           (0x28 with $s0 at 0x20 and $ra at 0x24), not by a recovered
 *           declaration.
 */
void invokeBattlerStatusHelperThenSetGlobalWorkByteBToFFAndWorkByte2To9WhenNonZeroElseWaitWhenRecordByte1IsSixElseClearQueuedSlotOrRunEnemyScriptSeedWorkByte9AdvanceWorkByte2ThenReparkOnRandomBmagicMagic00800_801F18F4(void) {
  u8 frameReservation[0x10];
  u8 *work;
  u8 *battle;
  u8 next;
  u8 slot;
  u32 selector;
  u32 index;

  battle = &D_80146384;
  if (func_801DB524(battle[0]) == 0) {
    selector = battle[0];
    if (selector < 3) {
      if (D_80145E91[selector * 0x140] == 6) {
        goto tail;
      }
      if (D_80146374 < 3) {
        func_8014D6B8(g_battle_work[8] + 0xC);
        slot = --D_80145A00;
        *(u32 *)&D_80145A04[slot * 4] = 0;
        *(u16 *)&D_80145A54[slot * 2] = 0;
        *(u16 *)&D_80145A7C[slot * 2] = 0;
      } else {
        g_battle_work[0x4B] = 0xFF;
        func_801E257C(D_80146374, 2);
      }
      g_battle_work[9] = func_800AFF98();
      work = g_battle_work;
      next = work[2] + 1;
    } else {
      index = selector - 3;
      if (D_801EB631[index * 0x118] == 6) {
        goto tail;
      }
      if (D_80146374 < 3) {
        func_8014D6B8(g_battle_work[8] + 0xC);
        slot = --D_80145A00;
        *(u32 *)&D_80145A04[slot * 4] = 0;
        *(u16 *)&D_80145A54[slot * 2] = 0;
        *(u16 *)&D_80145A7C[slot * 2] = 0;
      } else {
        g_battle_work[0x4B] = 0xFF;
        func_801E257C(D_80146374, 2);
      }
      g_battle_work[9] = func_800AFF98();
      work = g_battle_work;
      next = work[2];
      next++;
    }
  } else {
    D_801EB4E0[0xB] = 0xFF;
    work = g_battle_work;
    next = 9;
  }
  work[2] = next;
tail:
  if (g_battle_work[2] == 9) {
    return;
  }
  if (D_801463C0 != 0xB) {
    return;
  }
  if ((rand() & 7) != 0) {
    return;
  }
  D_801EB4E0[0xB] = 0xFF;
  g_battle_work[2] = 9;
}
