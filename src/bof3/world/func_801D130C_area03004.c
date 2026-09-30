#include "bof3/world/area03004_internal.h"

/* @behavior Stage handler 0 of the AREA030 work-record stage table at
 * 0x801E1D00: rebuilds 30 halfwords (row elements 0x61..0x7E) of the
 * 32-halfword color row at 0x800396C0 from the 0x40-byte row of the 0x80037800
 * color block selected by the shared AREA030 row byte 0x80145EB7, setting the
 * high bit of every copied halfword; afterwards it clears the row's end
 * elements 0x800396C0 and 0x800396FE, copies the scratch work-record byte 0x27
 * into work byte 0xB, sets bit 5 of work byte 0, publishes 0x7B into work byte
 * 0x27, seeds work bytes 0x5C/0x5D/0x5E/0x5F with 1/0xC0/0xC0/0xC0, publishes 1
 * into the palette-stage serial 0x80145988, advances the work-record stage byte
 * at work offset 3 and calls func_8014D4E0 without arguments.
 * @source 0x801D130C
 * @status partial
 * @match 38.67
 * @residual Live mismatch is a same-shape register/scheduling residual, not a
 * semantic one (live `lift gate`: 29/75 insns, 296->300 bytes, first +0x0008,
 * byte-match DIFFER): the whole function is instruction-for-instruction the
 * original's shape except (1) the original schedules the independent
 * `sll i<<1` into the lbu load-delay slot (`lbu v0,0(t0); sll a0,a1,1;
 * srl v1,v0,3`) while cc1 fills that slot with a nop and recomputes the shift
 * after the address adds (one extra instruction, +4 bytes), and (2) the
 * register web lands one class lower (i in $4, row byte address in $3, source
 * base in $2, destination base in $1 versus the original's i in $5, address in
 * $8, base in $7, destination in $6); the tail block including the $at
 * row-end stores, the signed -64 byte seeds and the per-access cursor reloads
 * is already an exact shape match. The source base and sub-block terms are
 * re-derived through the exact sibling func_801C3154 split form to keep the
 * original `row + base` then `+ sub` operand order; the remaining clean-C lever
 * is making the source element shift available before the shared row byte is
 * used, i.e. an explicitly early element-index temporary.
 */
void func_801D130C(void) {
  u16* dst;
  u16* src;
  u8*  cur;
  u8*  rows;
  u16  value;
  u32  row;
  u32  sub;
  u8   index;
  s32  i;

  i = 1;
  rows = &D_80145EB7;
  dst = PSX_PTR(u16, 0x80039600u);
  do {
    index = rows[0];
    row = (u32)(index >> 3) * 0x200u;
    src = PSX_PTR(u16, 0x80037800u + row);
    sub = (u32)(index & 7u) * 0x40u;
    src = PSX_PTR(u16, 0x80037800u + sub + row);
    value = src[i];
    dst[0x60 + i] = value | 0x8000u;
    i++;
  } while (i < 0x1F);

  cur = D_1F800044;
  PSX_REF(u16, 0x800396C0u) = 0;
  PSX_REF(u16, 0x800396FEu) = 0;
  cur[0xB] = cur[0x27];
  cur = D_1F800044;
  cur[0] |= 0x20u;
  cur = D_1F800044;
  cur[0x27] = 0x7B;
  cur = D_1F800044;
  cur[0x5C] = 1;
  cur = D_1F800044;
  ((s8*)cur)[0x5D] = -0x40;
  cur = D_1F800044;
  ((s8*)cur)[0x5E] = -0x40;
  cur = D_1F800044;
  ((s8*)cur)[0x5F] = -0x40;
  D_80145988 = 1;
  cur = D_1F800044;
  cur[3] = cur[3] + 1u;
  func_8014D4E0();
}
