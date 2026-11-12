#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound at that address by this
 * target's map row g_battle_work; the constant address folds into each load as
 * lui $vN,0x1F80 / lw $vN,68($vN). The original loads the cell once before the
 * 0x20-byte frame is created and keeps that pointer in $a0 for the whole motion
 * group, then re-materialises the cell for every later group because a store
 * through the loaded pointer may republish it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans load it, so no target-map row is needed;
 * only its word 0x3C is read here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared halfword channel at 0x801492D8 in the main executable, outside this
 * overlay's payload. The original materialises the address with lui / addiu and
 * reads the channel at displacement 0 (lhu), so the body takes a pointer to it
 * instead of folding the address into the load. */
extern u16 D_801492D8; /* @source 0x801492D8 @kind unknown */

/* Fields of the 32-record, 0x78-byte-stride pool at 0x801F0578 in this
 * overlay's own image, whose free records
 * reserveFirstFreeRecordBmagicMagic05303_801F0494 hands out. The original
 * reaches each seeded field as its own symbol: lui $at,%hi(0x801F05xx) /
 * addu $at,$at,index*0x78 / sb or sw %lo(0x801F05xx)($at), so the three
 * fields stay three flat addresses rather than one base plus offset. */
extern u8 D_801F0579[]; /* @source 0x801F0579 @kind data */
extern u8 D_801F0583[]; /* @source 0x801F0583 @kind data */
extern u8 D_801F05EC[]; /* @source 0x801F05EC @kind data */

/* This target's own pool claim helper, lifted in
 * reserveFirstFreeRecordBmagicMagic05303_801F0494.c: it returns the index of
 * the first record whose flags byte has bit 0 clear, or 0xFF when all 32
 * records are taken, and the original zero-extends that byte at the call site
 * (andi 0xFF) before both the 0xFF test and the stride scaling. */
u8 reserveFirstFreeRecordBmagicMagic05303_801F0494(void);

/* Main-executable helper at 0x8015D71C, outside this overlay's image and bound
 * by this target's generated undefined_funcs_auto.txt. The handler family
 * passes it a constant selector; here the selector is 0x100 or 0x101, chosen by
 * bit 0 of work byte 0x04. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* Main-executable helper at 0x8014D6B8, outside this overlay's image and bound
 * by this target's generated undefined_funcs_auto.txt. The original passes the
 * constant 1 in the jal delay slot. */
extern void func_8014D6B8(s32 arg0); /* @source 0x8014D6B8 */

