#include "bof3/bof3.h"

/* Shared map-cell queries and primitive linker of the main executable. */
/* @source 0x80166CB0 */
extern u8 func_80166CB0(s16 x, s16 y);
/* @source 0x801558F0 */
extern s32 func_801558F0(s32 x, s32 y);
/* @source 0x8017A9A4 */
extern void func_8017A9A4(void* primitive);
/* @source 0x80155A08 */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size);

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */
extern u8 D_80143D44; /* @source 0x80143D44 @kind data */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 D_8012D888[]; /* @source 0x8012D888 @kind table */
extern u8 D_8012D88A[]; /* @source 0x8012D88A @kind table */
extern u8 D_8012D890[]; /* @source 0x8012D890 @kind table */
extern u8 D_8012D892[]; /* @source 0x8012D892 @kind table */
extern u8 D_8012D898[]; /* @source 0x8012D898 @kind table */
extern u8 D_8012D89A[]; /* @source 0x8012D89A @kind table */
extern u8 D_8012D8A0[]; /* @source 0x8012D8A0 @kind table */
extern u8 D_8012D8A2[]; /* @source 0x8012D8A2 @kind table */

/* @source 0x801F37D4
 * @behavior Visits the four-by-four marker block x=0x94..0x97, y=0x20..0x23.
 *           For cells whose map code is 0x50 and whose resolved record is
 *           nonzero, initializes a semitransparent quad at the shared primitive
 *           cursor. Copies its four vertex pairs from the resolved record's
 *           0x50-byte entry, selecting offset 0x28 when the shared buffer byte
 *           is zero and offset zero otherwise. Sets all three color bytes to
 *           31 times abs(((shared frame counter >> 1) & 15) - 8), then links
 *           the primitive at the cell's packed coordinates with kind 0 and
 *           size 0x18. Takes no arguments and returns nothing.
 * @status partial
 * @match 51.08
 * @residual 71 of 139 instructions match; 552 bytes against 556 original.
 *           The first mismatch is the saved row-counter register. Row setup,
 *           stride lifetime and primitive-register allocation also differ.
 */
void drawPulsingQuadsForCode50MarkerBlockWorld01Area04013_801F37D4(void) {
  u8 y;
  u8 x;
  s32 record;
  POLY_F4* primitive;
  s32 pulse;
  s32 packedY;
  s32 offset;
  u8 buffer;
  s32 cellX;
  s32 cellY;

  for (y = 0; y < 4; y++) {
    packedY = (y + 0x20) << 16;
    cellY = y + 0x20;
    for (x = 0; x < 4; x++) {
      if ((u8)func_80166CB0(x + 0x94, packedY >> 16) == 0x50) {
        cellX = x + 0x94;
        record = func_801558F0(cellX, cellY);
        if (record != 0) {
          primitive = (POLY_F4*)g_PrimCursor;
          func_8017A9A4(primitive);
          SetSemiTrans(primitive, 1);
          record *= 0x50;
          buffer = D_80143D44;
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->x0 = *(u16*)(D_8012D888 + offset);
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->y0 = *(u16*)(D_8012D88A + offset);
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->x1 = *(u16*)(D_8012D890 + offset);
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->y1 = *(u16*)(D_8012D892 + offset);
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->x2 = *(u16*)(D_8012D898 + offset);
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->y2 = *(u16*)(D_8012D89A + offset);
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->x3 = *(u16*)(D_8012D8A0 + offset);
          offset = record;
          if (!buffer) {
            offset += 0x28;
          }
          primitive->y3 = *(u16*)(D_8012D8A2 + offset);
          pulse = ((D_80143E6C >> 1) & 15) - 8;
          if (pulse < 0) {
            pulse = -pulse;
          }
          primitive->r0 = primitive->g0 = primitive->b0 = pulse * 31;
          func_80155A08(cellX << 16, cellY << 16, 0, 0x18);
        }
      }
    }
  }
}
