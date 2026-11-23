#include "bof3/emi/dispatchers.h"

/* This overlay's handler run continues past the 0x800C2CDC dispatcher table and
 * sits inside the 0x800C2C38 `textbin` row, which the reviewed Splat layout
 * places from 0x800C2C38 to the end of the payload at 0x800C2DD4, so the run is
 * only referenced here. This dispatcher indexes it from 0x800C2CE8, an interior
 * word of that run, so the run stays one reviewed row and this declaration binds
 * only that word. Entry 0 is this payload's 0x800C1A2C and entry 1 its
 * 0x800C1ADC, then the shared main-executable handlers 0x801E35D8, 0x801E3638,
 * 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64,
 * 0x801E52E8 and 0x801E5288, and this payload's pointer run continues after them
 * from 0x800C1B50. The pointer data ends at 0x800C2DC4, where the two-byte origin
 * record that renderTwoCellBlock reads begins.
 */
extern void (*D_800C2CE8[])(void); /* @source 0x800C2CE8 @kind data */

/* @source 0x800C19E8
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C2CE8 with no arguments.
 * The original does not bound the index before the scaled load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * selected handler's $v0 is left untouched by the epilogue, so this shim returns
 * whatever that handler returns. The slots after the index byte and after the
 * scaled table load stay nops. The payload contains no word or jal site naming
 * 0x800C19E8, so its caller is outside this overlay. Takes no arguments and
 * forwards none. Same shape as the exact sibling
 * dispatchStateByte1Table2d24BossBoss01816_800C1B84 of this overlay: the two
 * 68-byte spans are equal at every word except the %lo half of the scaled table
 * load (0x800C2D24 there, 0x800C2CE8 here), both in the same 0x8000 page.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2ce8BossBoss01816_800C19E8, D_800C2CE8);
