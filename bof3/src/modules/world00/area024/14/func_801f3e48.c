#include "internal.h"

/* does: remaps the scratch pointer to the local line table, seeds one matrix,
 * then projects and queues each transformed line strip selected by the count
 * byte behind `0x80147aac`.
 * @source: 0x801f3e48 FUN_801f3e48
 */
s32 func_801f3e48(u8 arg0) {
  volatile u8*      saved_scratch;
  volatile u8*      src;
  MATRIX            matrix;
  MATRIX            matrix_copy;
  volatile LINE_F4* primitive;
  SVECTOR           vertices[4];
  long              depth;
  long              flag;
  u16               count;
  u16               i;
  volatile u32      seed_copy[4];

  seed_copy[0] = *(volatile u32*)0x801f2c04u;
  seed_copy[1] = *(volatile u32*)0x801f2c08u;
  seed_copy[2] = *(volatile u32*)0x801f2c0cu;
  seed_copy[3] = *(volatile u32*)0x801f2c10u;

  saved_scratch = *(volatile u8**)0x1f800044u;
  *(volatile u8**)0x1f800044u = BOF3_WORLD00_AREA024_SCRATCH_REMAP;

  PushMatrix();

  count = *BOF3_WORLD00_AREA024_PTR_7AAC;
  func_8015b410(&matrix);
  SetRotMatrix(&matrix);
  SetTransMatrix(&matrix);

  matrix_copy = matrix;
  func_8015b4b0(&matrix_copy);

  func_8017c2d8((void*)BOF3_WORLD00_AREA024_PRIMITIVE_PTR, 0, 0,
                func_8017a620(0, 1, 0x380, 0x100), 0);
  func_8014e5a0(1u, 0x0cu);

  if (count != 0u) {
    src = BOF3_WORLD00_AREA024_PTR_7AA8 + 2u;
    i = 0u;

    do {
      primitive = (volatile LINE_F4*)BOF3_WORLD00_AREA024_PRIMITIVE_PTR;
      func_8017aae8((void*)primitive);
      SetSemiTrans((void*)primitive, 1);
      primitive->r0 = arg0;
      primitive->g0 = arg0;
      primitive->b0 = arg0;

      vertices[0].vx = *(volatile s16*)(src + 0x00);
      vertices[0].vy = *(volatile s16*)(src + 0x02);
      vertices[0].vz = *(volatile s16*)(src + 0x04);
      vertices[1].vx = *(volatile s16*)(src + 0x06);
      vertices[1].vy = *(volatile s16*)(src + 0x08);
      vertices[1].vz = *(volatile s16*)(src + 0x0a);
      vertices[2].vx = *(volatile s16*)(src + 0x0c);
      vertices[2].vy = *(volatile s16*)(src + 0x0e);
      vertices[2].vz = *(volatile s16*)(src + 0x10);
      vertices[3].vx = *(volatile s16*)(src + 0x12);
      vertices[3].vy = *(volatile s16*)(src + 0x14);
      vertices[3].vz = *(volatile s16*)(src + 0x16);

      RotTransPers(&vertices[0], (long*)((u8*)primitive + 0x08), &depth, &flag);
      RotTransPers(&vertices[1], (long*)((u8*)primitive + 0x0c), &depth, &flag);
      RotTransPers(&vertices[2], (long*)((u8*)primitive + 0x14), &depth, &flag);
      RotTransPers(&vertices[3], (long*)((u8*)primitive + 0x10), &depth, &flag);

      if ((func_801f4158((const s16*)((u8*)primitive + 0x08),
                         (const s16*)((u8*)primitive + 0x0c),
                         (const s16*)((u8*)primitive + 0x10)) > 0) ||
          (func_801f4158((const s16*)((u8*)primitive + 0x10),
                         (const s16*)((u8*)primitive + 0x14),
                         (const s16*)((u8*)primitive + 0x08)) > 0)) {
        func_8014e5a0(1u, 0x1cu);
      }

      src += 0x18u;
      i += 1u;
    } while (i < count);
  }

  PopMatrix();
  *(volatile u8**)0x1f800044u = saved_scratch;

  return 0;
}
