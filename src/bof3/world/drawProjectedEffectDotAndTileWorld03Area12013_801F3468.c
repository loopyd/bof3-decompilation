#include "bof3/bof3.h"

extern int rand(void);

/* Shared primitive cursor of the main executable: every packet of this emitter
 * is built at the cursor it publishes.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* One 0x1C-byte entry of the 0x100-entry effect table at 0x800E4800, the work
 * area this overlay shares with the world and scenario overlays. This emitter
 * reads only the position word pair at +0x0C/+0x10, so the layout mirrors the
 * target-local declaration in
 * advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0.c, the
 * sweep that owns the table and passes one entry pointer here. */
typedef struct Area12013EffectRecord {
  u8 flags_00;
  u8 timer_01;
  u8 state_02;
  u8 dir_03;
  s32 speed_04;
  u16 angle_08;
  u8 pad_0a[2];
  s32 pos_x_0c;
  s32 pos_y_10;
  s32 vel_14;
  u8 pad_18[4];
} Area12013EffectRecord;

/* One eight-byte entry of the address-taken local projection workspace: the
 * shared projector writes the x/y halfword pair at +0x00/+0x02 and this emitter
 * reads only that pair of the first entry. The eight-byte stride is the one the
 * sibling projection workspaces prove; the seven-entry count is what reproduces
 * the original 0x60-byte frame beside the 0x18-byte outgoing-argument area the
 * five-argument draw-mode call forces and the saved $s0/$s1/$ra. */
typedef struct Area12013ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} Area12013ScreenPoint;

/* 0x80155A08 - shared grid-cell primitive linker of the main executable: it
 * resolves the passed position word pair through the shared cell-size pair,
 * links the packet at the primitive cursor with the passed kind and advances the
 * cursor by the passed size. */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* 0x801AFF04 - shared projector of the main executable: it projects the
 * three-word point it is given into the x/y halfword pair of the passed screen
 * entry. */
extern void func_801AFF04(const void* arg0, void* arg1);

/* 0x8017AA30 - the libgpu entry between SetSprt (0x8017AA1C) and SetTile
 * (0x8017AA6C) that stamps the fixed 0x0C-byte one-pixel tile packet (TILE_1)
 * at the passed cursor. The SDK map carries no row for it, so it keeps its
 * address-anchored name, and the lifted sibling overlays declare the same
 * single cursor argument. */
extern void func_8017AA30(TILE_1* arg0);

/* @source 0x801F3468
 * @behavior Per-record emitter of the effect sweep at 0x801F36B0, which calls it
 *           once for every live entry of the 0x1C-byte table at 0x800E4800. It
 *           first writes the draw mode at the shared primitive cursor from
 *           texture page 0x380/0x100 with both flags clear and no texture
 *           window, links that 0x0C-byte packet through func_80155A08 with kind
 *           0, then projects the entry position pair at +0x0C/+0x10 into a local
 *           screen point. The first pass stamps the fixed one-pixel tile at the
 *           cursor, tags it translucent, places it on the projected point and
 *           stores 0x80 as its red and green components and the random low bit
 *           shifted left seven as its blue component, and links it at 0x0C
 *           bytes. The second pass stamps the size-carrying tile at the cursor,
 *           tags it translucent, places its corner one pixel up and left of the
 *           projected point, sets both its width and height to 3, stores 0x20 as
 *           its red and green components and the random low bit shifted left
 *           five as its blue, and links it at 0x10 bytes. Both links pass kind 0
 *           and the entry's own position pair. Takes the entry pointer and
 *           returns nothing; its 0x60-byte frame holds the projection workspace
 *           at $sp+0x18, the stack argument of the draw-mode call at $sp+0x10
 *           and saves $s1, $s0 and $ra. Each pass keeps its blue component in a
 *           one-byte local before storing it, which is what orders the random
 *           shift ahead of the next link's argument setup.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawProjectedEffectDotAndTileWorld03Area12013_801F3468(
    Area12013EffectRecord* record) {
  Area12013ScreenPoint screen[7];
  TILE_1* dot;
  TILE* tile;
  u8 blue;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x380, 0x100), 0);
  func_80155A08(record->pos_x_0c, record->pos_y_10, 0, 0xC);

  func_801AFF04(&record->pos_x_0c, &screen[0]);

  dot = (TILE_1*)g_PrimCursor;
  func_8017AA30(dot);
  SetSemiTrans(dot, 1);
  dot->x0 = screen[0].x;
  dot->y0 = screen[0].y;
  dot->g0 = 0x80;
  dot->r0 = 0x80;
  blue = (rand() & 1) << 7;
  dot->b0 = blue;
  func_80155A08(record->pos_x_0c, record->pos_y_10, 0, 0xC);

  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  SetSemiTrans(tile, 1);
  tile->x0 = screen[0].x - 1;
  tile->y0 = screen[0].y - 1;
  tile->w = 3;
  tile->h = 3;
  tile->g0 = 0x20;
  tile->r0 = 0x20;
  blue = (rand() & 1) << 5;
  tile->b0 = blue;
  func_80155A08(record->pos_x_0c, record->pos_y_10, 0, 0x10);
}
