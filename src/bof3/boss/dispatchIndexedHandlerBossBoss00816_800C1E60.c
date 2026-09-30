#include "bof3/bof3.h"

typedef void (*Boss00816Handler)(void);

/*
 * Handler table at 0x800C2738, the table this dispatcher selects from:
 * entry 0 = 0x800C1E98, entry 1 = 0x800C1EA0, entry 2 = 0x800C1EA8,
 * entry 3 = 0x800C1EF4, then main-program (0x801E....) handlers.
 */
extern Boss00816Handler D_800C2738[]; /* @source 0x800C2738 @kind data */

/* @source 0x800C1E60
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and tail-calls the handler selected from the
 * 0x800C2738 handler table, so the selected handler's result becomes this
 * function's result.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00816_800C1E60(u8 index) {
  D_800C2738[index]();
}
