#include "bof3/bof3.h"

/* Record pool of this overlay at 0x801F07F8: 48 records of stride 0x78 whose
 * byte 0 is the flags byte that
 * reserveFirstFreeRecordBmagicMagic09203_801F0758 tests for bit 0 and sets on
 * reservation, and whose bytes 0x0B and word 0x74 the claimant at 0x801EF0CC
 * fills. This body clears bytes 0x00, 0x01 and 0x02 of every record, so the
 * three separate symbols the original addresses are the pool base and the two
 * bytes that follow it, each bound by this target's generated
 * undefined_syms_auto.txt. */
extern u8 D_801F07F8[]; /* @source 0x801F07F8 @kind data */
extern u8 D_801F07F9[]; /* @source 0x801F07F9 @kind data */
extern u8 D_801F07FA[]; /* @source 0x801F07FA @kind data */

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The body reads the cell once per site: the record count store here, the
 * work-pointer publication inside each claimed slot, and the count increment
 * after it, which keeps its own read because the store through the previously
 * loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384 in main RAM, outside this overlay's
 * image and bound by this target's generated undefined_syms_auto.txt. The
 * sibling raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09203_801EEF1C
 * reads the same byte as the shared battle target. Bit 0x40 of it selects which
 * roster this body queues: with the bit set it walks the eight enemy slots,
 * without it the three party slots. */
extern u8 D_80146384; /* @source 0x80146384 @kind data */

/* Shared main-RAM byte at 0x80145988, bound by this target's generated
 * undefined_syms_auto.txt. The exact sibling
 * clearRecordPoolBytes0To2PublishRecordPoseSetGlobalByte80145988ToOneCopy16HalfwordsSettingBit15RequestResource1ThenAdvanceHandlerIndexBmagicMagic08103_801EED44
 * publishes 1 into the same byte in the same tail position; nothing in this
 * target establishes its role, so it keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* First halfword of the shared main-RAM halfword table at 0x8003AC00, bound by
 * this target's generated undefined_syms_auto.txt. The body zeroes this entry
 * and then fills the fifteen entries that follow it from the table at
 * 0x80036C02 with bit 15 set, both reached by pointer, so the two table bases
 * stay address-anchored literals and no map row is added. */
extern u16 D_8003AC00; /* @source 0x8003AC00 @kind data */

/* Action-slot pool at 0x801EC330, whose 0x78-byte stride these accesses scale
 * by. The original reaches each field as `lui $at,%hi(ADDR)` + `addu
 * $at,$at,index*0x78` + `sb|sw %lo(ADDR)($at)` rather than through one shared
 * record base, so every addressed field keeps its own flat symbol: byte +0x03,
 * byte +0x09, the words at +0x34, +0x38, +0x3C and word +0x74. The sibling
 * queueSlotPerEligibleEnemyAndPartyBattlerSetByte1To2OrdinalByte4SelectorByteBAndRecordPointerBmagicMagic06903_801F0274
 * stamps the same pool with the same stride, and
 * copyRecordTripletIntoWorkQueueSixActingBattlerSlotsStampOrdinalAndStepPublishWorkCopyStatBytesAndRaiseFlag40BmagicMagic01703_801EEC98
 * fills the same +0x34/+0x38/+0x3C triplet and the word +0x74 pointer. */
extern u8 D_801EC333[]; /* @source 0x801EC333 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Enemy battler records at 0x801EB630 with the 0x118-byte enemy stride, read
 * here at words +0x34, +0x38 and +0x3C only. Splat binds the three word
 * addresses through this target's generated undefined_syms_auto.txt, and the
 * enemy stride and base are the ones the sibling
 * queueSlotPerEligibleEnemyAndPartyBattlerSetByte1To2OrdinalByte4SelectorByteBAndRecordPointerBmagicMagic06903_801F0274
 * walks. */
extern u8 D_801EB664[]; /* @source 0x801EB664 @kind data */
extern u8 D_801EB668[]; /* @source 0x801EB668 @kind data */
extern u8 D_801EB66C[]; /* @source 0x801EB66C @kind data */

/* Party battler records at 0x80145E90 with the 0x140-byte party stride, read
 * here at words +0x34, +0x38 and +0x3C only. Same generated bindings and the
 * party base and stride the sibling above walks. */
extern u8 D_80145EC4[]; /* @source 0x80145EC4 @kind data */
extern u8 D_80145EC8[]; /* @source 0x80145EC8 @kind data */
extern u8 D_80145ECC[]; /* @source 0x80145ECC @kind data */

/* Shared main-executable helpers, both outside this payload. The availability
 * test reports a nonzero value for an inactive battler or one whose 0x4000 flag
 * is set, and the slot allocator at 0x801E590C returns the index of the entry it
 * claimed, or 0xFF when the pool holds no free entry. Splat binds both through
 * this target's generated undefined_funcs_auto.txt, and the sibling above
 * declares them with the same shapes. */
