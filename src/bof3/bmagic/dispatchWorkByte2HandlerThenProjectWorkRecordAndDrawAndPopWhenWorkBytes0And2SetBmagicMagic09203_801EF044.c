#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The original re-materialises the cell with its own
 * lui/lw pair for the dispatch index and again with a second lui/lw pair after
 * the indirect call, because that call may republish the pointer, so both reads
 * stay on this named cell rather than on a pointer local, as in the exact
 * bmagic sibling
 * dispatchWorkByte2HandlerThenInvokeUpdateAndDrawWhenWorkBytes0And2SetBmagicMagic09203_801EF840,
 * which reads the same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Word 1 of the code-pointer run based at 0x801F07BC, which Splat places as the
 * single textbin row T_801F07BC: the fifteen non-zero words from 0x801EF044
 * through 0x801F0420, followed by the record pool at 0x801F07F8. The byte-1
 * dispatcher at 0x801EF000 indexes that run from its head, while this body
 * dispatches on work byte 2 from the word at 0x801F07C0, whose own entries are
 * 0x801EF0CC, 0x801EF1F0, 0x801EF350, 0x801EF388, 0x801EF840, 0x801F0260,
 * 0x801EF8C0, 0x801EF940 and on through the same run. The work record's byte 2
 * scales by four and is added to 0x801F07C0, so the extern is declared at the
 * indexed word and the run stays that one map row; the generated
 * undefined_syms_auto.txt row for 0x801F07C0 already names this word. */
extern void (*D_801F07C0[])(void); /* @source 0x801F07C0 @kind data */

/* Pushes the GTE matrix stack and installs the work record's transform: it
 * builds the record's translation and rotation triplets from the record words
 * 0x34 and 0x38 and signed halfword 0x3E, projects them with RotTrans, composes
 * the shared screen matrix at 0x801492E8 through MulMatrix2 and sets both GTE
 * matrices, leaving the pushed matrix for this function's PopMatrix. Still an
 * asm row in this target, so it keeps its raw name. */
void func_801EF4D0(void);

/* This overlay's own draw pass at 0x801EF580: it picks a draw mode from
 * GetGraphType, installs it with SetDrawMode on the primitive cursor, refreshes
 * the scratchpad screen scale through func_801782FC and func_801783C8 and then
 * emits the work-record effect with SetSemiTrans, func_8017A97C and
 * RotTransPers3 under a second draw mode. Still an asm row in this target, so it
 * keeps its raw name; it draws under the matrix func_801EF4D0 installed and
 * does not pop it. */
void func_801EF580(void);

/* @source 0x801EF044
 * @behavior Dispatches one step of the overlay work handler: the work record is
 *   read from the scratchpad pointer cell at 0x1F800044 before frame creation,
 *   its unsigned byte at offset 0x2 is scaled by four and selects one entry of
 *   the handler table word at 0x801F07C0, which is called with no arguments;
 *   the original does not bound the index. The cell is then re-read and the
 *   record's bytes at offsets 0x0 and 0x2 are tested in that order, so only
 *   when both are non-zero does the arm run: it installs the work record's
 *   transform with func_801EF4D0, draws the effect with func_801EF580 and then
 *   restores the GTE matrix stack with PopMatrix. The cell is re-read rather
 *   than kept live because the dispatched handler may republish it, which is
 *   why the original materialises it twice. The 0x18-byte frame only keeps $ra
 *   at 0x10, and the pointer-cell load is issued before frame creation. Entry 0
 *   of the fifteen-word handler run based at 0x801F07BC, which the byte-1
 *   dispatcher at 0x801EF000 indexes, so no jal in this payload names the
 *   0x88-byte span; it takes no arguments and returns nothing. Work bytes 0x0
 *   and 0x2 stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic09203_801EF044(
    void) {
  D_801F07C0[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_801EF4D0();
    func_801EF580();
    PopMatrix();
  }
}
