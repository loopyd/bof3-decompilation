#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F6E04
 * @behavior Adopts the word at offset 0x54 of the shared 152-byte work record
 * selected by the scratch pad work object's word at offset 0x18 into that same
 * work object and marks the work object's dispatch byte at offset 0x01 with 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void adoptSelectedRecordWord(void) {
  u8 *work;
  s32 index;
  s32 value;

  work = D_1F800044;
  index = *(s32 *)(work + 0x18);
  value = *(s32 *)(D_80147A58 + index * 152 + 0x54);

  work[1] = 1;
  *(s32 *)(work + 0x54) = value;
}
