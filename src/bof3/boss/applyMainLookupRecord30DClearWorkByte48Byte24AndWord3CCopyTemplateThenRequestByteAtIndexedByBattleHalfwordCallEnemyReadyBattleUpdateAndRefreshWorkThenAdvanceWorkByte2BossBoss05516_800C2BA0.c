#include "bof3/bof3.h"

/* The leading data run of this payload starts at 0x800C1804 and holds 12-byte
 * slots whose first 0xA bytes are copied into a frame slot before use. This body
 * copies the slot at 0x800C1828, whose first ten bytes are
 * 00 02 00 04 00 05 00 00 00 00; the step body 0x800C27E8 copies the slot at
 * 0x800C1804, 0x800C2948 the one at 0x800C1810, 0x800C2AA8 the one at
 * 0x800C181C and the alternative step bodies 0x800C2D2C and 0x800C305C the ones
 * at 0x800C1834 and 0x800C1858. As in those lifts the copy is one aggregate
 * move: both 4-byte groups travel through lwl/lwr and swl/swr pairs and the two
 * trailing bytes through lb/sb, which is what this compiler emits for a 10-byte
 * aggregate whose alignment is one, so the record type is a byte aggregate
 * rather than a plain u32 pair. The copied bytes are then indexed by the battle
 * halfword 0x801463BE, so the frame copy is read as bytes.
 */
typedef struct Boss05516StepTemplate {
  u8 bytes[0xA];
} Boss05516StepTemplate;

extern Boss05516StepTemplate D_800C1828; /* @source 0x800C1828 @kind table */

/* Battle halfword this overlay's step helpers use as the index into a step
 * template: the exact sibling lifts raise it, or store 8 into it to end a step
 * sequence, and it is not established which table of the battle state it
 * indexes. */
extern u16 D_801463BE; /* @source 0x801463BE @kind unknown */

/* Scratchpad work-record pointer cell: the battle code publishes the work
 * record here and every access reaches it through this cell. The cell is a
 * plain fixed-address pointer, so each store reloads it, because the store
 * through the loaded pointer may alias the cell itself. */
extern u8* g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable lookup helper. Its exact lift at 0x8014DD3C shows it
 * applying the "main lookup" record whose id matches its argument to the active
 * work record, so the argument here is a record id. This target's generated
 * undefined_funcs_auto.txt row binds the address-anchored name, so the call
 * keeps it. */
extern void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Shared main-executable resource request helper; the sibling battle, world and
 * plchar lifts declare the same prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* Shared main-executable readiness helper of this overlay step, called without
 * arguments and with its result discarded, exactly as the exact sibling step
 * body 0x800C2E04 declares it. */
extern u8 func_801E3160(void); /* @source 0x801E3160 */

/* Shared main-executable battle update, called without arguments. Its role
 * beyond that is not established, so it keeps its address-anchored spelling. */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 */

/* This overlay's own work-refresh step, defined in its own source file. */
extern void refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284(void); /* @source 0x800C3284 */

/* @source 0x800C2BA0
 * @behavior Entry 0 body of this overlay's local two-entry handler pair, the
 *           pair dispatchStateByte2LocalPairBossBoss05516_800C2B48 seeds with
 *           0x800C2BA0 then 0x800C2C78 and selects with work byte +2. It copies
 *           the 10-byte step template at 0x800C1828 into frame slot 0x10, then
 *           applies main lookup record 0x30D through the shared main-executable
 *           helper 0x8014DD3C, and then clears the work record published at
 *           scratchpad pointer cell 0x1F800044 at byte +0x48, byte +0x24 and
 *           word +0x3C. It next requests the template byte selected by the
 *           battle halfword 0x801463BE through the shared resource request
 *           helper 0x8014D6B8, and it does so unconditionally: this body never
 *           reads the shared battle work record at 0x801EB4E0, so the guard byte
 *           +0xB that the exact sibling step bodies 0x800C2AA8 and 0x800C3134
 *           test has no counterpart here. It then calls the shared readiness
 *           helper 0x801E3160, the shared battle update 0x8014D4E0 and this
 *           overlay's own work refresh 0x800C3284, in that order and discarding
 *           each result. Finally it re-reads the cell and advances work byte +2
 *           by one, moving the pair dispatcher to entry 1, and leaves every
 *           other byte of the record alone.
 *           The original materialises the scratchpad cell address four times
 *           (the `lui`/`lw` pairs above 0x800C2BEC, 0x800C2BFC, 0x800C2C0C and
 *           0x800C2C50, each using the load destination as base), because a
 *           store through the loaded pointer may alias the cell; the template
 *           copy loads all three groups before it stores any of them, and the
 *           byte +2 store fills the return slot of the plain `jr $ra`. The
 *           0x28-byte frame keeps the template at 0x10 and $ra at 0x20 across
 *           the four calls.
 *           This 54-instruction 0xD8 body differs from the alternative step
 *           body 0x800C2D2C in exactly two payload words, the low immediate of
 *           the template slot (0x1828 here, 0x1834 there) and the lookup record
 *           id passed to 0x8014DD3C (0x30D here, 0x318 there), and from the
 *           alternative step body 0x800C305C in those two plus the readiness
 *           callee (0x801E3160 here, 0x801E30F8 there). The template slot lies
 *           inside this payload, so that literal and the two record ids are this
 *           overlay's own and no byte-identical twin exists for the span. The
 *           roles of work byte +2, of the lookup record ids and of the template
 *           own fields are not established, so they stay offset- and
 *           address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyMainLookupRecord30DClearWorkByte48Byte24AndWord3CCopyTemplateThenRequestByteAtIndexedByBattleHalfwordCallEnemyReadyBattleUpdateAndRefreshWorkThenAdvanceWorkByte2BossBoss05516_800C2BA0(
    void) {
  Boss05516StepTemplate stepTemplate;

  stepTemplate = D_800C1828;
  func_8014DD3C(0x30D);
  g_battle_work[0x48] = 0;
  g_battle_work[0x24] = 0;
  FIELD_REF(s32, g_battle_work, 0x3C) = 0;
  func_8014D6B8(stepTemplate.bytes[D_801463BE]);
  func_801E3160();
  func_8014D4E0();
  refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284();
  g_battle_work[2]++;
}
