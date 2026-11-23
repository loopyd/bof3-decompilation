#include "bof3/bof3.h"

/* Shared four-argument scripted-motion starter at 0x8015D554, which sits in the
 * main executable's image (0x80096800 to 0x801F7000) rather than in this
 * overlay payload. The scenario state sequences call it with (0, 0, 4, 0x1E),
 * the same tuple this entry passes, and with (0, 1, 4, 0x1E) as the armed form,
 * so all four parameters are real and no role beyond that is named here. The
 * address takes no row in this target's map because it is out of payload, which
 * keeps the raw prototype form the scenario sources use.
 * @source 0x8015D554 @kind function */
extern void func_8015D554(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* @source 0x801F2F68
 * @behavior Runs the shared four-argument scripted-motion starter
 *   func_8015D554 (0x8015D554) with (0, 0, 4, 0x1E). The original folds $a0
 *   and $a1 to zero with addu $reg,$zero,$zero, materialises 4 into $a2 ahead
 *   of the jal and puts 0x1E into $a3 in the branch delay slot. Takes no
 *   arguments, reads no state and builds a 0x18-byte frame only to keep the
 *   return address across the call. The payload has no jal to this address; its
 *   only in-image reference is the code-pointer word at 0x801F4BC4 in the
 *   trailing pointer run of the same overlay, which starts at 0x801F4B90 with
 *   the payload's first function 0x801F2C04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioScriptedMotionZero(void) {
  func_8015D554(0, 0, 4, 0x1E);
}
