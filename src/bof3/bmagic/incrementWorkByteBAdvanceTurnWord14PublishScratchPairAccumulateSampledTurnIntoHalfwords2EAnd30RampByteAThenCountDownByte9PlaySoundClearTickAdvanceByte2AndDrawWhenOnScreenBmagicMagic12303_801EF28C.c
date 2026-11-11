#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, already bound at that
 * address by this target's map as g_battle_work, the name its exact lifts
 * incrementWorkByteBAndWorkByte9DecrementWorkByteAThenClearWorkBytesAtZeroElseInvokeHelperAtEF9E0BmagicMagic12303_801EF4AC,
 * decrementGlobalWorkByteBThenClearWorkBytesBmagicMagic12303_801EF550 and
 * clearWorkBytesBmagicMagic12303_801EFD04 use. The cell is re-read once per
 * access group because every byte store through the pointer may republish the
 * cell itself, so the original re-materializes `lui $vN,0x1F80` /
 * `lw $vN,0x44($vN)` at 0x801EF28C, 0x801EF2B0, 0x801EF2D8, 0x801EF31C,
 * 0x801EF358, 0x801EF3AC, 0x801EF3EC, 0x801EF418, 0x801EF428, 0x801EF444.
 * Every record access below therefore reads the cell in its own statement
 * group instead of holding the pointer in a local: a single `u8 *work` local
 * kept for the 0x2E, 0x30 and low-byte group and for the final on-screen test
 * measured 124 of 136 instructions, and all twelve differences were the
 * pointer's register in the groups at 0x801EF2D8, 0x801EF31C and 0x801EF444,
 * with the byte count unchanged at 544. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad turn halfword 0x1F800004. The original forms its address as one
 * `lui $s0` plus `addiu $s0` pair and keeps it in a saved register across the
 * two sampler calls, because the projected turn store and the later sampler
 * argument both go through that one base. */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Scratchpad centre halfword 0x1F800000, published through a direct
 * fixed-address store and re-read as a signed halfword before each of the two
 * products the position pair accumulates. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad halfwords 0x1F800006 and 0x1F800008, which receive the updated
 * position pair for the draw routines this handler calls. */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared frame counter in main RAM, outside this overlay's image. This
 * target's generated undefined-syms map already binds the address because the
 * overlay's remaining assembly loads it. Bit 0 gates the sound request below,
 * the use the exact bmagic sibling
 * dispatchWorkByte2HandlerThenProjectRecordPositionPairAndDrawWhenActiveBmagicMagic05903_801EFE8C
 * makes of the low bits of the same word. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Shared phase samplers in the main executable: both take an angle in the
 * fixed-point scale the products below shift and return its sample. */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 @kind function */

/* Shared main-executable sound request helper at 0x8015D71C, invoked with a
 * 0x100 or 0x101 selector. Unlifted and unnamed, so it keeps its raw name. */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C @kind function */

/* This overlay's two draw routines at 0x801EF584 and 0x801EF750, called in
 * that order once the position pair is on screen. Unlifted, so they keep their
 * raw names; this caller passes no arguments and reads no result. */
extern void func_801EF584(void); /* @source 0x801EF584 @kind function */
extern void func_801EF750(void); /* @source 0x801EF750 @kind function */

/* @source 0x801EF28C
 * @behavior Entry index 1 of the four-word handler run based at 0x801EFD5C
 *           that this target's exact dispatcher
 *           dispatchStateByte2Tablefd5cBmagicMagic12303_801EF108 indexes with
 *           work byte 2, which is why no jal word in this payload names
 *           0x801EF28C. It advances the work record's unsigned byte 0xB by one
 *           and samples 0x801782FC at that tick masked to six bits and shifted
 *           left six; the sample shifted left seven and then right twelve is
 *           added to the record word 0x14 and stored as the scratchpad turn
 *           halfword 0x1F800004. It publishes the record word 0xC as the
 *           scratchpad centre halfword 0x1F800000, samples 0x801782FC at the
 *           turn it just stored and adds that product with the centre shifted
 *           right twelve to the record halfword 0x2E, then samples 0x801783C8
 *           at the same turn and adds the like product to the record halfword
 *           0x30. The updated pair is copied to scratchpad halfwords 0x1F800006
 *           and 0x1F800008. While the record byte 0xA is below byte 0x3 plus
 *           six it advances that byte by two. It then decrements the unsigned
 *           byte 9 and tests the truncated value: at zero, and only when bit 0
 *           of the shared frame counter at 0x80143E6C is set, it requests sound
 *           0x100 or 0x101 from 0x8015D71C according to the low bit of work byte
 *           1, clears byte 0xB and advances byte 2. Finally, while the record
 *           halfwords 0x2E and 0x30 both lie inside the 0x140 by 0xF0 screen,
 *           it calls the draw routines at 0x801EF584 and 0x801EF750 in that
 *           order. The 0x18-byte frame keeps $ra at 0x14 and $s0 at 0x10 across
 *           the calls. Takes no arguments, returns nothing through the jr $ra
 *           at 0x801EF4A4 and reads or writes no other state. The roles of work
 *           bytes 1, 2, 3, 9, 0xA and 0xB, of work words 0xC and 0x14 and of
 *           both draw routines are not established, so they stay offset- or
 *           address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteBAdvanceTurnWord14PublishScratchPairAccumulateSampledTurnIntoHalfwords2EAnd30RampByteAThenCountDownByte9PlaySoundClearTickAdvanceByte2AndDrawWhenOnScreenBmagicMagic12303_801EF28C(void) {
  s16 *turn_centre;
  s32 sample;

  g_battle_work[0xB]++;
  sample = func_801782FC((g_battle_work[0xB] & 0x3F) << 6);
  turn_centre = &D_1F800004;
  *turn_centre = *(s32 *)(g_battle_work + 0x14) + ((sample << 7) >> 12);
  D_1F800000 = *(s32 *)(g_battle_work + 0xC);
  sample = func_801782FC(*turn_centre);
  *(u16 *)(g_battle_work + 0x2E) += (sample * D_1F800000) >> 12;
  sample = func_801783C8(*turn_centre);
  *(u16 *)(g_battle_work + 0x30) += (sample * D_1F800000) >> 12;
  D_1F800006 = *(u16 *)(g_battle_work + 0x2E);
  D_1F800008 = *(u16 *)(g_battle_work + 0x30);
  if (g_battle_work[0xA] < g_battle_work[0x3] + 6) {
    g_battle_work[0xA] += 2;
  }
  if (--g_battle_work[9] == 0) {
    if (D_80143E6C & 1) {
      func_8015D71C((g_battle_work[1] & 1) ? 0x100 : 0x101);
    }
    g_battle_work[0xB] = 0;
    g_battle_work[2]++;
  }
  if (*(s16 *)(g_battle_work + 0x2E) < 0x140 &&
      *(s16 *)(g_battle_work + 0x30) < 0xF0 &&
      *(s16 *)(g_battle_work + 0x2E) > 0 && *(s16 *)(g_battle_work + 0x30) > 0) {
    func_801EF584();
    func_801EF750();
  }
}
