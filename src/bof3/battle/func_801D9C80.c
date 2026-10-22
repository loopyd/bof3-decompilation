#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9C80
 * @behavior Emits one colored tile primitive at the current primitive cursor:
 * the tile dimensions come from the 2-entry row of D_801EAE50 selected by the
 * low byte of arg2, the red/green/blue channels are packed from the palette
 * entry selected by the CLUT bank byte D_80144952 and the low byte of arg3, and
 * the primitive is marked semi-transparent before the 0x10-mode close.
 * @status partial
 * @match 83.54
 * @residual Live asm diff 66/79 instructions, 316 versus 316 bytes, with the
 * frame, the instruction count and the whole body reproduced. The palette reads
 * above go through the fixed-address byte view, and that is what makes the
 * assembler emit the original's addu $at,<offset>,$at operand order instead of
 * the named-symbol order: it is the only change since 63/79 and it cleared all
 * three operand-order instructions. Two classes remain. First mismatch +0x10 is
 * the saved-register assignment: the original holds arg0 in s3, arg1 in s4 and
 * the masked arg3 in s2, this build holds arg0 in s2, arg1 in s3 and arg3 in
 * s4, which shifts the prologue stores, the two dimension stores and the andi.
 * That accounts for 12 instructions and is an allocator result with no source
 * handle left; every measured family (index local, colour local, offset local,
 * element or byte view, split or single scale statement) leaves it unchanged. The last
 * instruction is the first palette read narrowing to lbu where the original
 * keeps lhu: that pairing is common to every measured spelling here, including
 * the named-symbol and byte-offset forms, and to func_801D9AB4 in this same
 * module, so it is a compiler narrowing of (load & 0x1f) << 3 rather than an
 * address-representation effect. Rungs 4 (compiler profile) and 5 (permuter)
 * are untried and outside this lane.
 */
void func_801D9C80(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
  TILE* tile;
  s8* paletteBank;
  s32 palette_offset;

  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  tile->x0 = arg0;
  tile->y0 = arg1;
  tile->w = D_801EAE50[(u8)arg2][0];
  tile->h = D_801EAE50[(u8)arg2][1];
  paletteBank = &D_80144952;
  palette_offset = (((*paletteBank) * 2) + (u8)arg3) * 32;
  tile->r0 = (*(u16 *)(PSX_PTR(u8, 0x80033A08u) + palette_offset) & 0x1f) << 3;
  palette_offset = (((*paletteBank) * 2) + (u8)arg3) * 32;
  tile->g0 = (*(u16 *)(PSX_PTR(u8, 0x80033A08u) + palette_offset) >> 2) & 0xf8;
  palette_offset = (((*paletteBank) * 2) + (u8)arg3) * 32;
  tile->b0 = (*(u16 *)(PSX_PTR(u8, 0x80033A08u) + palette_offset) >> 7) & 0xf8;

  SetSemiTrans(tile, (u8)arg3);
  func_8014E5A0(1, 0x10);
}
