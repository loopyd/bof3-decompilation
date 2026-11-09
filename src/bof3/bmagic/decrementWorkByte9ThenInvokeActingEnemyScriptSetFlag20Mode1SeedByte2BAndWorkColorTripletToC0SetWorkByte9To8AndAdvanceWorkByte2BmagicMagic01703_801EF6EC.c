#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named fixed address, the
 * form the exact siblings of this overlay use, so the assembler folds the
 * constant into each access (lui $vN,0x1F80 / lw $vN,68($vN)); the original
 * re-materialises the cell before every store group because a store through the
 * pointer it supplied may alias the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Acting battler byte of the main executable. Party slots are 0, 1 and 2 and
 * enemy panel slots are 3 and above, so the unsigned compare below against 3
 * selects an enemy battler. The exact bmagic lift
 * seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPair
 * AndAdvanceWorkByte2BmagicMagic04403_801EEEFC reads the same byte with the same
 * split. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Shared battle helper of the main executable, outside this overlay's image: it
 * publishes the 0x118-byte enemy work record at 0x801EB2E8 indexed by its first
 * argument as the current enemy work, runs the enemy script byte helper
 * func_801E2314 for the second argument, then restores the previously published
 * record. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is needed. */
extern void func_801E257C(u8 index, u8 arg1);

/* @source 0x801EF6EC
 * @behavior Per-frame step handler of this overlay's work-byte-2 run: word 6 of
 *           the ten-word code pointer table at 0x801EEC10, stored at
 *           0x801EEC28, holds this function's address, and the run dispatcher
 *           func_801EF058 copies that table into its frame and indexes it with
 *           scratchpad work-object byte 2, so the body runs while work byte 2 is
 *           6. That dispatcher is the single entry of the byte-1 run at
 *           0x801EF990 which dispatchWorkByte1HandlerBmagicMagic01703_801EF014
 *           selects, and no jal in this payload names this function, so the
 *           table word is its only incoming path. It decrements the work
 *           object's unsigned countdown byte 9 in place (lbu / addiu -1 / sb /
 *           andi 0xFF / bnez), so the rest of the body runs only when that
 *           countdown reaches zero. On zero it first tests the acting battler
 *           byte 0x80146374 against 3 with an unsigned compare and, for an enemy
 *           panel slot, invokes the shared battle helper func_801E257C with that
 *           byte as the enemy work index and 0 as the script byte selector. Then
 *           it re-reads the pointer cell and raises bit 0x20 of work byte 0,
 *           sets mode byte 0x5C and work byte 0x2B to 1 from one materialised
 *           constant, sets the signed tint triplet at 0x5D, 0x5E and 0x5F to
 *           -0x40, re-seeds countdown byte 9 with 8 and advances work byte 2 by
 *           one, moving the run to index 7 at 0x801EF7E0. That entry subtracts 8
 *           per frame from the same triplet while its own byte-9 countdown runs,
 *           so this handler seeds the 0xC0 byte those channels fade from. The
 *           0x18-byte frame only keeps $ra at 0x10 across the helper call, and
 *           the cell read for countdown byte 9 is scheduled ahead of the frame
 *           creation. Takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenInvokeActingEnemyScriptSetFlag20Mode1SeedByte2BAndWorkColorTripletToC0SetWorkByte9To8AndAdvanceWorkByte2BmagicMagic01703_801EF6EC(void) {
  u8 value;

  if (--D_1F800044[9] == 0) {
    if (D_80146374 >= 3) {
      func_801E257C(D_80146374, 0);
    }
    D_1F800044[0] |= 0x20;
    value = 1;
    D_1F800044[0x5C] = value;
    D_1F800044[0x2B] = value;
    ((s8 *)D_1F800044)[0x5D] = -0x40;
    ((s8 *)D_1F800044)[0x5E] = -0x40;
    ((s8 *)D_1F800044)[0x5F] = -0x40;
    D_1F800044[9] = 8;
    D_1F800044[2]++;
  }
}
