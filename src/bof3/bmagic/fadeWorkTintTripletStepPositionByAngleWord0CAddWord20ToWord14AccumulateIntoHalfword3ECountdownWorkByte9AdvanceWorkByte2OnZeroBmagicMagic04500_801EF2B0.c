#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own lui 0x1F80 plus lw 0x44 pair for every access of this body: the
 * three tint channels and the angle word are each read through a fresh pair,
 * the position add and the second rotation argument share one read because the
 * two 0x34 accesses are word stores, and the countdown group and the phase
 * advance take one read each. The cell is named rather than written as a bare
 * scratchpad constant, and kept as the plain cell this target's exact sibling
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * declares it: every pair here is separated by a byte store or by a call. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared frame counter in main RAM, outside this overlay's image. Splat binds
 * the address through this target's generated undefined_syms_auto.txt row, so
 * no target-map row is needed. The exact bmagic sibling
 * stepWorkRecordAngleProjectPositionPairAndCountDownByte9OnFrameParityThenClear
 * RecordAtZeroBmagicMagic10603_801EF418 reads the same word and tests its low
 * bit as the per-frame phase that gates a countdown. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Shared per-frame update helper of the main executable. The same-target
 * sibling copyQueuedSlotPositionPairAndDistanceThenAdvanceWorkByte2WhenSlotByte2
 * IsTwoBmagicMagic04500_801EF608 calls it and discards the result.
 * Bound by this target's generated undefined_funcs_auto.txt. */
extern void func_8014D978(void);

/* Rotates the halfword angle argument and returns the scaled step word. */
extern s32 func_801782FC(s32 arg0);

/* Rotates the halfword angle argument and returns the scaled step word. */
extern s32 func_801783C8(s32 arg0);

/* @source 0x801EF2B0
 * @behavior Work-step handler, entry 2 of the eight-entry handler table at
 *           0x801EF8E0, so work-object byte 0x2 holds 2 while this body runs.
 *           It first tests the low bit of the shared frame counter 0x80143E6C
 *           and runs the main-executable per-frame update func_8014D978 only
 *           when that bit is set. It then fades the scratchpad work object
 *           published at the pointer cell 0x1F800044: tint channels 0x5D, 0x5E
 *           and 0x5F each take a byte-width read-modify-write minus six, and
 *           each channel is preceded by its own pointer-cell read. The same
 *           cell supplies the object's direction word 0xC to the shared rotator
 *           0x801782FC, whose result scaled by 0x2000 and shifted down 12
 *           (sll 13 / sra 12) is added onto word 0x34; the second rotator
 *           0x801783C8 receives the re-read word 0xC and its identically scaled
 *           result is added onto word 0x38, the 0x34 store filling that call's
 *           delay slot because both 0x34 accesses are word stores and do not
 *           force a cell reload. One further pointer-cell read then carries the
 *           distance accumulation and the countdown: word 0x14 takes word 0x14
 *           plus word 0x20, halfword 0x3E takes itself plus the new word 0x14
 *           at halfword width, and countdown byte 0x9 is decremented once and
 *           its truncated value tested against zero; when it reaches zero a
 *           final pointer-cell read increments work byte 0x2, which advances
 *           the effect to the next handler. Takes no arguments, returns
 *           nothing, and the 0x18-byte frame only keeps $ra across the three
 *           calls.
 *
 *           The relocation-different magic012 copy at 0x801EF3C4 carries the
 *           same rotator, accumulation and countdown group but adds a frame
 *           byte 0x0A increment, a shared helper call and an overlay sprite
 *           emitter, so it is shape evidence only; this body was written from
 *           the original bytes 0x801EF2B0 through 0x801EF3D4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fadeWorkTintTripletStepPositionByAngleWord0CAddWord20ToWord14AccumulateIntoHalfword3ECountdownWorkByte9AdvanceWorkByte2OnZeroBmagicMagic04500_801EF2B0(void) {
  u8 *work;
  s32 step;
  u8 count;

  if (D_80143E6C & 1) {
    func_8014D978();
  }

  D_1F800044[0x5D] -= 6;
  D_1F800044[0x5E] -= 6;
  D_1F800044[0x5F] -= 6;

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
