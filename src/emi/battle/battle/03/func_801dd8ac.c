#include "internal.h"

/* @behavior copies the current local battler's visible values and masked flags into
 * the template record selected by byte `0x13c`.
 * @source 0x801dd8ac FUN_801dd8ac
 */
void func_801dd8ac(u32 arg0) {
  u32 local_offset;
  u32 product;
  u32 record_offset;
  u16 flags;

  local_offset = arg0 & 0xffu;
  product = local_offset << 2;
  product += local_offset;
  local_offset = product << 6;
  if ((*(volatile u8*)(local_offset + 0x80140000u + 0x5e90u) & 1u) != 0u) {
    record_offset =
        *(volatile u8*)(0x80140000u + local_offset + 0x5fccu) * 0xa4u;
    *(volatile u16*)(0x80140000u + record_offset + 0x497cu) =
        *(volatile u16*)(0x80140000u + local_offset + 0x5f18u);
    *(volatile u16*)(0x80140000u + record_offset + 0x497eu) =
        *(volatile u16*)(0x80140000u + local_offset + 0x5f1au);
    *(volatile u8*)(0x80140000u + record_offset + 0x4980u) =
        *(volatile u8*)(0x80140000u + local_offset + 0x5f1cu);
    flags = *(volatile u16*)(0x80140000u + local_offset + 0x5f10u) & 0x60a0u;
    *(volatile u16*)(0x80140000u + local_offset + 0x5f10u) = flags;
    *(volatile u16*)(0x80140000u + record_offset + 0x4974u) = flags;
  }
}
