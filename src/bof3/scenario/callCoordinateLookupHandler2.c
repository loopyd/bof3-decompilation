#include "bof3/bof3.h"

typedef void (*ScenarioScena0700CoordinateLookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8.
 * @source 0x801A7AD8
 */
extern s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* One five-byte lookup record at 0x801FDFA0, inside the overlay data blob
 * T_801FDE34: key byte 0x55, coordinates 0x25 and 0x3D, flags 0x01 (raised-span
 * direction clear, category nibble 1) and span count 2.
 * @source 0x801FDFA0 @kind table
 */
extern u8 scenarioScena0700CoordinateLookupRecords[];

/* Five-word handler run at 0x801FDFA8: 0x801FDAFC, 0x801FDB34, 0x801FDB80,
 * 0x801FDBCC and 0x801FDBF4, followed by the zero word at 0x801FDFBC.
 * @source 0x801FDFA8 @kind table
 */
extern ScenarioScena0700CoordinateLookupHandler scenarioCoordinateLookupHandlers[];

/* @source 0x801FDA98
 * @behavior Overlay coordinate-record selector, word 3 of the four-word
 * code-pointer run at 0x801FDE00, so no in-image `jal` and no other image word
 * holds its address. It takes the two unsigned halfword coordinates its caller
 * forwards (the two `andi ...,0xFFFF` at 0x801FDAA0 and 0x801FDAA4 move $a0 and
 * $a1 into $a2 and $a3) and passes them with the overlay's single five-byte
 * lookup record at 0x801FDFA0 and the length 1 to the resident helper
 * func_801A7AD8 (0x801A7AD8), which matches that record against the shared
 * route/world word D_80143F00, the coordinate pair and the shared category byte
 * D_80145E98. A negative sign-extended byte answer returns -1 without touching
 * any other state; otherwise it scales that index by four in the `bltz` delay
 * slot, loads the word of the five-word handler run at 0x801FDFA8, calls it
 * with no arguments through `jalr` and answers the constant 1, discarding the
 * handler's result. With one record the lookup can only answer 0 or -1, so the
 * reachable entry is word 0, requestPrimaryState6Substate10
 * (0x801FDAFC). It performs no stores of its own, its 0x18-byte frame only keeps
 * $ra at 0x10 across the two calls, and the success constant is materialised as
 * `addiu $v0,$zero,0x1` in the `j` delay slot at 0x801FDAE4.
 * The body is the instruction shape of the exact lift
 * dispatchRoute30CoordinateLookupHandlerScenarioScena0400_801FA010
 * (0x801FA010 of emi/scenario/scena04/00, recorded exact at 25 instructions):
 * its 0x64 bytes compare equal word for word with that span in every word except
 * the `addiu` low half of the lookup-record base (0xA228 there, 0xDFA0 here), the
 * `lw` displacement of the handler-run base (0xA230 there, 0xDFA8 here) and the
 * absolute `j` word, which shifts by exactly the 0x3A88 offset delta between the
 * two spans. That sibling supplied the statement order, operand forms,
 * declaration style and name this body writes; the record it reads, its key 0x55,
 * and both table addresses come from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 callCoordinateLookupHandler2(u16 coordA, u16 coordB) {
  s8 index;

  index = func_801A7AD8(scenarioScena0700CoordinateLookupRecords, 1, coordA, coordB);
  if (index < 0) {
    return -1;
  }
  scenarioCoordinateLookupHandlers[index]();
  return 1;
}
