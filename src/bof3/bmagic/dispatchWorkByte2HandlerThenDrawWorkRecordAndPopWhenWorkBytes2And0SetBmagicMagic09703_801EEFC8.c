#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044. The original
 * re-materializes the cell with its own lui/lw pair at every use, so the
 * constant-address accessor's folded address would stay live across a call in
 * a saved register and grow the frame; the cell is read through this named row
 * instead, as in the exact bmagic sibling
 * dispatchWorkByte2HandlerThenShiftMotionPairAndDrawWhenActiveBmagicMagic15903_801EEE28. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Four-entry work-byte-2 handler table for this overlay. The original adds the
 * scaled byte to the word at 0x801F0AC4, which is the second entry of the
 * five-word code-pointer run based at 0x801F0AC0 (0x801EEFC8, this function's
 * own address, 0x801EF114, 0x801EF1A4, 0x801EF324 and 0x801EF488), so the
 * extern is declared at the indexed entry rather than at the run head that the
 * byte-1 dispatcher at 0x801EEF84 indexes. The generated
 * undefined_syms_auto.txt row for 0x801F0AC4 already names this word. */
extern void (*D_801F0AC4[])(void); /* @source 0x801F0AC4 @kind data */

/* Shared main-executable helper at 0x800AF764. This overlay calls it once per
 * step before its own active guard, with no arguments and no result. No lifted
 * body for it existed in this tree when this seed was written, so it keeps its
 * raw name and the generated undefined_funcs_auto.txt row for that address. */
void func_800AF764(void);

/* Pushes the GTE matrix stack and installs this overlay's work-record
 * transform: builds the record's translation and rotation and sets both GTE
 * matrices. Still an asm row in this target, so it keeps its raw name. */
void func_801EF510(void);

/* Draws one band of the work-record effect from the arguments (band base,
 * band offset, band mask). Still an asm row in this target. */
void func_801EF5C4(s32, s32, s32);

/* Flushes the primitives the band calls have queued. Still an asm row. */
void func_801F0568(void);

/* Applies the halfword the caller has just stored into the work record. Still
 * an asm row. */
void func_801F083C(void);

/* @source 0x801EEFC8
 * @behavior Dispatches one step of the overlay work handler from the work
 *   record published at the scratchpad pointer cell 0x1F800044. The record's
 *   unsigned byte at offset 0x2, scaled by four, selects one entry of the
 *   handler table based at 0x801F0AC4 and that entry is called with no
 *   arguments; the original does not bound the index. Then, unconditionally,
 *   it calls the shared main-executable helper at 0x800AF764 and reads the
 *   pointer cell again: only when the record's byte at offset 0x2 and its byte
 *   at offset 0x0 are both non-zero does it draw the active record. That arm
 *   calls the transform installer at 0x801EF510, then three band draws through
 *   0x801EF5C4 with the offsets 0x20, 0x40 and 0x60 and the masks 7, 0xF and
 *   0x1F, adding 4 to the record's byte at offset 0xB before the second and
 *   third of them and flushing through 0x801F0568 after the second and third,
 *   then subtracts 8 from that byte, adds 0x10 to the record's unsigned
 *   halfword at offset 0x2E and applies it through 0x801F083C, subtracts 0x20
 *   from the same halfword and applies it again, and finally pops the matrix
 *   stack with PopMatrix. Each store's opcode fills the delay slot of the call
 *   that follows it while its load and add are issued before that call. The
 *   pointer cell is re-read at every use because both the intervening calls and
 *   the stores through the pointer the cell supplied may change it, which is
 *   why the original materializes it seven times. The 0x18-byte frame only
 *   keeps $ra at 0x10 across the calls. Nothing is returned and no other state
 *   is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawWorkRecordAndPopWhenWorkBytes2And0SetBmagicMagic09703_801EEFC8(
    void) {
  D_801F0AC4[g_battle_work[2]]();
  func_800AF764();
  if (g_battle_work[2] != 0 && g_battle_work[0] != 0) {
    func_801EF510();
    func_801EF5C4(12, 0x20, 7);
    g_battle_work[0xB] += 4;
    func_801EF5C4(12, 0x40, 0xF);
    func_801F0568();
    g_battle_work[0xB] += 4;
    func_801EF5C4(12, 0x60, 0x1F);
    func_801F0568();
    g_battle_work[0xB] -= 8;
    FIELD_REF(u16, g_battle_work, 0x2Eu) += 0x10;
    func_801F083C();
    FIELD_REF(u16, g_battle_work, 0x2Eu) -= 0x20;
    func_801F083C();
    PopMatrix();
  }
}
