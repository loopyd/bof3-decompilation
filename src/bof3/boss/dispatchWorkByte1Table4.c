#include "bof3/emi/dispatchers.h"

/* Interior base of the overlay's pointer run: the promoted `textbin` row
 * D_800C1B1C covers 0x800C1B1C to the end of this payload at 0x800C1BA8 and the
 * reviewed Splat layout places it whole, so only the entry base this dispatcher
 * indexes is named here. From 0x800C1B6C the run holds twelve handler pointers,
 * read from this payload's own bytes: 0x800C1940 (this overlay's second
 * installer, published here as
 * publishStepSetup4),
 * 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288, which ends the
 * run immediately before the indexed handler table at 0x800C1B9C that the
 * byte-indexed dispatcher 0x800C19A4 selects from. The generated
 * undefined_syms_auto.txt row for D_800C1B6C already binds the word the scaled
 * index is added to, so this declaration needs no map row.
 */
extern void (*D_800C1B6C[])(void); /* @source 0x800C1B6C @kind data */

/* @source 0x800C18FC
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of the twelve-pointer handler run based at 0x800C1B6C with no arguments.
 * The original does not bound the index before the scaled table load, and the
 * slots after the index byte, after the table load and in the return delay slot
 * stay nops. It writes no state and returns whatever the called handler leaves in
 * $v0; the pointer-cell load precedes frame creation, and the 0x18-byte frame
 * only keeps $ra at 0x10 across the indirect call. Entry 0 of the run is the
 * installer at 0x800C1940, which stores D_800C1B1C into record offset 0xEC,
 * 0x800C19A4 into 0xE4 and D_800C1B28 into 0xE8, raises flag 0x8 in the word at
 * 0x104, writes work byte 1 as 2 and calls func_8014D978; the remaining eleven
 * entries are main-executable handlers. No word of this payload holds 0x800C18FC,
 * no jal reaches it and no lui/addiu pair materializes its address, so the overlay
 * loader enters it from outside the payload: it takes no arguments and forwards
 * none.
 *
 * Shape: the 0x44-byte span at payload offset 0xFC compared byte-equal with the
 * span at 0x800C18FC of BIN/BOSS/BOSS049.EMI#16, whose source is already exact,
 * and differs from this target's exact dispatcher at 0x800C1804 in the single low
 * byte of the scaled table load, 0x1B6C here against 0x1B30 there. Both siblings
 * spell the body through the corpus-wide macro in include/bof3/emi/dispatchers.h,
 * so the statement order, the table-typed extern and the macro spelling come from
 * that header and only this payload's own table literal is written from these
 * bytes. The clone adapter could not read the boss049 source, which emits its
 * function from a macro invocation: "representative source requires one
 * unambiguous function definition for 0x800C18FC", so the seed was taken from the
 * same-target 0x800C1804 dispatcher instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchWorkByte1Table4,
                      dispatchWorkByte1Table4,
                      D_800C1B6C);