extern u32 func_801DB524(u32 arg0); /* @source 0x801DB524 */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x801EED04
 * @behavior Stage 0 of this overlay. The frame dispatcher func_801EEC04 builds
 *           the two-word pointer run func_801EED04/func_801EEFB4 in its frame
 *           and calls the word selected by scratchpad work byte 1 scaled by
 *           four, so this body runs while that byte is 0 and no jal in the
 *           payload names it. It first clears bytes 0x00, 0x01 and 0x02 of all
 *           48 records of the 0x78-byte-stride record pool at 0x801F07F8 with
 *           one sb per byte, computing the record offset as the counter times
 *           0x78 and testing that counter against 0x30 at the loop bottom. It
 *           then zeroes work byte 0xB, the record count the overlay's claimants
 *           increment, and reads the shared battle target byte at 0x80146384:
 *           with bit 0x40 set it walks the eight enemy slots one record stride
 *           0x118 apart, without it the three party slots one stride 0x140
 *           apart. A slot is skipped when the shared availability test reports
 *           the battler unusable, called with the slot ordinal plus three
 *           masked to a byte on the enemy walk and with the ordinal itself
 *           masked to a byte on the party walk. Each remaining slot is claimed
 *           from the allocator with the request class 0x28, and the claimed
 *           entry of the action-slot pool at 0x801EC330 receives byte +0x03
 *           with the walk ordinal, byte +0x09 with the running count of claimed
 *           slots stepped by 0x10 and raised by one, words +0x34, +0x38 and
 *           +0x3C with the battler record triplet, and word +0x74 with the
 *           work-object pointer read from the cell at 0x1F800044; work byte
 *           0xB is incremented through its own read of that cell. The common
 *           tail zeroes the first halfword of the shared main-RAM table at
 *           0x8003AC00, copies the fifteen halfwords that follow it from the
 *           table at 0x80036C02 with bit 15 set, publishes 1 into the shared
 *           main-RAM byte 0x80145988 and increments work byte 1 through a final
 *           read of the cell, so the next frame runs the following entry of the
 *           dispatcher's run. The 0x28-byte frame keeps ra at 0x20 and s0 to s3
 *           at 0x10 to 0x1C across the helper calls. Takes no arguments and
 *           returns nothing. The roles of the action-slot bytes and of the two
 *           main-RAM tables are not established here, so they stay
 *           address-anchored.
 * @status partial
 * @match 71.10
 * @residual 123/173 instructions, 692 of 688 bytes. The first difference is
 *           +0x0004: the original entry-block zeroing `move $a0,$zero` has no
 *           corresponding instruction. The enemy arm also differs in the
 *           ordinal/step register allocation and the record-triplet load/store
 *           scheduling. Both roster cursors advance regardless of eligibility;
 *           only eligible battlers claim slots and advance the step byte.
 */
void clearRecordPoolBytes0To2AndWorkCountThenQueueSlotPerEligibleBattlerCopyTripletSetByte3StepByte9AndWorkPointerThenCopy15HalfwordsSettingBit15SetGlobalByte80145988AndAdvanceWorkByte1BmagicMagic09203_801EED04(void) {
  u32 index;
  u32 offset;
  u32 recordOffset;
  u8 battler;
  u8 step;
  u32 slotOffset;
  u16 *src;
  u16 *dst;
  u32 selector;
  selector = 0;
  for (index = 0; index < 0x30; index++) {
    offset = index * 0x78;
    D_801F07F8[offset] = 0;
    D_801F07F9[offset] = 0;
    D_801F07FA[offset] = 0;
  }

  g_battle_work[0xB] = 0;

  if (D_80146384 & 0x40) {
    index = 0;
    recordOffset = 0;
    step = 1;
    do {
      battler = index;
      selector = index + 3;
      if ((u8)func_801DB524(selector & 0xFF) == 0) {
        slotOffset = ((u8)func_801E590C(1u, 0x28u)) * 0x78u;
        D_801EC333[slotOffset] = battler;
        *(u32 *)&D_801EC364[slotOffset] = *(u32 *)&D_801EB664[recordOffset];
        *(u32 *)&D_801EC368[slotOffset] = *(u32 *)&D_801EB668[recordOffset];
        *(u32 *)&D_801EC36C[slotOffset] = *(u32 *)&D_801EB66C[recordOffset];
        *(u32 *)&D_801EC3A4[slotOffset] = (u32)g_battle_work;
        D_801EC339[slotOffset] = step;
        g_battle_work[0xB]++;
        step += 0x10;
      }
      index++;
      recordOffset += 0x118;
    } while (index < 0x8);
  } else {
    index = 0;
    recordOffset = 0;
    step = 1;
    do {
      selector = index;
      if ((u8)func_801DB524(selector & 0xFF) == 0) {
        slotOffset = ((u8)func_801E590C(1u, 0x28u)) * 0x78u;
        D_801EC333[slotOffset] = index;
        *(u32 *)&D_801EC364[slotOffset] = *(u32 *)&D_80145EC4[recordOffset];
        *(u32 *)&D_801EC368[slotOffset] = *(u32 *)&D_80145EC8[recordOffset];
        *(u32 *)&D_801EC36C[slotOffset] = *(u32 *)&D_80145ECC[recordOffset];
        *(u32 *)&D_801EC3A4[slotOffset] = (u32)g_battle_work;
        D_801EC339[slotOffset] = step;
        g_battle_work[0xB]++;
        step += 0x10;
      }
      index++;
      recordOffset += 0x140;
    } while (index < 0x3);
  }

  D_8003AC00 = 0;
  index = 1;
  dst = (u16 *)0x8003AC02;
  src = (u16 *)0x80036C02;
  do {
    *dst = *src | 0x8000;
    src++;
    index++;
    dst++;
  } while (index < 0x10);

  D_80145988 = 1;
  g_battle_work[1]++;
}
