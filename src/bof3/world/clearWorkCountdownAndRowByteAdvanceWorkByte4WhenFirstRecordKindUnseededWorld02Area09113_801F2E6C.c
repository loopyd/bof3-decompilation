#include "bof3/bof3.h"

/* Leading byte of the shared 0x140-byte-stride record table based at
 * 0x80145FBB. The exact game00 lift func_8019FF88 copies the per-kind byte
 * D_801C8380[kind] into that leading byte of every record, so zero here means
 * the first record still carries no kind. */
extern u8 D_80145FBB; /* @source 0x80145FBB @kind unknown */

/* Scratchpad work-record pointer cell. Every access group re-materialises the
 * cell through lui plus the low-half load, which is how the original reads the
 * record again for each byte store. The area091 sibling
 * copyHalvedPalettePairOnCountdownAndAdvanceRowWorld02Area09113_801F3318 binds
 * the same cell the same way. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Same-binary sound-cue dispatcher at 0x8015DF18 in executable space, reached
 * through this payload's undefined_funcs_auto row. */
void func_8015DF18(u16 arg0);

/* @source 0x801F2E6C
 * @behavior Reads the shared record-kind byte at 0x80145FBB (lbu). While that
 *           byte is zero it dispatches the cue 0x200 through func_8015DF18,
 *           then clears the scratchpad work record read through pointer cell
 *           0x1F800044: byte +0xA (the palette-stage countdown the area091
 *           sibling decrements) and byte +0xB (the colour-row index that
 *           sibling re-arms and advances to 0x10) go to zero, and byte +4,
 *           the value that sibling increments on the row wrap, is
 *           read-modify-written with +1. Each of the three accesses reloads
 *           the cell, so the record is reached three times. A nonzero kind
 *           byte skips the whole body. No arguments, no return value; the only
 *           frame is the 0x18-byte caller frame with ra at 0x10. The 0x68-byte
 *           range holds no in-image pointer to 0x801F2E6C, so the boundary
 *           rests on the body's own prologue, single `jr $ra` return and the
 *           neighbouring Splat rows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkCountdownAndRowByteAdvanceWorkByte4WhenFirstRecordKindUnseededWorld02Area09113_801F2E6C(
    void) {
  if (D_80145FBB == 0) {
    func_8015DF18(0x200);
    D_1F800044[0xA] = 0;
    D_1F800044[0xB] = 0;
    D_1F800044[4] = D_1F800044[4] + 1;
  }
}
