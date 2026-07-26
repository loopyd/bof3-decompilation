#include "internal.h"

/* @source 0x801DA5A8
 * @behavior emits a textured semi-transparent battle sprite primitive
 */
void func_801DA5A8(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 arg4, u8 arg5,
                   u8 arg6) {
  Battle03SpritePrimitive* primitive;

  SetDrawMode((DR_MODE*)D_8014598C, 0, 0, GetTPage(0, 1, 0x3c0, 0), 0);
  func_8014E5A0(1, 0x0c);
  primitive = (Battle03SpritePrimitive*)D_8014598C;
  func_8017AA80(primitive);
  primitive->r0 = arg4;
  primitive->g0 = arg5;
  primitive->b0 = arg6;
  primitive->x0 = arg0;
  primitive->y0 = arg1;
  primitive->unk_0c = arg2;
  primitive->unk_0e = arg3;
  SetSemiTrans(primitive, 1);
  func_8014E5A0(1, 0x10);
}
