#include "bof3/bof3.h"

/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;
/* @source 0x801F3450 */
extern void func_801F3450(void);

/* @source 0x801F351C
 * @behavior Overlay handler: loads the work record published at scratchpad
 *           pointer cell 0x1F800044 and tests its signed halfword at offset
 *           0x3A against 0x3A. When that halfword matches, the unsigned
 *           halfword at offset 0x36 minus 0x40 selects the value written to the
 *           record byte at offset 8: 0 writes 3 and 2 writes 7, after which the
 *           in-overlay routine at 0x801F3450 runs. Any other halfword value
 *           advances the halfword at offset 0x0A of the record published
 *           through the global pointer cell 0x80148210 by 2 instead. The
 *           record base is loaded once and serves every field access. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte8To3Or7ByHalfword36When3aThenInvokeHelperElseAdvanceHalfword0aBy2World02Area09513_801F351C(void) {
  u8* work;
  u8 value;
  u16 step;

  work = SPAD_PTR_SLOT(u8, 0x44);
  if (FIELD_REF(s16, work, 0x3A) == 0x3A) {
    step = (u16)(FIELD_REF(u16, work, 0x36) - 0x40);
    switch (step) {
      case 0:
        value = 3;
        break;
      case 2:
        value = 7;
        break;
      default:
        D_80148210[5] += 2;
        return;
    }
    work[8] = value;
    func_801F3450();
  } else {
    D_80148210[5] += 2;
  }
}
