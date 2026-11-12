#include "bof3/bof3.h"

/* Shared battle buffer-copy helper at 0x800AFC54, outside this overlay's image
 * and owned by the battle targets (raw entry, emi/battle/battle/15@0x800AFC54).
 * This lift only calls it and sets no argument register, so the prototype
 * follows the call site and returns nothing. Splat binds the address through
 * this target's generated undefined_funcs_auto.txt, so no target-map row is
 * added here. */
void func_800AFC54(void);

/* Shared battle event dispatcher at 0x800AEF98, outside this overlay's image
 * and owned by the battle targets (raw entry, emi/battle/battle/15@0x800AEF98).
 * This lift only calls it, so the prototype follows the call site: the first
 * argument is a byte-sized event selector and the second a byte-sized value
 * that the dispatcher masks, and no result is used. Splat binds the address
 * through this target's generated undefined_funcs_auto.txt, so no target-map
 * row is added here. */
void func_800AEF98(s32 event, s32 value);

/* Scratchpad work-object pointer cell. This target's generated
 * undefined-syms map already binds the address because the overlay's remaining
 * assembly reads it, so the name needs no map row. The original
 * re-materializes the cell address before each of the two reads, which the
 * call between them forces, so the cell stays an ordinary pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF150
 * @behavior Work byte 1 state 1 of this overlay: entry index 1 of the
 *           four-entry handler run at 0x801EEC04 (0x801EED0C, 0x801EF150,
 *           0x801EF1CC, 0x801EF200), the word the run stores at 0x801EEC08 and
 *           that the dispatch bodies select on unsigned byte 1 of the work
 *           object held in scratchpad cell 0x1F800044. It reads that object's
 *           byte 0xB and, only when the byte is zero, calls the shared battle
 *           buffer-copy helper at 0x800AFC54, calls the shared battle event
 *           dispatcher at 0x800AEF98 with the event selector 4 and the value
 *           0, clears bit 0x40 of byte 0 of the shared battle work record read
 *           through the pointer at 0x801EB4E0 with the 0xBF mask, and then
 *           re-reads the scratchpad cell and increments work byte 1 with a
 *           byte-width add, which selects run entry 2 (0x801EF1CC). The record
 *           base stays in one register across its read, mask and store, the
 *           store group follows both calls rather than filling a delay slot,
 *           and the 0x18-byte frame only keeps $ra across those calls. Nothing
 *           is returned and no other state is read or written. The 124-byte
 *           body is byte-identical to the spans at 0x801EF594 in this payload
 *           (the word the run at 0x801EEC14 stores at 0x801EEC18) and at
 *           0x801EF6AC of emi/bmagic/magic008/00, proven by comparing the
 *           extracted payloads; both of those rows were still assembly when
 *           this lift was written, so neither supplied source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callAFC54ThenCallAEF98With4And0ClearRecordFlag40AndAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic01500_801EF150(
    void) {
  if (D_1F800044[0xB] == 0) {
    func_800AFC54();
    func_800AEF98(4, 0);
    D_801EB4E0[0] &= 0xBF;
    D_1F800044[1]++;
  }
}
