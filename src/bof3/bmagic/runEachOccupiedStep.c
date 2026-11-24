#include "base/types.h"

typedef void (*BmagicMagic01303Handler)(void);

/* The overlay header four-word code-pointer run at 0x801EEC04: this payload
 * words 0x801EED28, 0x801EEE68, 0x801EEEA0 and 0x801EEF84, the overlay own
 * byte-1 step bodies. This body copies the whole run into its frame, so the
 * declaration covers it exactly. The run sits in the leading tabledata block,
 * which no map row labels, so the reference binds through this target
 * generated undefined-symbol table. */
typedef struct BmagicMagic01303DispatchTable {
    BmagicMagic01303Handler handlers[4];
} BmagicMagic01303DispatchTable;

extern BmagicMagic01303DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target map.
 * The original loads it at the head of the body, before the frame is created,
 * for the byte-1 dispatch, and reloads it inside the sweep to save it across
 * the record step. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pool of this overlay at 0x801F03E4: 64 records of stride 0x78, the
 * pool reserveFirstFreeFlag also claims from. Byte 0
 * bit 0 is the in-use flag this sweep tests, and the word at 0x74 holds the
 * work pointer stored when the record was claimed. The flags read
 * re-materialises this base, so the array keeps the pool name. */
extern u8 recordFlagTable[]; /* @source 0x801F03E4 @kind data */

/* Shared work-object pointer cell at 0x801EB4E0. It sits outside this overlay
 * image, which a generated row of undefined_syms_auto.txt for 0x801EB4E0
 * already names, so the name needs no target-map row. The sweep saves the cell,
 * publishes the record own work pointer into it and restores it once the record
 * step handler returns. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* This target byte-1 step dispatcher at 0x801EFDB4, which the earlier lift of
 * src/bof3/bmagic/magic01303DispatchTables.c measured exact at 100.00: it reads
 * the work object published at 0x1F800044 and calls the handler its unsigned
 * byte 1 selects from the three-entry run at 0x801F03D8, so publishing a record
 * there makes it run that record step. */
extern void callWorkByte1Handler13(void); /* @source 0x801EFDB4 @kind function */

/* The statement order, operand forms, offset spelling and declaration style
 * follow the near-identical body at 0x801EEC28 of emi/bmagic/magic021/03, which
 * the earlier lift measured exact: a payload word comparison of this span
 * against that one, shifted by the three instructions this body spends on the
 * fourth table word, differed in four words only, the two pool-base
 * materialisations, the record word 0x74 read and the jal to this target own
 * step dispatcher, so the two spans are not byte-identical and no clone lever
 * was applied. The record count, stride, table length, pool address and callee
 * here are this payload own words.
 */

/* @source 0x801EEC14
 * @behavior Per-frame entry of this overlay. It copies the four code-pointer
 *           words at 0x801EEC04 - the overlay header handler run, 0x801EED28,
 *           0x801EEE68, 0x801EEEA0 and 0x801EEF84 - into its own 0x38-byte
 *           frame at 0x10 through 0x1C and calls the copy's entry selected by
 *           the unsigned byte 0x01 of the work object published at 0x1F800044,
 *           with no arguments and no bound on the index; the cell is read
 *           before the frame is created and the selected word is read back
 *           from the frame copy. It then walks the 64 records of the
 *           0x78-stride pool at 0x801F03E4 and, for each record whose flags
 *           byte has bit 0 set, saves the scratchpad work cell and 0x801EB4E0,
 *           loads the record word 0x74 at 0x801F0458 plus the record offset -
 *           the work pointer stored there when the record was claimed -
 *           publishes the record itself as the work object and that word into
 *           0x801EB4E0, runs that record byte-1 step through the dispatcher at
 *           0x801EFDB4 and restores both cells. The flags read re-materialises
 *           the pool base, while the published record pointer reuses one
 *           materialised base held in a saved register, so the original keeps
 *           both forms. Takes no arguments and returns nothing; the frame only
 *           keeps $ra and $s0 to $s3 across the two calls, and the sweep index
 *           is masked to a byte and compared unsigned against 0x40. No jal and
 *           no payload word names 0x801EEC14, so the overlay loader is its only
 *           caller and the parameter roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runEachOccupiedStep(void) {
    BmagicMagic01303DispatchTable handlers;
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
        if (recordFlagTable[offset] & 1) {
            record = &recordFlagTable[offset];
            savedWork = g_battle_work;
            savedRecord = D_801EB4E0;
            recordWork = *(u8 **)&recordFlagTable[offset + 0x74];
            g_battle_work = record;
            D_801EB4E0 = recordWork;
            callWorkByte1Handler13();
            D_801EB4E0 = savedRecord;
            g_battle_work = savedWork;
        }
    }
}
