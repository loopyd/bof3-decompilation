#include "bof3/battle/battle03_internal.h"

/* @source 0x801D9DBC
 * @behavior Emits one colored tile primitive at the current primitive cursor:
 * the tile dimensions come from the 2-entry row of D_801EAE50 selected by the
 * low byte of arg2, the colour bytes take the three 5-bit channels of the
 * 15-bit value in arg3 (bits 10-14 at +0x04, bits 5-9 at +0x05 and bits 0-4 at
 * +0x06), the primitive is marked semi-transparent with the arg4 byte and the
 * packet is closed with the 0x10-byte primitive-append helper.
 * @status partial
 * @match 96.43
 * @residual Live asm diff 54/56 instructions, 224 versus 224 bytes. The whole
 * body, the frame and the epilogue are reproduced; the only difference is the
 * entry-block order of the four saved-argument copies, which the original emits
 * as arg3/arg0/arg1/arg2 (sw s1/move s1,a3, sw s4/move s4,a0, sw s3/move s3,a1,
 * sw s0/move s0,a2) while this build emits arg2 first
 * (sw s0/move s0,a2 before those three). The sched2 dump of this candidate
 * gives every one of those copies priority 1 in the same basic block, so the
 * order is an equal-priority ready-list tie; the u16 `color` temp is needed for
 * the in-body `move v0,s1`/`andi v0,v0,0xffff` pair and for the arg3 / arg2
 * reference counts the copies carry. Twelve clean-C shapes were measured
 * (inline casts, u16 and s32 colour temps, declared initialisers, nested block,
 * TILE* versus u8* views, index/semi locals, explicit store casts) and every one
 * of them reproduces this same 54/56 entry ordering.
 */
void drawColorTilePrim(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4) {
  TILE* tile;
  u16 color;

  color = arg3;
  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  tile->x0 = arg0;
  tile->y0 = arg1;
  tile->w = D_801EAE50[(u8)arg2][0];
  tile->h = D_801EAE50[(u8)arg2][1];
  tile->r0 = (color >> 7) & 0xF8;
  tile->g0 = (color >> 2) & 0xF8;
  tile->b0 = (arg3 & 0x1F) << 3;
  SetSemiTrans(tile, arg4);
  func_8014E5A0(1, 0x10);
}
