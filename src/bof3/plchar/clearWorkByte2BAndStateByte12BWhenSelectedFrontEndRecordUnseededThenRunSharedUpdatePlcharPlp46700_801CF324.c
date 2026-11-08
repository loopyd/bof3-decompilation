#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original reads it once, ahead of
 * frame creation, and that load serves both the row index byte and the arm's
 * clear, so the source keeps one named-symbol read rather than a reload per
 * use. The exact siblings 0x801CF180 and 0x801CEDB0 of this same target bind
 * the cell the same way.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Front-end record table base 0x80143FC8 and the field bases of its 0x74-byte
 * rows. The original re-materialises `lui at,0x8014` for each lookup, so row
 * byte 0 and row byte 1 are addressed as their own address-derived symbols,
 * both indexed by the byte offset of the selected row.
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
/* Pointer cell of the shared main-executable record whose state byte 0x12B this
 * step clears. Other overlays publish their work record through the same cell
 * and its role here is not established, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Shared no-argument update helper of the main executable, outside this
 * payload. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt, and the remaining asm rows of this payload
 * reference it too. The original sets nothing up in the argument registers and
 * reads no result from $v0.
 */
void func_8014D978(void);

/* Clone dry-run: this 156-byte span matched the original payload bytes of every
 * comparison word for word, 156 of 156 bytes equal including the shared `jal`
 * immediate, against the lifts that had already passed exact at plp016
 * 0x801D0204, plp026 0x801D0628, plp056 0x801D042C, plp267 0x801CF900 and
 * plp567 0x801CF704, so the statement order, the single pointer-cell load, the
 * separate row-byte lookups and the declaration style carry over with this
 * function's own name and address.
 */
/* @source 0x801CF324
 * @behavior Work byte 3 == 2 step of this overlay: it is the word 0x801CF324 at
 *           0x801D03E8, index 2 of the work-byte-3 handler run based at
 *           0x801D03E0, which this target's dispatcher
 *           dispatchWorkByte3Table03e0PlcharPlp46700_801CF13C indexes from
 *           0x801D03E0 by work byte 3 scaled by four, so it takes no arguments
 *           and any result is discarded. It reads the work record published at
 *           the scratchpad pointer cell 0x1F800044 and takes that record's
 *           unsigned byte 0x0B as the byte offset of a row in the shared
 *           0x74-byte front-end record table at 0x80143FC8, scaling the byte to
 *           0x74 bytes through the original's multiply-and-add chain, and it
 *           enters its body when the selected row's byte 0 reads 0 (the record
 *           is still free) or its byte 1 reads 1, provided the shared gate byte
 *           0x80149332 reads zero. The body then clears work byte 0x2B and the
 *           state byte at 0x12B of the record published at 0x80146250, and both
 *           paths run the shared no-argument update helper func_8014D978. The
 *           roles of work byte 0x2B, of row byte 1, of the gate byte and of the
 *           0x12B state byte in this overlay are not established, so they stay
 *           address-anchored. The pointer-cell load precedes frame creation and
 *           is not repeated after the lookups, the 0x18-byte frame only keeps
 *           $ra at 0x10 across the call, and the slots after the index byte,
 *           after each of the three lookup bytes and after the
 *           published-record load stay nops, together with the untaken-branch,
 *           call and return delay slots. No payload holds a jal site for
 *           0x801CF324 and this payload's only in-image word for it is that
 *           handler-run entry, so the work-byte-3 dispatch is its caller and
 *           its own return path places the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte2BAndStateByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp46700_801CF324(
    void) {
  u8* record;
  u32 index;

  record = D_1F800044;
  index = (u32)record[0x0B] * 0x74u;
  if ((D_80143FC8[index] == 0 || D_80143FC9[index] == 1) && D_80149332 == 0) {
    record[0x2B] = 0;
    D_80146250[0x12B] = 0;
  }
  func_8014D978();
}
