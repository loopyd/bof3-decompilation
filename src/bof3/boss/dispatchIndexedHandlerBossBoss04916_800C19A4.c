#include "bof3/bof3.h"

typedef void (*Boss04916Handler)(void);

/*
 * Handler table at 0x800C1B9C, the table this dispatcher selects from: entry 0 =
 * 0x800C19DC, entry 1 = 0x800C19E4 and entry 2 = 0x800C19EC, the three inert
 * handlers that close this overlay's code. The base sits 0x80 bytes into the
 * pointer run the payload keeps from 0x800C1B1C to its end at 0x800C1BA8,
 * reviewed as the textbin row at 0x800C1B1C, so this declaration binds only the
 * word the scaled index is added to and the run stays one row.
 */
extern Boss04916Handler D_800C1B9C[]; /* @source 0x800C1B9C @kind data */

/* @source 0x800C19A4
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the 0x800C1B9C handler
 * table, so the selected handler's result stays in the return register. The
 * 0x38 bytes of this span compared equal to the lift of
 * dispatchIndexedHandlerBossBoss04016_800C19A4 in BIN/BOSS/BOSS040.EMI#16, a
 * payload with the same 0x800C1800 load address, whose source gated exact; no
 * word of the span is payload-relative beyond that table base, so the body
 * transfers unchanged. The installer at 0x800C1940 stores this function's
 * address into offset 0xE4 of the record it reaches through D_801EB4E8, the
 * slot the earlier installer at 0x800C1848 filled with the 0x800C18AC
 * dispatcher.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss04916_800C19A4(u8 index) {
  D_800C1B9C[index]();
}
