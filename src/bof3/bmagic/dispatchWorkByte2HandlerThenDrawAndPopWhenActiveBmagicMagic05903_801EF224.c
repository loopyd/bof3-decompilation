#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map.
 * The cell is not volatile: every read in this function is separated from the
 * next by a call that may store through the pointer, so the original
 * re-materializes `lui $vN,0x1F80` / `lw $vN,0x44($vN)` for the dispatch read,
 * for the guard read and again for each of the sixteen draw reads. Declaration
 * style follows the exact same-target sibling
 * incrementWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndClear
 * WorkBytesWhenZeroBmagicMagic05903_801F012C. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Overlay handler table for work-record byte 2. It is the second word of the
 * code-pointer run whose row starts at 0x801F065C and its ten entries are the
 * ten work byte 2 handlers of this overlay; the byte-1 table at 0x801F066C
 * begins one word earlier with this function as its own entry 0. This target's
 * generated undefined-syms map already binds the interior word, so the binding
 * needs no map row. */
extern void (*D_801F0670[])(void); /* @source 0x801F0670 @kind data */

/* Pushes the GTE matrix stack, builds the work record's translation and
 * rotation and installs both GTE matrices. */
void func_801EF638(void);

/* Draws one element of the work record's active effect: the angular index in
 * the first argument selects a position in the sixty-four step turn and the
 * second argument is the element's positional offset. */
void func_801EF70C(int angle, int offset);

/* @source 0x801EF224
 * @behavior Step handler for state 0 of the overlay work object published at
 *           the scratchpad pointer cell 0x1F800044. It reads that object's
 *           unsigned byte 0x2, scales it by four and calls the handler pointer
 *           at that index of the work byte 2 handler table based at 0x801F0670.
 *           When the handler returns it re-reads the cell and, only when the
 *           record byte 0x0 and the byte 0x2 are both non-zero, calls the
 *           projection helper at 0x801EF638, then the sixteen positional draw
 *           calls to 0x801EF70C and finally PopMatrix, restoring the GTE stack
 *           the projection helper pushed. The draw calls pass an angular index
 *           in $a0 and a positional offset in $a1: the first eight subtract the
 *           record byte 0x9 from the base angles 0x00, 0x08, 0x10, 0x18, 0x20,
 *           0x28, 0x30 and 0x38 and mask the result to the six-bit turn (0x3F)
 *           with offset 0x20; the next eight subtract twice that byte from the
 *           base angles 0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34 and 0x3C and
 *           mask to 0x3E with offset 0x50. Each draw re-reads the cell and the
 *           byte instead of holding the record pointer across the calls. The
 *           0x18-byte frame only keeps $ra across the indirect call; takes no
 *           arguments, returns nothing and reads or writes no other state. No
 *           jal in this payload reaches it: its address is entry 0 of the
 *           handler table based at 0x801F066C, the word at 0x801F066C inside
 *           the textbin row T_801F065C that the byte-1 dispatcher
 *           dispatchStateByte1Table066cBmagicMagic05903_801EF1E0 indexes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic05903_801EF224(void) {
  u8 *step_record;
  u8 *draw_record;

  step_record = g_battle_work;
  D_801F0670[step_record[2]]();
  draw_record = g_battle_work;
  if (draw_record[0] != 0 && draw_record[2] != 0) {
    func_801EF638();
    func_801EF70C((-g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x08 - g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x10 - g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x18 - g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x20 - g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x28 - g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x30 - g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x38 - g_battle_work[9]) & 0x3F, 0x20);
    func_801EF70C((0x04 - g_battle_work[9] * 2) & 0x3E, 0x50);
    func_801EF70C((0x0C - g_battle_work[9] * 2) & 0x3E, 0x50);
    func_801EF70C((0x14 - g_battle_work[9] * 2) & 0x3E, 0x50);
    func_801EF70C((0x1C - g_battle_work[9] * 2) & 0x3E, 0x50);
    func_801EF70C((0x24 - g_battle_work[9] * 2) & 0x3E, 0x50);
    func_801EF70C((0x2C - g_battle_work[9] * 2) & 0x3E, 0x50);
    func_801EF70C((0x34 - g_battle_work[9] * 2) & 0x3E, 0x50);
    func_801EF70C((0x3C - g_battle_work[9] * 2) & 0x3E, 0x50);
    PopMatrix();
  }
}
