#include "bof3/bof3.h"

void func_801F54B8(void);

/* @source 0x801F53BC
 * @behavior Loads the work record from scratchpad pointer cell 0x1F800044,
 *           adds 0x2000 to word +0x38 and 0x10 to halfword +0x3E.
 *           If word +0x38 is then 0x350000, increments byte +2.
 *           Always calls the local helper at 0x801F54B8 afterward.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkFieldsAndAdvanceStepAt350000World02Area07714_801F53BC(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u32*)(work + 0x38) += 0x2000;
  *(u16*)(work + 0x3E) += 0x10;
  if (*(u32*)(work + 0x38) == 0x350000) {
    work[2]++;
  }
  func_801F54B8();
}
