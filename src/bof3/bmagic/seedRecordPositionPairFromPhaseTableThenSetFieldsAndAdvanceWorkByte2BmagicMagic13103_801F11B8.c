#include "bof3/bof3.h"

extern u8 *workRecord801F24E0; /* @source 0x801F24E0 @kind data */
extern u8 D_801F1C38[]; /* @source 0x801F1C38 @kind data */
extern s16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u32 word80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */
extern u32 *record801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared angular sample helpers, taken as sines of the published phase. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* @source 0x801F11B8
 * @behavior Reads the record pointer cell at 0x801F24E0 and uses the record
 * byte at offset 0x7 as an unsigned index into the byte table at 0x801F1C38;
 * the selected byte shifted left seven becomes the phase published to the
 * shared scratchpad halfword at 0x1F800006, and the same shift is passed to
 * the shared angular helper at 0x801782FC. That helper's result is scaled by
 * eight and added to the word at 0x80149308 to form the record word at offset
 * 0x14. The phase is then read back from 0x1F800006 as a signed halfword,
 * passed to the second shared angular helper at 0x801783C8, and that result
 * scaled by eight and added to the word at 0x8014930C forms the record word at
 * offset 0x18. The record halfword at offset 0xC is then set to 0x40, the byte
 * at offset 0x5 to 0x10, and word 0x3C of the record reached through the
 * pointer cell at 0x801EB4E0 copied into the record word at offset 0x1C. Two
 * closing statements re-read the cell at 0x801F24E0 to store 0x14 into the
 * record byte at offset 0x6 and to increment the record byte at offset 0x2.
 * Takes no arguments and returns nothing; the roles of record bytes 0x5, 0x6
 * and 0x7 and of the record halfword at 0xC remain offset-based. Its address
 * is the first word of the three-entry handler table at 0x801F1C48.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRecordPositionPairFromPhaseTableThenSetFieldsAndAdvanceWorkByte2BmagicMagic13103_801F11B8(
    void) {
  u8 speed;
  s32 sine;
  s32 cosine;
  s16 *phase;
  u32 *words;

  speed = D_801F1C38[workRecord801F24E0[7]];
  phase = &D_1F800006;
  *phase = speed << 7;
  sine = func_801782FC(speed << 7);
  *(s32 *)(workRecord801F24E0 + 0x14) = word80149308[0] + ((sine << 6) >> 3);
  cosine = func_801783C8(*phase);
  words = (u32 *)workRecord801F24E0;
  words[6] = D_8014930C[0] + ((cosine << 6) >> 3);
  words[7] = record801EB4E0[15];
  ((u16 *)words)[6] = 0x40;
  ((u8 *)words)[5] = 0x10;
  ((u8 *)workRecord801F24E0)[6] = 0x14;
  ((u8 *)workRecord801F24E0)[2]++;
}
