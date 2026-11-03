#include "bof3/bof3.h"

/*
 * Handler table based at 0x801F0C98, the table this dispatcher selects from.
 * Its three words are 0x801EEC94 (this overlay's byte-2 dispatcher), 0x801EF018
 * and 0x801EF30C (overlay dispatchers of that same shape). The handler run
 * those dispatchers select from starts at 0x801F0CA4, so only the three words at
 * 0x801F0C98 are claimed here. The run continues to 0x801F0CD0, whose first word
 * is the scalar 0x60 and is not a code pointer.
 */
extern void (*D_801F0C98[])(void); /* @source 0x801F0C98 @kind data */

/* Battle selection substate index maintained outside this overlay; read once,
 * non-volatile, exactly as the original does. */
extern u8 D_801462E3; /* @source 0x801462E3 @kind unknown */

/* @source 0x801EEC58
 * @behavior Battle substate dispatch: reads the unsigned byte at 0x801462E3,
 *   scales it by four and calls the handler pointer at that index of the table
 *   based at 0x801F0C98 with no arguments. The selected handler's $v0 is left
 *   in place by the epilogue, so this shim's result is whatever that handler
 *   returns. The index load precedes the 0x18-byte frame, which only keeps $ra
 *   across the indirect call, and the table address is built as
 *   `lui $at, %hi(table)` / `addu $at, $at, index * 4` / `lw $v0, %lo(table)($at)`.
 *   Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstateTable0c98BatlEnd00_801EEC58(void) {
  D_801F0C98[D_801462E3]();
}
