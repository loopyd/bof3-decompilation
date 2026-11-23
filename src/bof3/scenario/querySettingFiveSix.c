#include "bof3/bof3.h"

/* @source 0x80144F5A @kind data */
extern u8 D_80144F5A[];
/* @source 0x80146865 @kind data */
extern u8 D_80146865;

/* @source 0x801FC808
 * @behavior Two mirrored scans of the three shared selection slots at
 *           D_80144F5A. The first walks slots 0, 1 and 2 and stores 1 into the
 *           shared state byte D_80146865 for every slot that reads 5; the
 *           second walks the same three slots and stores 2 for every slot that
 *           reads 6. The second scan runs last, so the byte ends as 2 when any
 *           slot reads 6, as 1 when no slot reads 6 but one reads 5, and
 *           unchanged when no slot reads either value. Takes no arguments and
 *           returns nothing, and the body is a leaf with no frame. Each slot
 *           load materialises the 0x8014 page in $at and adds the
 *           zero-extended index to it, while the comparison constant, the
 *           stored constant and the 0x80146865 address of each scan are loop
 *           invariants its preheader materialises once, the address into $a1,
 *           the stored constant into $a0 and the comparison constant into $a2.
 *           The first scan copies its index mask into its branch delay slot and
 *           the second recomputes it at the top of its body. The only call to
 *           this body is the jump at 0x801F9CCC inside func_801F98A4, which
 *           passes no argument, so the roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void querySettingFiveSix(void) {
  u8 i;

  for (i = 0; i < 3u; i++) {
    if (D_80144F5A[i] == 5u) {
      u8 *slot = &D_80146865;
      *slot = 1u;
    }
  }

  for (i = 0; i < 3u; i++) {
    if (D_80144F5A[i] == 6u) {
      u8 *slot = &D_80146865;
      *slot = 2u;
    }
  }
}
