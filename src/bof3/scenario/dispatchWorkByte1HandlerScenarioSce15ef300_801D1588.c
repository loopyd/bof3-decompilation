#include "bof3/emi/dispatchers.h"

/* This overlay's work-state handler run based at 0x801D280C: it is entry 16 of
 * the nineteen-entry run that begins at 0x801D27CC, so only three of its words
 * lie inside the payload - 0x801D280C, 0x801D2810 and 0x801D2814 holding
 * 0x801D15CC, 0x801D1614 and 0x801D16BC, the last being the final word before
 * the payload ends at 0x801D2818. Entry 0 is therefore the handler at
 * 0x801D15CC, which writes 0x12C to the work object halfword at +0x2E, clears
 * its byte at +0x9, sweeps the record table at 0x800E60A0 through
 * clearWorkTableAt800E60A0ScenarioSce15ef300_801D25C0 and only then advances
 * this dispatcher's own index byte, so a work byte of 0 here does not re-enter
 * it. The base word is the last word of the T_801D27D0 `textbin` span
 * (0x801D27D0 through 0x801D280F) and the following two fall inside the Splat
 * `asm` span labelled func_801D2810, whose eight bytes are those pointer words
 * rather than instructions, so no reviewed data row places the base; this
 * dispatcher is its only reference.
 * @source 0x801D280C @kind table
 */
extern void (*D_801D280C[])(void);

/* @source 0x801D1588
 * @behavior Overlay work-state dispatcher of this payload: it loads the
 * work-object pointer published at the scratchpad pointer cell 0x1F800044
 * before it creates its frame, takes that object's unsigned byte at offset +0x1
 * (the work byte the handler runs' entries advance), scales it by four and calls
 * that entry of the handler run at 0x801D280C with no arguments, discarding any
 * result. In-payload entries 0 through 2 of that run are 0x801D15CC, 0x801D1614
 * and 0x801D16BC at 0x801D2814, so work bytes 0 through 2 select this
 * dispatcher's handlers; the index is not bounded before the scaled load, so a
 * larger work byte reads past the end of the payload. It reads no other state
 * and writes none; the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the index load's delay slot, the table load's delay slot
 * and the `jalr` delay slot all stay nops. No `jal` and no word of this payload
 * addresses 0x801D1588, so the overlay loader outside the payload enters it. Its
 * 68 bytes are the 17-instruction dispatcher shape the four exact siblings
 * dispatchWorkByte1HandlerScenarioSce15ef300_801D0D38, ..._801D0EAC,
 * ..._801D1068 and ..._801D13DC record; a span comparison against each leaves
 * the same two differing bytes at offset 0x24, the table address's low immediate
 * (0x280C here against 0x27CC, 0x27D8, 0x27EC and 0x2800 there).
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef300_801D1588, D_801D280C);
