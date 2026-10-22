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
 * @match 98.65
 * @residual The row-base add follows the sub-block shift instead of preceding it; one instruction placement differs at unchanged 296-byte size.
 */
/* The loop keeps the original's `src[i]` indexed load and gives the element
 * offset a second use as the destination byte offset, so cc1 shares one scale
 * multiply between the load and the store: that places `sll i<<1` in the lbu
 * load-delay slot. The destination base is the row address minus its
 * 0x60-element prefix so the store keeps the original's `0xC0` displacement and
 * two-byte pointer step. The tail re-reads the volatile cursor cell once per
 * original read; each read owns its local because a shared local moves the
 * whole tail web into $a0 and loses the original's $v1/$v0 split.
 */
void func_801D130C(void) {
  u8*  cur;
  u8*  cur2;
  u8*  cur3;
  u8*  cur4;
  u8*  rows;
  u16* src;
  u16* dst;
  u16  value;
  u32  row;
  u32  sub;
  u32  elem;
  u8   index;
  s32  i;

  i = 1;
  rows = &D_80145EB7;
  dst = PSX_PTR(u16, 0x80039600u);
  do {
    elem = (u32)i * 2u;
    index = rows[0];
    row = (u32)(index >> 3) * 0x200u;
    sub = (u32)(index & 7u) * 0x40u;
    src = PSX_PTR(u16, 0x80037800u + sub + row);
    value = src[i];
    *(u16*)((u8*)dst + 0xC0u + elem) = value | 0x8000u;
    i++;
  } while (i < 0x1F);

  cur = D_1F800044;
  PSX_REF(u16, 0x800396C0u) = 0;
  PSX_REF(u16, 0x800396FEu) = 0;
  cur[0xB] = cur[0x27];
  cur2 = D_1F800044;
  cur2[0] |= 0x20u;
  cur2 = D_1F800044;
  cur2[0x27] = 0x7B;
  cur3 = D_1F800044;
  cur3[0x5C] = 1;
  cur3 = D_1F800044;
  ((s8*)cur3)[0x5D] = -0x40;
  cur3 = D_1F800044;
  ((s8*)cur3)[0x5E] = -0x40;
  cur4 = D_1F800044;
  ((s8*)cur4)[0x5F] = -0x40;
  D_80145988 = 1;
  cur = D_1F800044;
  cur[3] = cur[3] + 1u;
  func_8014D4E0();
}
