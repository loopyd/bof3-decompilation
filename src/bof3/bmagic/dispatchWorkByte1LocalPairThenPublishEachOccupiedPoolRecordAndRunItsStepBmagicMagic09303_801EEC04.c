#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original loads it once, ahead of the local handler table's stores, and
 * reads the object's unsigned byte 0x01 as the step selector, so the cell is
 * read once here too. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared work-object pointer cell at 0x801EB4E0. It sits outside this overlay
 * image, which the generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0
 * already names, so the name needs no target-map row. The sweep below saves the
 * cell, publishes the record's own work pointer into it and restores it after
 * the record's step handler returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The record pool's word 0x74 slot: 0x801F08D4 + 0x74, at the same 0x78 stride as
 * the flags byte. func_801EED04 stores the scratchpad work object there when it
 * claims a record, at 0x801EEF2C and 0x801EEFA0, so this word holds the work
 * pointer that claimed the record. The pool bytes are part of this payload and
 * their own generated undefined_syms_auto.txt row names the word. */
extern u8 D_801F0948[]; /* @source 0x801F0948 @kind data */

/* Flags byte of each pool record, bound by this target's map at 0x801F08D4 with
 * stride 0x78. Bit 0 is the in-use flag, as in
 * reserveFirstFreeRecordBmagicMagic09303_801F05BC, which sets it and which this
 * sweep tests. */
extern u8 magic09303RecordFlags[]; /* @source 0x801F08D4 @kind data */

/* Entry 0 of the local pair is this overlay's own leading code body at
 * 0x801EED04, still an `asm` row of this target's Splat: it clears the 32 pool
 * records' bytes 0 through 2 and claims records for this overlay's own setup,
 * advancing work byte 1. Entry 1 is
 * raiseFlag4AndInvokeHelperWhenWorkBytes8AndAZeroBmagicMagic09303_801EF02C, exact
 * at 0x801EF02C: it acts when the work object's word 0x08 masked with 0xFF00FF00
 * is zero, raising bit 0x4 of the halfword at 0x801462E8 and calling
 * func_801E5988. No jal and no pointer word in this payload names 0x801EEC04, so
 * the overlay loader is its only caller and both handler roles stay structural. */
void func_801EED04(void);
void raiseFlag4AndInvokeHelperWhenWorkBytes8AndAZeroBmagicMagic09303_801EF02C(void);

/* This target's byte-1 step dispatcher, exact at 0x801EF3D8 through
 * src/bof3/bmagic/magic09303DispatchTables.c: it calls the handler this record's
 * work byte 0x01 selects from the table at 0x801F08AC. */
void dispatchStateByte1Table08acBmagicMagic09303_801EF3D8(void);

/* @source 0x801EEC04
 * @behavior Per-frame entry of this overlay. It seeds a two-word local handler
 *           table in its own 0x30-byte frame with {0x801EED04, 0x801EF02C} and
 *           calls the entry selected by the unsigned byte 0x01 of the work
 *           object published at 0x1F800044, with no arguments and no bound on the
 *           index. It then walks the 32 records of the pool at 0x801F08D4 with
 *           stride 0x78 and, for each record whose flags byte has bit 0 set,
 *           saves the scratchpad work cell and 0x801EB4E0, publishes the record
 *           itself as the work object, publishes the record's word 0x74 - the
 *           work pointer stored there when the record was claimed - into
 *           0x801EB4E0, runs that record's byte-1 step through the dispatcher at
 *           0x801EF3D8 and restores both cells. The flags read re-materialises the
 *           pool base, while the published record pointer reuses one materialised
 *           base held in a saved register, so the original keeps both forms. Takes
 *           no arguments and returns nothing; its frame only keeps $ra and $s0 to
 *           $s3 across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1LocalPairThenPublishEachOccupiedPoolRecordAndRunItsStepBmagicMagic09303_801EEC04(
    void) {
  void (*handlers[2])(void) = {
      func_801EED04,
      raiseFlag4AndInvokeHelperWhenWorkBytes8AndAZeroBmagicMagic09303_801EF02C};
  u8 *record;
  u8 *saved_work;
  u8 *saved_pointer;
  u8 *record_work;
  u8 index;
  u32 offset;

  handlers[g_battle_work[1]]();
  for (index = 0; index < 0x20; index++) {
    offset = index * 0x78;
    if (magic09303RecordFlags[offset] & 1) {
      record = &magic09303RecordFlags[offset];
      saved_work = g_battle_work;
      saved_pointer = D_801EB4E0;
      record_work = *(u8 **)((u8 *)D_801F0948 + offset);
      g_battle_work = record;
      D_801EB4E0 = record_work;
      dispatchStateByte1Table08acBmagicMagic09303_801EF3D8();
      D_801EB4E0 = saved_pointer;
      g_battle_work = saved_work;
    }
  }
}
