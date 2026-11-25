#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map
 * row g_battle_work. The original loads it once, ahead of the local handler
 * pair's stores, and reads the object's unsigned byte 0x01 as the step
 * selector, so the cell is read once here too. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared work-object pointer cell at 0x801EB4E0. It sits outside this overlay
 * image, which the generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0
 * already names, so the name needs no target-map row. The sweep below saves the
 * cell, publishes the record's own work pointer into it and restores it after
 * the record's step handler returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Record pool of this overlay at 0x801F050C: 48 records of stride 0x78, the
 * pool reserveFirstFreeRecordBmagicMagic14403_801F046C claims from and the
 * asm row func_801EED04 clears at the same stride. Byte 0 of each record is the
 * in-use flag this sweep tests and the word at 0x74 of each record holds the
 * work pointer stored there when the record was claimed. The flags read
 * re-materialises this base, while the word-0x74 read folds the same base and
 * its own displacement into one address. Splat binds both words through the
 * generated undefined_syms_auto.txt rows D_801F050C = 0x801F050C and
 * D_801F0580 = 0x801F0580, the second being this base plus 0x74. */
extern u8 D_801F050C[]; /* @source 0x801F050C @kind data */

/* Entry 0 of the local pair is this overlay's own leading code body at
 * 0x801EED04, still an asm row of this target's Splat: it clears work byte 0x0B,
 * clears bytes 0x00 through 0x02 of all 48 pool records and then runs the
 * overlay's opening sequence, which requests scene resources through
 * func_801E590C. Entry 1 at 0x801EEE50 is the other asm row: while work byte
 * 0x0B is zero it raises bit 0x4 of the halfword at 0x801462E8 and calls
 * func_801E5988. No jal and no pointer word in this payload names 0x801EEC04, so
 * the overlay loader is its only caller and both handler roles stay structural;
 * both entries take no arguments and return nothing. */
extern void func_801EED04(void); /* @source 0x801EED04 @kind function */
extern void func_801EEE50(void); /* @source 0x801EEE50 @kind function */

/* This target's byte-1 step dispatcher at 0x801EF510, defined in
 * src/bof3/bmagic/magic14403DispatchTables.c: it calls the handler that the
 * unsigned byte 0x01 of the work object published at 0x1F800044 selects from the
 * local table based at 0x801F04E4, so publishing a record there makes it run
 * that record's step. */
extern void callStateByte1Handlers(void); /* @source 0x801EF510 @kind function */

/* @source 0x801EEC04
 * @behavior Per-frame entry of this overlay. It seeds a two-word local handler
 *           pair in its own 0x30-byte frame with {0x801EED04, 0x801EEE50} and
 *           calls the entry selected by the unsigned byte 0x01 of the work
 *           object published at 0x1F800044, with no arguments and no bound on
 *           the index. It then walks the 48 records of the pool at 0x801F050C
 *           with stride 0x78 and, for each record whose flags byte has bit 0
 *           set, saves the scratchpad work cell and 0x801EB4E0, publishes the
 *           record itself as the work object, publishes the record's word 0x74 -
 *           the work pointer stored there when the record was claimed - into
 *           0x801EB4E0, runs that record's byte-1 step through the dispatcher at
 *           0x801EF510 and restores both cells. The flags read re-materialises
 *           the pool base, while the published record pointer reuses one
 *           materialised base held in a saved register, so the original keeps
 *           both forms. Takes no arguments and returns nothing; the frame only
 *           keeps $ra and $s0 to $s3 across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callPoolRecordStep(
    void) {
  void (*handlers[2])(void) = {func_801EED04, func_801EEE50};
  u8 *record;
  u8 *saved_work;
  u8 *saved_pointer;
  u8 *record_work;
  u8 index;
  u32 offset;

  handlers[g_battle_work[1]]();
  for (index = 0; index < 0x30; index++) {
    offset = index * 0x78;
    if (D_801F050C[offset] & 1) {
      record = &D_801F050C[offset];
      saved_work = g_battle_work;
      saved_pointer = D_801EB4E0;
      record_work = *(u8 **)&D_801F050C[offset + 0x74];
      g_battle_work = record;
      D_801EB4E0 = record_work;
      callStateByte1Handlers();
      D_801EB4E0 = saved_pointer;
      g_battle_work = saved_work;
    }
  }
}
