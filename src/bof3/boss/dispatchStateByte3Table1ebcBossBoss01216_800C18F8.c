#include "bof3/emi/dispatchers.h"

/*
 * Handler run the byte-3 dispatcher at 0x800C18F8 indexes.
 *
 * The seven words from 0x800C1EBC through the payload end 0x800C1ED4 sit inside
 * this overlay's reviewed textbin pointer row D_800C1E14 (0x800C1E14 through
 * 0x800C1ED8) and hold 0x800C193C, 0x800C1978, 0x800C1A24, 0x800C1A90,
 * 0x800C1B50, 0x800C1B58 and 0x800C1B60, all functions of this payload. The row
 * stays one map entry, so this declaration binds only the word the scaled index
 * is added to.
 */
extern void (*D_800C1EBC[])(void); /* @source 0x800C1EBC @kind data */

/* @source 0x800C18F8
 * @behavior Overlay state dispatch on work-record byte 3: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 3, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C1EBC with no arguments.
 * The chosen entry is read from the table itself, not from a frame copy. The
 * original does not bound the index before the scaled table load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, the slots
 * after the index byte, after the table load and in the branch delay slot stay
 * nops, and the epilogue leaves the handler's $v0 untouched, so this shim
 * returns whatever that handler returns. This function's only in-image
 * reference is the word 0x800C18F8 at 0x800C1EB4, word 40 of the same
 * code-pointer row, so the overlay loader that steps that row and the byte-2
 * dispatcher dispatchStateByte2Table1ea4BossBoss01216_800C18B4, whose table base
 * 0x800C1EA4 holds it as entry 4, reach it inside the payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3,
                      dispatchStateByte3Table1ebcBossBoss01216_800C18F8,
                      D_800C1EBC);
