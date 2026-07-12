#include "internal.h"

/* @behavior returns the signed 2D cross product of the edge from `arg0` to `arg1`
 * against the edge from `arg1` to `arg2`.
 * @source 0x801f4158 FUN_801f4158
 */
s16 func_801f4158(const s16* arg0, const s16* arg1, const s16* arg2) {
  struct {
    s16 field_0;
    s16 field_2;
    s16 field_4;
    s16 field_6;
    s16 field_8;
    s16 field_A;
  } volatile sp;
  s16 temp_a0;
  s16 temp_a3;
  s16 temp_t0;
  s16 temp_v0;

  temp_t0 = arg1[0] - arg0[0];
  sp.field_0 = temp_t0;
  temp_a3 = arg1[1] - arg0[1];
  sp.field_2 = temp_a3;
  temp_a0 = arg2[0] - arg1[0];
  sp.field_8 = temp_a0;
  temp_v0 = arg2[1] - arg1[1];
  sp.field_A = temp_v0;

  return (s16)((temp_t0 * temp_v0) - (temp_a3 * temp_a0));
}
