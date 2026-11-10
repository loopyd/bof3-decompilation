#include "bof3/bof3.h"

/* Record pool of this overlay at 0x801F04A4: 0x40 records of stride 0x78, the
 * 0x1E00-byte span that fills the payload from offset 0x18A4 to its end. The
 * frame dispatcher func_801EEC28 walks the pool record by record after it
 * returns from the stage this file seeds. Byte 0 of every record plus the two
 * bytes that follow it are cleared here, so the original addresses the base and
 * the two following bytes with their own `lui` / `addu` / `sb` triple. The map
 * carries no row at this address; the generated undefined_syms_auto.txt binds
 * all three from the names the payload's own asm used. */
extern u8 D_801F04A4[]; /* @source 0x801F04A4 @kind data */
extern u8 D_801F04A5[]; /* @source 0x801F04A5 @kind data */
extern u8 D_801F04A6[]; /* @source 0x801F04A6 @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The body loads the cell once at each site that uses it -
 * the battle-state byte, record byte 8, the pose word triplet, work byte 9, work
 * byte 0xA, the handler-index increment, the published work pointer and the
 * queue count - because the stores through the loaded pointer may alias the
 * cell, so the original re-materializes the address with its own `lui` / `lw`
 * pair at each read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle-record pointer cell at 0x8014638C in main executable RAM,
 * outside this overlay image and bound by this target's generated
 * undefined_syms_auto.txt. Byte 0x8 and words 0x34, 0x38 and 0x3C are read from
 * the record it points at; the sibling lift
 * decrementWorkByte9ThenCopyRecordByte4AndTripletAddC00000ClearByte9SetATo3CAndAdvanceWorkByte2BmagicMagic07603_801EF3F4
 * copies the same word triplet from its own record cell 0x801EB4E0 into the
 * work object, so the triplet is the shared record pose this body republishes. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Action-slot store in main RAM spanning 0x801EC330 plus the 0x78-byte stride.
 * The original reaches each field through its own flat symbol - `lui
 * $at,%hi(SYM)`, `addu $at,$at,offset`, store `%lo(SYM)($at)` - with the scaled
 * allocator answer in the one register shared by all three stores, exactly as
 * the sibling lifts
 * clearRecordPoolBytes0To2AndWorkBytesBAnd9ThenQueueSlotPerEligibleEnemyAndPartyBattlerClearByte1SetBattlerByte4AndWorkPointerCountWorkByteBAdvanceWorkByte1InvokeHelperAt100BmagicMagic06303_801EECF8
 * and
 * clearRecordPoolBytes0To2AndWorkBytes4AndBThenQueueSlotPerEligibleEnemyOrPartyBattlerClearByte1SetByte4ClaimOrdinalByte9AndWorkPointerCountWorkByteBAdvanceWorkByte1InvokeHelperAt100BmagicMagic07303_801EED18
 * declare them. This body clears slot byte 0x01, stores 1 into slot byte 0x09
 * and publishes the work-object pointer into slot word 0x74. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Shared main-RAM byte outside this overlay's image, bound by this target's
 * generated undefined_syms_auto.txt row D_80145988 = 0x80145988. Other lifted
 * targets in this tree describe it as a stage serial, but nothing in this
 * target establishes that role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* Battle-state test of this target at 0x801EF2E8. Its body reports
 * whether the battle state byte is four and the kind byte is neither 0x4C nor
 * 0xB4, and this body stores that answer into work byte 0x4. */
extern u8
    isBattleStateFourWithKindOtherThan4COrB4BmagicMagic07603_801EF2E8(
        void); /* @source 0x801EF2E8 */

/* Action-slot allocator of the shared main executable at 0x801E590C, outside
 * this overlay image and bound through this target's generated
 * undefined_funcs_auto.txt. This body passes request class 1 and code 0x62 and
 * uses the zero-extended low byte of the answer as the index of the slot it
 * claimed, the same two-step use the sibling lifts of the bmagic overlays
 * record for this allocator. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* Main-executable helper at 0x8015D71C, outside this overlay image and bound
 * through this target's generated undefined_funcs_auto.txt. The bmagic siblings
 * call it with one code and use no part of an answer, so only its effect is
 * established and its role stays address-anchored. */
extern void func_8015D71C(u16 selector); /* @source 0x8015D71C */

