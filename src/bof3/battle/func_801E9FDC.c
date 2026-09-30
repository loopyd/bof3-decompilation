#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9FDC
 * @behavior Recomposes the active panel halfwords +0x04/+0x06 from the
 * D_80145E90 local work record selected by panel byte +0x0A: halfword +0x04 is
 * the record signed halfword +0x2E plus the signed byte at D_801EB0B0 indexed by
 * (record byte +0x08) * 2 + (record byte +0x79) * 8, plus D_801EAEA4 while that
 * record byte +0x08 is 0 or 3 and D_801EAEA6 otherwise; halfword +0x06 is the
 * record unsigned halfword +0x30 plus that same index byte at D_801EB0B1
 * sign-extended plus 0xC. While the selection gate byte 0x801462EF is set, a
 * queued slot byte D_801EB4D8 with no 0xC0 bits that equals panel byte +0x0A, or
 * that has bit 0x80 set, forwards panel halfwords +0x04/+0x06 and panel byte
 * +0x0A to func_801D8DF8 and returns; otherwise panel byte +0x0F is cleared and
 * panel byte +0x03 is decremented.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the 0x140 record stride has to stay inline in each D_80145EXX
 * subscript. Binding it to an `offset` local leaves the scaled value in a live
 * caller temporary, so gcc spends $v0 on that temporary and re-allocates the
 * whole first block ($v0/$v1/$a0 rotate; measured 90/102 instructions, 408
 * bytes, register names only). With the stride inline the shift-add chain
 * accumulates in one register ($v0) and the block matches byte for byte. The
 * direction key is written kind-major (`b1 * 8 + b0 * 2`) because that order is
 * what keeps the second block's `b1 * 8` shift ahead of `b0 * 2` (measured
 * 89/102 for the byte-major spelling).
 */
void func_801E9FDC(void) {
  u8* panel;
  u8  index;
  u8  b0;
  u8  b1;
  u8  flags;

  panel = D_80148648;
  index = panel[0xA];
  b0 = D_80145E98[index * 0x140];
  b1 = D_80145F09[index * 0x140];
  *(s16*)&panel[4] = *(s16*)&D_80145EBE[index * 0x140] +
                     *(s8*)&D_801EB0B0[b1 * 8 + b0 * 2] +
                     (b0 == 0 || b0 == 3 ? D_801EAEA4 : D_801EAEA6);

  *(s16*)&D_80148648[6] =
      *(u16*)&D_80145EC0[D_80148648[0xA] * 0x140] +
      (s8)D_801EB0B1[b1 * 8 + b0 * 2] + 12;

  if (D_801462EF != 0) {
    flags = *D_801EB4D8;
    if ((flags & 0xC0) == 0) {
      if (flags != D_80148648[0xA]) {
        D_80148648[0xF] = 0;
        D_80148648[3] -= 1;
        return;
      }
    } else if ((flags & 0x80) == 0) {
      D_80148648[0xF] = 0;
      D_80148648[3] -= 1;
      return;
    }
  } else {
    D_80148648[0xF] = 0;
    D_80148648[3] -= 1;
    return;
  }
  func_801D8DF8(*(s16*)&D_80148648[4], *(s16*)&D_80148648[6], D_80148648[0xA]);
}
