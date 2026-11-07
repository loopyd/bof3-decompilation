#include "bof3/emi/dispatchers.h"

/* The eight bytes before this function at 0x800C1844 are not code: they are the
 * last two words of the overlay's leading pointer run, which the reviewed layout
 * places as its own `textbin` row, so this body starts at 0x800C184C.
 *
 * This overlay's handler run sits inside the 0x800C2C38 `textbin` row, which the
 * reviewed Splat layout places from 0x800C2C38 to the end of the payload at
 * 0x800C2DD4, so the run is only referenced here. This dispatcher indexes it from
 * 0x800C2CAC, an interior word of that run, so the run stays one reviewed row and
 * this declaration binds only that word. The twelve words from 0x800C2CAC are
 * this payload's own 0x800C1890 and 0x800C1940 followed by the shared
 * main-executable handlers 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00,
 * 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288,
 * and this payload's pointer run continues after them from 0x800C19B4.
 */
extern void (*D_800C2CAC[])(void); /* @source 0x800C2CAC @kind data */

/* @source 0x800C184C
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler table based at 0x800C2CAC with no arguments.
 * The original does not bound the index before the scaled load. It reads no
 * other state and writes none; the pointer-cell load precedes frame creation,
 * the 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * selected handler's $v0 is left untouched by the epilogue, so this shim
 * returns whatever that handler returns. The slots after the index byte and
 * after the scaled table load stay nops. The payload contains no word or jal
 * site naming 0x800C184C, so its caller is outside this overlay. Takes no
 * arguments and forwards none. Same shape as the already-exact siblings
 * dispatchStateByte1Table2d24BossBoss01816_800C1B84 of this overlay and
 * dispatchStateByte1Table2744BossBoss00816_800C1EB0 of boss008/16, which read
 * the same dispatch byte through their own tables; this body differs from them
 * only in the table address of its scaled load word.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2cacBossBoss01816_800C184C, D_800C2CAC);
