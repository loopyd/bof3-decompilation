#include "bof3/bof3.h"

typedef void (*Boss01316Handler)(void);

/*
 * Handler-run word at 0x800C268C, the table this dispatcher selects from: word
 * 27 of the run that the reviewed `textbin` row D_800C25D8 holds from its word
 * 18 at 0x800C2620, ending with the zero word 0x800C26E0. From this word the
 * entries are 0x800C1B4C, the step handler 0x800C1C4C, the inert handler
 * 0x800C1CBC, 0x800C1D08 (entry 0 of the run at 0x800C2698 that
 * callStateTable2698Boss indexes), the shared
 * main-executable handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8 and 0x801E4D8C, then this overlay's
 * further steps. The word is interior to that one row, so the row stays one
 * data row and this word is named at the offset the scaled index adds.
 */
extern Boss01316Handler D_800C268C[]; /* @source 0x800C268C @kind data */

/* @source 0x800C1B14
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the run word 0x800C268C,
 * so the selected handler's result stays in the return register. The original
 * bounds the index only to a byte and never checks it against the run's length,
 * so the zero word 0x800C26E0 that ends the run is reachable; the slot after the
 * scaled table load and the indirect call's delay slot both stay nops, and the
 * indirect call receives no argument of its own. The 0x18-byte frame only keeps
 * $ra at 0x10 across that call, and the epilogue leaves $v0 untouched. No jal and
 * no word of this payload names 0x800C1B14, so the overlay entry is what reaches
 * it. Only the load word's %lo displacement differs from the already-exact
 * dispatcher callIndexedHandler6 of this overlay, whose
 * 56-byte body holds this statement order and these operand forms exactly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01316_800C1B14(u8 index) {
  D_800C268C[index]();
}
