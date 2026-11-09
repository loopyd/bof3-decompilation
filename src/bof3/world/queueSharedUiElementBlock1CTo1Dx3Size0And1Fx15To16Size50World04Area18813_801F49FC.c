#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F49FC
 * @behavior Queues two runs of shared UI elements through the shared marker
 *           queue func_801A4BC0: first the two elements at y = 3 of x = 0x1C and
 *           0x1D with size 0, then the two elements at x = 0x1F of y = 0x15 and
 *           0x16 with size 0x50. Takes no arguments and returns nothing, and its
 *           0x18-byte frame only keeps $ra across the four calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the four jal instructions at 0x801F4A0C, 0x801F4A1C,
 * 0x801F4A2C and 0x801F4A3C all target the shared marker queue func_801A4BC0,
 * and their delay slots carry this function's own literals from the original
 * instruction words - a0 = 0x1C, 0x1D, 0x1F, 0x1F with a1 = 3, 3, 0x15, 0x16
 * and a2 = 0, 0, 0x50, 0x50. That is a single row of two cells followed by a
 * single column of two cells, so the name follows the block grammar of
 * queueSharedUiElementBlockFTo10x3BTo3CSize0And51World04Area18813_801F494C and
 * queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC with the
 * sizes in body order. The body has its own prologue, its four calls and its own
 * jr $ra, so it is code rather than data, and a scan of the 396 payload binaries
 * for these 0x58 bytes finds no second copy, so the clone lever does not apply.
 * The pointer word at 0x801F67CC lists this address inside the overlay's
 * 14-entry handler pointer run at 0x801F67A4, so no in-overlay caller names an
 * extra role.
 */
void queueSharedUiElementBlock1CTo1Dx3Size0And1Fx15To16Size50World04Area18813_801F49FC(
    void) {
  func_801A4BC0(0x1c, 0x3, 0);
  func_801A4BC0(0x1d, 0x3, 0);
  func_801A4BC0(0x1f, 0x15, 0x50);
  func_801A4BC0(0x1f, 0x16, 0x50);
}
