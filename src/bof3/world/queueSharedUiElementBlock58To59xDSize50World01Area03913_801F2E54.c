#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2E54
 * @behavior Queues the two shared UI elements at x = 0x58 and x = 0x59 with
 *           y = 0xD, both at size 0x50, through the shared marker queue
 *           func_801A4BC0; takes no arguments, returns nothing and its
 *           0x18-byte frame only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the two jal instructions at 0x801F2E64 and 0x801F2E74 both
 * target the shared marker queue func_801A4BC0, and the instructions beside
 * them carry this function's own literals from the original instruction words
 * 0x801F2E5C, 0x801F2E60, 0x801F2E68, 0x801F2E6C, 0x801F2E70 and 0x801F2E78:
 * a0 = 0x58 then 0x59, a1 = 0xD in both, and a2 = 0x50 in both. That is one
 * pair of adjacent x columns at a single y and a single size, so the name
 * follows the block grammar of the exact same-target sibling
 * queueSharedUiElementBlock58To59xDSize51World01Area03913_801F2E1C at
 * 0x801F2E1C, whose 14 instruction words differ from this span only in those
 * two 0x51-versus-0x50 size immediates.
 *
 * The payload's only in-image reference is the code-pointer word at
 * 0x801F4BA8 in the trailing pointer run that starts at 0x801F4B90 with this
 * overlay's first function 0x801F2C04, so no caller names argument roles.
 */
void queueSharedUiElementBlock58To59xDSize50World01Area03913_801F2E54(void) {
  func_801A4BC0(0x58, 0xD, 0x50);
  func_801A4BC0(0x59, 0xD, 0x50);
}
