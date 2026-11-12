#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad work-record pointer cell at 0x1F800044, written with the name
 * every other lifted body of this overlay gives it and bound by this target's
 * map through the shared symbols support file. The original re-materialises the
 * cell with its own lui 0x1F80 plus lw 0x44 pair at each of the five reads it
 * makes, so the spelling stays one cell read per statement group: the first
 * read feeds both the work word 0x0C store and the work byte 0xB load, because
 * no store separates them in the original. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad angle halfword at 0x1F800004. The seed path stores work byte 0xB
 * shifted left nine into it at halfword width and hands the same signed
 * halfword to the first rotator; the second rotator reads the cell back, so the
 * original folds the base once into a saved register (lui $s0,0x1F80 / addiu
 * $s0,$s0,4) instead of re-materialising it at the two accesses. The generated
 * undefined_syms_auto.txt already binds the address, so this lift adds no
 * target-map row. */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Battle record pointer published in main RAM, outside this overlay's
 * 0x801EEC00-0x801F1478 image, so the address is the same cell every bmagic
 * overlay reads and Splat binds it through this target's generated
 * undefined_syms_auto.txt row. Its role is not proven, so the name stays
 * address-anchored. Only the record words 0x34, 0x38 and 0x3C are read, and the
 * second group holds the pointer in one local across both of its reads. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The overlay rotation pair: arg0 is the signed halfword angle taken from the
 * scratchpad cell and each call returns the projected step for one axis, which
 * this body multiplies by work word 0x0C without any shift. */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */

/* @source 0x801EFE1C
 * @behavior Overlay work-handler step reached as entry 0 of the handler run
 *   based at 0x801F056C, the word the byte-1 dispatcher 0x801EFD50 names at
 *   0x801F0568 calls the byte-2 dispatcher 0x801EFD94, and that dispatcher
 *   indexes this base with the work record byte 0x02. No jal in the payload
 *   addresses this span and its address appears at the table word 0x801F056C
 *   alone, so it takes no arguments and returns nothing.
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
 *   The 264-byte body is byte-identical to the still-asm span at
 *   emi/bmagic/magic222/03 0x801EFE74, proved by one cmp of the two payload
 *   spans (264 bytes, no difference), and that twin's row is still asm so it
 *   carries no source to clone. The statement order, operand forms and
 *   held-pointer spelling came instead from two exact bmagic siblings: the
 *   word-0x0C scale projection of
 *   seedRandomWorkWord14AndBytes5DTo5FProjectPositionPairAtWordCScaleEightSetWord3CAndAdvanceWorkByte2OnWorkByte9CountdownBmagicMagic07603_801EFA60
 *   and the held scratchpad angle cell of
 *   countDownWorkByte9ThenSeedAngleFromByteBAddHalvedRotatorStepsToWords34And38CopyWord3CSetWord14ToRandPlus128Word20ToMinus12ResetByte9To16AndAdvanceWorkByte2BmagicMagic01303_801EFE80;
 *   the literals, the shifts, the absent word-0x3C bias, the byte 9 value and
 *   the byte 0xA value are this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRandomWordCScaleEightToFifteenAndProjectPositionPairAtByteBAngleCopyRecordWord3CSetByte9ToEightByteATo16AdvanceWorkByte2BmagicMagic05303_801EFE1C(
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
