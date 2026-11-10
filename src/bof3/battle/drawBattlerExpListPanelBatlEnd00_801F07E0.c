#include "bof3/bof3.h"

extern u8 *D_80148648;

/* Volatile party-size byte at 0x801462F0. The original loads it once for the
 * entry test and again on every loop test, so the load is never hoisted. */
extern volatile u8 localBattlerCount;

/* Four-character name string at row offset 0x74 of the 0x140-byte battler work
 * records based at 0x80145E90. The original scales the battler index by 0x140
 * and reads the name field and the byte six bytes after it through their own
 * 32-bit displacements (`lui $at,%hi(X)` + `addu $at,$at,index*0x140` + load
 * `%lo(X)($at)`), so the field keeps its own address-derived symbol and the
 * byte it reads later is spelled as an offset from this base: that load's
 * folded symbol is D_80145F0A, row offset 0x7A, the byte after the party byte
 * at 0x79. */
extern u8 D_80145F04[]; /* @source 0x80145F04 @kind table */

/* The shared 0x20-byte text buffer at 0x80149118 that the overlay formats its
 * two numbers into, and the two overlay format strings of the same run:
 * "%6d" at 0x801EEC44 and "%2d" at 0x801EEC48, both in the leading tabledata
 * of this overlay. */
extern u8 D_80149118[];
extern u8 D_801EEC44[];
extern u8 D_801EEC48[];

extern int sprintf(char *buffer, const char *format, ...);

/* The overlay panel frame drawer at 0x801F039C: sets the draw mode and the
 * tile for a panel window and draws its border. Still an address-anchored asm
 * span, so it is declared here for its address. */
extern void func_801F039C(s16 x, s16 y, s32 width, u8 height);

/* Shared text submitter of the main executable at 0x8014F800. It draws the
 * character run its fifth argument points at — a NUL-terminated glyph string —
 * at (arg0, arg1), carrying arg2 and arg3 through to the primitive. Declared
 * here for its address. */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* The second shared text submitter at 0x8014FF0C: same pen position and run
 * pointer, but no mask argument. Declared here for its address. */
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);

/* The EXE texture-row handle lookup at 0x801502D0: answers the handle of one
 * row of the loaded texture bank, which the submitter above draws as its
 * fifth argument. Declared here for its address. */
extern u32 func_801502D0(u32 row);

/* The overlay remaining-experience readout at 0x801F0BB0, lifted as
 * sumRemainingLevelExpBatlEnd00_801F0BB0: it takes a local battler index byte
 * and answers that battler's experience short of the next level, or zero once
 * the level table is exhausted. */
extern s32 sumRemainingLevelExpBatlEnd00_801F0BB0(u8 battler);

/* @source 0x801F07E0
 * @behavior Battle-end panel state handler 1 of the two-entry local handler
 *   table built by dispatchPanelStateLocalTableBatlEnd00_801F0768: it draws
 *   the per-battler experience panel. It opens the panel frame at (0x14, 0x28)
 *   through the overlay frame drawer func_801F039C with the width 0x118 and
 *   byte +0x09 of the object published at 0x80148648 as the height, then walks
 *   the local battlers below the volatile count byte at 0x801462F0 and draws
 *   one row per battler, stepping y by 0x10 from 0x2C. Every row submits the
 *   battler's four-character name, the name field of the 0x140-byte work
 *   record selected by the row index, through func_8014F800 at (0x19, y) with
 *   the mask 5. When the row's byte +0x06 reads 0x63 the row draws only the
 *   overlay texture row 0x41 at (0x55, y) with the mask 0xFF; otherwise it
 *   prints the remaining experience sumRemainingLevelExpBatlEnd00_801F0BB0
 *   answers for that battler through the overlay format "%6d" at 0x801EEC44
 *   into the shared text buffer 0x80149118 and submits it at (0x50, y), draws
 *   the texture row 0x15 at (0x85, y) with the mask 0xFF, prints that row byte
 *   plus one through the overlay format "%2d" at 0x801EEC48 into the same
 *   buffer and submits it at (0x118, y). The count byte is re-read on every
 *   loop test, so the walk stops as soon as it shrinks below the running
 *   index, and a zero count byte skips the walk after the frame is drawn. The
 *   0x38-byte frame keeps $ra and $s0-$s5: the running y, the buffer pointer,
 *   the running index, the row byte offset, the row's eight-bit y copy and the
 *   row's eight-bit index copy. Takes no arguments and returns nothing.
 * @status partial
 * @match 75.96
 * @residual The candidate stops at 79 of 104 instructions with four extra bytes;
 *           the first difference is at +0x0004, in the panel setup where the
 *           row cursor and list-height stores differ from the original.
 * 0x801F07E0 is entry 1 of the state table at dispatchPanelStateLocalTableBatlEnd00_801F0768,
 * printed by the amount strip of the same overlay run as the per-battler
 * experience panel: its rows sit on the frame that the counted-list panel
 * drawer func_801F0650 of the same handler run draws above. The row offset
 * 0x7A byte it tests is the value the battle/magic060 overlay raises as the
 * delta source for that battler's record; this overlay prints it plus one as a
 * two-digit field and treats 0x63 as the arm that leaves the experience figure
 * out.
 */
void drawBattlerExpListPanelBatlEnd00_801F07E0(void) {
  u8 *buffer;
  u8 i;
  u8 y;
  s32 offset;

  y = 0x2c;
  func_801F039C(0x14, 0x28, 0x118, ((u8 *)D_80148648)[9]);
  if (localBattlerCount != 0) {
    i = 0;
    buffer = D_80149118;
    do {
      offset = (u8)i * 0x140;
      func_8014F800(0x19, y, 0, 5, (u32)&D_80145F04[offset]);
      if (D_80145F04[offset + 6] == 0x63) {
        func_8014F800(0x55, y, 0, 0xff, func_801502D0(0x41));
      } else {
        sprintf((char *)buffer, (const char *)D_801EEC44,
                sumRemainingLevelExpBatlEnd00_801F0BB0(i));
        func_8014FF0C(0x50, y, 0, buffer);
        func_8014F800(0x85, y, 0, 0xff, func_801502D0(0x15));
        sprintf((char *)buffer, (const char *)D_801EEC48,
                D_80145F04[offset + 6] + 1);
        func_8014FF0C(0x118, y, 0, buffer);
      }
      i += 1;
      y += 0x10;
    } while (i < localBattlerCount);
  }
}
