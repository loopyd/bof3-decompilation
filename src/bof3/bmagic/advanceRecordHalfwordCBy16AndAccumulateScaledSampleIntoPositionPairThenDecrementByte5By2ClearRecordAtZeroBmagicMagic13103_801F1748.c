#include "bof3/bof3.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */
extern u8 D_801F1C54[]; /* @source 0x801F1C54 @kind data */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u32 word80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */
extern u32 *record801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared angular sample helpers, taken as sines of the published phase. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Clears the active work record's bytes 0 through 4. */
extern void clearRecordBytes0Through4BmagicMagic13103_801F1AC4(void);

/* @source 0x801F1748
 * @behavior Reads the record pointer cell at 0x801F24E0 and adds 0x10 to the
 * record halfword at offset 0xC with an unsigned load, an immediate add and a
 * store. It then uses the record byte at offset 0x7 as an unsigned index into
 * the byte table at 0x801F1C54; the selected byte shifted left six becomes the
 * phase published to the shared scratchpad halfword at 0x1F800006 and the same
 * shift is passed to the shared angular helper at 0x801782FC. That helper's
 * result times the signed halfword at offset 0xC, shifted right three and added
 * to the word at 0x80149308, forms the record word at offset 0x14. The phase
 * read back from 0x1F800006 as a signed halfword feeds the second shared
 * angular helper at 0x801783C8, whose result times the same signed halfword at
 * offset 0xC, shifted right three and added to the word at 0x8014930C, forms
 * the record word at offset 0x18. It then subtracts 2 from the record byte at
 * offset 0x5 and tests the stored byte truncated to eight bits, so the
 * truncated register and not a reload supplies the compared value. Only when
 * that countdown reaches zero does it read the shared record pointer cell at
 * 0x801EB4E0, decrement that record's byte at offset 0xB by one with the store
 * filling the call delay slot, and call clearRecordBytes0Through4, which clears
 * this work record's bytes 0 through 4. Takes no arguments, returns nothing and
 * keeps its 0x18-byte frame only for $ra and the scratchpad phase register
 * across the three calls. Its address is the third word (index 2) of the
 * three-entry handler table at 0x801F1C94 that
 * dispatchRecordByte2HandlerThenDecrementCountdownOrDrawWhenActiveBmagicMagic13103_801F14A0
 * indexes with the record byte at offset 0x2 scaled by four. The roles of
 * record bytes 0x5 and 0x7 and of the record halfword at 0xC remain
 * offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceRecordHalfwordCBy16AndAccumulateScaledSampleIntoPositionPairThenDecrementByte5By2ClearRecordAtZeroBmagicMagic13103_801F1748(void) {
  u8 speed;
  s32 sine;
  s32 cosine;
  s16 *phase;

  *(u16 *)(workRecord801F24E0 + 0xC) += 0x10;
  speed = D_801F1C54[workRecord801F24E0[7]];
  phase = &D_1F800006;
  *phase = speed << 6;
  sine = func_801782FC(speed << 6);
  *(s32 *)(workRecord801F24E0 + 0x14) =
      word80149308[0] + ((sine * *(s16 *)(workRecord801F24E0 + 0xC)) >> 3);
  cosine = func_801783C8(*phase);
  *(s32 *)(workRecord801F24E0 + 0x18) =
      D_8014930C[0] + ((cosine * *(s16 *)(workRecord801F24E0 + 0xC)) >> 3);
  if ((workRecord801F24E0[5] -= 2) == 0) {
    ((u8 *)record801EB4E0)[0xB] -= 1;
    clearRecordBytes0Through4BmagicMagic13103_801F1AC4();
  }
}
