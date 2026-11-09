#include "bof3/bof3.h"

/* Shared battle record pointer cell in main RAM, outside this overlay image.
 * Splat binds the address through this target generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed. This body reads
 * its words 0x34 and 0x38. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target map binds it at 0x1F800044 as g_battle_work. The original
 * materialises the cell with its own lui 0x1F80 / lw 0x44 pair before every
 * store group, so each group re-reads the cell through its own local: a store
 * through the loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Two-argument distance helper of the main executable, bound by this target
 * generated undefined_funcs_auto.txt row func_8015477C = 0x8015477C. The
 * scenario, world, battle, plchar and bmagic lifts in this tree declare the
 * same signed-halfword prototype; the plchar lifts name the pair it measures
 * position coordinates and the work halfword 0x3E the distance halfword. */
extern s16 func_8015477C(s32 x, s32 y); /* @source 0x8015477C */

/* Resource request helper of the main executable, bound by this target
 * generated undefined_funcs_auto.txt row func_8014D6B8 = 0x8014D6B8. The
 * sibling bmagic overlays declare the same one-argument prototype and pass a
 * resource id. */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* @source 0x801EF900
 * @behavior Opening handler of this overlay second-phase dispatch, entry 0 of
 *           the three-word table at 0x801EEC44. The state-2 handler
 *           func_801EF83C copies those three words (0x801EF900, 0x801EFA38,
 *           0x801EFABC) into its own 0x28-byte frame and indexes them with the
 *           scratchpad work object byte 0x02, and no jal in the image names
 *           this span.
 *           The body reads the shared record pointer at 0x801EB4E0 and the
 *           work-object pointer cell at 0x1F800044 once each, writes the record
 *           word 0x34 minus one into work word 0x34, re-reads that work word
 *           through the loaded pointer into a local, and writes the record word
 *           0x38 minus one into work word 0x38, that store filling the jump
 *           delay slot of the call below.
 *           It then calls the shared distance helper func_8015477C with that
 *           one value as both of its arguments and stores the returned signed
 *           halfword at work halfword 0x3E. The first pointer re-read after the
 *           call serves the 0x25 and 0x3E stores; a second re-read serves the
 *           0x26 store.
 *           The remainder seeds the work object: byte 0x48 = 1, byte 0x27 =
 *           0xA0, byte 0x28 = 0, byte 0x24 = 4, the signed tint triplet at 0x5D
 *           to 0x5F = -0x80, byte 0x5C = 0, byte 0x2A = 0, byte 0x29 = 4 and
 *           the halfword at 0x2C = 0, that last store filling the delay slot of
 *           the func_8014D6B8 call. It requests resource 0 through
 *           func_8014D6B8, then increments work byte 0x02 by one at byte width
 *           through a final reloaded work pointer, which hands the next frame
 *           of that sequence to entry 1 (0x801EFA38).
 *           Every store group re-reads the pointer cell through its own local
 *           because a store through the loaded pointer may alias the cell; one
 *           local reused for all groups collapses those loads onto a single
 *           register. The two 4 constants and the three -0x80 constants each
 *           stay in one register across their stores, as the original does.
 *           Takes no arguments and returns nothing through the jr $ra at
 *           0x801EFA30 inside a 0x18-byte frame that keeps $ra at 0x10. Words
 *           0x34 and 0x38 stay offset-anchored: the record group they come from
 *           and the work group they land in are the same offsets the distance
 *           helper measures. Bytes 0x24 to 0x2A, 0x48 and 0x5C to 0x5F stay
 *           offset-anchored because this seed is their only writer here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkObjectFromDecrementedRecordPositionPairPublishDistanceRequestResource0AndAdvanceWorkByte2BmagicMagic05703_801EF900(void) {
  u8 *record;
  u8 *work;
  u8 *object;
  u8 *next;
  u8 *mode;
  u8 *tilt;
  u8 *offset;
  u8 *kind;
  u8 *tintR;
  u8 *tintG;
  u8 *tintB;
  u8 *shape;
  u8 *tail;
  u8 *index;
  u8 *pair;
  u8 *step;
  s32 coordinate;
  s16 distance;

  record = D_801EB4E0;
  work = g_battle_work;
  *(s32 *)(work + 0x34) = *(s32 *)(record + 0x34) - 1;
  *(s32 *)(work + 0x38) = *(s32 *)(record + 0x38) - 1;
  coordinate = *(s32 *)(work + 0x34);
  distance = func_8015477C(coordinate, coordinate);
  object = g_battle_work;
  object[0x25] = 0x1D;
  next = g_battle_work;
  *(u16 *)(object + 0x3E) = (u16)distance;
  next[0x26] = 0;
  mode = g_battle_work;
  mode[0x48] = 1;
  tilt = g_battle_work;
  tilt[0x27] = 0xA0;
  offset = g_battle_work;
  offset[0x28] = 0;
  kind = g_battle_work;
  kind[0x24] = 4;
  tintR = g_battle_work;
  *(s8 *)(tintR + 0x5D) = -0x80;
  tintG = g_battle_work;
  *(s8 *)(tintG + 0x5E) = -0x80;
  tintB = g_battle_work;
  *(s8 *)(tintB + 0x5F) = -0x80;
  shape = g_battle_work;
  shape[0x5C] = 0;
  tail = g_battle_work;
  tail[0x2A] = 0;
  index = g_battle_work;
  index[0x29] = 4;
  pair = g_battle_work;
  *(u16 *)(pair + 0x2C) = 0;
  func_8014D6B8(0);
  step = g_battle_work;
  step[2]++;
}
