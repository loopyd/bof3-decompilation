#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F494C
 * @behavior Queues the four shared UI elements of one two-by-two block through
 *           the shared marker queue func_801A4BC0: the two cells at y = 0x3C of
 *           x = 0xF and 0x10 with size 0, then the two cells at y = 0x3B of the
 *           same two columns with size 0x51. Takes no arguments and returns
 *           nothing, and its 0x18-byte frame only keeps $ra across the four
 *           calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the four jal instructions at 0x801F495C, 0x801F496C,
 * 0x801F497C and 0x801F498C all target the shared marker queue func_801A4BC0,
 * and their delay slots carry this function's own literals from the original
 * instruction words - a0 = 0xF, 0x10, 0xF, 0x10 with a1 = 0x3C, 0x3C, 0x3B,
 * 0x3B and a2 = 0, 0, 0x51, 0x51. That is two adjacent x columns over two rows,
 * so the name follows the block grammar of
 * queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC and the
 * size-suffixed form of
 * queueSharedUiElement9x3Size50World04Area17213_801F4D58, with both sizes in
 * body order. The body has its own prologue, its four calls and its own jr $ra,
 * so it is code rather than data, and a scan of the 396 payload binaries for
 * these 0x58 bytes finds no second copy, so the clone lever does not apply.
 * The pointer word at 0x801F67C4 lists this address inside the overlay's
 * handler pointer run, so no in-overlay caller names an extra role.
 */
void queueSharedUiElementBlockFTo10x3BTo3CSize0And51World04Area18813_801F494C(void) {
  func_801A4BC0(0xf, 0x3c, 0);
  func_801A4BC0(0x10, 0x3c, 0);
  func_801A4BC0(0xf, 0x3b, 0x51);
  func_801A4BC0(0x10, 0x3b, 0x51);
}
