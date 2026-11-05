#include "bof3/bof3.h"

typedef s32 (*ScenarioScena0100LookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8. */
s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* Two identical five-byte lookup records at 0x801FE350 - key byte 0x08,
 * coordinates 0x32 and 0x16, flags 0x07 and span count 2 - followed by two
 * alignment bytes up to the handler run at 0x801FE35C.
 * @source 0x801FE350 @kind table
 */
extern u8 scenarioScena0100CoordinateLookupRecords[];

/* Six code pointers at 0x801FE35C: 0x801FE0A4 twice, then 0x801FE10C,
 * 0x801FE1B0, 0x801FE224 and 0x801FE244, the last word ending the payload at
 * 0x801FE374.
 * @source 0x801FE35C @kind table
 */
extern ScenarioScena0100LookupHandler scenarioScena0100CoordinateLookupHandlers[];

/* @source 0x801FE03C
 * @behavior Passes the two unsigned halfword coordinates its caller forwards,
 * the two lookup records at 0x801FE350 and the length 2 to the resident helper
 * func_801A7AD8, which matches a record's key against the shared world byte
 * D_80143F00, its coordinate span against those coordinates and its low-nibble
 * category against the shared record byte at 0x80145E98. It reads the answer as
 * a signed byte: a negative index returns -1, otherwise it calls the indexed
 * no-argument handler of the six-word run at 0x801FE35C and returns that
 * handler's signed low byte. Only indices 0 and 1 are reachable with two
 * records and both hold 0x801FE0A4, and the two records are identical, so a
 * matching pair always runs func_801FE0A4. It writes no memory; the 0x18-byte
 * frame only keeps $ra across the two calls. This address is reached only
 * indirectly: it is word 4 of the code-pointer run at 0x801FE288, the word
 * stored at 0x801FE298, and no in-image jal targets it.
 * The 26 instructions are the same shape as the exact sibling
 * dispatchCoordinateLookupHandlerScenarioScena0900_801FD280 (0x801FD280 of
 * emi/scenario/scena09/00) and differ only in the lookup length, the record
 * table address and the handler table address.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchCoordinateLookupHandlerScenarioScena0100_801FE03C(u16 coordA, u16 coordB) {
  s8 index;

  index = func_801A7AD8(scenarioScena0100CoordinateLookupRecords, 2, coordA, coordB);
  if (index < 0) {
    return -1;
  }
  return (s8)scenarioScena0100CoordinateLookupHandlers[index]();
}
