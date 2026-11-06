#include "bof3/bof3.h"

typedef s32 (*ScenarioScena0600CoordinateLookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8. */
s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* Four five-byte lookup records at 0x801FE478, the twentieth byte ending on
 * the handler-run boundary at 0x801FE48C. Every record carries world key 0x4D,
 * flags 0x01 and span count 2; the coordinate pairs are 0x5D/0x34, 0x5E/0x34,
 * 0x5D/0x2E and 0x74/0x06.
 * @source 0x801FE478 @kind table
 */
extern u8 scenarioScena0600CoordinateLookupRecords[];

/* The 22 in-image code pointers at 0x801FE48C, running from 0x801FDD7C to
 * 0x801FE38C and terminated by the null word at 0x801FE4E4. With four lookup
 * records only entries 0 through 3 are reachable.
 * @source 0x801FE48C @kind table
 */
extern ScenarioScena0600CoordinateLookupHandler scenarioScena0600CoordinateLookupHandlers[];

/* @source 0x801FDD14
 * @behavior Overlay dispatch reached only indirectly: no in-image jal targets
 * the address, and its only image word is the pointer at 0x801FE3D4, word 4 of
 * the five-word code-pointer run 0x801FE3C4..0x801FE3D4 (0x801F7298,
 * dispatchRecordCallbackScenarioScena0600_801FCDD0, 0x801FD800, 0x801FD294,
 * this address). It forwards its two unsigned halfword coordinates and the four
 * five-byte lookup records at 0x801FE478 with the length 4 to the resident
 * helper func_801A7AD8 (0x801A7AD8), which matches a record's key byte against
 * the shared world id D_80143F00 and its coordinate pair against those
 * arguments, and reads the answer as a signed byte: a negative index returns -1
 * to the shared epilogue without touching other state, otherwise it calls the
 * indexed no-argument handler of the 22-entry in-image list at 0x801FE48C and
 * answers that handler's signed low byte. With four records only entries 0
 * through 3 are reachable: 0x801FDD7C, the inert
 * noopHandlerScenarioScena0600_801FDE2C, 0x801FDE34 and 0x801FDEA4. It performs
 * no stores of its own and its 0x18-byte frame only keeps $ra across the two
 * calls; the length constant is materialised in the first jal's delay slot and
 * the handler base in $at per the assembler's symbol-plus-register expansion.
 * The 26 instructions are the same shape as the exact sibling
 * dispatchCoordinateLookupHandlerScenarioScena0900_801FD280 (0x801FD280 of
 * emi/scenario/scena09/00) and differ only in the lookup length, 4 here and 14
 * there, and the two table addresses, 0x801FE474/0x801FE4BC there and
 * 0x801FE478/0x801FE48C here.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchCoordinateLookupHandlerScenarioScena0600_801FDD14(u16 coordA, u16 coordB) {
  s8 index;

  index = func_801A7AD8(scenarioScena0600CoordinateLookupRecords, 4, coordA, coordB);
  if (index < 0) {
    return -1;
  }
  return (s8)scenarioScena0600CoordinateLookupHandlers[index]();
}
