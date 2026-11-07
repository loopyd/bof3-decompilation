#include "bof3/bof3.h"

/* No-argument update of the main executable, called at entry before the work
 * object is touched. The game00 lifts record the same address as a byte-returning
 * no-argument routine whose subsystem is still unknown, and this handler discards
 * the result, so the value is not consumed here. */
extern u8 func_8014D978(void);

/* Distance helper of the main executable. The scenario, world, battle and
 * bmagic lifts declare the same two-argument signed-halfword prototype and use
 * it for the work word pair at 0x34 and 0x38: func_801EEC5C of this family
 * publishes its result in work halfword 0x3E exactly as this entry does. */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* Scratchpad work-object pointer cell. Declared as the named cell the exact
 * sibling seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500
 * _801EF4A4 on this target already binds, so the encoded name keeps the address
 * with no target-map row. Each store group re-reads the cell because a store
 * through the loaded pointer may alias it, which is what the original shows
 * before the second group and before the state-advance group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Fields of the shared 0x78-byte action-slot store record. The original reaches
 * each field through its own flat symbol (lui $at,%hi(SYM) + addu at,at,
 * index*0x78 + load %lo(SYM)(at)), so every addressed field keeps its own
 * address-anchored name instead of a shared record base: byte +0x02, words
 * +0x34, +0x38, +0x3C and the signed halfword +0x3E of the entry that the
 * sibling lift names as the queued action-slot store based at 0x801EC330. This
 * target's generated undefined_syms_auto.txt already carries all five addresses
 * because the neighbouring asm handlers still reference them. */
extern u8 D_801EC332[]; /* @source 0x801EC332 @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC36E[]; /* @source 0x801EC36E @kind data */

/* @source 0x801EF608
 * @behavior Work-step handler, entry 5 of the eight-entry handler table at
 *           0x801EF8E0. It first calls the no-argument main-executable update
 *           func_8014D978 and discards its result. It then selects the 0x78-byte
 *           action-slot store record with the scratchpad work-object byte at
 *           offset 0xB, copies that record word +0x34 into work word +0x34 and
 *           record word +0x38 into work word +0x38 through one value local,
 *           passes the pair (work word +0x34 re-read through the pointer, that
 *           value local) to the distance helper func_8015477C and stores the
 *           returned signed halfword at work +0x3E. When the record signed
 *           halfword +0x3E is smaller than that distance it copies record word
 *           +0x3C into work word +0x3C. At the end, when record byte +0x02
 *           equals 2, it advances the work state byte +0x02 by one with a
 *           byte-width read-modify-write. The index byte is re-read from the
 *           loaded pointer before every scaled access group because a store
 *           through that pointer may alias it, and the cell itself is re-read
 *           for each of the three groups, so three lui/lw base pairs appear in
 *           the body; the second group reads its index once and reuses the
 *           scaled offset for the halfword test and the conditional word copy.
 *           The 0x18-byte frame only keeps $ra across the two calls. The
 *           function takes no arguments, returns nothing and touches no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyQueuedSlotPositionPairAndDistanceThenAdvanceWorkByte2WhenSlotByte2IsTwoBmagicMagic04500_801EF608(
    void) {
  u8 *work;
  u8 *next;
  u8 *slot;
  s32 target;
  s16 distance;

  func_8014D978();
  work = D_1F800044;
  *(u32 *)(work + 0x34) = *(u32 *)(D_801EC364 + work[0xB] * 0x78);
  target = (s32) * (u32 *)(D_801EC368 + work[0xB] * 0x78);
  *(u32 *)(work + 0x38) = target;
  distance = func_8015477C((s32) * (u32 *)(work + 0x34), target);
  next = D_1F800044;
  *(u16 *)(next + 0x3E) = (u16)distance;
  if (*(s16 *)(D_801EC36E + next[0xB] * 0x78) < distance)
    *(u32 *)(next + 0x3C) = *(u32 *)(D_801EC36C + next[0xB] * 0x78);
  slot = D_1F800044;
  if (D_801EC332[slot[0xB] * 0x78] == 2)
    slot[2]++;
}
