#include "bof3/emi/dispatchers.h"

/* Interior base of the overlay's pointer run: the promoted `textbin` row
 * D_800C1B1C covers 0x800C1B1C to the end of this payload and is placed whole by
 * the reviewed Splat layout, so only the entry base this dispatcher indexes is
 * named here. From 0x800C1B30 the run holds eleven handler pointers: 0x800C1848,
 * 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64 and 0x801E52E8. The word after them at
 * 0x800C1B5C is 0x801E5288, and the next word 0x800C1B60 begins the separate
 * table the indexed dispatcher at 0x800C18AC selects from. The generated
 * undefined_syms_auto.txt row for D_800C1B30 already binds the word the scaled
 * index is added to, so this declaration needs no map row.
 */
extern void (*D_800C1B30[])(void); /* @source 0x800C1B30 @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1, the first code word
 * after this payload's four-byte entry header: loads the work-record pointer
 * published at the scratchpad pointer cell 0x1F800044 once, takes that record's
 * unsigned byte at offset 1, scales it by four and calls the matching entry of
 * the code pointer run based at 0x800C1B30 (0x800C1848, 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8) with no arguments. The original does not bound the
 * index before the scaled table load, and the slots after the index byte, after
 * the table load and in the return delay slot stay nops. It writes no state and
 * returns whatever the called handler leaves in $v0; the pointer-cell load
 * precedes frame creation, and the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call. No word of this payload holds 0x800C1804 and no jal reaches
 * it, so the overlay loader enters it from outside the payload: it takes no
 * arguments and forwards none.
 *
 * Shape: seeded from the already-exact sibling at 0x800C1804 of
 * emi/boss/boss022/13. This payload's own 0x44-byte span at offset 4 compares
 * equal word for word with the spans at 0x800C1804 of boss022/13, boss028/16 and
 * boss032/16 except the scaled table load's low immediate, 0x1B30 here against
 * 0x1B2C, 0x2154 and 0x1C40 there, so only this payload's own table literal is
 * written from these bytes. The statement order, table-typed extern and macro
 * spelling are the corpus-wide dispatcher form in include/bof3/emi/dispatchers.h.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchWorkByte1Table3,
                      dispatchWorkByte1Table3,
                      D_800C1B30);
