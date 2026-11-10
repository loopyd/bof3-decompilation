#include "bof3/bof3.h"

extern u8 D_80146867; /* @source 0x80146867 @kind data */
extern u8 *D_80148210; /* @source 0x80148210 @kind data */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* @source 0x801F3980 */
extern u8 func_801F3980(s32 *x, s32 *y);
/* @source 0x801F3BA0 */
extern void func_801F3BA0(s32 x, s32 y);
/* @source 0x801F3A14 */
extern void func_801F3A14(s32 x, s32 y);

/* @source 0x801F3894
 * @behavior Obtains two work-position words through 0x801F3980. When its
 *           byte result is nonzero, mode 10 calls 0x801F3BA0 and advances
 *           the shared record halfword at +0x0A by 6 unless work word +0x34
 *           is 0x668000. Mode 20 calls 0x801F3A14, then advances that
 *           halfword by 3 and writes 0x2D to 0x801448EC if the reloaded
 *           work word +0x34 is 0x858000. Other modes do nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void updateMode10Or20WorkPositionAndAdvanceHalfword0aWorld01Area05213_801F3894(void) {
  s32 x;
  s32 y;

  if (func_801F3980(&x, &y)) {
    if (D_80146867 == 10) {
      if (SPAD_PTR_SLOT(s32, 0x44u)[0xD] != 0x668000) {
        func_801F3BA0(x, y);
        FIELD_REF(u16, D_80148210, 0x0A) += 6;
      }
    } else if (D_80146867 == 20) {
      func_801F3A14(x, y);
      if (SPAD_PTR_SLOT(s32, 0x44u)[0xD] == 0x858000) {
        FIELD_REF(u16, D_80148210, 0x0A) += 3;
        D_801448EC = 0x2D;
      }
    }
  }
}
