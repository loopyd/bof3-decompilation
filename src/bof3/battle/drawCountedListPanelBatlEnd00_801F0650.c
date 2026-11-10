#include "bof3/bof3.h"

extern u8 *D_80148648;
extern u8 D_80146327; /* @source 0x80146327 @kind data */
extern u8 D_80146354[]; /* @source 0x80146354 @kind data */

/* The overlay panel frame drawer at 0x801F039C: sets the draw mode and the
 * tile for a panel window and draws its border. Its own tile stores place the
 * first argument at tile +0x08, the second plus two at +0x0A, the third at
 * +0x0C and the fourth at +0x0E, so the call passes the panel origin, its
 * width and its height. Still an address-anchored asm span, so it is declared
 * here for its address. */
extern void func_801F039C(s16 x, s16 y, s32 width, u8 height);

/* The overlay per-entry drawer at 0x801F0A74: hands the two halves of the id
 * to the shared equipment record base 0x80165D48 (item type and item index),
 * draws the entry icon with the palette byte the overlay table at 0x801F0CD4
 * holds for it together with the entry name, and prints the per-id count
 * through the overlay format 0x801EEC50 ("*%2d") when the count is above one.
 * Still an address-anchored asm span, so it is declared here for its address. */
extern void func_801F0A74(s16 x, s16 y, s32 flags, u32 item_type,
                          u32 item_index, u32 count, u32 arg6);

/* @source 0x801F0650
 * @behavior Battle-end panel task handler 1 of the overlay handler run at
 *   0x801EEC28: draws the counted-list panel by calling the overlay frame
 *   drawer func_801F039C with the signed halfwords +0x04 and +0x06 of the
 *   object published at 0x80148648, the width 0x118 and byte +0x09 as the
 *   height, then walks the counted list (length byte at 0x80146327, halfword
 *   ids at 0x80146334, per-id counts at 0x80146354) and draws every non-zero
 *   id through func_801F0A74 in two columns: column (index & 1) steps 138
 *   pixels with a 5-pixel inset, row (index >> 1) steps 13 pixels with a
 *   4-pixel inset, and the call passes the id split into its high byte and its
 *   low byte plus the per-id count. The length byte is re-read every iteration
 *   and a zero id is skipped without drawing. The 0x30-byte frame holds the
 *   three stacked arguments of that call at 0x10/0x14/0x18($sp) and $ra at
 *   0x28($sp). Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawCountedListPanelBatlEnd00_801F0650(void) {
  u8 *count;
  u16 *ids;
  u16 id;
  u8 i;
  s32 x_inset;
  s32 y_inset;

  func_801F039C(*(s16 *)((u8 *)D_80148648 + 4),
                *(s16 *)((u8 *)D_80148648 + 6), 0x118, ((u8 *)D_80148648)[9]);

  count = &D_80146327;
  if (*count == 0) {
    return;
  }

  i = 0;
  ids = (u16 *)(count + 0xd);
  do {
    id = ids[i];
    if (id != 0) {
      x_inset = (i & 1) * 138 + 5;
      y_inset = (i >> 1) * 13 + 4;
      func_801F0A74((s16)(*(u16 *)((u8 *)D_80148648 + 4) + x_inset),
                    (s16)(*(u16 *)((u8 *)D_80148648 + 6) + y_inset), 0,
                    id >> 8, id & 0xff,
                    D_80146354[i], 0);
    }
    i += 1;
  } while (i < *((u8 *)ids - 0xd));
}
