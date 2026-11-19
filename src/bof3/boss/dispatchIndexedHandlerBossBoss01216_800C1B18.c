#include "bof3/bof3.h"

typedef void (*Boss01216Handler)(void);

/*
 * Handler table at 0x800C1ECC, the table this dispatcher selects from:
 * entry 0 = 0x800C1B50, entry 1 = 0x800C1B58, entry 2 = 0x800C1B60, the three
 * inert handlers this overlay ends on. The base is word 46 of this overlay's
 * reviewed textbin pointer run D_800C1E14 (0x800C1E14 through the payload end
 * 0x800C1ED8) and its last entry is that run's last word, so the run stays one
 * row and this declaration binds only the interior word the scaled index adds
 * to.
 */
extern Boss01216Handler D_800C1ECC[]; /* @source 0x800C1ECC @kind data */

/* @source 0x800C1B18
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C1ECC
 * handler table of this overlay, so the selected handler's result stays in the
 * return register. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the 0x18-byte frame
 * only keeps $ra at 0x10 across the indirect call, and the epilogue leaves the
 * handler's $v0 untouched. No instruction in this payload references this
 * function's address, so the resident code that publishes this overlay's
 * handler run is its caller and the argument role stays structural. Same C
 * shape as the sibling indexed dispatchers at 0x800C1B18 of boss018/16 and at
 * 0x800C1AF0 of boss028/16; this span's bytes equal that boss018/16 body in
 * thirteen of fourteen words, the table address in the load word alone
 * differing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchIndexedHandlerBossBoss01216_800C1B18(u8 index) {
  D_800C1ECC[index]();
}
