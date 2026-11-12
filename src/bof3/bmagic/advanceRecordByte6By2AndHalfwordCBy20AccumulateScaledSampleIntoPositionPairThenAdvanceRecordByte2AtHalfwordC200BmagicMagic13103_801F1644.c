#include "bof3/bof3.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */
extern u8 D_801F1C54[]; /* @source 0x801F1C54 @kind data */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u32 word80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Shared angular sample helpers, taken as sines of the published phase. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* @source 0x801F1644
 * @behavior Reads the record pointer cell at 0x801F24E0 and adds 2 to the
 * record byte at offset 0x6, then re-reads the cell and adds 0x20 to the record
 * halfword at offset 0xC with an unsigned load, an immediate add and a store.
 * It uses the record byte at offset 0x7 as an unsigned index into the byte
 * table at 0x801F1C54; the selected byte shifted left six becomes the phase
 * published to the shared scratchpad halfword at 0x1F800006 and the same shift
 * is passed to the shared angular helper at 0x801782FC. That helper's result
 * times the signed halfword at offset 0xC, shifted right three and added to the
 * word at 0x80149308, forms the record word at offset 0x14. The phase read back
 * from 0x1F800006 as a signed halfword feeds the second shared angular helper
 * at 0x801783C8, whose result times the same signed halfword at offset 0xC,
 * shifted right three and added to the word at 0x8014930C, forms the record
 * word at offset 0x18. A closing guard compares the record halfword at offset
 * 0xC with 0x200 and, only on equality, increments the record byte at offset
 * 0x2. Takes no arguments, returns nothing and keeps its 0x18-byte frame only
 * for $ra and the scratchpad phase register across the two calls. Its address
 * is the second word (index 1) of the three-entry handler table at 0x801F1C94.
 * The roles of record bytes 0x2, 0x6 and 0x7 and of the record halfword at
 * 0xC remain offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceRecordByte6By2AndHalfwordCBy20AccumulateScaledSampleIntoPositionPairThenAdvanceRecordByte2AtHalfwordC200BmagicMagic13103_801F1644(
    void) {
  u8 speed;
  s32 sine;
  s32 cosine;
  s16 *phase;

  workRecord801F24E0[6] += 2;
  *(u16 *)(workRecord801F24E0 + 0xC) += 0x20;
  speed = D_801F1C54[workRecord801F24E0[7]];
  phase = &D_1F800006;
  *phase = speed << 6;
  sine = func_801782FC(speed << 6);
  *(s32 *)(workRecord801F24E0 + 0x14) =
      word80149308[0] + ((sine * *(s16 *)(workRecord801F24E0 + 0xC)) >> 3);
  cosine = func_801783C8(*phase);
  *(s32 *)(workRecord801F24E0 + 0x18) =
      D_8014930C[0] + ((cosine * *(s16 *)(workRecord801F24E0 + 0xC)) >> 3);
  if (*(s16 *)(workRecord801F24E0 + 0xC) == 0x200) {
    workRecord801F24E0[2] += 1;
  }
}
