#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's handler run. The reviewed 0x800C2744
 * `textbin` row covers 0x800C2744 through 0x800C27D7: its first 24 bytes hold
 * the two byte tables the installer
 * publishStepDataBoss hands to
 * its callees, and the code-pointer
 * run that starts at 0x800C275C runs from there to the payload end, mixing this
 * payload's own functions with the shared main-executable handlers from
 * 0x801E35D8 up. This dispatcher indexes the run from its first word, whose
 * entry 0 is 0x800C1848, this overlay's next code span, so that base word is
 * named here on its own while the reviewed row stays whole; the sibling
 * dispatchers of this target index words further into the same run. */
extern void (*D_800C275C[])(void); /* @source 0x800C275C @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 before it creates
 * its frame, takes that record's unsigned byte at offset 1, scales it by four
 * and calls the matching entry of the code-pointer run based at 0x800C275C with
 * no arguments. The original does not bound the index before the scaled table
 * load, and the slots after the index byte, after the table load and in the
 * branch delay slot stay nops. It writes no state and leaves the handler's
 * return register untouched; the 0x18-byte frame only keeps the return address
 * at 0x10 across the indirect call. No word and no jal in this payload holds
 * 0x800C1804, so the caller that reaches this shim sits outside the payload: it
 * is the first code span of this overlay, directly behind the four header bytes
 * and ahead of the installer
 * publishStepDataBoss. Seeded
 * from the exact sibling
 * dispatchStateByte1Table2660BossBoss00816_800C1804 of boss008/16, whose
 * 0x44-byte span compares equal word for word except the scaled table load's
 * low immediate, 0x800C275C here against 0x800C2660 there, so only this
 * target's own table literal is written from these bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable275Boss,
                      callStateTable275Boss,
                      D_800C275C);
