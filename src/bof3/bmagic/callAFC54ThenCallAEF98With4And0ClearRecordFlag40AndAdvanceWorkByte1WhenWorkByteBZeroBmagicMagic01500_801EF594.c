#include "bof3/bof3.h"

/* Shared battle buffer-copy helper at 0x800AFC54, outside this overlay's image
 * and owned by the battle targets. This lift only calls it and sets no argument
 * register, so the prototype follows the call site and returns nothing. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt,
 * so no target-map row is added here. */
void func_800AFC54(void);

/* Shared battle event dispatcher at 0x800AEF98, outside this overlay's image
 * and owned by the battle targets. This lift only calls it, so the prototype
 * follows the call site: the first argument is a byte-sized event selector and
 * the second a byte-sized value that the dispatcher masks, and no result is
 * used. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added here. */
void func_800AEF98(s32 event, s32 value);

/* Scratchpad work-object pointer cell. This target's generated
 * undefined-syms map already binds the address because the overlay's remaining
 * assembly reads it, so the name needs no map row. The original re-materializes
 * the cell address before each of the two reads, which the call between them
 * forces, so the cell stays an ordinary pointer. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF594
 * @behavior Work byte 1 state 1 of the second handler run of this overlay: the
 *           word the four-entry code pointer run at 0x801EEC14 stores at
 *           0x801EEC18, index 1 of 0x801EF2B4, 0x801EF594, 0x801EF610 and
 *           0x801EF200. The dispatcher dispatchWorkByte1HandlerBmagicMagic01500_801EF248
 *           copies those four words into its 0x28-byte frame and selects one
 *           with unsigned byte 1 of the work object held in scratchpad cell
 *           0x1F800044 scaled by four. It reads that object's byte 0xB and,
 *           only when the byte is zero, calls the shared battle buffer-copy
 *           helper at 0x800AFC54, calls the shared battle event dispatcher at
 *           0x800AEF98 with the event selector 4 and the value 0, clears bit
 *           0x40 of byte 0 of the shared battle work record read through the
 *           pointer at 0x801EB4E0 with the 0xBF mask, and then re-reads the
 *           scratchpad cell and increments work byte 1 with a byte-width add,
 *           which selects run entry 2 (0x801EF610). The record base stays in
 *           one register across its read, mask and store, the store group
 *           follows both calls rather than filling a delay slot, the first
 *           cell load is hoisted above the frame adjustment, and the
 *           0x18-byte frame only keeps $ra at 0x10 across those calls. Nothing
 *           is returned and no other state is read or written. A payload cmp
 *           proved the 124-byte body byte for byte identical to the span at
 *           0x801EF150 of this same payload, and that lifted sibling supplied
 *           the statement order, operand forms and declaration style.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callAFC54ThenCallAEF98With4And0ClearRecordFlag40AndAdvanceWorkByte1WhenWorkByteBZeroBmagicMagic01500_801EF594(
    void) {
  if (D_1F800044[0xB] == 0) {
    func_800AFC54();
    func_800AEF98(4, 0);
    D_801EB4E0[0] &= 0xBF;
    D_1F800044[1]++;
  }
}
