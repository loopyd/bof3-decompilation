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

/* Seeded from the twin lifts of the same 0x6C-byte body at plp278 0x801CFBC0
 * and plp016 0x801CF494, both of which measured exact when this seed was
 * written. The clone dry-run compared this span byte-identical to the plp278
 * representative in the payload, so the statement order, the row symbol and the
 * declaration style were carried over with this function's own name and
 * provenance.
 */
/* @source 0x801CF9C4
 * @behavior Overlay step reached from this payload's own handler run. It reads
 *           the front-end record index from work byte 0x0B of the record
 *           published at scratchpad pointer cell 0x1F800044 and looks that row
 *           up in the shared 0x74-byte front-end record table at 0x80143FC8.
 *           When the selected row byte 0 reads 0 -- the record is still free,
 *           since every seeder of this table stores 1 there -- it clears the
 *           state byte at 0x12B of the record published at 0x80146250. That
 *           byte role in this overlay is not established, so it stays
 *           address-anchored. Both paths then run the shared no-argument update
 *           helper func_8014D978. It takes no arguments, returns nothing and
 *           writes no other state. The pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           call, and the slots after the index byte, after the row byte and
 *           after the branch stay nops. The original does not bound the index
 *           before the scaled table load. The only word of this payload that
 *           names this function is the pointer 0x801CF9C4 stored at 0x801CFAF8,
 *           entry 16 of the twenty-three-word handler run based at 0x801CFAB8,
 *           so an in-payload dispatcher enters it: it has no jal caller.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp57800_801CF9C4(
    void) {
  u8* work;

  work = *(u8**)0x1F800044;
  if (D_80143FC8[(u32)work[0x0B] * 0x74u] == 0) {
    ((u8*)*(u32*)0x80146250)[0x12B] = 0;
  }
  func_8014D978();
}
