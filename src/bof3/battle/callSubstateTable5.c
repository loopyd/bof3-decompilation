#include "bof3/bof3.h"

/*
 * Dispatch run based at 0x801EF5F8, the table this shim selects from. It is
 * this overlay's reviewed `textbin` row D_801EF5F8: its first five words are
 * the code pointers 0x801EEC40, 0x801EEC7C, 0x801EEEB8
 * (initBattleScenePrimsAndAdvanceSubstate), 0x801EEF10 and 0x801EF0A8, and the
 * words from 0x801EF60C on are scalars (0xa0, 0x84, ...), not code pointers,
 * so only those five entries are claimed here and the row stays whole. Entry 0,
 * func_801EEC40, raises the index byte at 0x801462E3 by one, so the five words
 * are consecutive state handlers of that byte.
 */
extern void (*D_801EF5F8[])(void); /* @source 0x801EF5F8 @kind data */

/* Battle selection substate byte maintained outside this overlay and shared by
 * the other battle overlays; read once, non-volatile, exactly as the original
 * does. */
extern u8 D_801462E3; /* @source 0x801462E3 @kind unknown */

/* @source 0x801EEC04
 * @behavior Battle substate dispatch: reads the unsigned byte at 0x801462E3,
 *           scales it by four and calls the handler pointer at that index of
 *           this overlay's run based at 0x801EF5F8 with no arguments. The
 *           selected handler's $v0 is left in place by the epilogue, so this
 *           shim's result is whatever that handler returns. The index load
 *           precedes the 0x18-byte frame, which only keeps $ra across the
 *           indirect call, the original does not bound the index before the
 *           scaled table load, and the table address is built as
 *           `lui $at, %hi(D_801EF5F8)` / `addu $at, $at, index * 4` /
 *           `lw $v0, %lo(D_801EF5F8)($at)`. The slot after the table load and
 *           the delay slot after the indirect call stay nops. No word and no
 *           jal inside this payload names this body, so it is the overlay
 *           loader's entry and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callSubstateTable5(void) {
  D_801EF5F8[D_801462E3]();
}
