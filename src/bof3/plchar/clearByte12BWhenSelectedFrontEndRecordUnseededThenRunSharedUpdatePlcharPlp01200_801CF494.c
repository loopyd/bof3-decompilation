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

/* Provenance: this body's 0x6C original bytes at payload offset 0x1094 compare
 * equal to the span lifted exact at 0x801CF494 in the plp016 overlay, which
 * loads at this payload's own base address, so its statement order, operand
 * forms, offset spelling and declaration style were transcribed from that
 * source. Every symbol this body touches resolves in the shared main executable
 * or the scratchpad, so the bindings carry over unchanged; this span's own
 * reference word at 0x801CFDA8 and its absent jal were re-read from the payload
 * above.
 */
/* @source 0x801CF494
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. It reads the record index from
 *           work byte 0x0B and looks that row up in the shared 0x74-byte
 *           front-end record table at 0x80143FC8, scaling the index by the row
 *           size. When the selected row's byte 0 reads 0 -- the record is still
 *           free, since every seeder of this table stores 1 there -- it clears
 *           the state byte at 0x12B of the record published at 0x80146250.
 *           That byte's role in this overlay is not established, so it stays
 *           address-anchored. Both paths then run the shared no-argument update
 *           helper func_8014D978. The body writes no other state. The
 *           pointer-cell load precedes frame creation, the 0x18-byte frame only
 *           keeps $ra at 0x10 across the call, and the slots after the index
 *           byte, after the row byte and after the branch stay nops. No jal in
 *           this payload names it: its only reference is the word 0x801CF494 at
 *           0x801CFDA8, entry 10 of the sixteen-word handler run at 0x801CFD80,
 *           which this overlay's work-byte-2 dispatchers index -- the one based
 *           at 0x801CFDA4 reaches it at index 1 and the one based at 0x801CFD98
 *           at index 4 -- so it takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp01200_801CF494(
    void) {
  u8* work;

  work = *(u8**)0x1F800044;
  if (D_80143FC8[(u32)work[0x0B] * 0x74u] == 0) {
    ((u8*)*(u32*)0x80146250)[0x12B] = 0;
  }
  func_8014D978();
}
