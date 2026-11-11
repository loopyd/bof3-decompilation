#include "bof3/bof3.h"

/* Four-byte entry of this payload's sprite-layer table at 0x801F4DEC: the
 * leading halfword is the layer mask compared with the shared mask words and
 * the byte at 0x02 selects the sprite record drawn for that layer. The six
 * entries at 0x801F4DEC (masks 0x10, 0x80, 0x20, 0x40, 0x08, 0x04 selecting
 * records 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x12) are the ones the first two layers
 * scan; the eight-entry scans below run past them into the code-pointer run at
 * 0x801F4E04 (0x801F407C, 0x801F41B8, 0x801F43E8), exactly as the original
 * bounds do. */
typedef struct World04Area15213LayerEntry {
  u16 mask;
  u8 field_02;
  u8 field_03;
} World04Area15213LayerEntry;

/* Shared mask word tested against the six-entry first and second layer scans. */
extern u16 D_80145AB4;

/* Shared mask word tested against the eight-entry third layer scan. */
extern u16 D_80145AC0;

/* Shared halfword whose bit 0x4000 selects the fourth layer. */
extern u16 D_80146258;

/* Shared byte whose low seven bits select the fourth layer at 0x0C. */
extern u8 D_80145024;

/* Shared byte whose bits 0x1B gate the whole drawer. */
extern u8 D_8014832E;

/* Primitive cursor the drawing mode is opened on.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Sprite-layer table carrying the masks and record indices drawn below. */
extern const World04Area15213LayerEntry D_801F4DEC[];

/* Local sprite emitter this target's unlifted 0x801F389C span provides: it
 * narrows both position arguments with `sll`/`sra 16`, takes the record index
 * as a byte and consumes no result here. */
void func_801F389C(s16 arg0, s16 arg1, u8 arg2);

/* Local matrix-rotated patch this target's unlifted 0x801F3D90 span provides,
 * appended after the last layer from the same narrowed signed pair. */
void func_801F3D90(s16 arg0, s16 arg1);

/* Resident map probe returning the map code under the given coordinates. */
s32 func_80166CB0(s16 arg0, s16 arg1);

/* Resident gate probe tested before the fourth layer. */
u8 func_801B6610(s16 arg0, s16 arg1);

/* Resident primitive append helper reserving the given size. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* Resident graph-type selector. */
s32 GetGraphType(void);

/* @source 0x801F39C4
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
 *           at 0x801F4DEC, comparing its mask halfword with the shared mask word
 *           0x80145AB4 for the first pair and indexing its byte at offset 0x02.
 *           A fourth layer 0x30 to the right and 0x18 below is drawn through the
 *           local emitter at 0x801F389C when the map probe still reads zero, or
 *           any of bit 0x1000 of 0x8014625A, the masked byte 0x80145024 equal to
 *           0x0C or bit 0x4000 of 0x80146258 is set, and the local
 *           matrix-rotated patch at 0x801F3D90 is always appended from 0x18 to
 *           the right and 0x18 below. The 0x3CC bytes differ from the exact
 *           drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld03Area15113_801F3A70
 *           in emi/world03/area151/13 in twenty-three absolute words only: the
 *           fifteen j/jal target words each shift by exactly the 0xAC offset
 *           between the two spans, and the eight layer-table immediates carry
 *           this payload's own table at 0x801F4DEC, so this body follows the
 *           reviewed sibling statement order with this target's table and
 *           callees.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld04Area15213_801F39C4(
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
  func_801F389C((s16)arg0, (s16)arg1, 0);

  mapCode = func_80166CB0(PSX_REF(s16, 0x8014930au),
                          PSX_REF(s16, 0x8014930eu));
  if (((u32)((mapCode + 0x60) & 0xff) >= 2) && ((mapCode & 0xff) != 0xae) &&
      !(PSX_REF(u16, 0x8014625au) & 0x1000)) {
    func_801F389C((s16)(arg0 + 0x30), (s16)arg1, 1);
    for (i = 0; i < 6; i++) {
      if (D_80145AB4 & (D_801F4DEC[i].mask)) {
        func_801F389C((s16)(arg0 + 0x38), (s16)(arg1 + 0x8),
                      (u8)(D_801F4DEC[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 6; i++) {
      if (D_80145AB4 & (D_801F4DEC[i].mask)) {
        func_801F389C((s16)(arg0 + 0x38), (s16)(arg1 + 0x8),
                      D_801F4DEC[i].field_02);
        break;
      }
    }
  }

  if ((u32)((mapCode + 0x60) & 0xff) >= 2) {
    func_801F389C((s16)(arg0 + 0x30), (s16)(arg1 + 0x10), 2);
    for (i = 0; i < 8; i++) {
      if (D_80145AC0 & (D_801F4DEC[i].mask)) {
        func_801F389C((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                      (u8)(D_801F4DEC[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 8; i++) {
      if (D_80145AC0 & (D_801F4DEC[i].mask)) {
        func_801F389C((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                      D_801F4DEC[i].field_02);
        break;
      }
    }
  }

  if ((func_801B6610(PSX_REF(s16, 0x8014930au),
                     PSX_REF(s16, 0x8014930eu)) & 0xff) ||
      (PSX_REF(u16, 0x8014625au) & 0x1000) ||
      ((PSX_REF(u8, 0x80145024u) & 0x7f) == 0x0c) ||
      (D_80146258 & 0x4000)) {
    func_801F389C((s16)(arg0 + 0x30), (s16)(arg1 + 0x18), 3);
  }
  func_801F3D90((s16)(arg0 + 0x18), (s16)(arg1 + 0x18));
}
