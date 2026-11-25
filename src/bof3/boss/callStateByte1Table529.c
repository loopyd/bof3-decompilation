#include "bof3/emi/dispatchers.h"

/* This overlay's handler run begins at 0x800C1D2C, the first code pointer of the
 * 0xDC-byte `textbin` row D_800C1D04 that Splat places after this payload's code;
 * the ten words before it are byte-sized game data, not pointers. From 0x800C1D2C
 * the run starts with this payload's entry point 0x800C1860, then the shared
 * main-program handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and
 * 0x801E5288, then this payload's own 0x800C18FC, 0x800C1904, 0x800C190C,
 * 0x800C1994, 0x800C1A30, 0x800C1A44, 0x800C1A4C, 0x800C1A98, 0x800C1B34,
 * 0x800C1B3C and 0x800C1B44, the last at 0x800C1DDC. Splat places the whole row,
 * so this declaration names only the interior word the scaled index is added to. */
extern void (*D_800C1D2C[])(void); /* @source 0x800C1D2C @kind data */

/* @source 0x800C181C
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044, takes
 * that record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C1D2C with no arguments. The
 * original bounds the index nowhere before the scaled table load, and the slots
 * after the index byte and after the table load stay nops. It reads no other state
 * and writes none: the pointer-cell load precedes frame creation, the 0x18-byte
 * frame only keeps $ra at 0x10 across the indirect call, and the epilogue leaves
 * the selected handler's $v0 untouched, so this shim returns whatever that handler
 * returns. No word in this payload equals 0x800C181C, no jal targets it and no
 * lui/addiu pair forms it, so neither the handler run below nor a call names it,
 * and the main program's overlay handler table is what enters it. The work-record
 * byte it indexes is the one the entry point 0x800C1860 stores 2 into. The
 * 68-byte body is byte-identical to the exact sibling
 * dispatchStateByte1Table1d2cBossBoss02116_800C19E4, proven by comparing this
 * payload's span at offset 0x1C with that payload's span at offset 0x1E4, which
 * differ in no word; that sibling's source supplied the statement order and the
 * operand forms transcribed here.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      callStateByte1Table529,
                      D_800C1D2C);
