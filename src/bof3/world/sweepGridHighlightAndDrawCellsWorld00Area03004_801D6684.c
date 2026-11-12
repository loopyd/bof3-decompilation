#include "bof3/world/area03004_internal.h"

/* @source 0x801D6684
 * @behavior AREA030 grid handler, index 1 of the work-byte-3 dispatch table
 * 0x801E2048: work byte 0xA of the record published at the scratchpad cursor
 * 0x1F800044 is a countdown and work byte 9 a phase counter whose twelfth part
 * is the grid column and whose remainder of 12 gates a ten-frame highlight.
 * While the countdown runs it queues the icon strip at (0xD4, 0x8A) with the
 * work byte 0xB row plus one as the second sprite index; while the countdown is
 * zero it advances the phase by one and, once the cell of the four-by-eight row
 * table 0x801E2050 at that row and column is not the 0xFF end marker, arms the
 * countdown with 0x32. With the countdown still zero and that cell holding 1 it
 * runs the shared func_8014D978 helper on the highlight frames. It then draws
 * every cell of the table: the highlighted cell through the shaded text request
 * func_8014F800 three pixels above its row at the shade equal to the cell value,
 * and every other cell through the text drawer func_8014FC90 at the shade three
 * times the cell value inside the selected row or 7 minus the cell value
 * elsewhere, each as the one-character run of the two-entry run table 0x801E2070
 * indexed by that value. Columns step 0x0C0000 and rows 0x0D0000 of the fixed
 * origin 0x840000 at 0x9E, the highlighted row at 0x9B. After the shared
 * func_8014D4E0 update it decrements the countdown and, when that expires, clears
 * the phase and advances the work byte 0xB row, wrapping to zero at four.
 * @status partial
 * @match 51.98
 * @residual the recorded body shifts the two 16.16 cell origins down with a bare
 * arithmetic shift and hands them to the shared text drawers un-narrowed, while
 * include/gpu/prim.h declares func_8014F800's coordinates s16; the caller-side
 * sll/sra pair that prototype forces costs one callee-saved register, spills the
 * grid column to the frame and grows it from 0x40 to 0x48, which is the first
 * mismatch and the class behind most of the remaining 40 bytes. A /tmp probe with
 * both drawers' coordinates declared wide measures 221 of 217 instructions and
 * then leaves four allocation instructions in the two highlight arms.
 */
void sweepGridHighlightAndDrawCellsWorld00Area03004_801D6684(void) {
  u8* work;
  u8* state;
  u8* rowStart;
  u8* cell;
  u8 value;
  u8 col;
  s32 row;
  s32 column;
  s32 xpos;
  s32 ypos;
  s32 highlightYpos;

  work = D_1F800044;
  if (work[0xA] != 0) {
    queueIconStrip(0xD4, 0x8A, 0, work[0xB] + 1);
    state = D_1F800044;
    col = (u8)(state[9] / 12);
  } else {
    work[9] = work[9] + 1;
    state = D_1F800044;
    col = (u8)(state[9] / 12);
    if (D_801E2050[state[0xB]][col] != 0xFF) {
      state[0xA] = 0x32;
    }
  }

  state = D_1F800044;
  if (state[0xA] == 0) {
    if (D_801E2050[state[0xB]][col] == 1) {
      if ((u8)(state[9] % 12) < 10) {
        func_8014D978();
      }
    }
  }

  highlightYpos = 0x9B0000;
  ypos = 0x9E0000;
  rowStart = D_801E2050[0];
  for (row = 0; row < 4; row++) {
    column = 0;
    xpos = 0x840000;
    cell = rowStart;
    for (; column < 8; column++) {
      value = *cell;
      if (value == 0xFF) {
        break;
      }
      state = D_1F800044;
      if (row == state[0xB]) {
        if ((u8)(state[9] % 12) < 10) {
          if (column == col) {
            func_8014F800(xpos >> 16, highlightYpos >> 16, value, 1,
                          (u32)D_801E2070[value]);
          } else {
            func_8014FC90(xpos >> 16, ypos >> 16, (u8)(*cell * 3), 1,
                          D_801E2070[*cell]);
          }
        } else {
          func_8014FC90(xpos >> 16, ypos >> 16, (u8)(*cell * 3), 1,
                        D_801E2070[*cell]);
        }
      } else {
        func_8014FC90(xpos >> 16, ypos >> 16, (u8)(7 - value), 1,
                      D_801E2070[value]);
      }
      xpos = xpos + 0xC0000;
      cell = cell + 1;
    }
    ypos = ypos + 0xD0000;
    highlightYpos = highlightYpos + 0xD0000;
    rowStart = rowStart + 8;
  }

  func_8014D4E0();
  work = D_1F800044;
  if (work[0xA] != 0) {
    work[0xA] = work[0xA] - 1;
    state = D_1F800044;
    if (state[0xA] == 0) {
      state[9] = 0;
      work = D_1F800044;
      work[0xB] = work[0xB] + 1;
      state = D_1F800044;
      if (state[0xB] >= 4) {
        state[0xB] = 0;
      }
    }
  }
}
