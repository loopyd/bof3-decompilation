#include "bof3/bof3.h"

typedef void (*Boss04916Handler)(void);

/*
 * Handler table at 0x800C1B60, the table this dispatcher selects from: entry 0 =
 * 0x800C18E4, entry 1 = 0x800C18EC and entry 2 = 0x800C18F4 (the inert handlers
 * of this overlay), entry 3 = 0x800C1940, entries 4..14 = main-program handlers
 * from 0x801E52E8 on, and entries 15..17 = 0x800C19DC, 0x800C19E4, 0x800C19EC
 * (the overlay's closing inert handlers). The base sits 0x44 bytes into the
 * pointer run that fills the payload from 0x800C1B1C to its end at 0x800C1BA8,
 * reviewed as the textbin row at 0x800C1B1C, so this declaration binds only the
 * word the scaled index is added to and the run stays one row.
 */
extern Boss04916Handler D_800C1B60[]; /* @source 0x800C1B60 @kind data */

/* @source 0x800C18AC
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C1B60 handler
 * table, so the selected handler's result stays in the return register. The
 * overlay entry point at 0x800C1848 publishes this function's address into the
 * slot at offset 0xE4 of the record it reaches through D_801EB4E8. Same C shape
 * as the exact indexed dispatchers at 0x800C18C4 of BIN/BOSS/BOSS035.EMI#16 and
 * 0x800C1998 of BIN/BOSS/BOSS014.EMI#16; only the table address in the load word
 * differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss04916_800C18AC(u8 index) {
  D_800C1B60[index]();
}
