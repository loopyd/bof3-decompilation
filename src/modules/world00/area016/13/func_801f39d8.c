#include "internal.h"

/* @behavior selects one graph-type-dependent drawmode, then emits one indexed
 * translucent sprite using the local four-byte table at `0x801f513c`.
 * @source 0x801f39d8 FUN_801f39d8
 */
void func_801f39d8(s16 arg0, s16 arg1, u32 arg2) {
  u8* primitive;
  s32 table_index;

  SetDrawMode(
      (DR_MODE*)WORLD00_AREA016_PRIMITIVE_PTR, 0, 0,
      (GetGraphType() == 1) ? 0x22cu : ((GetGraphType() == 2) ? 0x22cu : 0x9cu),
      0);
  func_8014e5a0(1u, 0x0cu);

  primitive = WORLD00_AREA016_PRIMITIVE_PTR;
  SetSprt((SPRT*)primitive);
  SetSemiTrans((void*)primitive, 1);
  *(volatile u8*)(primitive + 4) = 0x80u;
  *(volatile u8*)(primitive + 5) = 0x80u;
  *(volatile u8*)(primitive + 6) = 0x80u;

  table_index = (s32)((arg2 & 0xffu) * 4u);
  *(volatile s16*)(primitive + 8) = arg0;
  *(volatile s16*)(primitive + 10) = arg1;
  *(volatile u16*)(primitive + 0xe) = 0x7b80u;
  *(volatile u16*)(primitive + 0x10) =
      WORLD00_AREA016_SPRT_TABLE[table_index + 0u];
  *(volatile u16*)(primitive + 0x12) =
      WORLD00_AREA016_SPRT_TABLE[table_index + 1u];
  *(volatile u8*)(primitive + 0xc) =
      WORLD00_AREA016_SPRT_TABLE[table_index + 2u];
  *(volatile u8*)(primitive + 0xd) =
      WORLD00_AREA016_SPRT_TABLE[table_index + 3u];
  func_8014e5a0(1u, 0x14u);
}
