#include "bof3/bof3.h"

/* Shared world-marker queue: the sibling world targets declare the same
 * prototype (it is absent from the shipped headers), and this payload's
 * generated undefined_funcs_auto.txt row binds the address, so the name keeps
 * the raw spelling.
 * @source 0x801A4BC0 @kind function */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2C4C
 * @behavior Queues three shared world-marker elements through func_801A4BC0 at
 *           (0xC, 0x1E), then (0xC, 0x1C) and (0xC, 0x1D), all with size 0xC0.
 *           Takes no arguments and returns nothing, and its 0x18-byte frame
 *           only keeps $ra across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the three jal instructions at 0x801F2C5C, 0x801F2C6C and
 * 0x801F2C7C all target the shared marker queue func_801A4BC0, and their
 * delay slots carry this function's own literals from the original instruction
 * words - a0 = 0xC for all three calls, a1 = 0x1E, 0x1C, 0x1D in emission order
 * and a2 = 0xC0, 0xC0, 0xC0. That is one column of three cells at x = 0xC
 * covering y = 0x1C to 0x1E, emitted with the 0x1E cell first, so the name
 * follows the block grammar of the queueSharedUiElementBlock11x15To16And12x16
 * Size0World00Area03313_801F2CD0 and queueSharedUiElementBlock1CTo1Dx3SizeC0And
 * 20x15To16SizeA1World04Area18813_801F4A54 siblings with the coordinates in
 * call order and the shared size last.
 *
 * Shape evidence: the body has its own prologue, its three calls and its own
 * jr $ra, so it is code rather than data. The same payload's func_801F2C04 at
 * 0x801F2C04 is instruction for instruction the same stream and differs only in
 * the three a2 immediates (0 there, 0xC0 here), so the shape is fixed by that
 * pair while this function writes its own literals; that twin is still on its
 * asm row, so it supplies shape only and no source to clone. A byte scan of the
 * 0x48-byte span over every payload under out/binaries finds only this span at
 * payload offset 0x4C of emi/world01/area038/13.bin, so the clone lever does
 * not apply either.
 */
void queueElementBlockSize(
    void) {
  func_801A4BC0(0xc, 0x1e, 0xc0);
  func_801A4BC0(0xc, 0x1c, 0xc0);
  func_801A4BC0(0xc, 0x1d, 0xc0);
}
