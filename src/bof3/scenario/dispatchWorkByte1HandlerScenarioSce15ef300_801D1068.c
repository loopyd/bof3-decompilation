#include "bof3/emi/dispatchers.h"

/* This overlay's handler run based at 0x801D27EC: eleven in-image code-pointer
 * words, entry 0 = 0x801D10AC (the function that follows this dispatcher in the
 * payload) through entry 10 = 0x801D16BC at 0x801D2814, the last word before the
 * payload ends at 0x801D2818. The run continues the register the neighbouring
 * dispatchers index: 0x801D27CC holds nineteen entries and 0x801D27D8 sixteen,
 * and this base sits seven words into the T_801D27D0 `textbin` span
 * (0x801D27D0 through 0x801D280F), so no reviewed data row places it; this
 * dispatcher is its only reference. Its last two words fall inside the Splat
 * `asm` span labelled func_801D2810, whose eight bytes are the pointer words
 * 0x801D1614 and 0x801D16BC rather than instructions.
 * @source 0x801D27EC @kind table
 */
extern void (*D_801D27EC[])(void);

/* @source 0x801D1068
 * @behavior Overlay work-state dispatcher: it loads the work-object pointer
 * published at the scratchpad pointer cell 0x1F800044 before it creates its
 * 0x18-byte frame, takes that object's unsigned byte at offset +0x1 (the work
 * byte the overlay's step handlers advance), scales it by four and calls that
 * entry of the handler run at 0x801D27EC with no arguments, discarding any
 * result. Entry 0 of that run is 0x801D10AC and entry 10 is 0x801D16BC, so work
 * bytes 0 through 10 select the overlay's handlers; the index is not bounded
 * before the scaled load. It reads no other state and writes none, and the frame
 * only keeps $ra at 0x10 across the indirect call. Its 68 bytes are the
 * 17-instruction dispatcher shape that the two exact siblings in this payload,
 * dispatchWorkByte1HandlerScenarioSce15ef300_801D0D38 and
 * dispatchWorkByte1HandlerScenarioSce15ef300_801D0EAC, record; a byte comparison
 * of the three spans leaves one differing halfword each, at 0x24, the table
 * address's low immediate. Neither a jal nor any word of this payload addresses
 * 0x801D1068, so the overlay loader enters it from outside the payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef300_801D1068, D_801D27EC);
