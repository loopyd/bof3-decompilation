#include "bof3/bof3.h"

/* Four-byte entry of this payload's sprite-layer table at 0x801F5308: the
 * leading halfword is the layer mask compared with the shared mask words and
 * the byte at 0x02 selects the sprite record drawn for that layer. The six
 * entries at 0x801F5308 are the ones the first two layers scan; the eight-entry
 * scans below run past them into the neighbouring tail words, exactly as the
 * original bounds do. */
typedef struct World01Area06513LayerEntry {
  u16 mask;
  u8 field_02;
  u8 field_03;
} World01Area06513LayerEntry;

/* Shared mask word tested against the six-entry first and second layer scans.
 * @source 0x80145AB4 @kind unknown
 */
extern u16 D_80145AB4;

/* Shared mask word tested against the eight-entry third layer scan.
 * @source 0x80145AC0 @kind unknown
 */
extern u16 D_80145AC0;

/* Shared halfword whose bit 0x4000 selects the fourth layer.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Shared byte whose low seven bits select the fourth layer at 0x0C.
 * @source 0x80145024 @kind unknown
 */
extern u8 D_80145024;

/* Shared byte whose bits 0x1B gate the whole drawer.
 * @source 0x8014832E @kind unknown
 */
extern u8 D_8014832E;

/* Primitive cursor the drawing mode is opened on.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Sprite-layer table carrying the masks and record indices drawn below.
 * @source 0x801F5308 @kind unknown
 */
extern const World01Area06513LayerEntry D_801F5308[];

/* Overlay sprite-record emitter this target still provides at 0x801F3A10 as an
 * asm sibling: it selects its draw mode and queues one semi-transparent SPRT
 * sized from the record its third argument indexes, so no call site here
 * consumes a result. */
void func_801F3A10(s16 arg0, s16 arg1, u8 arg2);

/* Overlay panel this target still provides at 0x801F3F04 as an asm sibling: it
 * pushes a matrix, rotates the absolute pair and appends one transformed G4
 * panel. */
void func_801F3F04(s16 arg0, s16 arg1);

/* Resident map probe returning the map code under the given coordinates. */
s32 func_80166CB0(s16 arg0, s16 arg1);

/* Resident gate probe tested before the fourth layer. */
u8 func_801B6610(s16 arg0, s16 arg1);

/* Resident primitive append helper reserving the given size. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* Resident graph-type selector. */
s32 GetGraphType(void);

/* @source 0x801F3B38
 * @behavior Returns immediately unless the shared byte 0x8014832E has any of
 *           the bits 0x1B set. Then picks tpage 0x22C when the graph type reads
 *           1 or 2 and 0x9C otherwise, opens that drawing mode on the primitive
 *           cursor, reserves twelve bytes through the resident append helper at
 *           0x8014E5A0 and draws the first sprite layer at the two signed
 *           arguments. Reads the map code under the shared probe coordinates
 *           0x8014930A and 0x8014930E. Unless that map code is 0xA0, 0xA1 or
 *           0xAE, or the shared halfword 0x8014625A has bit 0x1000 set, it also
 *           draws a second layer 0x30 to the right and takes the incremented
 *           record index, while those three exceptional cases skip that layer
 *           and take the plain record index. The same split repeats for a third
 *           layer 0x10 below, tested against the shared mask 0x80145AC0 over
 *           eight candidate entries. All three layers walk the four-byte table
 *           at 0x801F5308, comparing its mask halfword with the shared mask word
 *           0x80145AB4 for the first pair and indexing its byte at offset 0x02.
 *           A fourth layer 0x30 to the right and 0x18 below is drawn through the
 *           local emitter at 0x801F3A10 when the map probe still reads zero, or
 *           any of bit 0x1000 of 0x8014625A, the masked byte 0x80145024 equal to
 *           0x0C or bit 0x4000 of 0x80146258 is set, and the local panel at
 *           0x801F3F04 is always appended from 0x18 to the right and 0x18 below.
 *           The 0x3CC bytes differ from the exact
 *           drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld03Area15113_801F3A70
 *           span in emi/world03/area151/13 in twenty-three absolute words only:
 *           nine `jal` words carry this overlay's own emitter and panel
 *           addresses 0x801F3A10 and 0x801F3F04, six `j` words carry this span's
 *           own continuation labels shifted by the 0xC8 span delta, and eight
 *           layer-table immediates carry this payload's own table at 0x801F5308,
 *           so this body follows the reviewed sibling statement order with this
 *           target's table and callees. Both arguments are narrowed with
 *           `sll`/`sra 16` at their first use. Returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld01Area06513_801F3B38(
    s16 arg0, s16 arg1) {
  s32 mapCode;
  s32 i;

  if (!(D_8014832E & 0x1b)) {
    return;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0,
              GetGraphType() == 1 ? 0x22c
                                  : (GetGraphType() == 2 ? 0x22c : 0x9c),
              0);
  func_8014E5A0(1, 0x0c);
  func_801F3A10((s16)arg0, (s16)arg1, 0);

  mapCode = func_80166CB0(PSX_REF(s16, 0x8014930au),
                          PSX_REF(s16, 0x8014930eu));
  if (((u32)((mapCode + 0x60) & 0xff) >= 2) && ((mapCode & 0xff) != 0xae) &&
      !(PSX_REF(u16, 0x8014625au) & 0x1000)) {
    func_801F3A10((s16)(arg0 + 0x30), (s16)arg1, 1);
    for (i = 0; i < 6; i++) {
      if (D_80145AB4 & (D_801F5308[i].mask)) {
        func_801F3A10((s16)(arg0 + 0x38), (s16)(arg1 + 0x8),
                      (u8)(D_801F5308[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 6; i++) {
      if (D_80145AB4 & (D_801F5308[i].mask)) {
        func_801F3A10((s16)(arg0 + 0x38), (s16)(arg1 + 0x8),
                      D_801F5308[i].field_02);
        break;
      }
    }
  }

  if ((u32)((mapCode + 0x60) & 0xff) >= 2) {
    func_801F3A10((s16)(arg0 + 0x30), (s16)(arg1 + 0x10), 2);
    for (i = 0; i < 8; i++) {
      if (D_80145AC0 & (D_801F5308[i].mask)) {
        func_801F3A10((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                      (u8)(D_801F5308[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 8; i++) {
      if (D_80145AC0 & (D_801F5308[i].mask)) {
        func_801F3A10((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                      D_801F5308[i].field_02);
        break;
      }
    }
  }

  if ((func_801B6610(PSX_REF(s16, 0x8014930au),
                     PSX_REF(s16, 0x8014930eu)) & 0xff) ||
      (PSX_REF(u16, 0x8014625au) & 0x1000) ||
      ((PSX_REF(u8, 0x80145024u) & 0x7f) == 0x0c) ||
      (D_80146258 & 0x4000)) {
    func_801F3A10((s16)(arg0 + 0x30), (s16)(arg1 + 0x18), 3);
  }
  func_801F3F04((s16)(arg0 + 0x18), (s16)(arg1 + 0x18));
}
