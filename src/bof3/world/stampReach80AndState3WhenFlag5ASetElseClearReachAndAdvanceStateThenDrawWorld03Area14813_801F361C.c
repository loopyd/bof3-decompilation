#include "bof3/bof3.h"

extern u8 D_80144F28;

/* Overlay drawing routine of this payload, lifted at 0x801F37B8: three
 * selection rows of textured quads built from packed vertex triples, taking the
 * selection index and a signed byte mode. */
void drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
    s32 arg0, s32 arg1);

s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801F361C
 * @behavior Handler at word 0 of the overlay state-byte-2 dispatch run based
 *           at 0x801F5024, so it has no jal caller: the dispatcher at
 *           0x801F35D8 reaches it through that table. Probes flag 0x5A of the
 *           main-RAM flag bank D_80144F28 through the shared flag tester at
 *           0x8015B5D4. When the flag reads set it stamps 0x80 into the work
 *           record's halfword at +0x30 of the scratchpad pointer cell
 *           0x1F800044, writes 3 into the record dispatch byte at +2 so the
 *           next frame runs word 3 of the same run, and calls the overlay quad
 *           drawing routine at 0x801F37B8 with selection 0 and signed byte mode
 *           -2. When the flag reads clear it reloads the record pointer, reads
 *           the dispatch byte at +2 once, clears the halfword at +0x30, writes
 *           that byte plus one back so the next frame runs word 1, and calls
 *           the quad routine twice: selection 0 with mode 1, then selection 1
 *           with mode 1. Takes no arguments and returns nothing; its 0x18-byte
 *           frame only holds ra at +0x10, and the frame is created before the
 *           flag probe.
 *
 * Name evidence: the probed flag is the `lui/addiu $a0, %hi(D_80144F28)` plus
 * `addiu $a1, $zero, 0x5A` argument pair; the halfword at +0x30 is the record
 * reach counter the sibling handlers of this run step by 8 and advance past
 * 0x80, and the already-lifted entry 0 of the first run in this payload
 * (seedRecordFromIndexTables...801F2CA0) stamps the same 0x80 and the same
 * state 3 under a window gate. The record byte at +2 is the index the
 * state-byte-2 dispatcher reads, so the two stores to it are dispatch state
 * writes: 3 in the flag-set branch and one past the current index otherwise,
 * which lands on word 1 (0x801F36A0) as the table bytes show. The draw calls
 * carry the selection indexes and modes the bytes show, in order.
 *
 * Shape evidence: an exact scan of the 132-byte span over every payload under
 * out/binaries finds it only at its own offset 0x0A1C, and a masked rescan with
 * call-target and branch words masked finds no second copy, so the clone lever
 * does not apply; the range carries its own prologue and `jr $ra`, so the data
 * and carve levers do not apply either. The two-read shape of the scratchpad
 * cell is what the dispatcher and the sibling handler at 0x801F3720 of the same
 * run also produce. The nearest same-shape function in another payload,
 * zeroWorkStepHalfword30...World02Area11113_801F3130, interleaves the same
 * probes and stores in a different order with a different flag index and callee,
 * so it is shape evidence only. The two arms carry their own scratchpad pointer
 * load: the original materializes the cell in $v0 in the flag-set arm and in
 * $v1 in the flag-clear arm, so the two loads are independent values. Sharing
 * one pointer variable across the arms coalesces them into a single register
 * and diverges from the original at the first load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stampReach80AndState3WhenFlag5ASetElseClearReachAndAdvanceStateThenDrawWorld03Area14813_801F361C(
    void) {
  if (func_8015B5D4((u32)&D_80144F28, 0x5A) != 0) {
    u8* work = SPAD_PTR_SLOT(u8, 0x44u);

    FIELD_REF(u16, work, 0x30) = 0x80;
    work[2] = 3;
    drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
        0, -2);
  } else {
    u8* work = SPAD_PTR_SLOT(u8, 0x44u);

    work[2] = work[2] + 1;
    FIELD_REF(u16, work, 0x30) = 0;
    drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
        0, 1);
    drawThreeSelectionQuadsFromVertexTriplesScaledByHalfword30World03Area14813_801F37B8(
        1, 1);
  }
}
