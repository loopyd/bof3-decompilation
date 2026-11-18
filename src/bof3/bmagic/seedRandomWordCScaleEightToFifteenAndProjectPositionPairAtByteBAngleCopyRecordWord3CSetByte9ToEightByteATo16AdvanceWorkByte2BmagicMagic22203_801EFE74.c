#include "bof3/bof3.h"

/* PsyQ libc random source at 0x8017E3D4, the address this body's single jal
 * word reaches. The SDK map binds it, so no target-map row is owed. */
extern int rand(void);

/* Scratchpad work-record pointer cell at 0x1F800044, written with the name every
 * other lifted body of this target gives it. The original re-materialises the
 * cell with its own lui 0x1F80 plus lw 0x44 pair at each of the five reads it
 * makes, so the spelling stays one cell read per statement group: the first read
 * feeds both the work word 0x0C store and the work byte 0xB load, because no
 * store separates them in the original. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad angle halfword at 0x1F800004. The seed path stores work byte 0xB
 * shifted left nine into it at halfword width and hands the same signed
 * halfword to the first rotator; the second rotator reads the cell back, so the
 * original folds the base once into a saved register (lui $s0,0x1F80 / addiu
 * $s0,$s0,4) instead of re-materialising it at the two accesses. The generated
 * undefined-syms file already binds the address, so this lift adds no
 * target-map row. */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Battle record pointer published in main RAM, outside this overlay's
 * 0x801EEC00-0x801F21E4 image, so the address is the same cell every bmagic
 * overlay reads and the generated undefined-syms file binds it. Its role is not
 * proven, so the name stays address-anchored. Only the record words 0x34, 0x38
 * and 0x3C are read, and the second group holds the pointer in one local across
 * both of its reads. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The overlay rotation pair: arg0 is the signed halfword angle taken from the
 * scratchpad cell and each call returns the projected step for one axis, which
 * this body multiplies by work word 0x0C without any shift. Both callees lie in
 * the shared main executable, so their names stay raw and no map row is owed
 * for them. */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */

/* @source 0x801EFE74
 * @behavior Per-frame work-handler step, entry 0 of the eight-entry handler
 *   table based at 0x801F0D8C whose own word at 0x801F0D8C holds this
 *   function's address, so the byte-1 dispatcher
 *   dispatchWorkByte2HandlerThenDrawTransformedAndPopWhenWorkBytes0And2Set
 *   BmagicMagic22203_801EFDEC reaches it as work record byte 0x02 while that
 *   record's byte 0x01 is 2, and no jal in this payload addresses this span, so
 *   it takes no arguments and returns nothing.
 *   It first stores (rand() & 7) + 8 into work word 0x0C, the scale the two
 *   position stores multiply their rotator samples by, so the seeded scale runs
 *   from 8 through 15. The same pointer read supplies work byte 0xB, which is
 *   shifted left nine and stored to the scratchpad angle halfword at
 *   0x1F800004 at halfword width, keeping only the low seven bits of the byte
 *   and sign-extending bit six into the value handed to the rotator.
 *   The first rotator func_801782FC takes that signed halfword and its result
 *   times work word 0x0C is added to shared record word 0x34 into work word
 *   0x34 over a second read of the cell. The second rotator func_801783C8 reads
 *   the angle back from the scratchpad cell, and its result times work word 0x0C
 *   is added to shared record word 0x38 into work word 0x38 over a third read of
 *   the cell; the same read copies shared record word 0x3C into work word 0x3C
 *   with no bias.
 *   The closing group sets work byte 9 to 8, the frame countdown of the next
 *   step, sets work byte 0xA to 0x10 and increments work byte 0x02 by one so the
 *   byte-2 dispatcher leaves this entry for the next one of the run, each of the
 *   last two over its own cell read. Frame 0x18, $ra and $s0 only.
 *   The 264-byte body is byte-identical to the exact lift at
 *   emi/bmagic/magic053/03 0x801EFE1C, proved twice: by one cmp of the two
 *   payload spans at their own offsets (264 bytes, no difference) and by that
 *   sibling's own recorded comparison against this address. That sibling's
 *   statement order, operand forms and held-pointer spelling were transcribed
 *   here, and the literals, the shifts, the absent word-0x3C bias, the byte 9
 *   value and the byte 0xA value are this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRandomWordCScaleEightToFifteenAndProjectPositionPairAtByteBAngleCopyRecordWord3CSetByte9ToEightByteATo16AdvanceWorkByte2BmagicMagic22203_801EFE74(
    void) {
  u8 *work;
  u8 *work2;
  u8 *work3;
  u8 *record;
  s16 *angle_cell;
  s16 angle;
  s32 sample;
  s32 scale;

  scale = (rand() & 7) + 8;
  work = g_battle_work;
  *(s32 *)(work + 0x0C) = scale;
  angle_cell = &D_1F800004;
  angle = work[0xB];
  *angle_cell = angle << 9;
  sample = func_801782FC((s16)(angle << 9));
  work2 = g_battle_work;
  *(s32 *)(work2 + 0x34) =
      *(s32 *)(D_801EB4E0 + 0x34) + sample * *(s32 *)(work2 + 0x0C);
  sample = func_801783C8(*angle_cell);
  work3 = g_battle_work;
  record = D_801EB4E0;
  *(s32 *)(work3 + 0x38) =
      *(s32 *)(record + 0x38) + sample * *(s32 *)(work3 + 0x0C);
  *(s32 *)(work3 + 0x3C) = *(s32 *)(record + 0x3C);
  work3[9] = 8;
  g_battle_work[0xA] = 0x10;
  g_battle_work[2]++;
}
