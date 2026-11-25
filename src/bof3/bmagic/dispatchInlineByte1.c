#include "bof3/bof3.h"

/* Work-record pointer cell immediately after the 96-record pool at
 * 0x801EF9F8. The record loop publishes it at 0x801EEC98 before calling this
 * dispatcher at 0x801EECA4. This function reads the cell once before its
 * indirect call. */
extern u8 *D_801F05F8; /* @source 0x801F05F8 @kind data */

/* Four-entry overlay handler table. The code-pointer run begins at 0x801EF9C0
 * and the exact byte-2 dispatcher at 0x801EEF24 indexes its second word, so
 * this function indexes the run from its first word with work-record byte 1. */
extern void (*D_801EF9C0[])(void); /* @source 0x801EF9C0 @kind data */

/* @source 0x801EEEE0
 * @behavior Reads the work-record pointer cell at 0x801F05F8 once, scales the
 *           record byte at offset 0x1 by four and calls that entry of the
 *           handler table based at 0x801EF9C0. It takes no arguments, returns
 *           nothing, stores nothing and reads no other state; the 0x18-byte
 *           frame only keeps $ra across the indirect call. The 68 bytes differ
 *           from the copy at emi/bmagic/magic222/03@0x801EEEE0 only in the
 *           pointer cell address and the table offset word, both of which this
 *           payload encodes, so this source was seeded from the original bytes
 *           rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchInlineByte1(void) {
  u8 *record;

  record = D_801F05F8;
  D_801EF9C0[record[1]]();
}
