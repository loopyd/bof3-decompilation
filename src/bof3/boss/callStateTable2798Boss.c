#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's handler run. The reviewed 0x800C2744 `textbin`
 * row covers 0x800C2744 through the payload end at 0x800C27D8: its first 24
 * bytes hold the two byte tables the installer
 * publishStepDataBoss hands to
 * its callees, and the
 * code-pointer run starts at 0x800C275C and runs from there to that end. This
 * dispatcher indexes the run from its word 15, 0x800C2798, so that base word is
 * named here on its own while the reviewed row stays whole. The sixteen entries
 * reachable from that base are 0x800C1B9C, 0x800C1BD4, 0x800C1CDC, 0x800C1D14,
 * 0x800C1D68, 0x800C1E70, 0x800C256C, 0x800C1ED8, 0x800C1F38, 0x800C1F88,
 * 0x800C20CC, 0x800C2310, 0x800C244C, 0x800C24DC, 0x800C25D4 and 0x800C26A4;
 * the byte-2 dispatcher of this target indexes word 4 of the same run. */
extern void (*D_800C2798[])(void); /* @source 0x800C2798 @kind data */

/* @source 0x800C1B58
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 before it creates
 * its frame, takes that record's unsigned byte at offset 0x02 as the index,
 * scales it by four and calls the matching entry of the code-pointer run based
 * at 0x800C2798 with no arguments. The original does not bound the index before
 * the scaled table load, and the slots after the index byte, after the table
 * load and in the jalr delay slot stay nops. It writes no state and leaves the
 * handler's return register untouched; the 0x18-byte frame only keeps the
 * return address at 0x10 across the indirect call. No word and no jal in this
 * payload names 0x800C1B58, so the caller that reaches this shim sits outside
 * the payload. Seeded from the exact sibling
 * callStateTable278Boss of this target: comparing
 * the two payload spans shows the 0x44-byte body equal word for word except the
 * scaled table load low immediate at span offset 36, 0x800C2798 here against
 * 0x800C278C there, so only this target's own table literal is written from
 * these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable2798Boss,
                      callStateTable2798Boss,
                      D_800C2798);
