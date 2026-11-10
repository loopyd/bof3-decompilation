#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map as g_battle_work. Each store through the loaded pointer may alias the
 * cell itself, so the compiler re-materializes the address (lui $vN,0x1F80 /
 * lw $vN,68($vN)) for the byte-0xA increment's second read, for the argument
 * of the first rotator together with the word-0x34 accumulation, for the
 * word-0x38 and halfword-0x3E group, for the post-call halfword comparison
 * and once for each of the three trailing byte clears, which is where the
 * original puts those eight load pairs. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The exact same-target lift
 * decrementWorkByte9ByTwoIncrementWorkByteAOnFrameParityThenDecrementGlobalWork
 * ByteBAndClearWorkBytes0To4AtOneBmagicMagic09203_801EFA24 decrements the same
 * record's byte 0xB and clears the same work bytes in the same order, and
 * splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no map row or WEAK_SYMBOL_AT binding is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared rotators and readiness helper, all outside this overlay's image and
 * bound through this target's generated undefined_funcs_auto.txt. The 4.12
 * rotator pair at 0x801782FC and 0x801783C8 is the one the exact sibling
 * advanceWorkByteAStepWorkPositionPairByAngleWord0CAddWord20ToWord14
 * AccumulateIntoHalfword3ECountdownWorkByte9AdvanceWorkByte2OnZeroAndDrawSprite
 * Mode1BmagicMagic01200_801EF3C4 calls, and 0x8014DAEC reports the readiness
 * byte the exact lift
 * advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C tests. */
s32 func_801782FC(s32 angle);
s32 func_801783C8(s32 angle);
u8 func_8014DAEC(void);

/* @source 0x801EFB68
 * @behavior Step handler of the battle work object reached through the
 *           scratchpad pointer cell 0x1F800044. It advances the object's phase
 *           byte 0xA by one at byte width, then samples that phase twice: the
 *           low six bits of the incremented byte shifted left eight feed both
 *           the shared rotator at 0x801782FC and the second rotator at
 *           0x801783C8, and each 4.12 result scaled by eight and shifted back
 *           down by three is added onto the projected position words 0x34 and
 *           0x38 in that call order, the word-0x34 store filling the second
 *           call's delay slot. One further pointer-cell read then serves the
 *           word-0x38 accumulation and the halfword-0x3E decrement: halfword
 *           0x3E loses eight. When the shared readiness helper 0x8014DAEC
 *           reports non-zero, or when the re-read halfword 0x3E read as a
 *           signed halfword is less than the accumulated word 0x14, the body
 *           reads the shared battle
 *           work record at 0x801EB4E0, decrements that record's byte 0xB at
 *           byte width and clears work bytes 0x00, 0x01 and 0x02, deactivating
 *           the effect. Takes no arguments, returns nothing, and the 0x18-byte
 *           frame only keeps $ra across the three calls. The roles of work
 *           words 0x14, 0x34 and 0x38, of halfword 0x3E and of record byte 0xB
 *           are not established beyond their offset, so they stay
 *           offset-anchored. A comparison of this 292-byte span against every
 *           extracted payload under out/binaries, with its three call words
 *           masked, found no twin, so no sibling is a clone source; the
 *           statement order and operand forms follow the exact siblings named
 *           above.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByteAAccumulateSamplePairIntoPositionWordsSubtract8FromHalfword3EThenDecrementGlobalWorkByteBAndClearWorkAtHelperReadyOrBelowWord14BmagicMagic10503_801EFB68(
    void) {
  u8 *work;
  s32 sample;
  s32 phase;

  g_battle_work[0xA]++;

  sample = func_801782FC((g_battle_work[0xA] & 0x3F) << 8);

  phase = (g_battle_work[0xA] & 0x3F) << 8;
  *(s32 *)(g_battle_work + 0x34) += (sample * 8) >> 3;

  sample = func_801783C8(phase);

  work = g_battle_work;
  *(s32 *)(work + 0x38) += (sample * 8) >> 3;
  *(u16 *)(work + 0x3E) -= 8;

  if (func_8014DAEC() != 0u ||
      *(s16 *)(g_battle_work + 0x3E) < *(s32 *)(g_battle_work + 0x14)) {
    D_801EB4E0[0x0B]--;
    g_battle_work[0x00] = 0;
    g_battle_work[0x01] = 0;
    g_battle_work[0x02] = 0;
  }
}
