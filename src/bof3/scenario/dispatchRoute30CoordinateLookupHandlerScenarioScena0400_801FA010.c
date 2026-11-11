#include "bof3/bof3.h"

typedef void (*ScenarioScena0400CoordinateLookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8.
 * @source 0x801A7AD8
 */
extern s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* One five-byte lookup record at 0x801FA228, inside the overlay data blob
 * T_801FA180: route key 0x30, coordinates 0x83 and 0x06, flags 0x05 (raised-span
 * direction clear, category nibble 5) and span count 2.
 * @source 0x801FA228 @kind table
 */
extern u8 scenarioScena0400CoordinateLookupRecords[];

/* Four-word primary-state request handler run at 0x801FA230: 0x801FA074,
 * 0x801FA0B4, 0x801FA12C and 0x801FA178, the last of which is the no-op
 * terminator the lookup can also select.
 * @source 0x801FA230 @kind table
 */
extern ScenarioScena0400CoordinateLookupHandler scenarioScena0400CoordinateLookupHandlers[];

/* @source 0x801FA010
 * @behavior Work-object handler word 7 of the eight-word code-pointer run at
 * 0x801FA1E4, so no in-image `jal` and no other image word holds its address.
 * It takes the two unsigned halfword coordinates its caller forwards (the two
 * `andi ...,0xFFFF` at 0x801FA018 and 0x801FA01C move $a0 and $a1 into $a2 and
 * $a3) and passes them with the overlay's single five-byte lookup record at
 * 0x801FA228 and the length 1 to the resident helper func_801A7AD8 (0x801A7AD8),
 * which matches that record against the shared route word D_80143F00 and the
 * coordinate pair. A negative sign-extended byte answer returns -1 without
 * touching any other state; otherwise it scales that index by four in the
 * `bltz` delay slot, loads the word of the four-word primary-state request
 * handler run at 0x801FA230, calls it with no arguments through `jalr` and
 * answers the constant 1, discarding the handler's result. It performs no stores
 * of its own, its 0x18-byte frame only keeps $ra at 0x10 across the two calls,
 * and the success constant is materialised as `addiu $v0,$zero,0x1` in the `j`
 * delay slot at 0x801FA05C.
 * The body is the instruction shape of the lift
 * dispatchRoute63CoordinateLookupHandlerScenarioScena0300_801FCDB4 (0x801FCDB4
 * of emi/scenario/scena03/00, recorded exact at 25 instructions), whose bytes
 * differ here in two words only: the `addiu` low half of the lookup-record base
 * (0xD030 there, 0xA228 here) and the `lw` displacement of the handler-run base
 * (0xD038 there, 0xA230 here). The record the helper reads, its route key 0x30,
 * and both table addresses come from this span's own bytes. The route-word role
 * of D_80143F00 is the accepted form of
 * forwardRoute28RecordScenarioScena0400_801F9DB4 (0x801F9DB4) here and of
 * dispatchRoute63CoordinateLookupHandlerScenarioScena0300_801FCDB4 there.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchRoute30CoordinateLookupHandlerScenarioScena0400_801FA010(u16 coordA, u16 coordB) {
  s8 index;

  index = func_801A7AD8(scenarioScena0400CoordinateLookupRecords, 1, coordA, coordB);
  if (index < 0) {
    return -1;
  }
  scenarioScena0400CoordinateLookupHandlers[index]();
  return 1;
}
