#include "bof3/bof3.h"

/* Shared main-executable lookup helper. Its exact lift at 0x8014DD3C shows it
 * applying the "main lookup" record whose id matches its argument to the active
 * work record, so the argument here is a record id and the 0xF9 request names
 * that record. This target's generated undefined_funcs_auto row binds the
 * address-anchored name, so the call keeps it. */
void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Resource request helper of the main executable, reached through the same
 * generated undefined_funcs_auto row; the sibling boss and bmagic lifts declare
 * the same prototype and pass a resource id. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Scratchpad work-record pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. The original
 * re-reads the cell after the first call because the store through the loaded
 * pointer may alias the cell, so each access names the symbol instead of
 * keeping a pointer in a local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x800C1D9C
 * @behavior Step 0 handler of this overlay's work-state machine. It applies main
 *           lookup record 0xF9 through the shared main-executable helper
 *           0x8014DD3C, clears bit 0 of the scratch work record's byte at offset
 *           0x24 (the record the pointer cell 0x1F800044 publishes), requests
 *           resource 2 through the shared helper 0x8014D6B8, then re-reads the
 *           cell and advances that record's step byte at offset 2 by one, so the
 *           byte-2 dispatcher moves to step 1 instead of re-entering this
 *           handler. Reads or writes no other state and returns nothing.
 *           This function's own address is the word at 0x800C26C8, entry 0 of
 *           the handler run that dispatchStateByte2Table26c8BossBoss01316_800C1D58
 *           indexes with work byte 2, inside the reviewed `textbin` row
 *           D_800C25D8, so the overlay dispatcher is its only caller and no jal
 *           in the payload names it. The 0x18-byte prologue frame only keeps $ra
 *           at 0x10 across the two calls; the 0xFE mask store fills the second
 *           call's delay slot, the slots after the cell loads and after the step
 *           byte load stay nops, and the plain `jr $ra` return path with the
 *           stack restore in its delay slot places the end of the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyMainLookupRecordF9ClearWorkByte24Bit0RequestResource2ThenAdvanceWorkByte2BossBoss01316_800C1D9C(
    void) {
  func_8014DD3C(0xF9);
  D_1F800044[0x24] &= 0xFE;
  func_8014D6B8(2);
  D_1F800044[2]++;
}
