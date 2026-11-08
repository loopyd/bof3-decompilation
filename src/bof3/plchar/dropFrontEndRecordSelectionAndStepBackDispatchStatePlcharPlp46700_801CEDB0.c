#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-loads it after the store
 * through the record pointer, so the source keeps the named symbol form and
 * evaluates it again at that store's tail.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Front-end record table base 0x80143FC8 and the field bases of its 0x74-byte
 * rows. The original re-materialises `lui at,0x8014` for each lookup, so row
 * byte 0 and row byte 1 are addressed as their own address-derived symbols.
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

/* Seeded from the byte-identical lift of the same body at plp349 0x801CEB84,
 * which measured exact before this seed was written; that member was itself
 * seeded from plp034 0x801CF8AC. This 148-byte span compared byte-identical to
 * that representative range in the payload cmp, so the statement order, the
 * separate record and work pointer loads and the declaration style are carried
 * over with this function's own name.
 */
/* @source 0x801CEDB0
 * @behavior Drops the front-end record selection the scratchpad work record
 * carries: loads the work-record pointer published at the pointer cell
 * 0x1F800044, reads that record unsigned byte 6 as the index of the shared
 * 0x74-byte front-end record table at 0x80143FC8, and when the selected row
 * reads zero at row byte 0 or value 1 at row byte 1, and the shared gate byte
 * 0x80149332 reads zero, stores zero in work byte 6 and then, re-loading the
 * pointer cell, steps the work byte 3 dispatch index back by two; no index
 * bound, no calls, no frame, and it writes no other state. Its only in-image
 * reference is its own address at the pointer word 0x801D03DC, word 12 of the
 * twenty-eight-word code-pointer run based at 0x801D03AC that this overlay
 * indexes, so no jal in this payload names it and it takes no arguments. Row
 * bytes 0 and 2 through 5 and the gate byte role in this overlay are not
 * established.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dropFrontEndRecordSelectionAndStepBackDispatchStatePlcharPlp46700_801CEDB0(
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
