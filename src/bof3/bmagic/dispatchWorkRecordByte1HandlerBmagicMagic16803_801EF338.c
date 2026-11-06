#include "bof3/bof3.h"

/* Record pointer cell at 0x801F17B0 and handler table at 0x801F01AC.
 *
 * The overlay's frame loop at 0x801EEC28 publishes the current work record into
 * 0x801F17B0 (`sw $v0, %lo(D_801F17B0)($at)` at 0x801EECEC, where $v0 is
 * 0x801F01B0 + i * 0x2C) and calls this dispatcher at 0x801EECF8, so the cell
 * always holds a live record when the index byte is read. The table this
 * function indexes is the in-image word at 0x801F01AC, which holds the code
 * address 0x801EF37C on disc; both cells sit in the target's trailing data
 * block, which no map row owns, so the references bind through the harness
 * address encoding.
 */
extern u8 *D_801F17B0; /* @source 0x801F17B0 @kind data */
extern void (*D_801F01AC[])(void); /* @source 0x801F01AC @kind data */

/* @source 0x801EF338
 * @behavior Selects and runs the handler for the current bmagic work record. The
 * record pointer is read from the cell at 0x801F17B0 before the frame is created,
 * the record's unsigned byte at offset 1 is scaled by four and selects an entry of
 * the handler table based at 0x801F01AC, and that handler is invoked with no
 * arguments. The dispatcher bounds neither the byte nor the entry, reads no other
 * state and writes none, so the 0x18-byte frame only keeps $ra across the indirect
 * call and the load delays after the index byte and after the table load stay
 * nops. The 68-byte body is instruction-for-instruction identical to
 * emi/bmagic/magic073/03 0x801EF724, differing only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkRecordByte1HandlerBmagicMagic16803_801EF338(void) {
  D_801F01AC[D_801F17B0[1]]();
}
