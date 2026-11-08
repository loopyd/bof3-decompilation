#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2E08
 * @behavior Queues the three shared UI elements at x = 0x9 with y = 0x59,
 *           0x5A and 0x5B, every one at size 0x51, through the shared marker
 *           queue func_801A4BC0; takes no arguments and returns nothing, and
 *           its 0x18-byte frame only keeps $ra across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the three jal instructions at 0x801F2E18/0x801F2E28/
 * 0x801F2E38 all target the shared marker queue func_801A4BC0, and the delay
 * slots beside them carry this function's own literals from the original
 * instruction words - a0 = 0x9 and a2 = 0x51 in all three, a1 = 0x59, 0x5A,
 * 0x5B. That is one x column holding a three-cell run of consecutive y values
 * at a single size, so the name follows the established block grammar of
 * queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC and
 * the size-suffixed form of
 * queueSharedUiElement9x3Size50World04Area17213_801F4D58. The 0x48-byte body
 * with its own prologue, its three calls and its own jr $ra is code, not data,
 * and a full payload search for these 0x48 bytes finds only this copy, so the
 * clone lever does not apply. The sibling func_801F2FD0 at 0x801F2FD0 is
 * instruction-identical except that its three delay slots carry size 0x10, so
 * it supplies shape evidence only and this function writes its own literal.
 */
void queueSharedUiElementBlock9x59To5BSize51World03Area13613_801F2E08(void) {
  func_801A4BC0(0x9, 0x59, 0x51);
  func_801A4BC0(0x9, 0x5A, 0x51);
  func_801A4BC0(0x9, 0x5B, 0x51);
}
