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

/* @source 0x801CFBC0
 * @behavior Overlay step reached from outside this payload. It reads the
 *           front-end record index from work byte 0x0B of the record published
 *           at scratchpad pointer cell 0x1F800044 and looks that row up in the
 *           shared 0x74-byte front-end record table at 0x80143FC8. When the
 *           selected row byte 0 reads 0 -- the record is still free, since
 *           every seeder of this table stores 1 there -- it clears the state
 *           byte at 0x12B of the record published at 0x80146250. That byte
 *           role in this overlay is not established, so it stays
 *           address-anchored. Both paths then run the shared no-argument update
 *           helper func_8014D978. It takes no arguments, returns nothing and
 *           writes no other state. The pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           call, and the slots after the index byte, after the row byte and
 *           after the branch stay nops. No word of this payload names this
 *           function, so it is an overlay entry reached from outside the
 *           payload, and its prologue and return path alone place the
 *           boundary. The 0x6C-byte span was proven byte-identical to the
 *           plp016 overlay function at 0x801CF494, which was lifted exact when
 *           this body was seeded from it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp27800_801CFBC0(
    void) {
  u8* work;

  work = *(u8**)0x1F800044;
  if (D_80143FC8[(u32)work[0x0B] * 0x74u] == 0) {
    ((u8*)*(u32*)0x80146250)[0x12B] = 0;
  }
  func_8014D978();
}
