#include "bof3/emi/dispatchers.h"

/* Overlay handler run indexed from 0x801D27C0: three in-image code-pointer
 * words holding 0x801D0C48, 0x801D0CC4 and 0x801D0D18, the last of which is
 * the helper wrapper invokeHelper. The base word
 * sits inside the Splat `asm` row func_801D27B0, whose span 0x801D27B0 through
 * 0x801D27CC holds four packed halfword pairs followed by these three code
 * pointers, so no reviewed data row places the base; the dispatcher below is
 * its only reference.
 * @source 0x801D27C0 @kind table
 */
extern void (*D_801D27C0[])(void);

/* @source 0x801D0C04
 * @behavior Overlay work-state dispatcher of this payload, and its first code
 * word: it loads the work-object pointer published at the scratchpad pointer
 * cell 0x1F800044 before it creates its frame, takes that object's unsigned
 * byte at offset +0x1, scales it by four and calls that entry of the
 * three-word in-image code-pointer run at 0x801D27C0 with no arguments,
 * discarding any result. That run holds 0x801D0C48, 0x801D0CC4 and
 * 0x801D0D18, the last being the helper wrapper
 * invokeHelper, so work bytes 0 through 2 select
 * this dispatcher's handlers; the index is not bounded before the scaled table
 * load. It reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra at 0x10 across the indirect call, and the index load's delay slot,
 * the table load's delay slot and the `jalr` delay slot all stay nops. No
 * `jal` and no word of this payload addresses 0x801D0C04, so the overlay
 * loader outside the payload enters it. Its 68 bytes are the 17-instruction
 * dispatcher shape the five exact siblings
 * callByteStepHandler2, ..._801D0EAC,
 * ..._801D1068, ..._801D13DC and ..._801D1588 record; a span comparison leaves
 * one differing byte at 0x24 against the first three and that byte plus the
 * page low byte at 0x25 against the last two, the table-address low immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callByteStepHandler, callByteStepHandler, D_801D27C0);
