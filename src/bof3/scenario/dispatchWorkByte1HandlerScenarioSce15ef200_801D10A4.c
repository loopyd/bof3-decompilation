#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D1B1C: fourteen in-image code pointers from
 * 0x801D0C70 (entry 0) through 0x801D11A4 (entry 13) at 0x801D1B50, the last word
 * of the 3924-byte payload, which ends at 0x801D1B54. The target's Splat
 * `textbin` row labels the first eight words func_801D1B1C and the remaining six
 * sit in the neighbour `asm` span labelled func_801D1B3C, so no reviewed data row
 * places this dispatcher's base; the generated D_ row for that interior word and
 * this reference are all that bind it. This base is entry 11 of the run, the
 * neighbour arm handler 0x801D10E8, which the sibling dispatcher
 * dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4 reaches as its entry 7
 * from base 0x801D1B2C. */
extern void (*D_801D1B48[])(void); /* @source 0x801D1B48 @kind data */

/* @source 0x801D10A4
 * @behavior Overlay step dispatch on work-record byte 1 of this payload: loads the
 * work-object pointer published at the scratchpad pointer cell 0x1F800044 before
 * it creates its frame, takes that object's unsigned byte at offset +0x1 (the
 * same byte the overlay entry func_801D0C04 dispatches on and that the step
 * handlers advance), scales it by four and calls that entry of the handler run
 * based at 0x801D1B48 with no arguments, discarding any result. The base is entry
 * 11 of the run, so the reachable in-image entries are the arm handler
 * 0x801D10E8 and the two handlers after it, 0x801D1118 and 0x801D11A4; the
 * original does not bound the index before the scaled table load, so a work byte
 * above 2 reads past the payload's last table word. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra at 0x10 across the indirect
 * call, and the index load's delay slot and the table load's delay slot both stay
 * nops. No `jal` and no word of this payload addresses 0x801D10A4, so the overlay
 * loader outside the payload enters it, and the neighbour handler
 * armCountdownAndAdvanceWorkByte1ScenarioSce15ef200_801D10E8 records it as the
 * dispatcher of its own base. Its 68 bytes are the 17-instruction dispatcher shape
 * that the exact siblings dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4 in
 * this payload and dispatchWorkByte1HandlerScenarioSce15ef300_801D1068 record; a
 * byte comparison with the former leaves one differing halfword, at 0x24, the
 * table address's low immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef200_801D10A4, D_801D1B48);
