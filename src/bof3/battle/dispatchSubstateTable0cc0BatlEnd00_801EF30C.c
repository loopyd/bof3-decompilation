#include "bof3/bof3.h"

/*
 * Handler run based at 0x801F0CC0, the run this dispatcher selects from. It is
 * the tail of the reviewed `textbin` row D_801F0C98: that row holds fourteen
 * code pointers from 0x801F0C98 through 0x801F0CD0 before the scalar 0x60, and
 * 0x801F0CC0 is its eleventh word, so this dispatcher's base is entry 10 of the
 * run (0x801EF348 first) while entry 0 is 0x801EEC94. The three words at the
 * row's head are this overlay's dispatchers 0x801EEC94, 0x801EF018 and
 * 0x801EF30C, each of which selects from a later word of the same run. The
 * extern is declared at the indexed word and the row stays whole.
 */
extern void (*D_801F0CC0[])(void); /* @source 0x801F0CC0 @kind data */

/* Battle selection state byte maintained outside this overlay; read once,
 * non-volatile, exactly as the original does. */
extern u8 D_801462E4; /* @source 0x801462E4 @kind unknown */

/* @source 0x801EF30C
 * @behavior Battle handler-run dispatch: reads the unsigned byte at 0x801462E4,
 *   scales it by four and calls the handler pointer at that index of this
 *   overlay's handler run based at 0x801F0CC0 with no arguments. The selected
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
void dispatchSubstateTable0cc0BatlEnd00_801EF30C(void) {
  D_801F0CC0[D_801462E4]();
}
