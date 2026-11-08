#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original reads it once, ahead of
 * frame creation, and that single load serves the index byte, so the source
 * keeps one named-symbol read.
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

/* @source 0x801CFE14
 * @behavior Overlay step reached from outside this payload. It reads the
 *           front-end record index from work byte 0x0B of the record published
 *           at scratchpad pointer cell 0x1F800044 and looks that row up in the
 *           shared 0x74-byte front-end record table at 0x80143FC8. It enters
 *           its body when the selected row byte 0 reads 0 (the record is still
 *           free) or its row byte 1 reads 1, provided the shared gate byte
 *           D_80149332 reads zero; the body then clears the state byte at 0x12B
 *           of the record published at 0x80146250. Both paths run the shared
 *           no-argument update helper func_8014D978. The roles of row byte 1,
 *           of the gate byte and of the 0x12B state byte in this overlay are
 *           not established, so they stay address-anchored. It returns nothing
 *           and writes no dispatch byte; its address is published at the
 *           pointer word 0x801D033C inside the twenty-eight-word code-pointer
 *           run based at 0x801D02F4, which is index 2 of the run based at
 *           0x801D0334 that dispatchWorkByte3Table0334PlcharPlp03400_801CFC38
 *           indexes from work byte 3, and it has no jal caller. The
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the call, and the slots after the index
 *           byte, after row byte 0, after row byte 1 and after the gate byte
 *           stay nops. Its 152-byte span compared byte-identical to the
 *           still-asm row func_801CF0EC in PLP349 (payload cmp, all 152 bytes
 *           equal), so that twin is shape evidence and not a clone. The shape
 *           was seeded from the exact PLP026 lift of 0x801D0628, which is this
 *           body plus the work byte 0x2B clear that this overlay does not have.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStateByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp03400_801CFE14(
    void) {
  u8* work;
  u32 index;

  work = D_1F800044;
  index = (u32)work[0x0B] * 0x74u;
  if ((D_80143FC8[index] == 0 || D_80143FC9[index] == 1) && D_80149332 == 0) {
    D_80146250[0x12B] = 0;
  }
  func_8014D978();
}
