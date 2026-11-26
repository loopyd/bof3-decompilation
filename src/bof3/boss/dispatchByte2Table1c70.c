#include "bof3/emi/dispatchers.h"

/* The overlay's pointer run is the reviewed `textbin` row D_800C1C2C, which
 * covers 0x800C1C2C through the payload's last word and is placed whole by the
 * Splat layout, so only the entry base this dispatcher indexes is named here.
 * From 0x800C1C70 the run holds 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490,
 * this payload's own 0x800C18F0, the main-executable 0x801E4928 and the inert
 * handlers 0x800C19B0, 0x800C19B8 and 0x800C19C0. The generated
 * undefined_syms_auto.txt row for D_800C1C70 already binds the word the scaled
 * index is added to, so this declaration needs no map row.
 */
extern void (*D_800C1C70[])(void); /* @source 0x800C1C70 @kind data */

/* @source 0x800C18AC
 * @behavior Second-level overlay dispatch on the work record byte at offset 2:
 * loads the work-record pointer published at the scratchpad pointer cell
 * 0x1F800044 once, before frame creation, takes that record's unsigned byte at
 * offset 2, scales it by four and calls the matching entry of the code pointer
 * run based at 0x800C1C70 with no arguments, so it loads no argument register
 * and returns whatever the called handler leaves in $v0. The original does not
 * bound the index before the scaled table load, and the slots after the index
 * byte, after the table load and in the return delay slot stay nops. It writes
 * no state; the 0x18-byte frame only keeps $ra at 0x10 across the indirect call.
 * No jal and no word of this payload other than the level-1 run's slot at
 * 0x800C1C58 names this span, so the byte-1 dispatcher at 0x800C1804 selects it
 * as step six of its run and the walk continues here on the record byte at
 * offset 2.
 *
 * Shape: the corpus-wide dispatcher form in include/bof3/emi/dispatchers.h,
 * byte-2 variant, with this payload's own table word 0x800C1C70 written from
 * these bytes. The byte-1 dispatcher at 0x800C1804 of this same target, exact
 * with the same 17-instruction 68-byte body, supplied the statement order,
 * table-typed extern and macro spelling; the two bodies differ only in that
 * function's index slot and table literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2,
                      dispatchByte2Table1c70,
                      D_800C1C70);
