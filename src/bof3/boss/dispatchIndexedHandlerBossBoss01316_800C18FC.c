#include "bof3/bof3.h"

typedef void (*Boss01316Handler)(void);

/*
 * Handler run interior word at 0x800C2650, the table this dispatcher selects
 * from, run entry 12 of the 48-word run whose entry 0 is 0x800C2620 = 0x800C1860:
 * entry 0 = 0x800C1934, entry 1 = 0x800C1A04, entry 2 = 0x800C1A2C (inert
 * handler), entry 3 = 0x800C1A78, then the shared main-executable handlers
 * 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288 before this
 * overlay's steps 0x800C1B4C, 0x800C1C4C, 0x800C1CBC and 0x800C1D08 resume at
 * entry 15. The word is interior to the reviewed `textbin` row D_800C25D8, whose
 * run starts at its word 18 and ends with the zero word 0x800C26E0, so the row
 * stays one data row and this word is named at the offset the scaled index adds.
 */
extern Boss01316Handler D_800C2650[]; /* @source 0x800C2650 @kind data */

/* @source 0x800C18FC
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the run interior word
 * 0x800C2650, so the selected handler's result stays in the return register. The
 * original bounds the index only to a byte and never checks it against the run's
 * length; the slot after the scaled table load and the indirect call's delay slot
 * both stay nops, and the indirect call receives no argument of its own. The
 * 0x18-byte frame only keeps $ra at 0x10 across that call, and the epilogue leaves
 * $v0 untouched. No jal and no word of this payload names 0x800C18FC, so the
 * overlay entry is what reaches it. Only the load word's %lo displacement differs
 * from the already-exact dispatcher dispatchIndexedHandlerBossBoss01316_800C1E58
 * of this overlay, whose 56-byte body holds this statement order and these operand
 * forms exactly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01316_800C18FC(u8 index) {
  D_800C2650[index]();
}
