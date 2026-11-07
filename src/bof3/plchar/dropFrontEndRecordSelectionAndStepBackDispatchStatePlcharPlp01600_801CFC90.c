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

/* @source 0x801CFC90
 * @behavior Takes the work record published at the scratchpad pointer cell
 * 0x1F800044, reads its byte 6 as the index of the shared 0x74-byte front-end
 * record table at 0x80143FC8, and drops that selection when the selected row's
 * byte 0 is zero or its byte 1 is 1, provided the shared gate byte D_80149332
 * reads zero: it stores 0 in work byte 6 and then, re-reading the pointer cell,
 * steps the work record's dispatch byte 3 back by two. Row bytes 0 and 2..5 and
 * the gate byte's role in this overlay are not established. The original keeps
 * the entry pointer-cell load live across both lookups and uses it for the
 * byte-6 store, then re-loads the cell for the byte-3 update, so the scaled row
 * index is computed once and the store and the update take different loads. It
 * takes no arguments, calls nothing and returns nothing; nothing in this
 * payload holds its address, so it is an overlay entry reached from outside the
 * payload and its return path alone places the boundary. Its body is
 * byte-identical to func_801D00B4, func_801CF8AC, func_801CFEB8, func_801CF38C,
 * func_801CEB84, func_801CEDB0, func_801CF190 and func_801CEB94 in eight
 * sibling plchar overlays, none of them lifted yet.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dropFrontEndRecordSelectionAndStepBackDispatchStatePlcharPlp01600_801CFC90(
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
