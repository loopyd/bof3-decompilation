#include "bof3/bof3.h"

extern u32 D_80147A8C; /* @source 0x80147A8C @kind data */
extern u32 D_80147A90; /* @source 0x80147A90 @kind data */
extern u32 D_80147A94; /* @source 0x80147A94 @kind data */

/* @source 0x801F4434
 * @behavior Copies the words at 0x80147A8C, 0x80147A90 and 0x80147A94
 * into offsets 0x34, 0x38 and 0x3C of the work record published in
 * scratchpad pointer slot 0x44, in that order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWords7a8cToWork34383cWorld04Area16713_801F4434(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u32*)(work + 0x34) = D_80147A8C;
  *(u32*)(work + 0x38) = D_80147A90;
  *(u32*)(work + 0x3C) = D_80147A94;
}
