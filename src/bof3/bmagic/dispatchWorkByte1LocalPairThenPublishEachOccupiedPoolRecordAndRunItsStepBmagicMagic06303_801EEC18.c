#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original loads it once, ahead of the local handler table's stores, and
 * reads the object's unsigned byte 0x01 as the step selector, so the cell is
 * read once here too. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared work-object pointer cell at 0x801EB4E0. It sits outside this overlay
 * image, and the generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0
 * already names it, so the name needs no target-map row. The sweep below saves
 * the cell, publishes the record's own work pointer into it and restores it
 * after the record's step handler returns; the sibling
 * invokeHelperA9820WithWorkByte4WhenWorkByteBIsZeroThenDecrementGlobalWorkByteBA
 * ndInvokeHelperBmagicMagic06303_801EF2E4 of this target reads the same cell. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Record flags pool of this overlay at 0x801F0454: 0x60 records of stride 0x20,
 * so the pool ends exactly at 0x801F1054, the record pointer cell the sweep
 * below publishes into. Bit 0 of a record's flags byte is the in-use flag:
 * reserveFirstFreeRecordBmagicMagic06303_801F0354 sets it and this sweep tests
 * it. The flags read re-materialises this base, so the row keeps the pool name. */
extern u8 magic06303RecordFlags[]; /* @source 0x801F0454 @kind data */

/* Word 0x1C of every pool record, based at 0x801F0470 = 0x801F0454 + 0x1C at the
 * same 0x20 stride. seedRandomByteAndFortyComplementInEveryFreeWorkRecordCountin
 * gWorkByteBThenAdvanceWorkByte2BmagicMagic06303_801EF1C8 stores the work-object
 * pointer there when it claims a record, so this word holds the work pointer of
 * the record the sweep publishes. The pool bytes are part of this payload and
 * their own generated undefined_syms_auto.txt row names the word. */
extern u8 D_801F0470[]; /* @source 0x801F0470 @kind data */

/* Pointer cell at 0x801F1054 in this overlay's trailing data block. It holds the
 * bmagic work record that is being stepped: this sweep publishes the selected
 * record into it with one store, the dispatcher at 0x801EF340 selects a handler
 * with that record's unsigned byte at offset 1, and every handler of this
 * overlay reads the record back through the cell. The cell sits inside a
 * `textbin` row that no map row names, so the reference binds through the
 * target's generated undefined-symbol table. */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */

/* Entry 0 of the local pair is this overlay's init stage at 0x801EECF8: it
 * resets the pool, clears the queue count and the countdown, advances work byte
 * 0x01 and queues one action slot per eligible battler. Entry 1 is 0x801EEEE4,
 * still an `asm` row of this target's Splat, so it keeps its address-anchored
 * name; the two words form the code-pointer run this entry copies into its own
 * frame. Both handlers take no arguments and return nothing. */
extern void statePoolPerEligible(
    void); /* @source 0x801EECF8 */
extern void func_801EEEE4(void); /* @source 0x801EEEE4 */

/* This target's record byte-1 step dispatcher, exact at 0x801EF340: it scales
 * the published record's unsigned byte at offset 1 by four and invokes the
 * selected entry of the code-pointer table based at 0x801F0404. */
extern void callRecordHandlerBmagic(
    void); /* @source 0x801EF340 */

/* @source 0x801EEC18
 * @behavior Per-frame entry of this overlay. It seeds a two-word local handler
 *           table in its own 0x28-byte frame with this overlay's own stage
 *           bodies {0x801EECF8, 0x801EEEE4} and calls the entry selected by the
 *           unsigned byte 0x01 of the work object published at 0x1F800044, with
 *           no arguments and no bound on the index. It then walks the 0x60
 *           records of the pool at 0x801F0454 with stride 0x20 and, for each
 *           record whose flags byte has bit 0 set, publishes the record itself
 *           into the pointer cell at 0x801F1054, saves the shared work pointer
 *           at 0x801EB4E0, publishes that record's word 0x1C - the work-object
 *           pointer stored there when the record was claimed - into the shared
 *           cell, runs the record's byte 1 step through the dispatcher at
 *           0x801EF340 and restores the shared cell. The flags read
 *           re-materialises the pool base, while the published record pointer
 *           reuses one materialised base held in a saved register, so the
 *           original keeps both forms. Takes no arguments, returns nothing and
 *           writes no other state; the frame keeps $ra and $s0 through $s2
 *           across the two calls. Shape, statement order and declaration style
 *           follow the sibling lift
 *           dispatchWorkByte1LocalPairThenPublishEachOccupiedPoolRecordAndRunIts
 *           StepBmagicMagic09303_801EEC04, whose own metadata recorded an exact
 *           match when this seed was written; that body publishes into the
 *           scratchpad cell instead and carries the 0x78 stride, the 0x20-record
 *           bound and the word 0x74, while every address, stride, bound and cell
 *           here is written from this span's own bytes. No byte-identical copy of
 *           this span exists: a scan of every 0xE0-byte window of the 398
 *           extracted payloads under out/binaries returned this span plus the two
 *           same-body members emi/bmagic/magic106/03 0x801EEC04 and
 *           emi/bmagic/magic126/03 0x801EEC04, which differ in their own carried
 *           addresses alone (the second also in its 0x80 bound) while every other
 *           word stays equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1LocalPairThenPublishEachOccupiedPoolRecordAndRunItsStepBmagicMagic06303_801EEC18(
    void) {
  void (*handlers[2])(void) = {
      statePoolPerEligible,
      func_801EEEE4};
  u8 *record;
  u8 *saved_pointer;
  u8 *record_work;
  u8 index;
  u32 offset;

  handlers[g_battle_work[1]]();
  for (index = 0; index < 0x60; index++) {
    offset = index * 0x20;
    if (magic06303RecordFlags[offset] & 1) {
      record = &magic06303RecordFlags[offset];
      saved_pointer = D_801EB4E0;
      record_work = *(u8 **)((u8 *)D_801F0470 + offset);
      D_801F1054 = record;
      D_801EB4E0 = record_work;
      callRecordHandlerBmagic();
      D_801EB4E0 = saved_pointer;
    }
  }
}
