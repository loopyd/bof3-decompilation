#include "bof3/bof3.h"

/* Shared world-marker queue: the sibling world targets declare the same
 * prototype (it is absent from the shipped headers), and this payload's
 * generated undefined_funcs_auto.txt row binds the address, so the name keeps
 * the raw spelling.
 * @source 0x801A4BC0 @kind function */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2D88
 * @behavior Queues four shared UI elements through func_801A4BC0: the element
 *           at (0x55, 0x1C) with size 0xC0, the element at (0x55, 0x1D) with
 *           size 0xC0, the element at (0x54, 0x1C) with size 0xA1 and the
 *           element at (0x54, 0x1D) with size 0xA1. Takes no arguments and
 *           returns nothing; its 0x18-byte frame only keeps $ra across the four
 *           calls, and $v0 carries the zero-sized return of each call through
 *           the next call's delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the four jal words at 0x801F2D98, 0x801F2DA8, 0x801F2DB8 and
 * 0x801F2DC8 all target the shared marker queue func_801A4BC0, and their delay
 * slots carry this function's own literals from the original instruction words
 * - a0 = 0x55, 0x55, 0x54, 0x54, a1 = 0x1C, 0x1D, 0x1C, 0x1D and a2 = 0xC0 on
 * the two x = 0x55 calls and 0xA1 on the two x = 0x54 calls. That is a
 * two-column block, x = 0x55 covering y = 0x1C to 0x1D at one size and x = 0x54
 * covering the same rows at another, emitted column by column, so the name
 * follows the block grammar of the neighbouring
 * queueSharedUiElementBlock55x1CTo1DAnd54x1CTo1DSize0World00Area00313_801F2D30
 * with a size per column in the form
 * queueSharedUiElementBlock1CTo1Dx3SizeC0And20x15To16SizeA1World04Area18813_801F4A54
 * uses.
 *
 * Shape evidence: the body has its own prologue, its four calls and its own
 * jr $ra, so it is code rather than data, and no jal word in this payload
 * targets it, so it is a script entry whose argument roles stay structural. A
 * scan of every payload under out/binaries for this 88-byte span finds only
 * this copy, so the clone lever did not apply; the nearest exact sibling, the
 * same target's 0x801F2D30 block, is instruction-identical apart from the size
 * literals its delay slots carry, so it supplied the statement order, operand
 * forms and declaration style while the constants above come from this span's
 * own words.
 */
void queueSharedUiElementBlock55x1CTo1DSizeC0And54x1CTo1DSizeA1World00Area00313_801F2D88(void) {
  func_801A4BC0(0x55, 0x1C, 0xC0);
  func_801A4BC0(0x55, 0x1D, 0xC0);
  func_801A4BC0(0x54, 0x1C, 0xA1);
  func_801A4BC0(0x54, 0x1D, 0xA1);
}
