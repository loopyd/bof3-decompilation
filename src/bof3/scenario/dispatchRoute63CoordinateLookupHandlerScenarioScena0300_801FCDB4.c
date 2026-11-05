#include "bof3/bof3.h"

typedef void (*Scena0300Route63LookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8. */
s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* One five-byte lookup record at 0x801FD030, inside the overlay data blob
 * T_801FCF84: route key 0x63, coordinates 0x23 and 0x4F, flags 0x01 and span
 * count 1.
 * @source 0x801FD030 @kind table
 */
extern u8 scenarioScena0300Route63LookupRecords[];

/* Four-word handler run at 0x801FD038: 0x801FCE18, 0x801FCE6C, 0x801FCEDC and
 * 0x801FCEFC, the last word sitting on the 0x801FD044 boundary.
 * @source 0x801FD038 @kind table
 */
extern Scena0300Route63LookupHandler scenarioScena0300Route63LookupHandlers[];

/* @source 0x801FCDB4
 * @behavior Word 19 of the overlay 32-word code-pointer run at 0x801FCF84, so
 * no in-image jal targets it. It takes the two unsigned halfword coordinates
 * its caller forwards and passes them with the single five-byte lookup record
 * at 0x801FD030 (route key 0x63, coordinates 0x23 and 0x4F, flags 0x01, span
 * count 1) and the length 1 to the resident helper func_801A7AD8, which
 * matches that record against the shared route word D_80143F00 and the
 * coordinate pair. A negative signed-byte answer returns -1 without touching
 * any other state; otherwise it calls the indexed no-argument handler of the
 * run at 0x801FD038 (0x801FCE18, 0x801FCE6C, 0x801FCEDC, 0x801FCEFC, of which
 * only entry 0 is reachable with a single record) and answers the constant 1,
 * discarding the handler result. It performs no stores of its own and its
 * 0x18-byte frame only keeps $ra across the two calls.
 * The 25 instructions are the same shape as the exact sibling
 * dispatchCoordinateLookupHandlerScenarioScena0900_801FD280 (0x801FD280 of
 * emi/scenario/scena09/00, 26 instructions) and differ only in the lookup
 * length, the two table addresses and the success answer, which is the
 * constant 1 here where that sibling returns the handler's signed low byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchRoute63CoordinateLookupHandlerScenarioScena0300_801FCDB4(u16 coordA, u16 coordB) {
  s8 index;

  index = func_801A7AD8(scenarioScena0300Route63LookupRecords, 1, coordA, coordB);
  if (index < 0) {
    return -1;
  }
  scenarioScena0300Route63LookupHandlers[index]();
  return 1;
}
