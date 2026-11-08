#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows whose byte 0 reads 1 once a seeder claims the record and stays 0 while
 * the record is still free. The original re-materialises `lui at,0x8014` for
 * the lookup, so the row is addressed through its own address-derived symbol.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];

/* Shared no-argument update helper of the main executable, outside this
 * payload. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt, and the remaining asm rows of this payload
 * reference it too. The original sets nothing up in the argument registers and
 * reads no result from $v0.
 */
void func_8014D978(void);

/* @source 0x801CF5E4
 * @behavior Overlay step reached from outside this payload. It reads the
 *           front-end record index from work byte 0x0B of the record published
 *           at scratchpad pointer cell 0x1F800044 and looks that row up in the
 *           shared 0x74-byte front-end record table at 0x80143FC8. When the
 *           selected row's byte 0 reads 0 -- the record is still free, since
 *           every seeder of this table stores 1 there -- it clears the state
 *           byte at 0x12B of the record published at 0x80146250, the same byte
 *           this target's own accessor getAndClearPlcharPlp47800_801CE610
 *           clears. That byte's role in this overlay is not established, so it
 *           stays address-anchored. Both paths then run the shared no-argument
 *           update helper func_8014D978. It takes no arguments, returns nothing
 *           and writes no other state. Its only in-image reference is the word
 *           0x801CF5E4 at 0x801CF70C, entry 13 of the twenty-word code-pointer
 *           run based at 0x801CF6D8 and entry 1 of D_801CF708, so the overlay
 *           loader that steps that run enters it; it has no jal caller and
 *           writes no dispatch byte. The pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           call, and the slots after the index byte, after the row byte and
 *           after the branch stay nops.
 *
 * Evidence: the span is a complete 108-byte function -- addiu $sp,-0x18 with
 * sw $ra,0x10($sp), one jal to func_8014D978, and the epilogue
 * lw $ra,0x10($sp); addiu $sp,$sp,0x18; jr $ra; nop. The row lookup is the
 * recorded chain sll 3 / subu / sll 2 / addu / sll 2, which scales the index
 * byte by 0x74. Name evidence: all 108 bytes are byte-identical to the exact
 * lift at 0x801CF494 in PLP016 under the same 0x801CE400 load address, so this
 * body is that twin's shape and not a clone claim of its own; the twin names
 * the shape clearByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdate,
 * which the identical PLP012 bytes at 0x801CF494 confirm. This target's own
 * exact accessor clears the same 0x12B byte of the same 0x80146250 record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp47800_801CF5E4(
    void) {
  u8* work;

  work = *(u8**)0x1F800044;
  if (D_80143FC8[(u32)work[0x0B] * 0x74u] == 0) {
    ((u8*)*(u32*)0x80146250)[0x12B] = 0;
  }
  func_8014D978();
}
