#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F41F8
 * @behavior Queues the two shared UI elements of one cell column through the
 *           shared marker queue func_801A4BC0: the cell at x = 0x40, y = 0x28
 *           with size 0, then the cell at the same x with y = 0x27 and size 0.
 *           Takes no arguments and returns nothing, and its 0x18-byte frame
 *           only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the two jal instructions at 0x801F4208 and 0x801F4218 both
 * target the shared marker queue func_801A4BC0, and the delay slots beside them
 * carry this function's own literals from the original instruction words -
 * a0 = 0x40 in both, a1 = 0x28 then 0x27, and a2 = 0 in both. That is one x
 * column holding a two-cell run of adjacent y values at a single size, so the
 * name follows the block grammar of
 * queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC and the
 * size-suffixed form of
 * queueSharedUiElement9x3Size50World04Area17213_801F4D58. The 0x38-byte body
 * has its own prologue, its two calls and its own jr $ra at 0x801F4228, and the
 * next function begins immediately at 0x801F4230, so this is a whole function
 * rather than data or a shared epilogue. No pointer run or dispatch table in
 * the overlay lists 0x801F41F8, so no in-overlay caller names an extra role and
 * the role stays anchored to the caller-visible queue arguments.
 */
void queueSharedUiElementBlock40x27To28Size0World01Area04113_801F41F8(void) {
  func_801A4BC0(0x40, 0x28, 0);
  func_801A4BC0(0x40, 0x27, 0);
}
