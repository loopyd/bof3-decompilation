#include "bof3/bof3.h"

/* Local sprite-layer table in this target's tail blob: four-byte entries whose
 * first halfword is the layer mask compared with the shared mask words and
 * whose byte at 0x02 selects the sprite record used for the drawn layer. */
typedef struct World02Area08813MarkerEntry {
  u16 mask;
  u8  field_02;
  u8  field_03;
} World02Area08813MarkerEntry;

/* Shared mask word tested against the two six-entry layer pairs.
 * @source 0x80145AB4 @kind unknown
 */
extern u16 D_80145AB4;

/* Shared mask word tested against the eight-entry third layer.
 * @source 0x80145AC0 @kind unknown
 */
extern u16 D_80145AC0;

/* Shared gate halfword whose bit 0x4000 selects the fourth layer.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Shared byte whose bits 0x1B gate the whole drawer.
 * @source 0x8014832E @kind unknown
 */
extern u8 D_8014832E;

/* Primitive cursor the drawing mode is opened on.
 * @source 0x8014598C @kind bss
 */
extern u8* g_PrimCursor;

/* Sprite-layer table carrying the masks and record indices drawn below.
 * @source 0x801F6234 @kind table
 */
extern const World02Area08813MarkerEntry D_801F6234[];

/* Local drawer that emits one semi-transparent sprite layer from the indexed
 * table entry its third argument selects. */
void func_801F453C(s16 arg0, s16 arg1, u8 arg2);

/* Local panel the last layer is followed by. */
void appendTransformedG4PanelWorld02Area08813_801F4A30(s16 arg0, s16 arg1);

/* Resident map probe returning the map byte under the given coordinates. */
s32 func_80166CB0(s16 arg0, s16 arg1);

/* Resident gate probe the fourth layer is tested against. */
u8 func_801B6610(s16 arg0, s16 arg1);

/* Primitive append helper that reserves the given size on the cursor. */
void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801F4664
 * @behavior Returns immediately unless the shared byte 0x8014832E has any of
 *           the bits 0x1B set. Then picks tpage 0x22C when the graph type reads
 *           1 or 2 and 0x9C otherwise, opens that drawing mode on the primitive
 *           cursor, reserves twelve bytes through the local append helper and
 *           draws the first sprite layer at the two signed arguments. Reads the
 *           map byte under the shared probe coordinates 0x8014930A and
 *           0x8014930E. Unless that map byte is 0xA0, 0xA1 or 0xAE, or the
 *           shared flag halfword 0x8014625A has bit 0x1000 set, it also draws a
 *           second layer 0x30 to the right and takes the incremented record
 *           index, while those three exceptional cases skip that layer and take
 *           the plain record index. The same split repeats for a third layer
 *           0x10 below, tested against the shared mask 0x80145AC0 over eight
 *           candidate entries. All three layers walk the local four-byte table
 *           at 0x801F6234, comparing its mask halfword with the shared mask word
 *           0x80145AB4 for the first pair and indexing the record byte at offset
 *           0x02. A fourth layer 0x30 to the right and 0x18 below is drawn when
 *           the map probe still reads zero, or any of bit 0x1000 of 0x8014625A,
 *           the masked byte 0x80145024 equal to 0x0C or bit 0x4000 of
 *           0x80146258 is set, and the local panel 0x801F4A30 is always appended
 *           from 0x18 to the right and 0x18 below. Takes two signed halfword
 *           coordinates and returns nothing. Its 0x3CC original bytes differ
 *           from the exact
 *           drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld02Area08713_801F3B34
 *           lift in emi/world02/area087/13 in 23 of 243 words and no earlier
 *           word: the nine internal `jal` words of the local emitter
 *           0x801F453C and the local panel 0x801F4A30 and the six relative `j`
 *           words of that body are each higher here by the 0x2CC the two spans
 *           sit apart in words, and the eight table immediates are each higher
 *           by the 0xA6C between the two local layer tables, so the two spans are
 *           one body and this source follows that sibling's statement order with
 *           this target's own emitter, panel and table bindings.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawMarkerSpriteLayersSelectedByMapCodeThenAppendPanelWorld02Area08813_801F4664(
    s16 arg0, s16 arg1) {
  s32 marker;
  s32 i;

  if (!(D_8014832E & 0x1b)) {
    return;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0,
              GetGraphType() == 1 ? 0x22c
                                  : (GetGraphType() == 2 ? 0x22c : 0x9c),
              0);
  func_8014E5A0(1, 0x0c);
  func_801F453C((s16)arg0, (s16)arg1, 0);

  marker = func_80166CB0(PSX_REF(s16, 0x8014930au),
                         PSX_REF(s16, 0x8014930eu));
  if (((u32)((marker + 0x60) & 0xff) >= 2) && ((marker & 0xff) != 0xae) &&
      !(PSX_REF(u16, 0x8014625au) & 0x1000)) {
    func_801F453C((s16)(arg0 + 0x30), (s16)arg1, 1);
    for (i = 0; i < 6; i++) {
      if (D_80145AB4 & (D_801F6234[i].mask)) {
        func_801F453C((s16)(arg0 + 0x38), (s16)(arg1 + 0x8),
                      (u8)(D_801F6234[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 6; i++) {
      if (D_80145AB4 & (D_801F6234[i].mask)) {
        func_801F453C((s16)(arg0 + 0x38), (s16)(arg1 + 0x8),
                      D_801F6234[i].field_02);
        break;
      }
    }
  }

  if ((u32)((marker + 0x60) & 0xff) >= 2) {
    func_801F453C((s16)(arg0 + 0x30), (s16)(arg1 + 0x10), 2);
    for (i = 0; i < 8; i++) {
      if (D_80145AC0 & (D_801F6234[i].mask)) {
        func_801F453C((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                      (u8)(D_801F6234[i].field_02 + 1));
        break;
      }
    }
  } else {
    for (i = 0; i < 8; i++) {
      if (D_80145AC0 & (D_801F6234[i].mask)) {
        func_801F453C((s16)(arg0 + 0x38), (s16)(arg1 + 0x10),
                      D_801F6234[i].field_02);
        break;
      }
    }
  }

  if ((func_801B6610(PSX_REF(s16, 0x8014930au),
                     PSX_REF(s16, 0x8014930eu)) & 0xff) ||
      (PSX_REF(u16, 0x8014625au) & 0x1000) ||
      ((PSX_REF(u8, 0x80145024u) & 0x7f) == 0x0c) ||
      (D_80146258 & 0x4000)) {
    func_801F453C((s16)(arg0 + 0x30), (s16)(arg1 + 0x18), 3);
  }
  func_801F4A30((s16)(arg0 + 0x18), (s16)(arg1 + 0x18));
}
