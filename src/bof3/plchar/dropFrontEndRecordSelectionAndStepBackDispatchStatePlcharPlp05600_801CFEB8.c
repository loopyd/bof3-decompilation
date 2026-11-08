#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it after the store
 * through the record pointer, so the source keeps the named symbol form and
 * evaluates it again at that store's tail. The scratchpad macro would hoist the
 * constant cell address into a register across both loads and materialise it
 * with `lui`/`ori`, which the original does not do.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. The original re-materialises `lui at,0x8014` for
 * every lookup, so row byte 0, the occupancy flag, and row byte 1 are addressed
 * as their own address-derived symbols. Only byte 0 and the byte-1 comparison
 * against 1 are read here; the remaining row bytes are not touched.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FC9 @kind unknown */
extern u8 D_80143FC9[];
/* Shared gate byte D_80149332: other overlays test it for zero before advancing
 * a work record, but its role is not established, so it stays address-anchored.
 * @source 0x80149332 @kind unknown
 */
extern u8 D_80149332;

/* @source 0x801CFEB8
 * @behavior Takes the work record published at the scratchpad pointer cell
 *           0x1F800044, reads its byte 6 as the index of the shared 0x74-byte
 *           front-end record table at 0x80143FC8, and drops that selection when
 *           the selected row's byte 0 is zero or its byte 1 is 1, provided the
 *           shared gate byte D_80149332 reads zero: it stores 0 in work byte 6
 *           and then, re-reading the pointer cell, steps the work record's
 *           dispatch byte 3 back by two. Row bytes 0 and 2..5 and the gate
 *           byte's role in this overlay are not established. The original keeps
 *           the entry pointer-cell load live across both lookups and uses it for
 *           the byte-6 store, then re-loads the cell for the byte-3 update, so
 *           the scaled row index is computed once and the store and the update
 *           take different loads. The body is a leaf with no frame, no call and
 *           no nop outside the slots after the two table lookups and the two
 *           pointer-cell loads. Its only in-image reference is the word
 *           0x801CFEB8 at 0x801D08AC, entry 21 of the twenty-four-entry
 *           code-pointer run at 0x801D0858 that this overlay's work-byte-2
 *           dispatcher indexes, so that dispatcher reaches it when the work
 *           record's byte 2 reads 21. Its bytes are identical to
 *           func_801CFC90, func_801D00B4 and func_801CF38C of the sibling plchar
 *           overlays, whose lifted bodies take the same two pointer-cell loads.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dropFrontEndRecordSelectionAndStepBackDispatchStatePlcharPlp05600_801CFEB8(
    void) {
  u8* record;
  u8* work;
  u32 index;

  record = D_1F800044;
  index = (u32)record[6] * 0x74u;
  if ((D_80143FC8[index] == 0 || D_80143FC9[index] == 1) && D_80149332 == 0) {
    record[6] = 0;
    work = D_1F800044;
    work[3] = work[3] - 2;
  }
}
