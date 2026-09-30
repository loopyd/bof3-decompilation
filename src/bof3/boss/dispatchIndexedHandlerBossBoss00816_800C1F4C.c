#include "bof3/bof3.h"

typedef void (*Boss00816Handler)(void);

/*
 * Handler table at 0x800C2774, the table this dispatcher selects from:
 * entry 0 = 0x800C1F84, entry 1 = 0x800C1F8C, entry 2 = 0x800C1F94.
 */
extern Boss00816Handler D_800C2774[]; /* @source 0x800C2774 @kind data */

/* @source 0x800C1F4C
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and tail-calls the handler selected from the
 * 0x800C2774 handler table, so the selected handler's result becomes this
 * function's result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00816_800C1F4C(u8 index) {
  D_800C2774[index]();
}
