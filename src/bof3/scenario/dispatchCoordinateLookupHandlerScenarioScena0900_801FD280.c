#include "bof3/bof3.h"

typedef s32 (*ScenarioScena0900LookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8. */
s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* Fourteen five-byte lookup records, followed by two alignment bytes.
 * @source 0x801FE474 @kind table
 */
extern u8 scenarioScena0900CoordinateLookupRecords[];

/* @source 0x801FE4BC @kind table */
extern ScenarioScena0900LookupHandler scenarioScena0900CoordinateLookupHandlers[];

/* @source 0x801FD280
 * @behavior Passes two unsigned halfword coordinates and the fourteen lookup
 * records at 0x801FE474 to helper 0x801A7AD8. Interprets its result as a signed
 * byte. A negative index returns -1; otherwise calls the indexed no-argument
 * handler from the table at 0x801FE4BC and returns its signed low byte. Makes
 * no direct stores. The callback table has 38 entries and a null terminator;
 * the lookup helper can select only indices 0 through 13 for this call.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchCoordinateLookupHandlerScenarioScena0900_801FD280(u16 coordA, u16 coordB) {
    s8 index;

    index = func_801A7AD8(scenarioScena0900CoordinateLookupRecords, 14, coordA, coordB);
    if (index < 0) {
        return -1;
    }
    return (s8)scenarioScena0900CoordinateLookupHandlers[index]();
}
