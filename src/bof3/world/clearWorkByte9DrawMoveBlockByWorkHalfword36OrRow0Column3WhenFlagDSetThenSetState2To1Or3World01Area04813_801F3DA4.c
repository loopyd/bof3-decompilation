#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); spelling the
 * bare constant address makes the compiler keep one base in a saved register
 * across the two calls instead, which costs the frame slot.
 * @source 0x1F800044 @kind data
 */
extern u8* workRecordCursor1F800044;

/* Flag bank of the shared main executable that the shared flag tester below
 * reads. This body passes its address, so it is declared as the object the
 * address names.
 * @source 0x80144F28 @kind data
 */
extern u8 D_80144F28;

/* Shared flag tester of the main executable: reads the bit its second argument
 * selects in the flag bank its first argument addresses and reports it as a
 * nonzero return value.
 * @source 0x8015B5D4
 */
extern s32 func_8015B5D4(u32 flag_bank, s32 bit_index);

/* Draw-move emitter at 0x801F41F0, shared by this overlay's move-block
 * handlers. Its own bytes take a grid row index and an 8-pixel column index,
 * build the 0x8 by 0x78 rect x = 0x240 + row * 0x40 + (column + 1) * 8,
 * y = 0x160 at +0x10 of its frame, hand that rect to SetDrawMove with the
 * primitive published in g_PrimCursor and then call func_8014E5A0(6, 0x18);
 * both argument roles stay described by that arithmetic because the emitter is
 * unlifted.
 * @source 0x801F41F0
 */
extern void func_801F41F0(s16 row, u8 column);

/* @source 0x801F3DA4
 * @behavior Entry 0, the first of the six handlers of the byte-+0x02 run based
 *           at 0x801F4E54 that this target's dispatcher at 0x801F3D60 selects,
 *           so no jal reaches this body. It reads the scratchpad work-record
 *           pointer cell 0x1F800044 and clears that record's byte +0x09, the
 *           scroll column the later handlers of the same run advance, then
 *           re-reads the cell and tests the record's signed halfword at +0x36,
 *           the grid row the move-block emitter is handed. While that halfword
 *           is zero it asks the shared flag tester 0x8015B5D4 for bit 0xD of
 *           the main-RAM flag bank 0x80144F28, and when that bit reads set it
 *           calls the emitter at 0x801F41F0 with row 0 and column 3, re-reads
 *           the cell and stages dispatch state 3, which selects entry 3 of the
 *           same run for the next frame. In every other case - a nonzero +0x36,
 *           or a zero +0x36 with that flag clear - it re-reads the cell, calls
 *           the emitter with that halfword as the row and column 0, re-reads the
 *           cell once more and stages dispatch state 1, entry 1 of the same run.
 *           Each arm writes its own staged state into the record byte +0x02, and
 *           the build merges those two arm stores into the one store the
 *           original shows after the return address is restored. The 0x18-byte
 *           frame holds the return address at +0x10; the function takes no
 *           arguments and returns nothing.
 *
 * Name evidence: the handler table at 0x801F4E54 holds this span's address as
 *   its word 0, followed by 0x801F3E38, 0x801F3EE4, 0x801F3F98, 0x801F40B4 and
 *   0x801F4148, and the dispatcher lifted at 0x801F3D60 indexes that table with
 *   the work record's byte +0x02, so the two state values this body stores are
 *   run entries rather than arbitrary numbers. The clearing of byte +0x09 and
 *   the halfword +0x36 row argument are the shapes the two lifted move-block
 *   handlers of this run established: 0x801F40B4 and 0x801F4148 both hand that
 *   halfword to the emitting routine 0x801F41F0 as its grid row and both step
 *   byte +0x09. The flag probe is the same bit 0xD of the same bank 0x80144F28
 *   that the overlay's 0x801F4804 handler sets through the shared setter
 *   0x8015B5F4.
 *
 * Shape evidence: the statement order, the per-access pointer-local spelling of
 *   the scratchpad cell and the re-read grouping come from the two exact
 *   siblings of this payload at 0x801F40B4 and 0x801F4148, whose sources were
 *   written against the same cell. The two-test entry gate
 *   (halfword compare, then the shared flag probe, both leaving for the same
 *   arm) is the shape the unlifted neighbour 0x801F3F98 shows in its own bytes,
 *   and the per-arm pointer re-read is the shape the unlifted twin 0x801F3EE4
 *   shows. Every literal here is written from this span's own bytes: flag bit
 *   0xD, state 3 with row 0 and column 3, state 1 with column 0.
 *
 *   Store shape: the store sits in each arm rather than being selected into a
 *   value local that one post-branch store writes. One value local assigned in
 *   both arms measured 36 of 37 instructions at the original size: this build
 *   scheduled that literal ahead of the else arm's base re-materialisation,
 *   where the original issues the load first, and the two arm stores coalesce
 *   into the original's single join store once each arm writes its own.
 *
 * Clone declined: a scan of all 396 extracted payloads under out/binaries for
 *   this 0x94-byte body, and a rescan of every 4-byte offset that ignored the
 *   `jal`/`j` immediate words so a relocation-different sibling could still
 *   match, returned only this span's own offset 4516 in its own payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9DrawMoveBlockByWorkHalfword36OrRow0Column3WhenFlagDSetThenSetState2To1Or3World01Area04813_801F3DA4(
    void) {
  u8* clearWork;
  u8* gateWork;
  u8* drawWork;
  u8* stateWork;

  clearWork = workRecordCursor1F800044;
  clearWork[0x09] = 0;
  gateWork = workRecordCursor1F800044;
  if (FIELD_REF(s16, gateWork, 0x36) == 0 &&
      func_8015B5D4((u32)&D_80144F28, 0xD) != 0) {
    func_801F41F0(0, 3);
    stateWork = workRecordCursor1F800044;
    stateWork[0x02] = 3;
  } else {
    drawWork = workRecordCursor1F800044;
    func_801F41F0(FIELD_REF(s16, drawWork, 0x36), 0);
    stateWork = workRecordCursor1F800044;
    stateWork[0x02] = 1;
  }
}
