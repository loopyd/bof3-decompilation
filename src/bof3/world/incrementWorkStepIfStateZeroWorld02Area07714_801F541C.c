#include "bof3/bof3.h"

extern u8 D_8014933B;

void func_801F54B8(void);

/* @source 0x801F541C
 * @behavior Reads the state byte at 0x8014933B. If zero, increments
 *           byte +2 of the work record at scratchpad pointer cell 0x1F800044.
 *           Always calls the local helper at 0x801F54B8 afterward.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkStepIfStateZeroWorld02Area07714_801F541C(void) {
  u8* work;

  if (D_8014933B == 0) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    work[2]++;
  }
  func_801F54B8();
}
