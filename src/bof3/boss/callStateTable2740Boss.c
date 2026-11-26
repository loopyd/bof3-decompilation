#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C2740 through 0x800C279C are
 * in-image code pointers, this payload's own functions interleaved with the
 * shared main-executable handlers at 0x801E.... The run sits inside the
 * 0x800C2720 `textbin` row that the reviewed Splat layout places, so it is only
 * referenced here. This dispatcher indexes it from 0x800C2740, so with that base
 * the entries from 0x800C2740 are 0x800C18A4, 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x800C18FC, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8, 0x800C19C0, 0x801E3BAC, 0x801E3BD0, 0x801E3BAC,
 * 0x801E4490, 0x800C1940, 0x801E4928, 0x800C1A04, 0x800C1A4C, 0x800C1B9C,
 * 0x800C1BF4, 0x800C1BFC and 0x800C1C04. */
extern void (*D_800C2740[])(void); /* @source 0x800C2740 @kind data */

/* @source 0x800C1860
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at offset
 * 0x01 as the index and calls the handler pointer at that index of the local
 * handler table based at 0x800C2740. The original does not bound the index
 * before the scaled table load, and the slots after the index byte and after
 * that load stay nops. The pointer-cell load precedes frame creation; the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call. The epilogue
 * leaves $v0 untouched, so the shim's result is whatever the selected handler
 * returns. Takes no arguments and forwards none. No word in this payload names
 * 0x800C1860 - it is absent from the 0x800C2740 run and from every other
 * pointer or jal site here - so the main program reaches it through the overlay
 * entry. The body is word-identical to the lifted dispatcher at 0x800C1EB0 of
 * boss008/16 apart from the table address in its load word, which differs by
 * four and occupies the one `%lo` slot both spans share.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateTable2740Boss, D_800C2740);
