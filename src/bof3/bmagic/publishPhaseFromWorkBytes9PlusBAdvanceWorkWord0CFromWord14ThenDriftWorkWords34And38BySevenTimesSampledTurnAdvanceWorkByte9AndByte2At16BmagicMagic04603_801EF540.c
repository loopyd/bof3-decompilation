#include "base/types.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target map
 * row g_battle_work as the bmagic siblings of other targets declare it. The
 * body takes six separate reads of the cell - one for the turn bytes, one for
 * the target angle word 0x14 and the store into word 0x0C, one for the two
 * drift words, one for the cosine turn read, one for the byte 0x9 advance and
 * one for the stage byte 0x2 advance - because every call in between may
 * republish the cell, so the original re-materialises the address with its own
 * lui / lw pair at each read. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad phase halfword 0x1F800002. The original forms its address as one
 * lui $at plus a store at the low half, publishes the twelve-bit turn the
 * sampler below is also called with, and never reads it back. */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */

/* Shared phase samplers in the main executable: both take a twelve-bit turn
 * and return a sample in the fixed-point scale the products below shift right
 * by twelve, the same two helpers the exact bmagic draw siblings call as the
 * perpendicular pair of one radius. Unlifted, so they keep their raw names. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared main-executable helper invoked once the drift words are advanced.
 * Unlifted and unnamed, so it keeps its raw name; this caller passes no
 * arguments and reads no result. */
void func_800AF764(void);

/* This overlay draw routine at 0x801EF7E8, the last entry of the same
 * three-entry table and still an asm boundary in this target. It is invoked
 * after the drift words are advanced and takes no arguments here. */
void func_801EF7E8(void);

/* @source 0x801EF540
 * @behavior Frame handler of the second stage of this overlay draw state
 *           machine, entry 1 of the three-entry code pointer table at
 *           0x801EEC1C that func_801EF324 indexes by work byte 0x2. It reads
 *           the work object published at the scratchpad pointer cell
 *           0x1F800044, forms a twelve-bit turn from work byte 0x9 plus work
 *           byte 0xB masked with 0x3F and shifted left six, publishes it as a
 *           halfword into the scratchpad cell 0x1F800002 and passes the same
 *           turn to the sampler at 0x801782FC, stores work word 0x0C as work
 *           word 0x14 advanced by that sample times 300 shifted right twelve,
 *           samples 0x801782FC again at the new word 0x0C and adds seven times
 *           the sample into work word 0x34, samples 0x801783C8 at that same
 *           turn and adds seven times its sample into work word 0x38, invokes
 *           the main-executable helper at 0x800AF764 with no arguments, calls
 *           the overlay draw routine at 0x801EF7E8 with no arguments, and then
 *           increments work byte 0x9 modulo 256 and, once that byte reaches
 *           0x10, increments the dispatch byte 0x2 the table index reads.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishPhaseFromWorkBytes9PlusBAdvanceWorkWord0CFromWord14ThenDriftWorkWords34And38BySevenTimesSampledTurnAdvanceWorkByte9AndByte2At16BmagicMagic04603_801EF540(
    void) {
  s32 sample;
  u16 turn;

  /* The masked turn is a sixteen-bit local: that HImode spelling is the only
   * one measured to give the original's pair of six-bit shifts, one into the
   * argument register and one feeding the halfword store. */
  turn = (u16)((g_battle_work[9] + g_battle_work[0xB]) & 0x3F);
  D_1F800002 = turn << 6;
  sample = func_801782FC(turn << 6);
  *(s32 *)(g_battle_work + 0xC) =
      *(s32 *)(g_battle_work + 0x14) + ((sample * 300) >> 12);
  sample = func_801782FC(*(s32 *)(g_battle_work + 0xC));
  *(s32 *)(g_battle_work + 0x34) += (sample * 0x7000) >> 12;
  sample = func_801783C8(*(s32 *)(g_battle_work + 0xC));
  *(s32 *)(g_battle_work + 0x38) += (sample * 0x7000) >> 12;
  func_800AF764();
  func_801EF7E8();
  if ((g_battle_work[9] += 1) == 0x10) {
    g_battle_work[2]++;
  }
}
