#include "bof3/ui/shop00_internal.h"

/* EMI-local data of the shop sort option panel. The payload keeps its rodata in
 * the tail run the extracted label T_801E5144 carries, so no target-map row is
 * added for any of these words and the generated undefined-symbol list supplies
 * the ones the asm spelled as literals.
 * @source 0x801E5E7C @kind table */
extern const u8 D_801E5E7C[];
/* @source 0x801E5EA0 @kind table */
extern const u8 D_801E5EA0[];
/* @source 0x801E5EAC @kind table */
extern const u8 D_801E5EAC[];
/* Four caption handles of the panel, eight payload bytes each: 0x801E5EDC holds
 * the bytes `SORT`, 0x801E5EE4 `LOOK`, 0x801E5EEC `High`, 0xFF and `AP`, and
 * 0x801E5EF4 `Low`, 0xFF and `AP`. Entry 0 is the panel caption and the byte
 * list D_801E5F0D selects entries 1 through 3 as the option rows; the bytes
 * past each text are not interpreted.
 * @source 0x801E5EFC @kind table */
extern const u8* const D_801E5EFC[];
/* Option count cell of the panel: the payload holds 3 here. It is read as the
 * first byte of the cell at 0x801E5F0C, which the shipped bytes show sitting one
 * byte below the row index list D_801E5F0D. The byte-array declaration matters:
 * with the unqualified array element the cell address stays in a callee-saved
 * register across the three calls and the load is repeated on every row test,
 * while a plain byte scalar rematerialized the address (172/178 instructions,
 * 712 bytes), a const array cached the loaded value across the calls (164/178,
 * 700 bytes) and a byte-pointer local spent a sixth saved register (163/179,
 * 716 bytes).
 * @source 0x801E5F0C @kind unknown */
extern u8 D_801E5F0C[];
/* Option-row caption indices of D_801E5EFC: the payload holds 1, 2 and 3 here,
 * which select the LOOK, High and Low slots.
 * @source 0x801E5F0D @kind table */
extern const u8 D_801E5F0D[];

/* Main-exe record-table renderer (executable space) called with a panel origin,
 * one of this EMI's tile-record streams and a flag. */
/* @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* @source 0x801E3F5C
 * @behavior shop sort option panel of the panel task, reached through the
 *           wrapper func_801E3F34, which forwards the global panel task root:
 *           paints the shaded panel rectangle through func_801AE3F0 at (task x
 *           + 3, task y + 3) with the width 0x65, the height 0x19 plus 0x10
 *           times the option count byte D_801E5F0C[0], the constant 0 and the
 *           main-RAM CLUT-bank byte D_80144952 as the two trailing stack
 *           arguments; submits the caption D_801E5EFC[0] (the bytes SORT) at
 *           (x + 0x27, y + 7) through the main-exe caption emitter
 *           func_8014F800 with the texture-row constant 0 and 0xFF; submits the
 *           tile-record stream D_801E5E7C once at (x, y) through the main-exe
 *           record-table renderer func_801AF390 with the flag 1; then walks the
 *           D_801E5F0C[0] option rows, taking row i's caption from
 *           D_801E5EFC[D_801E5F0D[i]] and submitting it through func_8014F800
 *           at (x + 7, y + i * 0x10 + 0x1A) with the texture-row constant 7
 *           when i is the task byte 0xA or byte 0xB row and 0 otherwise, plus
 *           one more submission at (x + 7, y + i * 0x10 + 0x18) with 2 for the
 *           byte 0xB row and 0 for the byte 0xA row, and overlays that row with
 *           the tile-record stream D_801E5EA0 at (x, y + i * 0x10 + 0x18) and
 *           (x, y + i * 0x10 + 0x20) with the flag 1; once the walk ends it
 *           submits D_801E5EAC at (x, y + i * 0x10 + 0x18) with the flag 1,
 *           closing the panel below the last row.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Representation notes of the first measurement. The row label offset is a u16
 * local recomputed in each arm (yRow = i * 0x10 + 0x1A) and the second
 * submission of an arm subtracts 2 from it, which keeps one offset register
 * across the call; writing both offsets inline as
 * `task->field_06 + i * 0x10 + 0x1A` / `+ 0x18` measured 156/178 instructions
 * (704 bytes) and made the offset a word local that never left its arm 144/178
 * (696 bytes). The two row separators and the closing stream are spelled
 * `(i * 2 + 3) * 8` and `(i * 2 + 4) * 8`, which measured 164/178 at the exact
 * 712 bytes, where `i * 0x10 + 0x18` / `+ 0x20` measured 156/178 at 704. */
void drawShopSortOptionPanel(PanelTask* task) {
  const u8* caption;
  u16 yRow;
  u8 i;

  func_801AE3F0(task->x + 3, task->field_06 + 3, 0x65,
                D_801E5F0C[0] * 0x10 + 0x19, 0, D_80144952);
  func_8014F800(task->x + 0x27, task->field_06 + 7, 0, 0xFF,
                (u32)D_801E5EFC[0]);
  func_801AF390(task->x, task->field_06, D_801E5E7C, 1);
  for (i = 0; i < D_801E5F0C[0]; i++) {
    caption = D_801E5EFC[D_801E5F0D[i]];
    if (i == *((u8*)task + 0xB)) {
      yRow = i * 0x10 + 0x1A;
      func_8014F800(task->x + 7, task->field_06 + yRow, 7, 0xFF,
                    (u32)caption);
      func_8014F800(task->x + 7, task->field_06 + yRow - 2, 2, 0xFF,
                    (u32)caption);
    } else if (i == *((u8*)task + 0xA)) {
      yRow = i * 0x10 + 0x1A;
      func_8014F800(task->x + 7, task->field_06 + yRow, 7, 0xFF,
                    (u32)caption);
      func_8014F800(task->x + 7, task->field_06 + yRow - 2, 0, 0xFF,
                    (u32)caption);
    } else {
      yRow = i * 0x10 + 0x1A;
      func_8014F800(task->x + 7, task->field_06 + yRow, 0, 0xFF,
                    (u32)caption);
    }
    func_801AF390((s16)task->x, (s16)(task->field_06 + (i * 2 + 3) * 8),
                  D_801E5EA0, 1);
    func_801AF390((s16)task->x, (s16)(task->field_06 + (i * 2 + 4) * 8),
                  D_801E5EA0, 1);
  }
  func_801AF390((s16)task->x, (s16)(task->field_06 + (i * 2 + 3) * 8),
                D_801E5EAC, 1);
}
