#include "bof3/bof3.h"

extern s8 D_80146872;
extern u32 D_801FE288[];

/* @source 0x801F719C
 * @behavior Per-frame dispatcher of this overlay's scenario handler chain: it
 * reads the signed shared main-RAM scenario byte D_80146872 (0x80146872),
 * scales it by four and loads the selected word from the overlay-local
 * code-pointer run at 0x801FE288, then invokes that word with `jalr` and no
 * arguments. The run holds 31 words (0x801FE288 through 0x801FE300) and its
 * entry 0 is clearSharedByteAndProgressThenAdvanceStateScenarioScena0200_801F71D8
 * (0x801F71D8, this overlay, exact), which writes 1 to D_80146872 itself, so the
 * byte walks the chain forward one entry at a time; entry 30 is the asm boundary
 * func_801FBC6C (0x801FBC6C) and the run ends immediately before the overlay
 * callback table at 0x801FE304 that func_801FD084 (0x801FD084) indexes with byte
 * 0x7A of its record argument. It takes no arguments and returns nothing of its
 * own, reads no state other than that byte and writes none; the 0x18-byte frame
 * exists only to keep $ra across the indirect call and the state-byte read is
 * hoisted above the frame setup. Its 0x3C bytes are byte-identical to the exact
 * sibling dispatchers dispatchProgressHandlerScenarioScena1200_801F7F28
 * (0x801F7F28, emi/scenario/scena12/00) and
 * dispatchProgressHandlerScenarioScena0100_801F7FC4 (0x801F7FC4,
 * emi/scenario/scena01/00) apart from instruction 7, the `lw` naming each
 * overlay's own table (0x801FD1B8 and 0x801FE29C respectively), so the role word
 * is the family's evidenced name for a D_80146872-indexed handler dispatcher.
 * Its only image word is 0x801F719C at 0x801FE274, immediately before that chain
 * table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchProgressHandlerScenarioScena0200_801F719C(void) {
  ((void (*)(void))D_801FE288[(s8)D_80146872])();
}
