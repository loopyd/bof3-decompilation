#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds no name at the
 * address, so the D_ spelling auto-binds with no map row, as it does for the
 * scratchpad lifts of this target that already measure exact. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared angular helpers of the main executable, outside this overlay's image.
 * Each rotates the halfword angle argument and returns the scaled step word.
 * Both addresses are bound by this target's generated undefined_funcs_auto.txt,
 * so neither needs a target-map row. */
extern s32 func_801782FC(s32 arg0); /* @source 0x801782FC */
extern s32 func_801783C8(s32 arg0); /* @source 0x801783C8 */

/* @source 0x801F0248
 * @behavior Entry 23 of the overlay handler word run that starts at 0x801EEC04,
 *           the run of code pointers the overlay header stores at 0x801EEC40:
 *           the run stores this address at 0x801EEC60, and the payload holds
 *           that one pointer word and no jal or j word to this address, so the
 *           call site is the indirect dispatch through the copied run. The
 *           0xB8-byte body is byte-identical to the entry 15 body at
 *           0x801EFCC4 of this payload (cmp over all 184 bytes), which supplied
 *           the statement order. It reads the scratchpad work object through
 *           the pointer cell 0x1F800044, projects that object's direction word
 *           0xC through the shared rotator 0x801782FC, scales the result by
 *           0x2000 and shifts it down 12 (sll 13 / sra 12), and adds it onto
 *           word 0x34; the second rotator 0x801783C8 receives the re-read word
 *           0xC and its identically scaled result is added onto word 0x38, the
 *           0x34 store filling that call's delay slot. One further pointer-cell
 *           read then carries the distance accumulation and the countdown: word
 *           0x14 takes word 0x14 plus word 0x20, halfword 0x3E takes itself plus
 *           the new word 0x14 at halfword width, and countdown byte 0x9 is
 *           decremented once and its truncated value tested against zero; when
 *           it reaches zero a final pointer-cell read increments work byte 0x2,
 *           the handler index. The cell is re-read before each store group
 *           because a store through the loaded pointer may alias it, and the
 *           first cell load precedes frame creation because the rotator
 *           argument needs no stack; the 0x18-byte frame only keeps $ra across
 *           the two calls. Takes no arguments and returns nothing; work-field
 *           roles stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairByAngleWord0CAddWorkWord20ToWord14AccumulateIntoHalfword3ECountdownWorkByte9ThenAdvanceWorkByte2BmagicMagic01500_801F0248(void) {
  u8 *work;
  s32 step;
  u8 count;

  *(s32 *)(D_1F800044 + 0x34) +=
      (func_801782FC(*(s32 *)(D_1F800044 + 0xC)) * 0x2000) >> 12;

  step = (func_801783C8(*(s32 *)(D_1F800044 + 0xC)) * 0x2000) >> 12;

  work = D_1F800044;
  *(s32 *)(work + 0x38) += step;
  *(s32 *)(work + 0x14) += *(s32 *)(work + 0x20);
  *(u16 *)(work + 0x3E) += *(s32 *)(work + 0x14);
  count = (u8)(work[9] - 1);
  work[9] = count;
  if (count == 0) {
    D_1F800044[2]++;
  }
}
