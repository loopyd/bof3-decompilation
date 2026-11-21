#include "bof3/bof3.h"

void func_801E5988(void);

/* Shared battle record pointer cell in main RAM, outside this overlay image.
 * Splat binds the address through this target generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed. This body reads
 * only its byte 0x0A and leaves that offset anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF528
 * @behavior Advances the shared battle record by one frame and then runs the
 *           shared helper func_801E5988 with no arguments. It reads the record
 *           pointer cell at 0x801EB4E0 once, before it reserves its frame,
 *           increments that record byte 0x0A by one at byte width (lbu 0x0A,
 *           addiu 1, sb 0x0A) and stores it back through the pointer it already
 *           holds, the store filling the call delay slot. Byte 0x0A is the
 *           global work byte A this record carries, the same byte the target
 *           payload reads at 0x801EF83C. It reads no other state, writes none,
 *           takes no arguments and returns nothing through the jr $ra at
 *           0x801EF554 inside the 0x18-byte frame that keeps $ra at 0x10. Its
 *           own address is the third word of the three-word code pointer run at
 *           0x801EEC2C (0x801EF134, 0x801EF3C4, 0x801EF528), so the only
 *           in-image reference to the span is the payload word at 0x801EEC34
 *           and no jal names it. The 0x34 bytes here differ from the exact span
 *           at 0x801EF624 of emi/bmagic/magic054/03 in the three words that
 *           carry that sibling byte offset 0x0B and its decrement, and no other
 *           word, so its statement order, operand forms and declaration style
 *           were transcribed with this span own offset 0x0A and increment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementGlobalWorkByteAAndInvokeHelperBmagicMagic05703_801EF528(void) {
  D_801EB4E0[0xA]++;
  func_801E5988();
}
