#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

/*
 * Handler table at 0x800C35EC, the table this dispatcher selects from: ten
 * 4-byte overlay entry points, entry 0 = 0x800C2328, entry 1 = 0x800C2330,
 * entry 2 = 0x800C2338, entry 3 = 0x800C2728, entry 4 = 0x800C2888,
 * entry 5 = 0x800C29E8, entry 6 = 0x800C2B48, entry 7 = 0x800C2CD4,
 * entry 8 = 0x800C2E74, entry 9 = 0x800C2FFC.
 */
extern Boss05516Handler D_800C35EC[]; /* @source 0x800C35EC @kind table */

/* @source 0x800C22F0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C35EC
 * handler table, so the selected handler's result becomes this function's
 * result. No in-payload caller: the main program reaches it through its
 * overlay entry point. Byte-identical to the already-exact dispatcher at
 * 0x800C1C88 of the sibling overlay BIN/BOSS/BOSS008.EMI#16; only the table
 * address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss05516_800C22F0(u8 index) {
  D_800C35EC[index]();
}
