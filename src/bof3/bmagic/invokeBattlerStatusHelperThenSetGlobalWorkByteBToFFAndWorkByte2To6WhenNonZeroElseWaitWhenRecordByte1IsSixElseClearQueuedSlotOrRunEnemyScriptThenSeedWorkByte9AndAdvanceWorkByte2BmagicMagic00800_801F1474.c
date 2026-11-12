#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, the same named cell this target's exact
 * lifts advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C,
 * invokeReadinessHelperThenDecrementWorkByte9ThenInvokeBattleHelpersAndAdvance
 * WorkByte2OnZeroBmagicMagic00800_801F1384 and
 * setGlobalWorkByteBToFFAtWorkByte2State5ThenAdvanceWorkByte2WhenHelperReady
 * BmagicMagic00800_801F1400 declare. The named symbol lets the assembler fold
 * the constant address into each load (lui $v1,0x1F80 / lw $v1,68($v1)), and
 * the original re-reads the cell after every call and after every store through
 * the loaded pointer. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle byte in main RAM, outside this overlay's image. The original
 * reads it once for the battler-status helper call and once after it, spanning
 * the call, so the address is materialized once into $s0 (lui plus addiu) and
 * both reads use offset zero; the same byte is the record selector this
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
 * 0x140-byte-stride party rows the battle targets own, read here at byte 1 of
 * the row the shared battle byte selects. Kept a byte array with the stride
 * written at the index, the spelling the exact sibling lift
 * seedWorkObjectFromBattleRecordThenClaimSlotCopyActingBattlerRecordHead
 * StampSlotBytesPublishWorkPointerRaiseFlag40AndInvokeHelperAt100Bmagic
 * Magic00800_801EED5C used for the same table. Splat binds the address through
 * this target's generated undefined_syms_auto.txt, which the remaining asm span
 * at 0x801F18F4 still references. */
extern u8 D_80145E91[]; /* @source 0x80145E91 @kind table */

/* Enemy record table in main RAM, outside this overlay's image: the
 * 0x118-byte-stride enemy rows, read here at byte 1 of the row whose index is
 * the shared battle byte minus 3, and declared like the party table above. */
extern u8 D_801EB631[]; /* @source 0x801EB631 @kind table */

/* Shared battle work record pointer in main RAM, outside this overlay's image:
 * the original loads it and stores 0xFF into its byte 0xB, the same signal the
 * exact sibling lift setGlobalWorkByteBToFFAtWorkByte2State5ThenAdvance
 * WorkByte2WhenHelperReadyBmagicMagic00800_801F1400 writes through the same
 * declaration. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Byte counter of the shared queued-slot store in main RAM, outside this
 * overlay's image: decremented in place and then used as the index of the three
 * parallel slot arrays below. No other lifted source of this tree reads it, so
 * its role beyond the counter and the three cleared entries stays
 * address-anchored. */
extern u8 D_80145A00; /* @source 0x80145A00 @kind data */

/* The three parallel queued-slot arrays the counter above indexes: word-stride
 * four for the first and halfword-stride two for the other two. */
extern u8 D_80145A04[]; /* @source 0x80145A04 @kind table */
extern u8 D_80145A54[]; /* @source 0x80145A54 @kind table */
extern u8 D_80145A7C[]; /* @source 0x80145A7C @kind table */

/* Shared battler-status helper of the main executable at 0x801DB524, outside
 * this overlay's image. The original passes the shared battle byte zero
 * extended and tests the answer truncated to eight bits (andi 0xFF / bnez), the
 * same byte-wide contract the exact sibling lift
 * clearRecordPoolBytes0To2AndWorkBytesBAnd9ThenQueueSlotPerEligibleEnemyAndParty
 * BattlerClearByte1SetBattlerByte4AndWorkPointerCountWorkByteBAdvanceWorkByte1
 * InvokeHelperAt100BmagicMagic06303_801EECF8 records as (u8)func_801DB524(x). */
u8 func_801DB524(u8 arg0);

/* Resource request helper of the main executable at 0x8014D6B8, outside this
 * overlay's image. The original completes its single argument with andi 0xFF,
 * so the parameter is byte wide here. */
void func_8014D6B8(u8 arg0);

/* Enemy script helper of the main executable at 0x801E257C, outside this
 * overlay's image: it publishes the enemy work record indexed by its first
 * argument and runs the script byte helper for the second. Declared with the
 * same two byte-wide parameters the exact sibling lift
 * addWord0CToWord34AndWord10ToWord38And4ToTintTripletThenDecrementWorkByte9And
 * OnZeroDecrementGlobalWorkByteBOrClearFlag20ResetWorkAdvanceWorkByte2
 * BmagicMagic01703_801EF460 uses at its own func_801E257C(D_80146374, 2) site. */
void func_801E257C(u8 index, u8 scriptByte);

/* Shared seed helper of the main executable at 0x800AFF98, outside this
 * overlay's image: its returned byte seeds work byte 9 in this body and in the
 * exact sibling lift
 * addWord0CToWord34AndWord10ToWord38And4ToTintTripletThenDecrementWorkByte9And
 * OnZeroDecrementGlobalWorkByteBOrClearFlag20ResetWorkAdvanceWorkByte2
 * BmagicMagic01703_801EF460. */
