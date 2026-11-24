#include "bof3/bof3.h"
#include "bof3/core/slus_variables.h"

extern u8* workObjectPointer; /* @source 0x1F800044 @kind unknown */
void func_801F7678(u8 value);
void func_801F7AE4(void);
s32 stateTableReportActive(void);

/* @source 0x801F7480
 * @behavior Calls func_801F7678 with unsigned work byte 9, calls
 * func_801F7AE4 only when bit 0 of the shared word at 0x80143E6C is clear,
 * then calls stateTableReportActive and discards its result. Adds four to work byte 9
 * modulo 256 and tests the reloaded byte. On zero, reloads byte 9 with 32
 * and increments state byte 1. Reloads the scratchpad work pointer after
 * calls and stores, preserving possible aliases with its cell at 0x1F800044.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementAdvanceState(void) {
  func_801F7678(workObjectPointer[9]);
  if ((D_80143E6C & 1) == 0) {
    func_801F7AE4();
  }
  stateTableReportActive();
  workObjectPointer[9] += 4;
  if (workObjectPointer[9] == 0) {
    workObjectPointer[9] = 32;
    workObjectPointer[1]++;
  }
}
