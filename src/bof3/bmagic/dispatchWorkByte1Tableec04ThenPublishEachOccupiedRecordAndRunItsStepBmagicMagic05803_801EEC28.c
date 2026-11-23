#include "bof3/bof3.h"

typedef void (*BmagicMagic05803Handler)(void);

typedef struct BmagicMagic05803DispatchTable {
  BmagicMagic05803Handler handlers[3];
} BmagicMagic05803DispatchTable;

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map
 * row g_battle_work. The original reads it at the head of the body, before the
 * frame is created, for the byte-1 dispatch, and reloads it inside the sweep to
 * save it across the record step. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pool of this overlay at 0x801EFEF4: 36 records of stride 0x78, the
 * same run reserveFirstFreeRecordBmagicMagic05803_801EFE38 and the init stage
 * statePoolPosePointer
 * walk through this same base name. Byte 0 bit 0 is the in-use flag this sweep
 * tests. The flag read re-materialises the pool base, so it keeps the pool
 * name. */
extern u8 D_801EFEF4[]; /* @source 0x801EFEF4 @kind data */

/* The word at offset 0x74 of that same record, 0x801EFEF4 + 0x74, which holds
 * the record's work pointer. The original reaches it with its own
 * `lui %hi`/`addu`/`lw %lo` triple against this interior word, so the access
 * keeps the flat spelling instead of folding into the pool base. */
extern u8 D_801EFF68[]; /* @source 0x801EFF68 @kind data */

/* Battle record pointer published by the main executable at 0x801EB4E0,
 * outside this overlay image and already bound by the generated
 * undefined_syms_auto.txt row. The sweep saves the cell, publishes the record's
 * own work pointer into it and restores it once the record's step returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The overlay header's code-pointer run at 0x801EEC04: this payload's words
 * 0x801EED30, 0x801EEE78, 0x801EEEDC, 0x801EF008, 0x801EF080, 0x801EF338,
 * 0x801EF454, 0x801EF5D4 and 0x801EF61C, the overlay's own stage bodies. This
 * body copies the first three words into its frame, so the declaration covers
 * that prefix and not the run. The run sits in the leading `tabledata` block,
 * which no map row labels, so the reference binds through this target's
 * generated undefined-symbol table. */
extern BmagicMagic05803DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* This target's byte-1 step dispatcher at 0x801EF36C, exact through
 * src/bof3/bmagic/magic05803DispatchTables.c: it reads the work object
 * published at 0x1F800044 and calls the handler its unsigned byte 1 selects
 * from the table at 0x801EFEF0, so publishing a record there runs that record's
 * step. */
extern void callStateTablefef0Bmagic(void); /* @source 0x801EF36C @kind function */

/* @source 0x801EEC28
 * @behavior Per-frame entry of this overlay. It copies the three code-pointer
 *           words at 0x801EEC04 - the first three entries of the overlay
 *           header's nine-word stage run, 0x801EED30, 0x801EEE78 and
 *           0x801EEEDC - into its own 0x38-byte frame at 0x10 through 0x18 and
 *           calls the copy's entry selected by the unsigned byte 0x01 of the
 *           work object published at 0x1F800044, with no arguments and no bound
 *           on the index; the cell is read before the frame is created and the
 *           selected word is read back from the frame copy. It then walks the 36
 *           records of the 0x78-stride pool at 0x801EFEF4 and, for each record
 *           whose flags byte has bit 0 set, saves the scratchpad work cell and
 *           0x801EB4E0, reads the record's word 0x74 - the work pointer stored
 *           there when the record was claimed - publishes the record itself as
 *           the work object and that word into 0x801EB4E0, runs that record's
 *           byte-1 step through the dispatcher at 0x801EF36C and restores both
 *           cells. The flags read re-materialises the pool base, while the
 *           published record pointer reuses one materialised base held in a
 *           saved register, so the original keeps both forms. Takes no arguments
 *           and returns nothing; the frame only keeps $ra and $s0 to $s3 across
 *           the two calls. No jal and no payload word names 0x801EEC28, so the
 *           overlay loader is its only caller and the parameter roles stay
 *           structural.
 *           The statement order, operand forms and declaration style are
 *           transcribed from the exact same-address lift at 0x801EEC28 of
 *           emi/bmagic/magic021/03
 *           (dispatchWorkByte1Tableec04ThenPublishEachOccupiedRecordAndRunItsStepBmagicMagic02103_801EEC28);
 *           a payload cmp of this span against that one showed seven differing
 *           bytes only - the pool base at its three sites, the record count
 *           0x24 versus 0x40 and the step-dispatcher jal target - so the two are
 *           one body and the span has no byte-identical twin. This body writes
 *           its own pool base, record count, callee and interior word symbol.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Tableec04ThenPublishEachOccupiedRecordAndRunItsStepBmagicMagic05803_801EEC28(void) {
  BmagicMagic05803DispatchTable handlers;
  u8 index;
  u32 offset;
  u8 *record;
  u8 *savedWork;
  u8 *savedRecord;
  u8 *recordWork;

  handlers = D_801EEC04;
  handlers.handlers[g_battle_work[1]]();
  for (index = 0; index < 0x24; index++) {
    offset = index * 0x78;
    if (D_801EFEF4[offset] & 1) {
      record = &D_801EFEF4[offset];
      savedWork = g_battle_work;
      savedRecord = D_801EB4E0;
      recordWork = *(u8 **)&D_801EFF68[offset];
      g_battle_work = record;
      D_801EB4E0 = recordWork;
      callStateTablefef0Bmagic();
      D_801EB4E0 = savedRecord;
      g_battle_work = savedWork;
    }
  }
}
