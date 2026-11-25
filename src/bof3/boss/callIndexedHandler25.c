#include "bof3/bof3.h"

typedef void (*Boss03516Handler)(void);

/*
 * Handler table at 0x800C1D98, the table this dispatcher selects from: the
 * four overlay entries 0 = 0x800C1A30, 1 = 0x800C1A44, 2 = 0x800C1A4C and
 * 3 = 0x800C1A98, then the eleven main-program handlers 0x801E52E8,
 * 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8,
 * 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288. The base sits inside the
 * pointer run bound as the reviewed textbin row at 0x800C1D04, which stays one
 * row, so this declaration binds only the word the scaled index is added to.
 */
extern Boss03516Handler D_800C1D98[]; /* @source 0x800C1D98 @kind data */

/* @source 0x800C19F8
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1D98
 * handler table, so the value the selected handler returns stays in the
 * return register. The step setup at 0x800C1994 publishes this function
 * address into the slot at offset 0xE4 of the work record it reaches through
 * the main-program cell 0x801EB4E8, so the main program calls it through that
 * slot with the step index byte of that record. Same C shape as the sibling
 * indexed dispatchers at 0x800C18C4 and 0x800C1AFC of this overlay; only the
 * table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler25(u8 index) {
  D_800C1D98[index]();
}
