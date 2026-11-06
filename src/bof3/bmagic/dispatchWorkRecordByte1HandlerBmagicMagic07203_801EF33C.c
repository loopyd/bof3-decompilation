#include "bof3/bof3.h"

/* Record pointer cell at 0x801F17B4 and handler table at 0x801F01B0.
 *
 * The overlay frame loop at 0x801EEC28 walks the 0x80-entry, 0x2C-byte record
 * array based at 0x801F01B4 and, for each entry whose first byte has bit 0 set,
 * publishes 0x801F01B4 + index * 0x2C into 0x801F17B4 with `sw` at 0x801EECE8
 * and calls this dispatcher at 0x801EECF8, so the cell always holds a live
 * record when the index byte is read. Both addresses sit in this overlay's
 * trailing read-only block, which no map row owns, so the references bind
 * through the target's generated undefined-symbol table.
 */
extern u8 *D_801F17B4; /* @source 0x801F17B4 @kind data */
extern void (*D_801F01B0[])(void); /* @source 0x801F01B0 @kind data */

/* @source 0x801EF33C
 * @behavior Dispatches one step of the current bmagic work record. The record
 * pointer is read from the cell at 0x801F17B4 before the frame is created, the
 * record's unsigned byte at offset 1 is scaled by four and selects an entry of
 * the code-pointer table based at 0x801F01B0, and the selected handler is
 * invoked with no arguments. The shipped table holds the single word 0x801EF380,
 * the next function of this overlay, and the original bounds neither the byte
 * nor the entry, so a nonzero byte reads the record array that starts four bytes
 * after the table base as a handler address. The dispatcher reads no other state
 * and writes none; the 0x18-byte frame only keeps $ra across the indirect call,
 * and the load delays after the index byte and after the table load stay nops.
 * The 68-byte body is instruction-for-instruction identical to
 * emi/bmagic/magic063/03 0x801EF340 and emi/bmagic/magic073/03 0x801EF724,
 * differing only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkRecordByte1HandlerBmagicMagic07203_801EF33C(void) {
  D_801F01B0[D_801F17B4[1]]();
}
