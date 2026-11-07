#include "bof3/emi/dispatchers.h"

/* This overlay's handler run sits inside the 0x800C2C38 `textbin` row, which the
 * reviewed Splat layout places from 0x800C2C38 to the end of the payload at
 * 0x800C2DD4, so the run is only referenced here. This dispatcher indexes it from
 * 0x800C2DAC, an interior word of that run, so the run stays one reviewed row and
 * this declaration binds only that word. The six words from 0x800C2DAC are
 * in-image code pointers of this payload: 0x800C203C, 0x800C2064, 0x800C206C,
 * 0x800C20AC, 0x800C20BC and 0x800C210C. The word at 0x800C2DC4 holds
 * 0x0FC10FC7, so the pointer run ends after those six entries. The payload's only
 * reference to this function's own address is the run word at 0x800C2D90.
 */
extern void (*D_800C2DAC[])(void); /* @source 0x800C2DAC @kind data */

/* @source 0x800C1FF8
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C2DAC with no arguments.
 * The original does not bound the index before the scaled load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * selected handler's $v0 is left untouched by the epilogue, so this shim
 * returns whatever that handler returns. Same shape as the exact sibling
 * dispatchStateByte2Table2d94BossBoss01816_800C1EE4 of this overlay, whose body
 * differs only in the table address of its scaled load word.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table2dacBossBoss01816_800C1FF8, D_800C2DAC);
