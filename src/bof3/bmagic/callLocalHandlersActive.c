#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds neither
 * 0x1F800044 nor a name for it, so the word carries its own address. The
 * original loads the cell once, ahead of the handler table's stores, and reads
 * the object's unsigned byte 0x01 as the step selector. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* The 96-record work pool at 0x801EF9F8. This target's map row
 * magic10603RecordFlags = 0x801EF9F8 owns the address, and
 * reserveFirstFreeRecordBmagicMagic10603_801EF914 declares the same pool with
 * the same stride. The flags byte sits at offset 0x00 of each 0x20-byte record. */
extern u8 magic10603RecordFlags[]; /* @source 0x801EF9F8 @kind data */

/* Work-record pointer cell, the word just past the pool. The loop publishes
 * each active record here before calling the byte-1 dispatcher, which reads the
 * cell back once. Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801F05F8 = 0x801F05F8. */
extern u8 *D_801F05F8; /* @source 0x801F05F8 @kind data */

/* Shared battle word in main RAM, outside this overlay's image and bound by the
 * generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0. The loop saves
 * it, installs the active record's word 0x1C for the handler call and restores
 * it afterwards, so the cell holds a record pointer only across that call. */
extern u32 D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* This overlay's two work-byte-1 step handlers, both still `asm` rows of this
 * target's Splat. 0x801EECE4 is the begin step: it clears bytes 0x00 through
 * 0x02 of all 96 records and then runs the overlay's opening sequence, which
 * seeds the shared battle words. 0x801EEE80 is the finish step: it returns
 * while the work object's byte 0x0B is non-zero and otherwise forwards the
 * shared battle byte at 0x80146384, raises bit 0x04 of the halfword at
 * 0x801462E8 and invokes 0x801E5988. */
void func_801EECE4(void);
void forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic10603_801EEE80(void);

/* Re-runs the selected work record's own step handler, taken from the overlay
 * handler run at 0x801EF9C0 by the record's byte 0x01; it reads the cell this
 * loop publishes. */
extern void dispatchWorkByte1HandlerBmagicMagic10603_801EEEE0(void); /* @source 0x801EEEE0 @kind function */

/* @source 0x801EEC04
 * @behavior Entry of this overlay. It copies the two-word handler table
 *           {0x801EECE4, 0x801EEE80} into its frame and calls the entry that
 *           unsigned byte 0x01 of the scratchpad work object at 0x1F800044
 *           selects, with no arguments and no bound on the selector. It then
 *           walks the 96 records of the pool at 0x801EF9F8 in 0x20-byte
 *           strides and, for every record whose flags byte has bit 0 set,
 *           saves the shared word at 0x801EB4E0, loads the record word at
 *           offset 0x1C, publishes the record address to the work-record
 *           pointer cell at 0x801F05F8, installs that word into 0x801EB4E0,
 *           calls dispatchWorkByte1HandlerBmagicMagic10603_801EEEE0 and
 *           restores 0x801EB4E0. The index is a byte, masked to eight bits for the
 *           bound test and for the stride, and the walk reads no other state.
 *           Both handlers are this payload's own addresses. No jal and no
 *           aligned pointer word in the payload names 0x801EEC04, so the
 *           overlay loader is its only caller and the body takes no arguments;
 *           the 0x28-byte frame keeps $ra and $s0 through $s2 across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callLocalHandlersActive(void) {
  void (*handlers[2])(void) = {func_801EECE4, forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic10603_801EEE80};
  u8 index;
  u32 offset;
  u32 saved;
  u32 word;

  handlers[D_1F800044[1]]();

  for (index = 0; index < 0x60; index++) {
    offset = index * 0x20;
    if ((magic10603RecordFlags[offset] & 1) != 0) {
      saved = D_801EB4E0;
      word = *(u32 *)&magic10603RecordFlags[offset + 0x1C];
      D_801F05F8 = &magic10603RecordFlags[offset];
      D_801EB4E0 = word;
      dispatchWorkByte1HandlerBmagicMagic10603_801EEEE0();
      D_801EB4E0 = saved;
    }
  }
}
