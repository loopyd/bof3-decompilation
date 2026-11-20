#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the reviewed Splat layout places the payload's
 * trailing pointer and text block in the `textbin` row D_800C3174, and the run
 * of in-image code pointers starts at 0x800C319C with this overlay's entry
 * setup 0x800C1894. This dispatcher indexes that run from its word 12, the
 * interior word 0x800C31CC, whose entries are 0x801E3BAC, 0x801E3BD0,
 * 0x801E3BAC, 0x801E4490, this payload's own 0x800C19C0 and the
 * main-executable 0x801E4928, so the run stays one reviewed row and this
 * declaration binds only the word the scaled index is added to. */
extern void (*D_800C31CC[])(void); /* @source 0x800C31CC @kind data */

/* @source 0x800C197C
 * @behavior Second-level overlay dispatch on the work record byte at offset 2:
 * loads the work-record pointer published at the scratchpad pointer cell
 * 0x1F800044, takes that record's unsigned byte at offset 2, scales it by four
 * and calls the matching entry of the handler run from its word 12 at
 * 0x800C31CC with no arguments, so no argument register is loaded and the
 * callee's value in $v0 is what this function returns. The original does not
 * bound the index before the scaled table load. It reads no other state and
 * writes none; the pointer-cell load precedes frame creation, the 0x18-byte
 * frame only keeps $ra at 0x10 across the indirect call, and the slots after
 * the index byte and after the table load stay nops. The only in-image
 * reference to this span is the pointer word at 0x800C31B4, its own address as
 * word 6 of that same 0x800C319C run, and the payload holds no jal naming it,
 * so the overlay stepper that walks the run is its caller. Same body as the
 * exact sibling dispatcher at 0x800C1EC8 of this overlay: their 0x44-byte
 * spans are equal apart from the table word of the scaled load at
 * 0x800C19A0, which names 0x800C3274 in the sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table31ccBossBoss02513_800C197C, D_800C31CC);
