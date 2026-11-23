#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2E1C
 * @behavior Queues the two shared UI elements at x = 0x58 and x = 0x59 with
 *           y = 0xD, both at size 0x51, through the shared marker queue
 *           func_801A4BC0; takes no arguments, returns nothing and its
 *           0x18-byte frame only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the two jal instructions at 0x801F2E2C and 0x801F2E3C both
 * target the shared marker queue func_801A4BC0, and the delay slots beside
 * them carry this function's own literals from the original instruction words
 * 0x801F2E24, 0x801F2E28, 0x801F2E30, 0x801F2E34, 0x801F2E38 and 0x801F2E40:
 * a0 = 0x58 then 0x59, a1 = 0xD in both, and a2 = 0x51 in both. That is one
 * pair of adjacent x columns at a single y and a single size, so the name
 * follows the block grammar of the exact same-target sibling
 * queueUiPairZero at
 * 0x801F2DE4 and of
 * queueSharedUiElementBlock9x59To5BSize51World03Area13613_801F2E08.
 *
 * The payload's only in-image reference is the code-pointer word at
 * 0x801F4BA4 in the trailing pointer run that starts at 0x801F4B90 with this
 * overlay's first function 0x801F2C04, so no caller names argument roles.
 */
void queueUiElementX(void) {
  func_801A4BC0(0x58, 0xD, 0x51);
  func_801A4BC0(0x59, 0xD, 0x51);
}
