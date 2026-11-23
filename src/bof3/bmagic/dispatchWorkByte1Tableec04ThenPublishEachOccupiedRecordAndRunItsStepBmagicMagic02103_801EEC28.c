#include "base/types.h"

typedef void (*BmagicMagic02103Handler)(void);

typedef struct BmagicMagic02103DispatchTable {
    BmagicMagic02103Handler handlers[3];
} BmagicMagic02103DispatchTable;

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The original loads it at the head of the body, before the frame is created,
 * for the byte-1 dispatch, and reloads it inside the sweep to save it across the
 * record step. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pool of this overlay at 0x801EFE14: 64 records of stride 0x78, the
 * same pool reserveFirstFreeRecordBmagicMagic02103_801EFD4C claims from. Byte 0
 * bit 0 is the in-use flag this sweep tests, and the word at 0x74 holds the work
 * pointer stored when the record was claimed, as
 * invokeHelperThenCountDownWorkByte9AndQueueTwelveRecordsWithPairedStepAndWorkPointerBmagicMagic02103_801EF104
 * writes it. The flags read re-materialises this base, so the array keeps the
 * pool name. */
extern u8 magic02103RecordFlags[]; /* @source 0x801EFE14 @kind data */

/* Shared work-object pointer cell at 0x801EB4E0. It sits outside this overlay
 * image, which the generated undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0
 * already names, so the name needs no target-map row. The sweep saves the cell,
 * publishes the record's own work pointer into it and restores it once the
 * record's step handler returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* The overlay header's nine-word code-pointer run at 0x801EEC04: this payload's
 * words 0x801EED30, 0x801EEF38, 0x801EEFA8, 0x801EF0BC, 0x801EF104, 0x801EF208,
 * 0x801EF260, 0x801EF2CC and 0x801EF328, the overlay's own byte-1 step bodies.
 * This body copies the first three words into its frame, so the declaration
 * covers that prefix and not the run. The run sits in the leading `tabledata`
 * block, which no map row labels, so the reference binds through this target's
 * generated undefined-symbol table. */
extern BmagicMagic02103DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* The frame copy of that run follows the dispatchers at 0x801EEC1C of
 * emi/bmagic/magic062/03 and at 0x801EEFA4 of emi/bmagic/magic138/03, whose
 * bodies copy the words at 0x801EEC04 and 0x801EEC10 into the frame the same
 * way. The sweep that follows the call is transcribed from the exact lift of the
 * same-address entry emi/bmagic/magic003/03 (0x801EEC28: 32 records, a ten-entry
 * table indexed in place) and from the exact local-pair sweep at 0x801EEC04 of
 * emi/bmagic/magic093/03 (stride 0x78, one materialised pool base held in a
 * saved register); this body writes its own pool base, record count, header
 * prefix and callee. A payload cmp of this span against those two bodies showed
 * the three differ inside their first 16 bytes, so the span has no byte-identical
 * twin and the clone lever was declined. */

/* This target's byte-1 step dispatcher at 0x801EF534, exact through
 * src/bof3/bmagic/magic02103DispatchTables.c: it reads the work object published
 * at 0x1F800044 and calls the handler its unsigned byte 1 selects from the
 * four-entry run at 0x801EFE04, so publishing a record there makes it run that
 * record's step. */
extern void dispatchWorkByte1HandlerBmagicMagic02103_801EF534(void); /* @source 0x801EF534 @kind function */

/* @source 0x801EEC28
 * @behavior Per-frame entry of this overlay. It copies the three code-pointer
 *           words at 0x801EEC04 - the first three entries of the overlay
 *           header's nine-word handler run, 0x801EED30, 0x801EEF38 and
 *           0x801EEFA8 - into its own 0x38-byte frame at 0x10 through 0x18 and
 *           calls the copy's entry selected by the unsigned byte 0x01 of the
 *           work object published at 0x1F800044, with no arguments and no bound
 *           on the index; the cell is read before the frame is created and the
 *           selected word is read back from the frame copy. It then walks the 64
 *           records of the 0x78-stride pool at 0x801EFE14 and, for each record
 *           whose flags byte has bit 0 set, saves the scratchpad work cell and
 *           0x801EB4E0, loads the record's word 0x74 - the work pointer stored
 *           there when the record was claimed - publishes the record itself as
 *           the work object and that word into 0x801EB4E0, runs that record's
 *           byte-1 step through the dispatcher at 0x801EF534 and restores both
 *           cells. The flags read re-materialises the pool base, while the
 *           published record pointer reuses one materialised base held in a
 *           saved register, so the original keeps both forms. Takes no arguments
 *           and returns nothing; the frame only keeps $ra and $s0 to $s3 across
 *           the two calls. No jal and no payload word names 0x801EEC28, so the
 *           overlay loader is its only caller and the parameter roles stay
 *           structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Tableec04ThenPublishEachOccupiedRecordAndRunItsStepBmagicMagic02103_801EEC28(void) {
    BmagicMagic02103DispatchTable handlers;
    u8 index;
    u32 offset;
    u8 *record;
    u8 *savedWork;
    u8 *savedRecord;
    u8 *recordWork;

    handlers = D_801EEC04;
    handlers.handlers[g_battle_work[1]]();
    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        if (magic02103RecordFlags[offset] & 1) {
            record = &magic02103RecordFlags[offset];
            savedWork = g_battle_work;
            savedRecord = D_801EB4E0;
            recordWork = *(u8 **)&magic02103RecordFlags[offset + 0x74];
            g_battle_work = record;
            D_801EB4E0 = recordWork;
            dispatchWorkByte1HandlerBmagicMagic02103_801EF534();
            D_801EB4E0 = savedRecord;
            g_battle_work = savedWork;
        }
    }
}
