#include "bof3/bof3.h"

/* @source 0x801F3F64
 * @behavior Writes 0xA0 to the scratchpad halfword at pointer-slot 0x44 + 0x2E and 0x50 to the halfword at + 0x30.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkHalfwordsWorld02Area08813_801F3F64(void) {
  SPAD_PTR_SLOT(u16, 0x44)[23] = 0xA0;
  SPAD_PTR_SLOT(u16, 0x44)[24] = 0x50;
}
