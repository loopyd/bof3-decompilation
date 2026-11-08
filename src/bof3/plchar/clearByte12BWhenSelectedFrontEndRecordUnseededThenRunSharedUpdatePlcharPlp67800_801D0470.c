#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows whose byte 0 reads 1 once a seeder claims the record and stays 0 while
 * the record is still free. The original re-materialises `lui at,0x8014` for
 * the lookup, so the row is addressed through its own address-derived symbol,
 * which this target's generated undefined_syms_auto.txt already binds.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];

/* Shared no-argument update helper of the main executable, outside this
 * payload. Splat binds its address through this target's generated
 * undefined_funcs_auto.txt, and other asm rows of this payload call it too.
 * The original sets nothing up in the argument registers and reads no result
 * from $v0.
 */
void func_8014D978(void);

/* Seeded from the lift of the same body at plp016 0x801CF494, whose own source
 * measured exact before this seed was written: the clone dry-run compared this
 * 108-byte span byte-identical to that representative range. This target binds
 * the shared update helper as a function, so the body is copied here with its
 * own binding instead of the emitted raw-spelling clone being applied.
 */
/* @source 0x801D0470
 * @behavior Work byte 3 == 13 step of this overlay, published at the pointer
 *           word 0x801D05C0: word 23 of the thirty-word code-pointer run based
 *           at 0x801D0564 and word 13 of the arm the byte-3 dispatcher
 *           0x801CEF20 indexes from 0x801D058C by work byte 3 scaled by four, so
 *           no jal in this payload names it. It reads the front-end record
 *           index from work byte 0x0B of the record published at scratchpad
 *           pointer cell 0x1F800044 and looks that row up in the shared
 *           0x74-byte front-end record table at 0x80143FC8. When the selected
 *           row byte 0 reads 0, since every seeder of that table stores 1
 *           there, it clears the state byte at 0x12B of the record published at
 *           0x80146250; that byte role in this overlay is not established, so it
 *           stays address-anchored. Both paths then run the shared no-argument
 *           update helper func_8014D978. It takes no arguments, returns nothing
 *           and writes no other state. The pointer-cell load precedes frame
 *           creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           call, and the slots after the index byte, after the row byte and
 *           after the branch stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearByte12BWhenSelectedFrontEndRecordUnseededThenRunSharedUpdatePlcharPlp67800_801D0470(
    void) {
  u8* work;

  work = *(u8**)0x1F800044;
  if (D_80143FC8[(u32)work[0x0B] * 0x74u] == 0) {
    ((u8*)*(u32*)0x80146250)[0x12B] = 0;
  }
  func_8014D978();
}
