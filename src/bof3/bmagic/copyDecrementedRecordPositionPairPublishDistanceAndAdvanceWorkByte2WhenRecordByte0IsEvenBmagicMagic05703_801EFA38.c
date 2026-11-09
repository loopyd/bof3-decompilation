#include "bof3/bof3.h"

/* Shared battle record pointer cell in main RAM, outside this overlay image.
 * Splat binds the address through this target generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed. This body reads
 * its words 0x34 and 0x38 and its byte 0. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target map binds it at 0x1F800044 as g_battle_work. The original
 * materialises the cell with its own lui 0x1F80 / lw 0x44 pair at each of the
 * two groups, the second pair being a fresh load after the call rather than a
 * reuse of the first, so that store may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Two-argument distance helper of the main executable, bound by this target
 * generated undefined_funcs_auto.txt row func_8015477C = 0x8015477C. The
 * scenario, world, battle, plchar and bmagic lifts in this tree declare the
 * same signed-halfword prototype; the plchar lifts name the pair it measures
 * position coordinates and the work halfword 0x3E the distance halfword. */
extern s16 func_8015477C(s32 x, s32 y); /* @source 0x8015477C */

/* Shape read off the original bytes: the 0x34 word is stored before the 0x34
 * cell is re-read for the call argument, and the second record word is read
 * between them, so the helper receives that one re-read value twice and the
 * 0x38 store fills the call delay slot. The call can alias both pointer cells,
 * so the two cells are re-read once each afterwards into their own locals, and
 * those two locals are the only spelling that reproduces the original register
 * assignment after the call (a shared local made the compiler keep the
 * pre-call registers for all four uses). */

/* @source 0x801EFA38
 * @behavior Middle handler of this overlay second-phase table: the state-2
 *           handler func_801EF83C copies the three words at 0x801EEC44
 *           (0x801EF900, 0x801EFA38, 0x801EFABC) into its own 0x28-byte frame
 *           and indexes them with the scratchpad work object byte 0x02, so
 *           this span is entry 1 and no jal in the image names it.
 *           It reads the shared record pointer at 0x801EB4E0 and the
 *           work-object pointer cell at 0x1F800044 once each, writes the
 *           record word 0x34 minus one into work word 0x34, re-reads that work
 *           word through the loaded pointer into a local, and writes the
 *           record word 0x38 minus one into work word 0x38, that store filling
 *           the jump delay slot of the call below.
 *           It then calls the shared distance helper func_8015477C with that
 *           one value as both of its arguments, re-reads the work-object cell
 *           rather than reusing the pre-call pointer, and stores the returned
 *           signed halfword at work halfword 0x3E.
 *           Last it re-reads the record cell and, while record byte 0 bit 0 is
 *           clear (lbu / andi 1 / bnez to the epilogue), increments work byte
 *           0x02 by one at byte width through the reloaded work pointer, which
 *           hands the next frame of that sequence to entry 2 (0x801EFABC).
 *           It takes no arguments and returns nothing through the jr $ra at
 *           0x801EFAB4 inside a 0x18-byte frame that keeps $ra at 0x10. Words
 *           0x34 and 0x38 stay offset-anchored: the record group they come from
 *           and the work group they land in are the same offsets the distance
 *           helper measures.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyDecrementedRecordPositionPairPublishDistanceAndAdvanceWorkByte2WhenRecordByte0IsEvenBmagicMagic05703_801EFA38(void) {
  u8 *record;
  u8 *work;
  u8 *object;
  u8 *source;
  s32 coordinate;
  s16 distance;

  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) - 1;
  *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38) - 1;
  coordinate = *(s32 *)(work + 0x34);
  distance = func_8015477C(coordinate, coordinate);
  object = g_battle_work;
  source = D_801EB4E0;
  *(u16 *)(object + 0x3E) = (u16)distance;
  if ((source[0] & 1) == 0) {
    object[2] = object[2] + 1;
  }
}
