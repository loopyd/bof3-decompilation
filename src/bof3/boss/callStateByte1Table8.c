#include "bof3/emi/dispatchers.h"

/* This overlay's second handler table starts at 0x800C1B94 and holds in-image
 * code pointers for this payload's functions interleaved with the shared
 * main-executable handlers at 0x801E..... The dispatcher indexes it from its
 * first word, so its base entry is 0x800C1934, this overlay's own step setup
 * routine, then 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00,
 * 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288.
 * Those eleven words run up to 0x800C1BC0, where the table the indexed
 * dispatcher 0x800C1998 selects from begins. It all sits inside the 0x800C1B44
 * `textbin` row, which the reviewed Splat layout places, so it is only
 * referenced here; the generated undefined_syms_auto.txt row for D_800C1B94
 * already binds the word the scaled index is added to.
 */
extern void (*D_800C1B94[])(void); /* @source 0x800C1B94 @kind data */

/* @source 0x800C18F0
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at offset
 * 0x01 as the index and calls the handler pointer at that index of the local
 * handler table based at 0x800C1B94. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after that load stay nops. The
 * pointer-cell load precedes frame creation; the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call. The payload names 0x800C18F0 in no word and
 * no jal site, so its caller is outside this overlay. Takes no arguments and
 * forwards none. A cmp of this span against the one at 0x800C1804 in the same
 * payload showed the two 68-byte bodies equal in every word but the scaled
 * table load, whose low immediate reads 0x1B94 here and 0x1B58 there, because
 * both tables sit in the one 0x800C page; that span was lifted from the same
 * statement shape and measured exact.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateByte1Table8, callStateByte1Table8, D_800C1B94);
