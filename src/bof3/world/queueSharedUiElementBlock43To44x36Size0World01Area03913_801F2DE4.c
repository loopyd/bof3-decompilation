#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2DE4
 * @behavior Queues the two shared UI elements at x = 0x43 and x = 0x44 with
 *           y = 0x36, both at size 0, through the shared marker queue
 *           func_801A4BC0; takes no arguments, returns nothing and its
 *           0x18-byte frame only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the two jal instructions at 0x801F2DF4 and 0x801F2E04 both
 * target the shared marker queue func_801A4BC0, and the delay slots beside
 * them carry this function's own literals from the original instruction words
 * 0x801F2DEC, 0x801F2DF0, 0x801F2DFC and 0x801F2E00: a0 = 0x43 then 0x44,
 * a1 = 0x36 in both, and a2 = 0 in both, folded to the register copy
 * addu $a2,$zero,$zero. That is one pair of adjacent x columns at a single y
 * and a single size, so the name follows the block grammar of
 * queueSharedUiElementBlock3To4x7To8Size0World00Area02213_801F2CC4 and the
 * size-suffixed form of queueSharedUiElementBlockFTo10x3BTo3CSize0And51World04Area18813_801F494C.
 * The payload's only in-image reference is the code
 * pointer word at 0x801F4BA0 in the trailing pointer run that starts at
 * 0x801F4B90 with this overlay's first function 0x801F2C04, so no caller names
 * argument roles; the three parameter roles are those of the shared queue
 * declaration in include/bof3/world/area00813_internal.h.
 */
void queueSharedUiElementBlock43To44x36Size0World01Area03913_801F2DE4(void) {
  func_801A4BC0(0x43, 0x36, 0);
  func_801A4BC0(0x44, 0x36, 0);
}
