#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original reads it once, ahead of frame creation, and that one load serves both
 * the index byte and the arm's clear, so the source keeps a single named-symbol
 * read rather than a reload per use. This target's exact lift at 0x801D0288
 * binds the same cell the same way.
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
 * handler clears. Other overlays publish their work record through the same
 * cell and its role here is not established, so it stays address-anchored.
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

/* @source 0x801D042C
 * @behavior Entry 0 of this overlay's seven-entry code-pointer run at
 *           0x801D08B8: it is the word 0x801D042C at 0x801D08B8 itself, and the
 *           six words after it are 0x801CE404, 0x801CF12C, 0x801CF728,
 *           0x801CE72C, 0x801CF338 and 0x801CF954, which names the run as the
 *           work-byte-2 handler set of this payload because entry 1 is the
 *           dispatcher of that byte. At the time of this lift the run occupied
 *           the still-`asm` row func_801D08B8, whose 0x1C bytes decode as those
 *           seven pointer words rather than instructions, so the loader that
 *           copies the run or a shared-EXE table outside this payload is its
 *           caller and its argument roles stay structural. It reads the work
 *           record published at the scratchpad pointer cell 0x1F800044, takes
 *           that record's unsigned byte 0x0B as the byte offset of the row in
 *           the shared 0x74-byte front-end record table at 0x80143FC8, and
 *           enters its body when the selected row's byte 0 reads 0 (the record
 *           is still free) or its byte 1 reads 1, provided the shared gate byte
 *           D_80149332 reads zero. The body then clears work byte 0x2B and the
 *           state byte 0x12B of the record published at 0x80146250, and both
 *           paths run the shared no-argument update helper func_8014D978. The
 *           roles of work byte 0x2B, of row byte 1, of the gate byte and of the
 *           0x12B state byte in this overlay are not established, so they stay
 *           address-anchored. The pointer-cell load precedes frame creation and
 *           is not repeated after the lookups, the 0x18-byte frame only keeps
 *           $ra at 0x10 across the call, and the slots after the index byte,
 *           after row byte 0, after row byte 1 and after the gate byte stay
 *           nops. It takes no arguments, returns nothing and writes no dispatch
 *           byte. Its 156-byte body is byte-identical to the exact PLP026 lift
 *           at 0x801D0628 and the exact PLP016 lift at 0x801D0204, and to the
 *           still-`asm` row func_801CF324 in PLP467, cmp of the payload spans at
 *           their own offsets with all 156 bytes equal in every case, so those
 *           twins are shape evidence rather than clone candidates and the name
 *           they carry, which states the same behavior, is kept here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte2BAndStateByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp05600_801D042C(
    void) {
  u8* work;
  u32 index;

  work = D_1F800044;
  index = (u32)work[0x0B] * 0x74u;
  if ((D_80143FC8[index] == 0 || D_80143FC9[index] == 1) && D_80149332 == 0) {
    work[0x2B] = 0;
    D_80146250[0x12B] = 0;
  }
  func_8014D978();
}
