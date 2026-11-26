#include "bof3/emi/dispatchers.h"

/* This overlay's handler run based at 0x800C1DA4 sits inside the 0xDC-byte
 * `textbin` row D_800C1D04 that Splat places after this payload's code. Its
 * fifteen words are this payload's 0x800C1A98 followed by the shared
 * main-program handlers 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980,
 * 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and
 * 0x801E5288, then this payload's 0x800C1B34, 0x800C1B3C and 0x800C1B44, the
 * last at 0x800C1DDC. Splat places the whole row, so this declaration names only
 * the interior word the scaled index is added to. */
extern void (*D_800C1DA4[])(void); /* @source 0x800C1DA4 @kind data */

/* @source 0x800C1A54
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the non-volatile scratchpad pointer cell 0x1F800044,
 * takes that record's unsigned byte at offset 1, scales it by four and calls the
 * matching entry of this overlay's handler run based at 0x800C1DA4 with no
 * arguments. The original bounds the index nowhere before the scaled table load,
 * and the slots after the index byte and after the table load stay nops. It
 * reads no other state and writes none: the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * and the epilogue leaves the selected handler's $v0 untouched, so this shim
 * returns whatever that handler returns. No jal and no pointer word in this
 * payload names 0x800C1A54, so its caller is outside this overlay, while the
 * work-record byte it indexes is the one the entry point 0x800C1860 and the step
 * setup 0x800C1994 both store 2 into. The 68-byte body is
 * instruction-for-instruction equal to the exact sibling
 * callStateByte1Table522, whose only differing word is
 * the %lo table address of this body's scaled load.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      callStateByte1Table527,
                      D_800C1DA4);
