#include "bof3/bof3.h"

/*
 * Handler run based at 0x801F0CA4, the run this dispatcher selects from. It is
 * the tail of the reviewed `textbin` row D_801F0C98: that row holds fourteen
 * code pointers from 0x801F0C98 through 0x801F0CD0 before the scalar 0x60, and
 * 0x801F0CA4 is its fourth word, so this dispatcher's base is entry 3 of the
 * run while entry 0 is 0x801EEC94. The extern is declared at the indexed word
 * and the row stays whole.
 */
extern void (*D_801F0CA4[])(void); /* @source 0x801F0CA4 @kind data */

/* Battle selection state byte maintained outside this overlay; read once,
 * non-volatile, exactly as the original does. */
extern u8 D_801462E4; /* @source 0x801462E4 @kind unknown */

/* @source 0x801EEC94
 * @behavior Battle handler-run dispatch: reads the unsigned byte at 0x801462E4,
 *   scales it by four and calls the handler pointer at that index of this
 *   overlay's handler run based at 0x801F0CA4 with no arguments. The selected
 *   handler's $v0 is left in place by the epilogue, so this shim's result is
 *   whatever that handler returns. The index load precedes the 0x18-byte frame,
 *   which only keeps $ra across the indirect call, the original does not bound
 *   the index before the scaled table load, and the table address is built as
 *   `lui $at, %hi(run)` / `addu $at, $at, index * 4` / `lw $v0, %lo(run)($at)`.
 *   The slot after the index byte and the slot after the table load stay nops.
 *   Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstateTable0ca4BatlEnd00_801EEC94(void) {
  D_801F0CA4[D_801462E4]();
}