/* @source 0x801EF420
 * @behavior Handler at entry index 2 of the ten-pointer code run
 *   0x801F0550..0x801F0574 (0x801EF0A8, 0x801EF174, 0x801EF420, 0x801EF5C0,
 *   0x801EF674, 0x801EF6D8, 0x801EFD94, 0x801EFE1C, 0x801EFF24, 0x801F0018):
 *   its address is the word at 0x801F0558. That run's only in-payload readers
 *   are the byte-1 dispatchers dispatchStateByte1Table0550BmagicMagic05303
 *   801EF064 and dispatchStateByte1Table0568BmagicMagic05303_801EFD50, which
 *   index it with the work record's unsigned byte 1 and call the selected cell
 *   as void(void), so this body takes no arguments and returns nothing.
 *   It reads the scratchpad work-object pointer cell at 0x1F800044 once before
 *   the 0x20-byte frame is created and, through that one pointer, adds work word
 *   0x0C to work word 0x34, adds work word 0x10 to work word 0x38, subtracts
 *   work word 0x20 from work word 0x14, stores the shared battle record's word
 *   0x3C plus the new work word 0x14 into work word 0x3C, and adds 8 to work
 *   byte 0x0B at byte width. The emitted order is 0x34, 0x14, 0x38, 0x0B, 0x3C:
 *   the word view below reaches the five word fields by word index (0x03, 0x04,
 *   0x05, 0x08, 0x0D, 0x0E, 0x0F for bytes 0x0C, 0x10, 0x14, 0x20, 0x34, 0x38,
 *   0x3C) and the byte field through the same pointer cast back to bytes, and
 *   that view plus the 0x38 statement written ahead of the 0x14 pair is what
 *   leaves the motion pointer in $a0: the byte-indexed spelling of the same
 *   statements put the pointer in $a2 and kept a load-delay nop at the second
 *   cell read, and the same byte view with the 0x14 pair ahead of 0x38 measured
 *   99 of 104 instructions at the original size with the pointer already in
 *   $a0. The sibling 0x801EF728 reads work words 0x34 and 0x38
 *   as the GTE translation pair (sra 9, minus 0x4000) and the lift at 0x801F05FC
 *   seeds the same two words from the shared fixed-point channel pair, so the
 *   two words step a position pair; the roles of work words 0x0C, 0x10, 0x14,
 *   0x20 and 0x3C and of work byte 0x0B are not established, so they stay
 *   offset-anchored.
 *   It then re-reads the cell and decrements work byte 0x09 at byte width,
 *   testing the stored byte truncated to eight bits (andi $v0,$v0,0xFF /
 *   bnez), so the spawn body below runs only on the tick that countdown reaches
 *   zero. On that tick it re-reads the cell and passes 0x100 or 0x101 to
 *   func_8015D71C according to bit 0 of work byte 0x04, clears work byte 0x04,
 *   and in an eight-iteration loop claims a free record of the pool at
 *   0x801F0578 through reserveFirstFreeRecordBmagicMagic05303_801F0494,
 *   skipping the iteration when the claim returns 0xFF. For a claimed record it
 *   scales the index by the 0x78-byte stride, clears record byte 0x01, stores
 *   the loop ordinal in record byte 0x0B, and publishes the work pointer into
 *   record word 0x74 from that iteration's own cell read. A second cell read,
 *   issued before that store, advances work byte 0x04, counting the records this
 *   tick published; the two reads stay two lui/lw pairs because a store between
 *   them may republish the cell.
 *   After the loop it calls func_8014D6B8 with the constant 1, materialises the
 *   address of the shared halfword 0x801492D8 and adds 0x14 to it at halfword
 *   width, re-reads the cell to seed work byte 0x09 with 4, and re-reads the
 *   cell once more to advance work byte 0x02, which moves the dispatcher to the
 *   next work-byte-2 step. Frame 0x20 keeps $ra at 0x18, $s1 at 0x14 - the
 *   hoisted 0xFF claim failure value - and $s0 at 0x10, the loop ordinal; the
 *   body returns through the jr $ra at 0x801EF5B8 and reads or writes no other
 *   state. Every cell read after the motion group is a separate lui/lw pair,
 *   because a store through the previously loaded pointer may republish the
 *   cell. The 0x1A0-byte span has no byte-identical window in any extracted
 *   payload under out/binaries (only its own occurrence at payload offset
 *   0x820), and its jal words reach this overlay's own pool helper, so the clone
 *   lever offered no source; the statement order, operand forms, offset
 *   spelling and declaration style follow the same-family sibling
 *   decrementWorkByte9ThenClaimThreePoolRecordsSeedingSlotOrdinalAndWorkPointer
 *   InvokeHelperAt100AndAdvanceWorkByte2WhenZeroBmagicMagic09203_801EF0CC
 *   (countdown test, pool claim, record seeding and published work pointer) and
 *   the sibling stepHalfword801492D8ByWorkByte9LowBitDecrementWorkByte9
 *   ThenSeedNegative682OnZeroSetByte9To22AdvanceWorkByte2BmagicMagic22203
 *   801EF274 (address-of-halfword step and work byte 0x02 advance); both carried
 *   exact status in the bmagic sources when this body was seeded, and each
 *   differs from this span in its own constants and stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairAndWord3CAddEightToWorkByteBThenSpawnEightFreePoolRecordsOnByte9CountdownInvokeHelper100Or101StepHalfword801492D8By20SetByte9To4AdvanceWorkByte2BmagicMagic05303_801EF420(void) {
  s32 *words;
  u8 *spawn;
  u8 *counter;
  u16 *ramp;
  u8 slot;
  u8 index;
  u32 offset;
  s32 vz;

  words = (s32 *)g_battle_work;
  words[0x0D] += words[0x03];
  words[0x0E] += words[0x04];
  vz = words[0x05] - words[0x08];
  words[0x05] = vz;
  words[0x0F] = *(s32 *)(D_801EB4E0 + 0x3C) + vz;
  ((u8 *)words)[0x0B] += 8;
  if (--g_battle_work[0x09] == 0) {
    func_8015D71C((g_battle_work[0x04] & 1) ? 0x101 : 0x100);
    g_battle_work[0x04] = 0;
    for (slot = 0; slot < 8; slot++) {
      index = reserveFirstFreeRecordBmagicMagic05303_801F0494();
      if (index == 0xFF) {
        continue;
      }
      offset = index * 0x78;
      spawn = g_battle_work;
      D_801F0579[offset] = 0;
      D_801F0583[offset] = slot;
      counter = g_battle_work;
      *(u32 *)&D_801F05EC[offset] = (u32)spawn;
      counter[0x04]++;
    }
    func_8014D6B8(1);
    ramp = &D_801492D8;
    *ramp += 0x14;
    g_battle_work[0x09] = 4;
    g_battle_work[0x02]++;
  }
}
