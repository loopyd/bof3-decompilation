#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The original loads the cell before every store group
 * that runs through this name and reloads it after each group, because a store
 * through the loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Resource request wrapper of the shared main executable, outside this payload.
 * This target's generated undefined_funcs_auto.txt already binds the address
 * because the overlay's other spans call it too. */
extern void func_8014D6B8(u32 selector); /* @source 0x8014D6B8 @kind function */

/* @source 0x801EF528
 * @behavior Entry 4 (index 4 of the fourteen-word byte-1 handler run based at
 *           0x801F046C) that
 *           callStateTable046 selects with
 *           work-record byte 1 and invokes indirectly; the payload word at
 *           0x801F047C holds this body's address. It seeds the work record read
 *           from the scratchpad pointer cell 0x1F800044 and then moves the
 *           work-byte-2 dispatcher on. Byte 0x2E receives 0xA0 and byte 0x25
 *           receives 0x1D through the first base; the second base is loaded
 *           before the 0x30 store, which writes 0x12C to the unsigned halfword
 *           at 0x30 through the first base while the second base serves byte
 *           0x26 with zero. Bytes 0x27 and 0x28 then take 0x1A and one, byte
 *           0x24 takes 0x80, and word 0x40 takes 0x30000 while word 0x44 takes
 *           0xC000 and byte 0x48 takes two, all through one base that the third
 *           and fourth statements share. The clear run that follows re-reads
 *           the cell for every store: zero into bytes 0x5D, 0x5E, 0x5F and
 *           0x5C, then zero into byte 0x2A, two into byte 0x29, zero into the
 *           unsigned halfword at 0x2C and one into byte 0x2B. The body then
 *           requests resource 0 through the shared wrapper func_8014D6B8 and
 *           reads work byte 2, adds one and stores the sum back, which advances
 *           the work-byte-2 dispatcher to its next step. It takes no arguments,
 *           returns nothing and keeps no value live across the call, so its
 *           frame is $ra-only at 0x18 with $ra at 0x10. The sibling
 *           setRandomWorkWord2ENearA0DecrementWorkWord30By2AdvanceWorkWord44By2
 *           000AndAdvanceWorkByte2Past17FFFBmagicMagic11003_801EF648 sits one
 *           word later in the same handler run and steps the two halfwords this
 *           body writes together with the word at 0x44 it seeds.
 *
 *           No byte-identical twin exists: a scan of every payload under
 *           out/binaries for this 0x120-byte span, and a second scan for its
 *           0x78-byte interior window at span offset 0x28, find only this
 *           payload's own copy at offset 0x928, so the clone lever is declined
 *           and every statement comes from this span's own bytes. The
 *           declaration style and the separate-base form follow the
 *           same-family seeder
 *           seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_8
 *           01EF4A4 in target bmagic/magic045/00, whose body stores the same
 *           work-header group and calls the same resource wrapper; the
 *           literals, offsets and widths here are this span's own. The roles of
 *           bytes 0x24..0x2B, 0x48 and 0x5C..0x5F, of the halfword pair
 *           0x2E/0x30 and of the words 0x40/0x44 are not established in this
 *           payload, so the name anchors them at their offsets.
 * @status exact
 * @match 100.00
 * @residual none
 *
 *           Measured probes before the matching gate, all at the original 0x120
 *           bytes: one base local per store group measured 26 of 72, a local per
 *           group with the byte store at 0x48 first measured 60 of 73, one
 *           pointer local reused for the first and the 0x40 group measured 49 of
 *           74, and giving that group its own third local measured 62 of 73.
 *           Writing both word stores as offset casts off g_battle_work and the
 *           0x48 store first measured 63 of 72, and casting only those two
 *           stores through a u32 view while the byte store stayed first measured
 *           65 of 72. Issuing both word stores through the u32 view before the
 *           byte store measured 72 of 72 at that size, the retained form; the
 *           header's 0x2E/0x30 stores and the 0x2C store measured the same in
 *           both the offset-cast and the indexed-halfword spellings.
 */
void stateHeaderClearRequest(
    void) {
  u8 *work;
  u8 *next;

  work = g_battle_work;
  *(u16 *)(work + 0x2E) = 0xA0;
  work[0x25] = 0x1D;
  next = g_battle_work;
  *(u16 *)(work + 0x30) = 0x12C;
  next[0x26] = 0;
  g_battle_work[0x27] = 0x1A;
  g_battle_work[0x28] = 1;
  g_battle_work[0x24] = 0x80;
  ((u32 *)g_battle_work)[0x10] = 0x30000;
  ((u32 *)g_battle_work)[0x11] = 0xC000;
  g_battle_work[0x48] = 2;
  g_battle_work[0x5D] = 0;
  g_battle_work[0x5E] = 0;
  g_battle_work[0x5F] = 0;
  g_battle_work[0x5C] = 0;
  g_battle_work[0x2A] = 0;
  g_battle_work[0x29] = 2;
  *(u16 *)(g_battle_work + 0x2C) = 0;
  g_battle_work[0x2B] = 1;
  func_8014D6B8(0);
  g_battle_work[2]++;
}
