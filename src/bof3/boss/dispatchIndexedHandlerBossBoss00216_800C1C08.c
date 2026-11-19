#include "bof3/bof3.h"

typedef void (*Boss00216Handler)(void);

/*
 * Handler table at 0x800C2048, the run this dispatcher selects from: entries 0
 * through 2 are the noop handlers at 0x800C1C40, 0x800C1C48 and 0x800C1C50,
 * and the next word 0x800C2054 begins the table the dispatcher at 0x800C1C58
 * indexes. The base sits inside the overlay's reviewed `textbin` row
 * D_800C1F9C, at its +0xAC word, so it is referenced as its own data symbol
 * rather than as a row start.
 */
extern Boss00216Handler D_800C2048[]; /* @source 0x800C2048 @kind data */

/* @source 0x800C1C08
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2048
 * handler table, so the selected handler's result stays in the return
 * register. The step setup at 0x800C1AEC installs this function at +0xE4 of the
 * enemy work record and no other word in the payload holds its address, so it
 * is reached through that stored pointer. Its body is the same 14 instructions
 * as the sibling indexed dispatchers at 0x800C19C8 and 0x800C1D00 of this
 * payload, which differ only in the table address their load word names.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss00216_800C1C08(u8 index) {
  D_800C2048[index]();
}
