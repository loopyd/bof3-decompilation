#include "bof3/world/area02414_internal.h"

/* @behavior returns the signed 2D cross product of the edge from `arg0` to `arg1`
 * against the edge from `arg1` to `arg2`, reading every input component as an
 * unsigned halfword and the four signed deltas as the saved edge vectors.
 * @source 0x801F4158
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 func_801F4158(const u16* arg0, const u16* arg1, const u16* arg2) {
  /* The delta edges are read through unqualified halfword pointers: a
   * `const u16 *` view lets the compiler reuse the first edge load across the
   * slot stores, dropping the original's per-edge reloads. */
  u16* p0 = (u16*)arg0;
  u16* p1 = (u16*)arg1;
  u16* p2 = (u16*)arg2;
  struct {
    u16 field_0;
    u16 field_2;
    u16 field_4;
    u16 field_6;
    u16 field_8;
    u16 field_A;
  } sp;
  u16 temp_t0;
  u16 temp_a3;
  u16 temp_a0;
  u16 temp_v0;

  temp_t0 = p1[0] - p0[0];
  sp.field_0 = temp_t0;
  temp_a3 = p1[1] - p0[1];
  sp.field_2 = temp_a3;
  temp_a0 = p2[0] - p1[0];
  sp.field_8 = temp_a0;
  temp_v0 = p2[1] - p1[1];
  sp.field_A = temp_v0;

  return (s16)((temp_t0 * temp_v0) - (temp_a3 * temp_a0));
}
