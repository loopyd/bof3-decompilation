#include "internal.h"

/* @behavior copies the current local battler's visible values and masked flags into
 * the template record selected by byte `0x13c`.
 * @source 0x801DD8AC
 */
/* The volatile views on D_80145F10..D_80145F1C/D_80144974..D_80144980 pin the
 * original per-field load->store interleave; without them gcc hoists all loads
 * above the stores (asm-diff proven). D_80145FCC stays non-volatile: a volatile
 * selector read emits a redundant andi 0xff after the lbu.
 */
void func_801DD8AC(u32 arg0) {
  u32 local_offset;
  u32 template_offset;
  u16 flags;

  arg0 &= 0xffu;
  local_offset = ((arg0 << 2) + arg0) << 6;
  if ((((u8*)D_80145E90)[local_offset] & 1u) != 0u) {
    template_offset = D_80145FCC[local_offset] * 0xa4u;
    *(volatile u16*)&D_8014497C[template_offset] = *(volatile u16*)&D_80145F18[local_offset];
    *(volatile u16*)&D_8014497E[template_offset] = *(volatile u16*)&D_80145F1A[local_offset];
    D_80144980[template_offset] = D_80145F1C[local_offset];
    flags = *(volatile u16*)&D_80145F10[local_offset] & 0x60a0u;
    *(volatile u16*)&D_80145F10[local_offset] = flags;
    *(volatile u16*)&D_80144974[template_offset] = flags;
  }
}