/* @source 0x801EED48
 * @behavior Entry stage 0 of this overlay: word 0 of the nine-word code pointer
 *           run at 0x801EEC04, whose first six words the frame dispatcher
 *           func_801EEC28 copies onto its own stack and calls through the
 *           work-object byte 0x1 scaled by four, so no jal in this payload
 *           names the body. It clears bytes 0x00, 0x01 and 0x02 of all 0x40
 *           records of the 0x78-byte-stride pool at 0x801F04A4 with one sb per
 *           byte, computing the stride from the masked index and testing the
 *           incremented index against 0x40 at the loop bottom. It then stores
 *           the battle-state answer of
 *           isBattleStateFourWithKindOtherThan4COrB4BmagicMagic07603_801EF2E8
 *           into work byte 0x4, copies record byte 0x8 of the shared record at
 *           0x8014638C into work byte 0x8, and publishes that record's words
 *           0x34, 0x38 and 0x3C into the same work words, clearing work byte
 *           0xB between the word 0x3C load and its store. Next it rearms work
 *           byte 0x9 to 0x3C, clears work byte 0xA and increments work byte
 *           0x1, the handler index this overlay dispatches on, so the next
 *           frame runs the following stage. It claims one action slot through
 *           the shared allocator func_801E590C(1, 0x62), clears that slot's
 *           byte 0x01, stores 1 into its byte 0x09, publishes the work-object
 *           pointer read from the scratchpad cell 0x1F800044 into slot word
 *           0x74 and increments work byte 0xB, the queue count. It then copies
 *           the 0x100 unsigned halfwords of the table at 0x80036C00 into the
 *           table at 0x8003AC00 and the 0x10 unsigned halfwords of the table at
 *           0x80033C00 into the table at 0x80037C00, with bit 15 set on every
 *           entry, stores 1 into the shared main-RAM byte 0x80145988 and
 *           restores entry 0 of both destinations to the source value without
 *           that bit. Last it invokes the main-executable helper func_8015D71C
 *           with 0x100. Takes no arguments and returns nothing; the 0x18-byte
 *           frame keeps ra at 0x10 across the two calls. The roles of pool
 *           bytes 0x01 and 0x02, of the two halfword tables, of the allocator's
 *           code 0x62 and of helper code 0x100 remain unestablished, so those
 *           stay literal or address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordPoolBytes0To2CopyBattleStateToWorkByte4CopyRecordByte8TripletSetByte9To3CClaimSlot62PublishWorkPointerCopy256And16HalfwordsBit15Set80145988InvokeHelper100AdvanceWorkByte1BmagicMagic07603_801EED48(
    void) {
  u16 index;
  u32 offset;
  u8 state;
  u8 *flag;
  u8 *record;
  u8 *byte8;
  u32 *source;
  u32 *work;
  u32 value;
  u8 *rearm;
  u8 *clearA;
  u8 *step;
  u8 slot;
  u32 slotOffset;
  u8 *published;
  u8 *counter;

  for (index = 0; index < 0x40; index++) {
    offset = index * 0x78;
    D_801F04A4[offset] = 0;
    D_801F04A5[offset] = 0;
    D_801F04A6[offset] = 0;
  }

  state = isBattleStateFourWithKindOtherThan4COrB4BmagicMagic07603_801EF2E8();
  flag = g_battle_work;
  flag[4] = state;

  record = D_8014638C;
  byte8 = g_battle_work;
  byte8[8] = record[8];
  source = (u32 *)record;
  work = (u32 *)g_battle_work;
  work[13] = source[13];
  work[14] = source[14];
  value = source[15];
  ((u8 *)work)[0xB] = 0;
  work[15] = value;

  rearm = g_battle_work;
  rearm[9] = 0x3C;
  clearA = g_battle_work;
  clearA[0xA] = 0;
  step = g_battle_work;
  step[1]++;

  slot = (u8)func_801E590C(1u, 0x62u);
  slotOffset = slot << 4;
  slotOffset -= slot;
  slotOffset <<= 3;
  published = g_battle_work;
  D_801EC331[slotOffset] = 0;
  D_801EC339[slotOffset] = 1;
  counter = g_battle_work;
  *(u32 *)&D_801EC3A4[slotOffset] = (u32)published;
  counter[0xB]++;

  func_8015D71C(0x100);

  for (index = 0; index < 0x100; index++) {
    PSX_PTR(u16, 0x8003AC00u)[index] = PSX_PTR(u16, 0x80036C00u)[index] | 0x8000;
  }
  for (index = 0; index < 0x10; index++) {
    PSX_PTR(u16, 0x80037C00u)[index] = PSX_PTR(u16, 0x80033C00u)[index] | 0x8000;
  }
  D_80145988 = 1;
  PSX_REF(u16, 0x8003AC00u) = PSX_REF(u16, 0x80036C00u);
  PSX_REF(u16, 0x80037C00u) = PSX_REF(u16, 0x80033C00u);
}
