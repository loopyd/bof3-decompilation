#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F4A54
 * @behavior Queues two runs of shared UI elements through the shared marker
 *           queue func_801A4BC0: first the two elements at y = 3 of x = 0x1C
 *           and 0x1D with size 0xC0, then the two elements at x = 0x20 of
 *           y = 0x15 and 0x16 with size 0xA1. Takes no arguments and returns
 *           nothing, and its 0x18-byte frame only keeps $ra across the four
 *           calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the four jal instructions at 0x801F4A64, 0x801F4A74,
 * 0x801F4A84 and 0x801F4A94 all target the shared marker queue func_801A4BC0,
 * and their delay slots carry this function's own literals from the original
 * instruction words - a0 = 0x1C, 0x1D, 0x20, 0x20 with a1 = 3, 3, 0x15, 0x16
 * and a2 = 0xC0, 0xC0, 0xA1, 0xA1. That is a single row of two cells followed
 * by a single column of two cells, so the name follows the block grammar of
 * the neighbouring queueSharedUiElementBlock1CTo1Dx3Size0And1Fx15To16Size50
 * World04Area18813_801F49FC and its own queueSharedUiElementBlockFTo10x3BTo3C
 * Size0And51World04Area18813_801F494C sibling, with the sizes in body order.
 * The body has its own prologue, its four calls and its own jr $ra, so it is
 * code rather than data, and a byte scan of the 396 payload binaries for these
 * 0x58 bytes finds only this span at payload offset 0x1E54 of
 * emi/world04/area188/13.bin, so the clone lever does not apply. The pointer
 * word at 0x801F67D0 lists this address inside the overlay's 14-entry handler
 * pointer run at 0x801F67A4, entry 11 of 14, so no in-overlay caller names an
 * extra role.
 */
void queueSharedUiElementBlock1CTo1Dx3SizeC0And20x15To16SizeA1World04Area18813_801F4A54(
    void) {
  func_801A4BC0(0x1c, 0x3, 0xc0);
  func_801A4BC0(0x1d, 0x3, 0xc0);
  func_801A4BC0(0x20, 0x15, 0xa1);
  func_801A4BC0(0x20, 0x16, 0xa1);
}
