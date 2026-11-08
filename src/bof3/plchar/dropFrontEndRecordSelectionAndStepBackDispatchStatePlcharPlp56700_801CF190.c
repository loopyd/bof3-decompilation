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

/* Clone dry-run: this 148-byte span is byte-identical to the already exact lifts
 * of the same body at plp034 0x801CF8AC, plp349 0x801CEB84, plp267 0x801CF38C,
 * plp026 0x801D00B4, plp016 0x801CFC90 and plp056 0x801CFEB8, compared over the
 * original bytes of every payload. The statement order, the separate record and
 * work pointer loads and the declaration style carry over with this function's
 * own name and address.
 */
/* @source 0x801CF190
 * @behavior Drops the front-end record selection the scratchpad work record
 * carries: it loads the work-record pointer published at the pointer cell
 * 0x1F800044, reads that record byte 6 as the index of the shared 0x74-byte
 * front-end record table at 0x80143FC8, and when the selected row reads free at
 * row byte 0 or already at row byte 1 value 1, and the shared gate byte
 * 0x80149332 reads zero, it stores zero in work byte 6 and then, re-loading the
 * pointer cell, steps the work byte 3 dispatch index back by two. Row bytes 0
 * and 2..5 and the gate byte role in this overlay are not established. The
 * original keeps the entry pointer-cell load live across both lookups and uses
 * it for the byte-6 store, then re-loads the cell for the byte-3 update, so the
 * scaled row index is computed once and the store and the update take different
 * loads. It takes no arguments, calls nothing and returns nothing, and the two
 * slots after the table lookups and the final delay slot are the only nops, so
 * its own return path alone places the boundary: neither this payload nor any
 * other shipped binary holds an in-image pointer word or jal site for
 * 0x801CF190, so the selecting caller is outside the reviewed data here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dropFrontEndRecordSelectionAndStepBackDispatchStatePlcharPlp56700_801CF190(
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