u8 func_800AFF98(void);

/* @source 0x801F1474
 * @behavior Entry 3 of the seven-entry handler run at 0x801EECAC (0x801F133C,
 *           0x801F1384, 0x801F1400, 0x801F1474, 0x801F1384, 0x801F1400,
 *           0x801F16E8), which the still-asm dispatcher 0x801F1294 copies into
 *           its own 0x38-byte frame and indexes with scratchpad work-object
 *           byte 2 before running the shared main-executable service
 *           func_8014D290 when work byte 0 is non-zero; no jal in this payload
 *           names this span, so that one table word is its only incoming path.
 *           Per frame it reads the shared battle byte 0x80146384 and passes it
 *           to the shared battler-status helper func_801DB524. When the byte
 *           answer is non-zero, the 0xFF constant filling that test's branch
 *           delay slot, it stores 0xFF into byte 0xB of the shared battle work
 *           record reached through the pointer at 0x801EB4E0 and stores 6 into
 *           work byte 2, which parks the run on entry 6 (0x801F16E8, the
 *           0x801E5988 wrapper), and the frame ends. Otherwise it re-reads the
 *           same shared byte and, when it is below 3, reads byte 1 of the
 *           0x140-byte party record at 0x80145E91 selected by that byte, or,
 *           when it is 3 or above, byte 1 of the 0x118-byte enemy record at
 *           0x801EB631 selected by that byte minus 3; while that record byte
 *           reads 6 the frame ends with no state written. In every other case
 *           it branches on the acting battler byte 0x80146374: below 3 (a party
 *           slot) it requests the resource for work byte 8 plus 0xC through
 *           func_8014D6B8, decrements the byte counter 0x80145A00 in place and
 *           zeroes the entry at the new count in the word array 0x80145A04 and
 *           in the two halfword arrays 0x80145A54 and 0x80145A7C; for 3 and
 *           above (an enemy panel slot) it stores 0xFF into work byte 0x4B and
 *           runs func_801E257C with that byte as the enemy work index and 2 as
 *           the script byte. Both of those arms then seed countdown byte 9 from
 *           the byte func_800AFF98 returns and add 1 to work byte 2 with a
 *           byte-width add, moving dispatch to the next entry of the run. The
 *           two selector arms carry that whole tail twice, so the helper call,
 *           the byte-9 seed and the increment appear in each, and only the final
 *           byte-2 store is shared, entered by a jump from either increment and
 *           by the fall out of the non-zero answer; the work pointer each of the
 *           three stores writes through is loaded by its own arm, so one pointer
 *           local carries the read, the increment and the store. The 0x28-byte
 *           frame keeps $ra at 0x24 and $s0 at 0x20, $s0 holding the address of
 *           the shared battle byte across the helper call; nothing is returned.
 *           The span is not byte-identical to any other: the unlifted entry at
 *           0x801F18F4 differs in its four j immediates, in the terminal work
 *           byte it stores from the non-zero arm (9 there, 6 here) and in the
 *           0x64-byte tail it adds after the shared store, and one scan of the
 *           396 extracted payloads found no other copy, so nothing here is
 *           cloned. Roles of the helper's byte answer, of the record byte these
 *           two tables are read for and of the three cleared arrays are not
 *           established, so those stay address-anchored.
 * @status partial
 * @match 84.28
 * @residual The two selector arms duplicate the whole tail in the original and
 *           this body repeats them too, but the owning compiler merged the two
 *           identical arm bodies when the arms were written as a plain if/else
 *           (that variant measured 0x1B8 bytes against 0x274), so the arms are
 *           separated here by the selector re-test the original does not carry:
 *           that test costs the two surplus instructions. An unused 16-byte
 *           local reconstructs the original's untouched frame reservation;
 *           its size is evidenced by the frame and save offsets, not by a
 *           recovered source declaration. The non-zero answer materializes
 *           0xFF in the original branch delay slot where this body materializes
 *           6, and the retained selector also changes register allocation.
 */
void invokeBattlerStatusHelperThenSetGlobalWorkByteBToFFAndWorkByte2To6WhenNonZeroElseWaitWhenRecordByte1IsSixElseClearQueuedSlotOrRunEnemyScriptThenSeedWorkByte9AndAdvanceWorkByte2BmagicMagic00800_801F1474(void) {
  u8 frameReservation[0x10];
  u8 *work;
  u8 *battle;
  u32 selector;
  u8 next;
  u8 slot;

  battle = &D_80146384;
  if (func_801DB524(battle[0]) == 0) {
    selector = battle[0];
    if (selector < 3) {
      if (D_80145E91[selector * 0x140] == 6) {
        return;
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
    }
    if (selector >= 3) {
      if (D_801EB631[(selector - 3) * 0x118] == 6) {
        return;
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
    }
  } else {
    D_801EB4E0[0xB] = 0xFF;
    work = g_battle_work;
    next = 6;
  }
  work[2] = next;
}
