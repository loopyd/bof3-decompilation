#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, spelled through this address-derived
 * symbol, the same named cell the exact lifts of this target already use. A named
 * cell symbol lets the assembler fold the constant address into each load
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the original re-reads the cell before
 * every group of stores made through the pointer it supplied, because such a
 * store may alias the cell object. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so this lift adds no target-map row. The body reads
 * the record twice: once for the byte 0x27 it publishes into the work object, and
 * once for the word triplet 0x34/0x38/0x3C it copies there. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF498
 * @behavior Work byte 2 state 0 of this overlay: entry 0 of the eight-entry work
 *           byte 2 handler table at 0x801EEC1C (0x801EF498, 0x801EF518,
 *           0x801EF5A4, 0x801EF608, 0x801EF678, 0x801EF720, 0x801EF7D0,
 *           0x801EF874), which
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2Set
 *           BmagicMagic02003_801EF3DC copies into its frame and indexes with the
 *           scratchpad work object's unsigned byte 2, with no bounds test, so the
 *           body runs while work byte 2 is 0 and advances that byte to 1 itself.
 *           It first refreshes the work object from the shared battle work record
 *           read through 0x801EB4E0: byte 0x27 is copied, byte 0x2B is set to 1,
 *           and words 0x34, 0x38 and 0x3C are copied. The third copy keeps the
 *           record word in its own local because work byte 9 is stored to 0x10
 *           between the load and the store. Each store group reads the scratchpad
 *           pointer cell through its own local, because a store through the loaded
 *           pointer may alias the cell. Leaf, no frame, nothing returned; it takes
 *           no arguments and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordSeedTriplet(
    void) {
  u8 *source;
  u8 *copyByte;
  u8 *seed;
  u8 *record;
  u8 *work;
  u8 *work2;
  u32 word3C;

  source = D_801EB4E0;
  copyByte = D_1F800044;
  copyByte[0x27] = source[0x27];
  seed = D_1F800044;
  seed[0x2B] = 1;
  record = D_801EB4E0;
  work = D_1F800044;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  word3C = *(u32 *)(record + 0x3C);
  work[9] = 0x10;
  work2 = D_1F800044;
  *(u32 *)(work + 0x3C) = word3C;
  work2[2] += 1;
}
