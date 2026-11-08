#include "bof3/bof3.h"

/* Shared front-side gate byte: the resident executable publishes the cell code
 * of the point the player stands on into this byte, so the overlay steps of
 * this family branch on it (1, 2 and 5 in the lifted neighbours of this
 * payload). This step reads it three times, each read separated by a call. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Shared main-RAM flag bank whose bit 0x28 this step clears and then probes. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* 7x7 table of per-cell draw codes of this overlay: the code of cell (row,
 * column) is read as one byte and passed straight on as that cell's draw size.
 * The next declared span starts 0x34 bytes on, so the table holds 49 codes.
 * The original addresses it as a row pointer plus a row offset, which is what
 * the two-dimensional form here reproduces. */
extern u8 D_801F5BC8[7][7]; /* @source 0x801F5BC8 @kind data */

/* Seven-word template of the packed cell-code map: it ends exactly where
 * 0x801F5C18 begins, so it is the source copy of the 28-byte map below. */
extern u32 D_801F5BFC[]; /* @source 0x801F5BFC @kind data */

/* Packed 7x7 cell-code map at the end of this overlay, two nibbles per byte
 * with four bytes per map column (column * 4 + row / 2). The neighbouring
 * selector, request and stamp steps of this payload read it as bytes; this
 * step restores it a word at a time, hence the word binding here. */
extern u32 D_801F5C18[]; /* @source 0x801F5C18 @kind data */

/* This overlay's own cell draw: queues the four shared UI elements around the
 * grid point of a cell and takes that cell's code as the element size. */
void func_801F4604(s16 row, s16 col, u32 size);

/* Shared flag helpers of the resident executable: the bit setter and the bit
 * tester against the main-RAM bank above. */
void func_8015B5A8(void* arg0, u8 bit_index);
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Shared UI element queue of the resident executable. */
void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F46A8
 * @behavior While the shared front-side gate byte D_80143F03 reads clear this
 *           step clears flag 0x28 of the main-RAM bank D_80144F28 through
 *           0x8015B5A8.
 *
 *           When that byte then reads 1 it draws the whole 7x7 grid: the code
 *           of cell (row, column) of the grid at 0x801F5BC8 is passed to this
 *           overlay's cell draw at 0x801F4604 as that cell's size. It then
 *           restores the packed 28-byte cell map at 0x801F5C18 from the
 *           seven-word template at 0x801F5BFC, and queues
 *           the four cursor rectangles (0x11, 0xAD), (0x12, 0xAD), (0x11,
 *           0xAE) and (0x12, 0xAE) through the shared UI queue at 0x801A4BC0:
 *           sizes 0x50, 0x50, 0 and 0 while flag 0x28 of D_80144F28 reads
 *           clear through 0x8015B5D4, and 0xA1, 0xA1, 0xC0 and 0xC0 while it
 *           reads set. The last rectangle is queued from a single shared tail
 *           whose size the two arms load before it.
 *
 *           When that byte reads 5 instead it queues the two shared UI
 *           elements (0xA, 8) and (0xB, 8) with size 0x50.
 *
 *           Takes no arguments and returns nothing; the 0x28-byte frame keeps
 *           $ra and $s0..$s4 across the calls.
 *
 * Name evidence: 0x80143F03 is the front-side cell code the resident publisher
 *           func_8019FBF8 fills from the bounding-box record of the current
 *           point, and the same-target entry step at 0x801F4140 already reads
 *           it as the gate of this overlay's entry sequence; 0x80144F28 is the
 *           main-RAM flag bank that same step names; 0x801F5BC8 and 0x801F5BFC
 *           are this payload's own grid-code and map-template spans, sized by
 *           the following declared starts; 0x801F5C18 is the packed cell map
 *           the same-target selector, request and stamp steps name;
 *           0x801F4604 is this overlay's cell draw as declared by the stamp
 *           step; and 0x801A4BC0 is the shared UI element queue the
 *           same-target dispatch step already calls with these very four
 *           rectangles. The name stays at what the code does, since the
 *           meaning of cell codes 1 and 5 is not proven.
 *
 * Shape: no byte-identical twin. The full 0x1E8-byte range and each of its
 *           0x40, 0x80, 0x100 and 0x180-byte prefixes were compared against
 *           every payload under out/binaries with no hit outside this payload,
 *           so clone is ineligible. The range is one complete function
 *           (prologue, calls, one jr $ra epilogue) and is code, not a data
 *           span, so neither the data nor the carve lever applies. The body is
 *           seeded from the original bytes; the two gate blocks and the flag
 *           setter/tester calls follow the same-target entry step at
 *           0x801F4140, the cell draw call follows the same-target stamp step
 *           at 0x801F3808, and the four cursor rectangles follow the
 *           same-target dispatch step at 0x801F3EE8. Two further shapes were
 *           measured against the original bytes and rejected: a per-column row
 *           pointer local matched the address folding but kept the narrowed
 *           column in a caller-saved register across the cell draw, costing two
 *           instructions, and an explicitly narrowed column local lost three
 *           more. Indexing the table as the 7x7 grid it is reproduced both the
 *           preheader address and the reused narrowed column, and the restore
 *           loop reuses the grid row counter, which is why the original zeroes
 *           that counter in the outer loop back-edge delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void redrawGridCellCodesAndRestorePackedCellMapThenQueueCursorRectsWorld02Area11113_801F46A8(
    void) {
  s16 col;
  s16 row;

  if (D_80143F03 == 0) {
    func_8015B5A8(&D_80144F28, 0x28);
  }
  if (D_80143F03 == 1) {
    for (col = 0; col < 7; col++) {
      for (row = 0; row < 7; row++) {
        func_801F4604(row, col, D_801F5BC8[col][row]);
      }
    }
    for (row = 0; row < 7; row++) {
      D_801F5C18[row] = D_801F5BFC[row];
    }
    if (func_8015B5D4((u32)&D_80144F28, 0x28) == 0) {
      func_801A4BC0(0x11, 0xAD, 0x50);
      func_801A4BC0(0x12, 0xAD, 0x50);
      func_801A4BC0(0x11, 0xAE, 0x00);
      func_801A4BC0(0x12, 0xAE, 0x00);
    } else {
      func_801A4BC0(0x11, 0xAD, 0xA1);
      func_801A4BC0(0x12, 0xAD, 0xA1);
      func_801A4BC0(0x11, 0xAE, 0xC0);
      func_801A4BC0(0x12, 0xAE, 0xC0);
    }
  }
  if (D_80143F03 == 5) {
    func_801A4BC0(0x0A, 0x08, 0x50);
    func_801A4BC0(0x0B, 0x08, 0x50);
  }
}
