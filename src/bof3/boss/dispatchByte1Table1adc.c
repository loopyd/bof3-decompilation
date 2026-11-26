#include "bof3/emi/dispatchers.h"

/* Interior base of this overlay's work-state handler run. The reviewed
 * 0x800C1AB4 `textbin` row covers the last 160 bytes of the payload, and this
 * dispatcher indexes the code-pointer run that starts 0x28 bytes into it: entry
 * 0 is 0x800C1848, the step-0 installer of this payload, and entries 1 through
 * 11 are the shared main-executable handlers 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8 and 0x801E5288. Those twelve words end at 0x800C1B08,
 * where the four-word step run the sibling dispatcher 0x800C18A0 selects from
 * begins, so this declaration binds only the word the scaled index is added to
 * and the reviewed row stays one span. */
extern void (*D_800C1ADC[])(void); /* @source 0x800C1ADC @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 before it creates
 * its frame, takes that record's unsigned byte at offset 1, scales it by four
 * and calls the matching entry of the twelve-word code-pointer run based at
 * 0x800C1ADC with no arguments. The state byte is the one the installer at
 * 0x800C1848 sets to 2, so state 0 selects that installer while states 1
 * through 11 hand the frame to a shared main-executable handler. The original
 * does not bound the index before the scaled table load, and the slots after
 * the index byte, after the table load and in the branch delay slot stay nops.
 * It writes no state and leaves the handler's return register untouched; the
 * 0x18-byte frame only keeps the return address at 0x10 across the indirect
 * call. No word of this payload holds 0x800C1804 and no jal of it reaches this
 * address, so the caller that enters the shim sits outside the payload: this is
 * the payload's first code span, directly behind the four header bytes and
 * ahead of the installer 0x800C1848.
 *
 * Byte evidence: the sibling row at 0x800C18F0 of this payload is the same
 * 0x44-byte body, shown by a payload cmp whose only two differing bytes are the
 * low half of the scaled table load's offset word (0x1ADC here, 0x1B18 there),
 * so the statement order, operand forms and declaration style come from that
 * disassembly while the table literal is written from these bytes. That row was
 * still `asm` when this seed was written, so it carried no source to clone and
 * supplied shape only; the family spelling of this body is EMI_DEFINE_DISPATCHER,
 * whose expansion is the 17-instruction span measured here.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      dispatchByte1Table1adc,
                      D_800C1ADC);
