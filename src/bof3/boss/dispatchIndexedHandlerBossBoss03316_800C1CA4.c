#include "bof3/bof3.h"

typedef void (*Boss03316Handler)(void);

/*
 * Handler run at 0x800C27A0, the table this dispatcher selects from: the
 * fourteen in-image code pointers 0x800C1CDC, 0x800C1D14, 0x800C1D68,
 * 0x800C1E70, 0x800C256C, 0x800C1ED8, 0x800C1F38, 0x800C1F88, 0x800C20CC,
 * 0x800C2310, 0x800C244C, 0x800C24DC, 0x800C25D4 and 0x800C26A4, running to the
 * end of the payload. The base is an interior word of the reviewed 0x800C2744
 * `textbin` row, which stays one data row, and the generated undefined-symbol
 * file already names this word.
 */
extern Boss03316Handler D_800C27A0[]; /* @source 0x800C27A0 @kind data */

/* @source 0x800C1CA4
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the fourteen-word run at
 * 0x800C27A0, so the selected handler's result stays in the return register. The
 * original does not bound the index before the scaled table load, and the
 * load-delay slot after that load and the jalr delay slot both stay nops. It
 * reads no other state and writes none; the 0x18-byte frame only keeps $ra at
 * 0x10 across the indirect call. No jal in this payload names 0x800C1CA4, but the
 * installer
 * publishWorkStepHandlerAndDataThenSetStateTo2BossBoss03316_800C1848 takes its
 * address and stores it into byte offset 0xE4 of the
 * enemy work record published at 0x801EB4E8, so the main program reaches it
 * through that stored pointer. Same C shape as the indexed dispatchers at
 * 0x800C1AF0 of boss028/16, 0x800C1934 of boss023/16 and 0x800C1C88 of boss008/16,
 * whose bodies differ from this one in the table-address immediate alone; the
 * meaning of the index is not established, so it stays a parameter role.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss03316_800C1CA4(u8 index) {
  D_800C27A0[index]();
}
