#include "bof3/bof3.h"

typedef void (*Boss03616Handler)(void);

/*
 * Handler table at 0x800C2074, the base this dispatcher's scaled index is added
 * to: entry 0 = 0x800C1B5C and entry 1 = 0x800C1B64 (inert handlers), entry 2 =
 * 0x800C1B6C, entry 3 = 0x800C1CDC, entry 4 = 0x801E52E8, which is outside this
 * payload. The base sits inside the pointer run the textbin row D_800C2020
 * covers, so it binds as its own data symbol rather than a row boundary.
 */
extern Boss03616Handler D_800C2074[]; /* @source 0x800C2074 @kind data */

/* @source 0x800C1B24
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2074
 * handler table, so the selected handler's result stays in the return register.
 * The overlay entry block at 0x800C1854 publishes this address into the
 * step-handler slot at offset 0xE4 of the record from D_801EB4E8; no
 * instruction in the payload calls it directly. Same C shape as the sibling
 * indexed dispatchers at 0x800C2074 of boss018/16 and 0x800C1AF0 of boss028/16;
 * only the table address in the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss03616_800C1B24(u8 index) {
  D_800C2074[index]();
}
