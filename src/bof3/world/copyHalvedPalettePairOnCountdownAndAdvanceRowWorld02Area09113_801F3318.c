#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Declared as a named symbol so every
 * access group re-materialises the cell through lui plus an offset load, which
 * is how the original reads the record again after each byte store through it.
 * The sibling bmagic lift addWorkColorTripletBy16ThenDecrementGlobalWorkByteB
 * AndAdvanceWorkByte2BmagicMagic01500_801F1180 binds the same cell the same
 * way. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared palette-stage serial advanced once per call. The exact sibling lift
 * scalePaletteChannelsWorld02Area08213_801F5388 names this byte. */
extern u8 paletteStageSerial; /* @source 0x80145988 @kind unknown */

/* Target-local overlay fan drawing helper at 0x801F2ED4: projects a fixed
 * point into a scratch matrix, composes it under the shared screen matrix and
 * stamps thirty-two semi-transparent Gouraud wedges at the passed radius and
 * centre shade through the primitive cursor. It was still an asm subsegment of
 * this target when this caller was seeded, so only a prototype is needed. */
void drawTranslucentFanOfThirtyTwoGouraudWedgesFromProjectedOriginFadingToBlackWorld02Area09113_801F2ED4(
    s32 arg0, s32 arg1);

typedef struct {
  u16 entry[0x20];
} ClutBlock;

typedef struct {
  ClutBlock block[8];
} ClutRow;

/* @source 0x801F3318
 * @behavior Overlay palette-pair step. Reads the scratchpad work record through
 *           the pointer cell 0x1F800044, decrements its byte +0xA (lbu /
 *           addiu -1 / sb) and re-reads that byte. While it is non-zero the
 *           step publishes one mirrored pair of the 32-halfword colour row and
 *           halves every channel of both words with `(c & 0x7BDE) >> 1`: the
 *           mask clears bit 15 plus the low bit of each 5-bit channel so the
 *           following shift cannot bleed one channel into the next. When the
 *           decremented byte is zero it instead publishes the pair unchanged.
 *           The row is selected from the record byte +0x27 as
 *           `(byte >> 3) * 0x200 + (byte & 7) * 0x40`; the pair index is the
 *           record byte +0xB, and the two words are row[+0xB] and
 *           row[0x1F - +0xB]. The source row is built on 0x80033800 and the
 *           destination row on 0x80037800, the CLUT source and shadow windows
 *           the exact siblings scalePaletteChannelsWorld02Area08213_801F5388,
 *           copyPaletteBlock32World02Area07714_801F529C and the game00 lift
 *           func_801C3154 address, the last of them selecting the same
 *           0x200/0x40 block split from work byte +0x27.
 *           On the frame where the countdown just reached zero it re-arms
 *           +0xA with 2, increments +0xB, and when +0xB reaches 0x10 it arms
 *           +0xA with 0x1E and increments record byte +4. It then increments
 *           the shared palette-stage serial at 0x80145988 (lbu / addiu 1 / sb,
 *           so the cell is u8) and calls the drawing helper at 0x801F2ED4 with
 *           0x12C and 0xFF, which the branch delay slot shows as the constant
 *           0x12C materialised into a0 ahead of both exits.
 *           Takes no arguments, returns nothing and keeps no state of its own:
 *           the only frame is the 0x18-byte caller frame with ra at 0x10. This
 *           payload holds no in-image pointer or jal to 0x801F3318, so the
 *           boundary rests on the body's own prologue, single `jr $ra` return
 *           and the neighbouring Splat rows (0x1EC bytes, matching the
 *           generated `nonmatching` size).
 * @status partial
 * @match 77.24
 * @residual Register assignment only. Bytes, frame, instruction count and
 *           memory events agree; the temporary holding the record byte +0x27
 *           index and the row temporary derived from it hold each other's
 *           adjacent registers from the first divergent instruction (v0/v1,
 *           and a0/v1 in the two destination stores).
 */
void copyHalvedPalettePairOnCountdownAndAdvanceRowWorld02Area09113_801F3318(void) {
  u16 top;
  u16 bottom;

  D_1F800044[0xA] = D_1F800044[0xA] - 1;

  if (D_1F800044[0xA] != 0) {
    top = (u16)((PSX_PTR(ClutRow, 0x80033800u)[D_1F800044[0x27] >> 3]
                     .block[D_1F800044[0x27] & 7]
                     .entry[D_1F800044[0xB]] &
                 0x7BDE) >>
                1);
    bottom = (u16)((PSX_PTR(ClutRow, 0x80033800u)[D_1F800044[0x27] >> 3]
                        .block[D_1F800044[0x27] & 7]
                        .entry[0x1F - D_1F800044[0xB]] &
                    0x7BDE) >>
                   1);
  } else {
    top = PSX_PTR(ClutRow, 0x80033800u)[D_1F800044[0x27] >> 3]
              .block[D_1F800044[0x27] & 7]
              .entry[D_1F800044[0xB]];
    bottom = PSX_PTR(ClutRow, 0x80033800u)[D_1F800044[0x27] >> 3]
                 .block[D_1F800044[0x27] & 7]
                 .entry[0x1F - D_1F800044[0xB]];
  }

  PSX_PTR(ClutRow, 0x80037800u)[D_1F800044[0x27] >> 3]
      .block[D_1F800044[0x27] & 7]
      .entry[D_1F800044[0xB]] = top;

  PSX_PTR(ClutRow, 0x80037800u)[D_1F800044[0x27] >> 3]
      .block[D_1F800044[0x27] & 7]
      .entry[0x1F - D_1F800044[0xB]] = bottom;

  if (D_1F800044[0xA] == 0) {
    D_1F800044[0xA] = 2;
    D_1F800044[0xB] = D_1F800044[0xB] + 1;
    if (D_1F800044[0xB] == 0x10) {
      D_1F800044[0xA] = 0x1E;
      D_1F800044[4] = D_1F800044[4] + 1;
    }
  }

  paletteStageSerial = paletteStageSerial + 1;
  drawTranslucentFanOfThirtyTwoGouraudWedgesFromProjectedOriginFadingToBlackWorld02Area09113_801F2ED4(
      0x12C, 0xFF);
}
