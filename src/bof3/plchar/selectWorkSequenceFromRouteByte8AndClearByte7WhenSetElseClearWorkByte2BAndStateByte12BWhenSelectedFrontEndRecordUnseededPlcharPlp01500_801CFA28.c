#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original loads the cell once ahead of frame creation for the byte 0x07 test
 * and re-reads it after each call, because a call may change the record the
 * cell points at, so the source keeps the named-cell form and evaluates it
 * again at every access that the original reloads.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8, row byte 0 being the occupancy byte this body tests
 * for zero. The original re-materialises the base and adds the scaled row
 * offset, which is the compiler's multiply-by-0x74 chain, so the base stays the
 * address-derived symbol indexed by the byte offset.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];

/* Pointer cell of the shared main-executable record whose state byte 0x12B this
 * step clears. The sibling lifts of this overlay publish their work record
 * through the same cell and its role here is not established, so it stays
 * address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Shared readiness helper of the main executable, outside this payload: it
 * answers an unsigned byte and this body only tests it for zero. Splat binds its
 * address through this target's generated undefined_funcs_auto.txt, and the
 * remaining asm rows of this payload reference it too.
 * @source 0x8014DAEC
 */
extern u8 func_8014DAEC(void);

/* Shared work-sequence selector of the main executable, outside this payload:
 * it takes the unsigned route byte and answers a value this body discards. The
 * lifted siblings of this overlay family pass the work route byte 0x08 to it
 * the same way.
 * @source 0x8014D8D4
 */
extern u32 func_8014D8D4(u8 arg0);

/* @source 0x801CFA28
 * @behavior Work-step handler of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. Its only in-image reference is
 *           the word 0x801CFBC4, first word of the seven-word code-pointer run
 *           at 0x801CFBC4 that follows the sixteen-word run at 0x801CFB84; no
 *           jal in this payload names it, so the overlay loader that steps the
 *           run is what calls it, it takes no arguments and it returns nothing.
 *           When work byte 0x07 reads nonzero the body polls the shared
 *           readiness helper func_8014DAEC and, once that helper answers
 *           nonzero, passes the work route byte 0x08 to the shared sequence
 *           selector func_8014D8D4 and then clears work byte 0x07, which ends
 *           that arm. When work byte 0x07 reads zero the body polls the same
 *           readiness helper and, once it answers nonzero, takes work byte 0x0B
 *           as the byte offset of the row in the shared 0x74-byte front-end
 *           record table at 0x80143FC8 and, while the selected row's occupancy
 *           byte 0 reads zero, so the record is still unseeded, clears work
 *           byte 0x2B and then the state byte 0x12B of the record published at
 *           the pointer cell 0x80146250. Both arms join on one epilogue and no
 *           path reports a value. The roles of work byte 0x07, work byte 0x2B,
 *           of row byte 0 and of the 0x12B state byte in this overlay are not
 *           established, so they stay address-anchored. The pointer-cell load
 *           precedes frame creation, the cell is re-read after each call and
 *           again before the byte 0x07 store, the byte 0x2B store reuses the
 *           pointer the row lookup already holds, the 0x18-byte frame only
 *           keeps $ra at 0x10 because no local outlives a call, and the slots
 *           after each cell reload and after each tested byte load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkSequenceFromRouteByte8AndClearByte7WhenSetElseClearWorkByte2BAndStateByte12BWhenSelectedFrontEndRecordUnseededPlcharPlp01500_801CFA28(
    void) {
  u32 row;

  if (D_1F800044[7] != 0) {
    if (func_8014DAEC()) {
      func_8014D8D4(D_1F800044[8]);
      D_1F800044[7] = 0;
    }
  } else if (func_8014DAEC()) {
    row = (u32)D_1F800044[0xB] * 0x74;
    if (D_80143FC8[row] == 0) {
      D_1F800044[0x2B] = 0;
      D_80146250[0x12B] = 0;
    }
  }
}
