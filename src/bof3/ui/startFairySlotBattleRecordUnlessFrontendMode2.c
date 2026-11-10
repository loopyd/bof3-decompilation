#include "bof3/bof3.h"

extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Fairy slot index byte; it selects one stride-8 record of the shared
 * 0x801455C8 active-record table. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Byte views of the selected record: its kind byte at +2, its state byte at +3
 * whose high nibble is the state class and whose low nibble selects the kind's
 * value tables, and the progress-anchor word at +4. */
extern u8 D_801455CA[]; /* @source 0x801455CA @kind data */
extern u8 D_801455CB[]; /* @source 0x801455CB @kind data */
extern u8 D_801455CC[]; /* @source 0x801455CC @kind data */

/* Shared elapsed battle counter. */
extern u32 D_8014502C; /* @source 0x8014502C @kind data */

/* Overlay step byte the enclosing handler run advances once its request
 * completes. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* Shared main-exe slot setter; every reviewed call site passes 1 and 0 as the
 * last two arguments and drops the result, so the widths below are the widths
 * observed at those call sites, not a recovered callee contract. */
u8 func_801665A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801D1568
 * @behavior Reads the shared frontend mode byte at 0x80143BB0 and returns while
 *           it equals 2. Otherwise it starts the battle record of the fairy
 *           slot selected by the byte at 0x801448ED: it calls the shared
 *           main-exe slot setter 0x801665A0 with that record's state byte at
 *           +3 and its kind byte at +2, sets bit 4 of the state byte so the
 *           record reads as state class 0x10 while its low nibble keeps
 *           selecting the kind's value tables, publishes the shared battle
 *           counter at 0x8014502C as the record's progress-anchor word at +4
 *           and advances the overlay step byte at 0x801F294C by one. The
 *           advanced step value is formed before the anchor store and written
 *           after it. It takes no arguments and returns no value. Its only
 *           in-image reference is word 5 of this overlay's handler run based at
 *           0x801D1758, which the step dispatcher 0x801D0C5C indexes with the
 *           byte at 0x801F294D and calls with no arguments and no used result,
 *           so it has no jal caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startFairySlotBattleRecordUnlessFrontendMode2(void) {
    u8 *slot;
    u8 step;

    if (D_80143BB0 == 2) {
        return;
    }
    slot = &D_801448ED;
    func_801665A0(D_801455CB[*slot * 8], D_801455CA[*slot * 8], 1, 0);
    D_801455CB[*slot << 3] |= 0x10;
    step = D_801F294C + 1;
    *(u32 *)(D_801455CC + (*slot << 3)) = D_8014502C;
    D_801F294C = step;
}
