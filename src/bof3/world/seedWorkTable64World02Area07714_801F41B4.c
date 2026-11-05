#include "bof3/bof3.h"

/* @source 0x800E5500 @kind unknown */
extern u16 workTableHalfword800E5500;
/* @source 0x800E5502 @kind unknown */
extern u8 workTableByte800E5502;

/* @source 0x801F41B4
 * @behavior Clears the shared halfword at 0x800E5500 and byte at 0x800E5502,
 *           then seeds 64 records at 0x800E4800 with stride 0x34. Each record
 *           receives the scratch work record's words +0x34/+0x38/+0x3C at
 *           +0/+4/+8 and a cleared halfword at +0x30. Takes no arguments
 *           and returns nothing; the word values are reread on each iteration.
 * @status partial
 * @match 50.00
 * @residual Three extra instructions from a split loop cursor; register and store-order differences.
 */
void seedWorkTable64World02Area07714_801F41B4(void) {
  u32* entry;
  u8 i;
  u32* tail;
  u32* work;
  u32 value;

  entry = PSX_PTR(u32, 0x800E4800u);
  i = 0;
  tail = PSX_PTR(u32, 0x800E4830u);
  work = SPAD_PTR_SLOT(u32, 0x44);
  workTableHalfword800E5500 = 0;
  workTableByte800E5502 = 0;
  do {
    entry[0] = work[13];
    tail[-11] = work[14];
    value = work[15];
    *(u16*)tail = 0;
    tail[-10] = value;
    entry += 13;
    tail += 13;
    i++;
  } while (i < 0x40);
}
