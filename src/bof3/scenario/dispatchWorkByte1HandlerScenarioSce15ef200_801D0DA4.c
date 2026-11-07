#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801D1B1C: fourteen in-image code pointers from
 * 0x801D0C70 (entry 0) through 0x801D11A4 (entry 13) at 0x801D1B50, the last
 * word before the payload's zero padding. The target's Splat `textbin` row
 * labels the first eight words func_801D1B1C and the remaining six sit in the
 * neighbour span labelled func_801D1B3C, so no reviewed data row places this
 * dispatcher's base; the generated D_ row for its interior word and this
 * reference are all that bind it. The overlay entry func_801D0C04 indexes the
 * same run from its first word, so work byte 0 reaches 0x801D0C70 while this
 * dispatcher's base is entry 4. */
extern void (*D_801D1B2C[])(void); /* @source 0x801D1B2C @kind data */

/* @source 0x801D0DA4
 * @behavior Overlay step dispatch on work-record byte 1 of this payload: loads
 * the work-object pointer published at the scratchpad pointer cell 0x1F800044
 * before it creates its frame, takes that object's unsigned byte at offset +0x1
 * (the same byte the overlay entry func_801D0C04 dispatches on and that the
 * step handlers advance), scales it by four and calls that entry of the
 * handler run based at 0x801D1B2C with no arguments, discarding any result. The
 * base is entry 4 of the run, so the reachable entries are the step handlers
 * 0x801D0DE8, 0x801D0E30 and 0x801D0EC4 (entries 4, 5 and 6) and, since the
 * original does not bound the index before the scaled table load, the arm-handler
 * entries that follow: 0x801D0F68 with its countdown byte arm, then 0x801D0F9C,
 * 0x801D1008 and 0x801D1050. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the index
 * load's delay slot and the table load's delay slot both stay nops. No `jal` and
 * no word of this payload addresses 0x801D0DA4, so the overlay loader outside
 * the payload enters it. Its 68 bytes are the 17-instruction dispatcher shape
 * that the exact lift func_801D1B10 (0x801D1B10 in emi/world00/area030/04) and
 * dispatchWorkByte1HandlerScenarioSce15ef300_801D0D38 (0x801D0D38 in
 * emi/scenario/sce15ef3/00) record; they differ from it only in the
 * table-address immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4, D_801D1B2C);
