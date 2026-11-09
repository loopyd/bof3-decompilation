#include "bof3/bof3.h"

/* Shared frame counter in main RAM, outside this overlay's image. The shared
 * map owns the address (config/targets/shared/symbols.txt) and the sibling
 * frame-parity lift copyWorkCursorPairWithFrameParityDeltaThenCountdownWorkByte9
 * ResetTo8AndDecrementWorkByte2BmagicMagic22003_801EF40C reads the same word,
 * its low bit as a per-frame phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Scratchpad work-object pointer cell. The original re-materialises the cell
 * with its own lui/lw pair at each read, so the cell is named rather than
 * written as a bare scratchpad constant: a constant address folds into one
 * CSE'd register plus offset loads, while the named symbol keeps one lui 0x1F80
 * plus lw 0x44(reg) per read. The cell is not volatile, because every pair of
 * reads is separated by a store through the loaded pointer or by a call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

void func_800AF3A0(s32 x, s32 y, s32 z, s32 depth, s32 size);
void func_800AF764(void);
void drawRotatedTexturedQuadAtWorkScreenPairFromWorkByteBRowTintedByWorkByte9AndPhasedByWorkByteABmagicMagic01200_801EF79C(u8 index, s32 mode);

/* @source 0x801EF6A4
 * @behavior Work-byte-2 handler, entry 12 of this overlay's thirteen-entry
 *           dispatch table at 0x801EFBB0 (the payload word at 0x801EFBE0), so
 *           work-object byte 0x2 holds 12 while this body runs. The
 *           scratchpad work-object pointer cell at 0x1F800044 names the work
 *           object and the shared frame counter at 0x80143E6C supplies the
 *           phase. On a set low bit of that counter the handler advances the
 *           work object's byte 0xA, so that byte counts frames in pairs; the
 *           count is read and stored through one pointer-cell load. It then
 *           reads the object's projected point words 0xC, 0x10 and 0x14 with a
 *           single pointer-cell load into a local four-word point and passes
 *           the four words to the shared helper at 0x800AF3A0 together with the
 *           size word 0x30. The fourth component is never assigned by the
 *           original, which is why its argument load reads the point slot that
 *           follows the three written words (lw 0x24($sp)): the value is dead
 *           stack content, and the helper's use of it is unknown. The handler
 *           then counts the object's byte 0x9 down by one through one
 *           pointer-cell load (lbu/addiu/sb) and, when the truncation to a byte
 *           is zero, re-reads the cell to decrement the work-object phase byte
 *           0x2, which steps the object one phase back. Finally it calls the
 *           shared helper at 0x800AF764 and then passes the object's byte 0xB
 *           and the mode word 0 to the overlay's sprite emitter at 0x801EF79C,
 *           which builds a SetPolyFT4 sprite from the object's cursor and
 *           palette bytes. The function takes no arguments, returns nothing and
 *           reads or writes no other state; the 0x30-byte frame holds the point
 *           and keeps $ra across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByteAOnFrameParityInvokePointHelperCountdownWorkByte9DecrementWorkByte2AndDrawSpriteBmagicMagic01200_801EF6A4(void) {
  s32 point[4];
  u8 *countdown;
  u8 count;

  if (D_80143E6C & 1u) {
    g_battle_work[0xA]++;
  }

  point[0] = *(s32 *)(g_battle_work + 0xC);
  point[1] = *(s32 *)(g_battle_work + 0x10);
  point[2] = *(s32 *)(g_battle_work + 0x14);
  func_800AF3A0(point[0], point[1], point[2], point[3], 0x30);

  countdown = g_battle_work;
  count = (u8)(countdown[9] - 1);
  countdown[9] = count;
  if (count == 0) {
    g_battle_work[2]--;
  }

  func_800AF764();
  drawRotatedTexturedQuadAtWorkScreenPairFromWorkByteBRowTintedByWorkByte9AndPhasedByWorkByteABmagicMagic01200_801EF79C(g_battle_work[0xB], 0);
}
