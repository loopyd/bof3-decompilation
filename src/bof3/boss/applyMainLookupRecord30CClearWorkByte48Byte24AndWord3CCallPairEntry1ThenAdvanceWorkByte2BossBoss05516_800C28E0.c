#include "bof3/bof3.h"

/* Shared main-executable lookup helper. Its exact lift at 0x8014DD3C shows it
 * applying the "main lookup" record whose id matches its argument to the active
 * work record, so the argument here is a record id. This target's generated
 * undefined_funcs_auto row binds the address-anchored name, so the call keeps
 * it. */
void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Entry 1 of this overlay's local two-entry handler pair; the byte-2 dispatcher
 * dispatchStateByte2LocalPairBossBoss05516_800C2888 seeds the pair and this body
 * calls it directly. The boundary was still a raw asm row when this seed was
 * written, so the target map name is its canonical spelling. */
void func_800C2948(void);

/* Scratchpad work-record pointer cell: the battle code publishes the work
 * record here and every access reaches it through this cell. The cell is a
 * plain fixed-address pointer, so each store reloads it, because the store
 * through the loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x800C28E0
 * @behavior Entry 0 body of this overlay's local two-entry handler pair, the
 *           pair dispatchStateByte2LocalPairBossBoss05516_800C2888 seeds with
 *           0x800C28E0 then 0x800C2948 and selects with work byte +2. It
 *           applies main lookup record 0x30C through the shared main-executable
 *           helper 0x8014DD3C, then clears the work record published at
 *           scratchpad pointer cell 0x1F800044 at byte +0x48, byte +0x24 and
 *           word +0x3C. It next calls entry 1 of the same pair directly, so that
 *           body runs once from inside this handler and again when the
 *           dispatcher answers the advanced state byte. Finally it re-reads the
 *           cell and advances work byte +2 by one, moving the pair dispatcher to
 *           entry 1, and leaves every other byte of the record alone.
 *           The original materialises the cell address four times (the
 *           `lui`/`lw` pairs above 0x800C28F0, 0x800C2900, 0x800C2910 and
 *           0x800C2920, each using the load destination as base), because a
 *           store through the loaded pointer may alias the cell; the word store
 *           at +0x3C fills the delay slot of the 0x800C2948 call and the byte
 *           +2 store fills the return slot. No word of the payload holds this
 *           address or the pair base, so the overlay dispatcher is its only
 *           caller and it is an entry reached from outside the payload; the
 *           0x18-byte prologue frame keeps $ra at 0x10 across the two calls and
 *           the plain `jr $ra` return path places the end of the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyMainLookupRecord30CClearWorkByte48Byte24AndWord3CCallPairEntry1ThenAdvanceWorkByte2BossBoss05516_800C28E0(
    void) {
  func_8014DD3C(0x30C);
  g_battle_work[0x48] = 0;
  g_battle_work[0x24] = 0;
  FIELD_REF(s32, g_battle_work, 0x3C) = 0;
  func_800C2948();
  g_battle_work[2]++;
}
