#include "bof3/bof3.h"

typedef void (*Boss02116Handler)(void);

/*
 * Handler table at 0x800C1D5C, the table this dispatcher selects from: its
 * three words are this overlay's inert handlers 0x800C1AB8, 0x800C1AC0 and
 * 0x800C1AC8, and the last of them is the payload's final word. The base sits
 * inside the pointer run bound as the reviewed textbin row at 0x800C1CB0,
 * whose pointer words start at 0x800C1CD8; this base is word 33 of that run.
 * The dispatcher at 0x800C198C indexes the same run from its word 18
 * (0x800C1D20), so the run stays one row and this declaration binds only the
 * word the scaled index is added to.
 */
extern Boss02116Handler D_800C1D5C[]; /* @source 0x800C1D5C @kind data */

/* @source 0x800C1A80
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1D5C
 * handler table, so the selected handler's result stays in the return
 * register. The step publisher at 0x800C1A28 stores this function's address
 * into the slot at offset 0xE4 of the work record it reaches through
 * D_801EB4E8. Byte-identical, all 0x38 bytes, to the lifted exact dispatcher at
 * 0x800C18C4 of BIN/BOSS/BOSS035.EMI#16, which indexes the same table address;
 * this target keeps its own source and binding.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss02116_800C1A80(u8 index) {
  D_800C1D5C[index]();
}
