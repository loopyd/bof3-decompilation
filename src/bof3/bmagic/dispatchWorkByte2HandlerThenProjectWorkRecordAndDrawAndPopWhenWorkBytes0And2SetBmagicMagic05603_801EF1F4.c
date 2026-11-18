#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The code pointer run at 0x801F0744 is the one the byte-1 dispatcher
 * dispatchStateByte1Table0744BmagicMagic05603_801EF1B0 indexes; its eleven words
 * are 0x801EF1F4, 0x801EF1F4, 0x801EFA50, 0x801EFF28, 0x801EF27C, 0x801EF2DC,
 * 0x801EF37C, 0x801EFAD8, 0x801EFB38, 0x801EFBA4 and 0x801EFBF4, so this function
 * occupies words 0 and 1 of it. The byte-2 table this function dispatches through
 * begins four words into that run, at 0x801F0754. The original indexes from that
 * interior word without checking the byte-2 index, so the extern is declared at
 * the indexed word; the run's own first word keeps the reviewed `textbin` row for
 * 0x801F0744, and the generated undefined_syms_auto.txt row names this word. The
 * byte-2 step dispatcher at 0x801EFA50 reads the same run seven words in, at
 * 0x801F0760. */
extern void (*D_801F0754[])(void); /* @source 0x801F0754 @kind data */

/* Pushes the GTE matrix stack, projects the work-record position through
 * RotTrans and installs the resulting transform with RotMatrix, MulMatrix2,
 * SetRotMatrix and SetTransMatrix. Still an asm row in this target. */
extern void func_801EF410(void); /* @source 0x801EF410 @kind function */

/* Draws the work record's effect: reads the graph type, builds its primitives
 * with GetTPage/GetClut/SetDrawMode/SetSemiTrans and RotTransPers4 and emits
 * them through the primitive queue. Still an asm row in this target. */
extern void func_801EF4C0(void); /* @source 0x801EF4C0 @kind function */

/* @source 0x801EF1F4
 * @behavior One frame of the overlay's work handler over the record published at
 *           the scratchpad pointer cell 0x1F800044. It reads the cell, takes the
 *           record's unsigned byte 2 scaled by four and calls the entry at that
 *           index of the handler table based at 0x801F0754 with no arguments,
 *           without bounding the index. It re-reads the cell after the handler
 *           and tests the record's byte 0 and then its byte 2, both at unsigned
 *           byte width. Only when both are non-zero does it run the record's
 *           active arm: the transform installer at 0x801EF410, the draw pass at
 *           0x801EF4C0 and PopMatrix. The 0x18-byte frame only keeps $ra at 0x10,
 *           created after the dispatch index has been read and before it is
 *           scaled. Nothing is returned and no other state is read or written.
 *
 *           The record's byte 0 is its raised-flag byte, which this arm tests
 *           without masking a bit, and byte 2 is the byte-2 step counter its own
 *           handlers advance. No jal in this payload names 0x801EF1F4; it is
 *           reached as words 0 and 1 of the byte-1 handler run at 0x801F0744,
 *           which the exact byte-1 dispatcher
 *           dispatchStateByte1Table0744BmagicMagic05603_801EF1B0 indexes with
 *           work byte 1, so it has no direct caller and its parameter roles stay
 *           structural.
 *
 *           Statement order, operand forms and declaration style follow the
 *           same-family source
 *           dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic11003_801EFFB8,
 *           recorded exact with the identical call topology (byte-2 dispatch
 *           through an in-image run, the same byte-0-then-byte-2 unsigned guard
 *           on a re-read cell, a transform installer, a draw pass and
 *           PopMatrix). The 0x88-byte span is not byte-identical to any span in
 *           the 122 payloads under out/binaries, so the clone lever was declined;
 *           this target's own 0x88-byte span at 0x801EFA50 held the same 34
 *           instructions and differed only in its table word low half and in the
 *           draw it called before PopMatrix, so that row's asm body was shape
 *           evidence for this one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic05603_801EF1F4(
    void) {
  D_801F0754[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_801EF410();
    func_801EF4C0();
    PopMatrix();
  }
}
