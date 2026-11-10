#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Texture-state filler of the main executable: it writes the texture window of
 * the primitive at the shared cursor from the packed state word in the first
 * argument and the kind in the third. */
/* @source 0x80155560 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* Grid-record lookup of the main executable: the cell column and row return the
 * record the shared grid holds there, or 0 when the cell is empty. */
/* @source 0x801559AC */
extern void* func_801559AC(s32 arg0, s32 arg1);

/* Grid-cell primitive linker of the main executable: it links the primitive at
 * the shared cursor with the given kind and advances the cursor by the given
 * size. */
/* @source 0x80155A08 */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* The two layer push cursors, one byte per layer, holding the record slot each
 * layer writes its next sample into. */
/* @source 0x801F44B4 @kind bss */
extern u8 D_801F44B4[];

/* The 0x98-byte-stride layer state records of the shared overlay state: word 0
 * is the layer clock and the halfword at 6 its phase source. Only the words this
 * body reads are typed. */
/* @source 0x801468BC @kind data */
extern u8 D_801468BC[];

/* The 0x20-byte-stride sample slots of the trail buffer, three bytes per layer
 * at +0, +1 and +2: the sample column, the sample row and the flip flag. */
/* @source 0x801F54D0 @kind data */
extern u8 D_801F54D0[];
/* @source 0x801F54D1 @kind data */
extern u8 D_801F54D1[];
/* @source 0x801F54D2 @kind data */
extern u8 D_801F54D2[];

/* The per-layer, per-quad low-byte offsets of the packed texture state word, one
 * pair of bytes per layer. */
/* @source 0x801F4544 @kind data */
extern u8 D_801F4544[];
/* @source 0x801F4545 @kind data */
extern u8 D_801F4545[];
/* The per-layer row step of the companion quad, +1 on the first layer and -1 on
 * the second. */
/* @source 0x801F4548 @kind data */
extern s8 D_801F4548[];

/* @source 0x801F3398
 * @behavior Two-layer sample trail of textured quads. For each of the two layers
 *           it advances the layer's record in the 0x98-byte-stride overlay state
 *           and, when the integer part of that record's fixed-point clock has
 *           moved past the phase byte held in the layer's current slot, steps the
 *           layer's push cursor through the thirty-two slots of the trail buffer,
 *           writing the new phase, the record's halfword at +6 as the sample row
 *           and the toggled flip flag into the new slot. It then walks all
 *           thirty-two slots of the layer and, for each slot whose cell the
 *           shared grid lookup returns a record for, stamps the shared primitive
 *           cursor with SetPolyFT4 and SetShadeTex(kind 1), copies the record's
 *           four projected position words from +8, +0x10, +0x18 and +0x20, fills
 *           the texture window from the packed state word 0x25808000 plus the
 *           slot's flip flag and the layer's own byte offset, and links the
 *           0x28-byte packet through func_80155A08 with both sample bytes shifted
 *           left by 16. The same record is then looked up once more one row
 *           further along the layer's row step and, when present, emits a second
 *           quad with the second byte offset of the layer. Takes no arguments and
 *           returns no value.
 * @status partial
 * @match 90.52
 * @residual Allocation only: the body reproduces every operation, both loops,
 *           both quad blocks and the sample push, but GCC 2.7.2 spills the two
 *           loop values the other way round and keeps the push cursor
 *           rematerialised. The 0x98-byte state offset is a source marching
 *           value here, so it owns frame slot 0x18 and 2*layer takes 0x20, while
 *           the original holds 2*layer in 0x18 and the state offset in 0x20, and
 *           the original keeps the cursor pointer in $fp instead of recomputing
 *           D_801F44B4 + layer at each use. The phase byte read then
 *           materialises the D_801F54D0 base into a register where the original
 *           uses a %hi/%lo pair. Measured shapes that move the two spills,
 *           including explicit 2*layer and 3*layer marching values, an
 *           expression-form state offset (which becomes a pointer giv and adds
 *           two instructions), a word-scaled state array, a separately bound
 *           0x801468C2 symbol and every declaration and initialisation order
 *           tried, all reduce the match, so the residual is the allocator's
 *           choice and not a missing operation.
 */
void drawTwoLayerTrailOfThirtyTwoTexturedQuadPairsAdvancedByLayerClockWorld04Area19913_801F3398(
    void) {
  u8* pushCursor;
  void* record;
  POLY_FT4* prim;
  u32 tint;
  s32 layer;
  s32 i;
  s32 nextOffset;
  s32 next;
  s32 offset;
  s32 stateOffset;
  s32 clock;

  layer = 0;
  tint = 0x25808000;
  pushCursor = D_801F44B4;
  stateOffset = 0x98;
  do {
    clock = *(s32*)(D_801468BC + stateOffset);
    if (D_801F54D0[*pushCursor * 6 + layer * 3] > ((clock + 0x4000) >> 16)) {
      next = (*pushCursor + 1) & 0x1F;
      nextOffset = layer * 3 + next * 6;
      D_801F54D0[nextOffset] = (clock + 0x4000) >> 16;
      D_801F54D1[nextOffset] = *(u16*)(D_801468BC + stateOffset + 6);
      D_801F54D2[nextOffset] = (D_801F54D2[*pushCursor * 6 + layer * 3] - 1) & 1;
      *pushCursor = next;
    }
    i = 0;
    do {
      offset = layer * 3 + i * 6;
      record = func_801559AC(D_801F54D0[offset], D_801F54D1[offset]);
      if (record != 0) {
        prim = (POLY_FT4*)g_PrimCursor;
        SetPolyFT4(prim);
        SetShadeTex(prim, 1);
        *(u32*)((u8*)prim + 8) = *(u32*)((u8*)record + 8);
        *(u32*)((u8*)prim + 0x10) = *(u32*)((u8*)record + 0x10);
        *(u32*)((u8*)prim + 0x18) = *(u32*)((u8*)record + 0x18);
        *(u32*)((u8*)prim + 0x20) = *(u32*)((u8*)record + 0x20);
        func_80155560(D_801F4544[layer * 2] + (D_801F54D2[offset] + tint), prim,
                      1);
        func_80155A08(D_801F54D0[offset] << 16, D_801F54D1[offset] << 16, 0,
                      0x28);
        record = func_801559AC(D_801F54D0[offset],
                               D_801F54D1[offset] + D_801F4548[layer]);
        if (record != 0) {
          prim = (POLY_FT4*)g_PrimCursor;
          SetPolyFT4(prim);
          SetShadeTex(prim, 1);
          *(u32*)((u8*)prim + 8) = *(u32*)((u8*)record + 8);
          *(u32*)((u8*)prim + 0x10) = *(u32*)((u8*)record + 0x10);
          *(u32*)((u8*)prim + 0x18) = *(u32*)((u8*)record + 0x18);
          *(u32*)((u8*)prim + 0x20) = *(u32*)((u8*)record + 0x20);
          func_80155560(D_801F4545[layer * 2] + (D_801F54D2[offset] + tint),
                        prim, 1);
          func_80155A08(
              D_801F54D0[offset] << 16,
              (D_801F54D1[offset] + D_801F4548[layer]) << 16, 0, 0x28);
        }
      }
      i += 1;
    } while (i < 0x20);
    pushCursor += 1;
    layer += 1;
    stateOffset += 0x98;
  } while (layer < 2);
}
