#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's second work-state handler run. The reviewed
 * 0x800C1AB4 `textbin` row covers the last 160 bytes of the payload, and this
 * dispatcher indexes the twelve-word code-pointer run that starts 0x64 bytes
 * into it: entry 0 is the publisher 0x800C1934 and entries 1 through 11 are the
 * shared main-executable handlers 0x801E52E8, 0x801E35D8, 0x801E3638,
 * 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C, 0x801E4F64,
 * 0x801E52E8 and 0x801E5288. Those twelve words end at 0x800C1B44, where the
 * three-word step run the sibling dispatcher 0x800C198C selects from begins, so
 * this declaration binds only the word the scaled index is added to and the
 * reviewed row stays one span. */
extern void (*D_800C1B18[])(void); /* @source 0x800C1B18 @kind data */

/* @source 0x800C18F0
 * @behavior Overlay state dispatch on work-record byte 1, the second entry
 * dispatcher of this payload: loads the work-record pointer published at the
 * scratchpad pointer cell 0x1F800044 before it creates its frame, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of the twelve-word code-pointer run based at 0x800C1B18 with no
 * arguments. Entry 0 of that run is the publisher 0x800C1934, which installs
 * the dispatcher 0x800C198C and leaves the state byte at 2, while entries 1
 * through 11 hand the frame to a shared main-executable handler. The original
 * does not bound the index before the scaled table load, and the slots after
 * the index byte, after the table load and in the branch delay slot stay nops.
 * It writes no state and leaves the handler's return register untouched; the
 * 0x18-byte frame only keeps the return address at 0x10 across the indirect
 * call. No word of this payload holds 0x800C18F0 and no jal of it reaches this
 * address, so the caller that enters the shim sits outside the payload.
 *
 * Byte evidence: the exact sibling at 0x800C1804 is the same 0x44-byte body,
 * shown by a payload cmp whose only two differing bytes are the low half of the
 * scaled table load's offset word (0x1ADC there, 0x1B18 here), so its statement
 * order, operand forms, offset spelling and declaration style seed this body
 * while the table literal is written from these bytes. The two spans are not
 * byte-identical, so the clone lever does not reach here; the family spelling of
 * this body is EMI_DEFINE_DISPATCHER, whose expansion is the 17-instruction span
 * measured here.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchByte1Table1b18,
                      dispatchByte1Table1b18,
                      D_800C1B18);
