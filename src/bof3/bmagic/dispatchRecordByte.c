#include "bof3/bof3.h"

/* Work record pointer cell at 0x801F17B0 and handler table at 0x801F01AC.
 *
 * The overlay's frame loop at 0x801EEC28 publishes the current work record into
 * 0x801F17B0 (the word store to %lo(D_801F17B0) at 0x801EECEC, with the record
 * pointer formed as 0x801F01B0 + i * 0x2C) and calls this dispatcher at
 * 0x801EECF8 in the same iteration, so the cell holds a live record whenever the
 * index byte is read. The table this function indexes is the in-image word at
 * 0x801F01AC, which holds the code address 0x801EF37C at index 0 and is the only
 * word of that table the overlay references. Both cells sit inside the trailing
 * data blob at 0x801F00B8 and no map row names either address, so each one is
 * declared here.
 *
 * The 68-byte span is byte-identical to emi/bmagic/magic168/03 0x801EF338, which
 * both payloads hold at offset 0x738 under the load address 0x801EEC00; this body
 * follows that lifted source statement for statement.
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
 * nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordByte(void) {
  D_801F01AC[D_801F17B0[1]]();
}
