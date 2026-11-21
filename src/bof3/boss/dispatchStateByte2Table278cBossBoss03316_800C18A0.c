#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's handler run. The reviewed 0x800C2744 `textbin`
 * row covers 0x800C2744 through the payload end at 0x800C27D8: its first 24
 * bytes hold the two byte tables the installer
 * publishWorkStepHandlerAndDataThenSetStateTo2BossBoss03316_800C1848 hands to
 * its callees, and the
 * code-pointer run starts at 0x800C275C and runs from there to that end. This
 * dispatcher indexes the run from its word 12, 0x800C278C, so that base word is
 * named here on its own while the reviewed row stays whole; the byte-1
 * dispatcher of this target indexes word 0 of the same run. Run word 12 holds
 * the next code span of this overlay, 0x800C18E4. */
extern void (*D_800C278C[])(void); /* @source 0x800C278C @kind data */

/* @source 0x800C18A0
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 before it creates
 * its frame, takes that record's unsigned byte at offset 0x02 as the index,
 * scales it by four and calls the matching entry of the code-pointer run based
 * at 0x800C278C with no arguments. The original does not bound the index before
 * the scaled table load, and the slots after the index byte, after the table
 * load and in both branch delay slots stay nops. It writes no state and leaves
 * the handler's return register untouched; the 0x18-byte frame only keeps the
 * return address at 0x10 across the indirect call. The only payload word that
 * names this address is 0x800C276C, word 4 of the run based at 0x800C275C that
 * dispatchStateByte1Table275cBossBoss03316_800C1804 dispatches, and no jal in
 * this payload names it, so that shim is its caller: it passes no arguments and
 * forwards none. Seeded from the sibling
 * dispatchStateByte2Table1FECBossBoss00216_800C18A0 of boss002/16: comparing
 * the two payloads shows that 0x44-byte span equal word for word except the
 * scaled table load low immediate, 0x800C278C here against 0x800C1FEC there, so
 * only this target's own table literal is written from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2,
                      dispatchStateByte2Table278cBossBoss03316_800C18A0,
                      D_800C278C);
