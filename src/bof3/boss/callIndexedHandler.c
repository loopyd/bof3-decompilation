#include "bof3/bof3.h"

typedef void (*Boss00113Handler)(void);

/*
 * Handler table at 0x800C2210, the table this dispatcher selects from: word 37
 * of this overlay's reviewed `textbin` row D_800C217C. Its entries 0 through 2
 * are this overlay's inert handlers 0x800C1B28, 0x800C1B30 and 0x800C1B38,
 * entry 3 is this payload's 0x800C1B84, and the run continues with main
 * executable handlers interleaved with this overlay's own 0x800C1BDC and
 * 0x800C1CA0 dispatchers.
 */
extern Boss00113Handler D_800C2210[]; /* @source 0x800C2210 @kind data */

/* @source 0x800C1AF0
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with
 * 0xFF, scales it by four and calls the handler selected from the 0x800C2210
 * handler table, so the selected handler's result stays in the return
 * register. The chosen entry is read from the table itself, not from a frame
 * copy, and the original does not bound the index before the scaled table
 * load. It reads no other state and writes none; the 0x18-byte frame only keeps
 * $ra at 0x10 across the indirect call, the load-delay slot after the table
 * load and the call delay slot both stay nops, and the epilogue leaves the
 * handler's $v0 untouched. No word and no call in this payload names
 * 0x800C1AF0, so its caller stays outside this overlay. Same C shape as the
 * sibling at 0x800C1E00, whose own gate measured exact: the two bodies differ
 * in the table address word alone, and both tables sit in the 0x800C page, so
 * only the %lo half of the load word differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler(u8 index) {
  D_800C2210[index]();
}
