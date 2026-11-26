#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work exactly as the other lifts of this payload declare it. The
 * original re-materialises the cell with its own lui/lw pair before frame
 * creation for the countdown, once for the record copy group, twice around the
 * word 0x3C store and the byte 0x9 reset, once for each of the byte 0xA and byte
 * 0x2 reset stores, and twice inside every claim - once for the pointer word the
 * claim publishes and once for the spawn counter it advances - because a store
 * through a loaded pointer may republish the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Base of the local work-record table at 0x80145E90: 0x140-byte records whose
 * byte 0x8 and words 0x34, 0x38 and 0x3C this body copies into the scratchpad
 * work object. The selected record is the one indexed by work byte 0x4, which
 * the sibling decrementWorkByte9ThenSubmitIndexedRecordStoreEffectByteAResetCountdownAndAdvanceWorkByte2BmagicMagic16903_801EF444
 * reads through the same base and the same 0x140-byte stride. Splat binds the
 * address through this target's generated undefined_syms_auto.txt. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */

/* Record pool at 0x801F0424 inside this overlay image: the 80 records of
 * 0x78-byte stride that reserveFirstFreeRecord scans
 * and that entry stage 0 clears. The original reaches each field of the claimed
 * record as `lui $at,%hi(field address)` / `addu $at,$at,index*0x78` / store
 * `%lo(field address)($at)`, so every addressed field keeps its own
 * address-anchored name: byte 0x1, byte 0x9, byte 0xB and the work-object
 * pointer word at 0x74. */
extern u8 D_801F0425[]; /* @source 0x801F0425 @kind data */
extern u8 D_801F042D[]; /* @source 0x801F042D @kind data */
extern u8 D_801F042F[]; /* @source 0x801F042F @kind data */
extern u8 D_801F0498[]; /* @source 0x801F0498 @kind data */

/* Claims the first record of that pool whose flags byte has bit 0 clear, sets
 * the bit and returns its index, or returns 0xFF when all 80 records are taken.
 * Its own source in this module already gated exact. */
extern u8 reserveFirstFreeRecord(void); /* @source 0x801F0368 */

/* @source 0x801EF2CC
 * @behavior Decrements scratchpad work byte 0x9 with a byte-width
 *           read-modify-write and tests the stored value masked to a byte, so
 *           the rest of the body runs only on the tick that countdown reaches
 *           zero. On that tick it selects the 0x140-byte record at base
 *           0x80145E90 indexed by work byte 0x4, copies that record's byte 0x8
 *           into work byte 0x8 and its words 0x34, 0x38 and 0x3C into the same
 *           work words, clearing work byte 0xB between the last word read and
 *           that word's store. It then resets work byte 0x9 to 8, clears work
 *           byte 0xA and increments work byte 0x2, the handler-index advance
 *           this overlay dispatches on. A loop of 0x18 iterations then claims
 *           one pool record per iteration through
 *           reserveFirstFreeRecord and skips the
 *           iteration when that claim returns 0xFF. The returned index is
 *           scaled by the 0x78-byte record stride and the claimed record is
 *           seeded: byte 0x1 is cleared, byte 0xB receives the loop ordinal,
 *           byte 0x9 receives the ordinal's quarter plus 0x10, the work-object
 *           pointer is published at record word 0x74 and work byte 0xB, which
 *           counts the records this tick published, is incremented. The claim
 *           samples the scratchpad cell twice per iteration, once for the
 *           published pointer and once for the counted one. The 0x20-byte frame
 *           keeps $ra at 0x18, $s1 at 0x14 - the hoisted 0xFF claim failure
 *           value - and $s0 at 0x10, the loop ordinal; the countdown's cell load
 *           is issued before frame creation. The skip branch of a failed claim
 *           lands on the loop's own ordinal increment, so the increment is the
 *           loop's expression rather than a body statement. The span takes no
 *           arguments and returns nothing; the roles of record bytes 0x8, 0x9
 *           and 0xB, of the work words 0x34, 0x38 and 0x3C and of the constants
 *           8 and 0x10 stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation notes: one shared cell sample for the published pointer and
 * the spawn counter measured 86 of 94 instructions and 364 bytes, because the
 * map-row sample was reused for both; sampling the published pointer through
 * the scratchpad slot form and the counter through the map row restored both
 * loads and measured 94 of 94 instructions and 376 bytes. Emitting the whole
 * copy group from one sample measured 90 of 94; separating the byte 0x9 reset
 * sample from the word 0x3C store measured 91 of 94, and moving the published
 * sample ahead of the three record stores measured 94 of 94. The same two-load
 * shape with the counter sampled first measured 85 of 95 and 380 bytes.
 */
void decrementClearingStampingPublishing(
    void) {
    u8 *work;
    u8 *record;
    u8 *clear9;
    u8 *slotWork;
    u8 *countWork;
    u8 index;
    u8 i;
    u32 offset;
    u32 word3C;

    if (--g_battle_work[0x09] == 0) {
        record = D_80145E90 + g_battle_work[0x04] * 0x140u;
        g_battle_work[0x08] = record[0x08];
        work = g_battle_work;
        *(u32 *)&work[0x34] = *(u32 *)&record[0x34];
        *(u32 *)&work[0x38] = *(u32 *)&record[0x38];
        word3C = *(u32 *)&record[0x3C];
        work[0xB] = 0;
        clear9 = g_battle_work;
        *(u32 *)&work[0x3C] = word3C;
        clear9[0x09] = 8;
        g_battle_work[0x0A] = 0;
        g_battle_work[0x02]++;

        for (i = 0; i < 0x18; i++) {
            index = reserveFirstFreeRecord();
            if (index == 0xFF) {
                continue;
            }
            offset = index * 0x78;
            slotWork = PSX_REF(u8 *, 0x1F800044u);
            D_801F0425[offset] = 0;
            D_801F042F[offset] = i;
            D_801F042D[offset] = (i >> 2) + 0x10;
            countWork = g_battle_work;
            *(u32 *)&D_801F0498[offset] = (u32)slotWork;
            countWork[0xB]++;
        }
    }
}
