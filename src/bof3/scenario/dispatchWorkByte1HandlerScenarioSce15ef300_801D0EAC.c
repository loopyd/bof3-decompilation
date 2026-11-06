#include "bof3/emi/dispatchers.h"

/* Overlay work-state handler run indexed from 0x801D27D8: sixteen in-image
 * code-pointer words, 0x801D27D8 through 0x801D2814, holding 0x801D0EF0,
 * 0x801D0F94, 0x801D1008, 0x801D1028, 0x801D1048, 0x801D10AC, 0x801D11C8,
 * 0x801D1250, 0x801D12E0, 0x801D1370, 0x801D1420, 0x801D1460, 0x801D14DC,
 * 0x801D15CC, 0x801D1614 and 0x801D16BC, which is the last word before the
 * payload ends at 0x801D2818. The base word sits three entries inside the
 * longer run that begins at 0x801D27CC and lies in the Splat `textbin` row
 * T_801D27D0 at payload offset 0x1BD0, while its final two words fall in the
 * trailing `asm` row at 0x801D2810, so no reviewed data row places the base;
 * this dispatcher is its only reference.
 * @source 0x801D27D8 @kind table
 */
extern void (*D_801D27D8[])(void);

/* @source 0x801D0EAC
 * @behavior Overlay work-state dispatcher of this payload: it loads the
 * work-object pointer published at the scratchpad pointer cell 0x1F800044
 * before it creates its frame, takes that object's unsigned byte at offset
 * +0x1 (the work byte the handler runs' entries advance), scales it by four
 * and calls that entry of the overlay handler run at 0x801D27D8 with no
 * arguments, discarding any result. That run's entry 0 is 0x801D0EF0 and its
 * entry 15 is 0x801D16BC at 0x801D2814, so work bytes 0 through 15 select this
 * dispatcher's handlers. The index is not bounded before the scaled table
 * load. It reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra at 0x10 across the indirect call, and the index load's delay slot,
 * the table load's delay slot and the `jalr` delay slot all stay nops. No
 * `jal` and no word of this payload addresses 0x801D0EAC, so the overlay
 * loader outside the payload enters it. Its 68 bytes are the 17-instruction
 * dispatcher shape the exact sibling
 * dispatchWorkByte1HandlerScenarioSce15ef300_801D0D38 (0x801D0D38) records,
 * from which it differs solely in the table-address low immediate (0x27D8
 * here against 0x27CC there, the single differing byte of the two spans).
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef300_801D0EAC, D_801D27D8);
