#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's map
 * and shared by the overlay's dispatchers and handlers. The original
 * re-materialises the cell with its own lui/lw pair at every use, seven times in
 * this body, because the intervening calls and the stores through the pointer
 * the cell supplied may change it; the named row keeps those loads separate
 * instead of folding one address into a saved register. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Four-entry handler table for work-record byte 2. The code-pointer run starts
 * one word earlier at 0x801EFEE8 (0x801EEDB4, 0x801EEF10, 0x801EEF78,
 * 0x801EF054, 0x801EF100), which the byte-1 dispatcher
 * dispatchStateByte1Tablefee8BmagicMagic14303_801EED70 indexes; this function
 * indexes the run from its second word, so the extern is declared at the
 * indexed entry. The generated undefined_syms_auto.txt row names this word. */
extern void (*D_801EFEEC[])(void); /* @source 0x801EFEEC @kind data */

/* Shared main-executable helper at 0x800AF764, called once per step before this
 * overlay's own active guard. Outside this payload, so it keeps the raw name and
 * the generated undefined_funcs_auto.txt row for that address. */
void func_800AF764(void);

/* Pushes the GTE matrix stack, projects the record position and sets both
 * matrices (PushMatrix / RotTrans / RotMatrix / MulMatrix2 / SetRotMatrix /
 * SetTransMatrix). Still an asm row in this target. */
void func_801EF188(void);

/* Draws one band of the record's effect from (band base, band offset, band
 * mask); it stores half of the offset at 0x1F800000 and the low nibble of the
 * record's byte 0xB, shifted up eight bits, at 0x1F800004. Still an asm row. */
void func_801EF23C(s32, s32, s32);

/* Draws the argument-free band of the same effect, whose first step stores the
 * constant 0x80 where the 0x801EF23C path stores half its second argument. It
 * emits translucent primitives through RotTransPers / SetDrawMode / SetSemiTrans;
 * lifted at 0x801EF9A4 (partial on the entry-block copy). */
void drawRandomScaledTranslucentGouraudBandOverWorkByteACountFromWorkByteBPhaseBmagicMagic14303_801EF9A4(
    void);

/* Draws the translucent fan whose centre vertex is the record's screen pair at
 * halfword 0x2E, through rand / RotTransPers and the primitive queue. Still an
 * asm row in this target. */
void func_801EFC74(void);

/* @source 0x801EEDB4
 * @behavior One step of the overlay's work handler over the record published at
 *           the scratchpad pointer cell 0x1F800044. It first dispatches on the
 *           record's unsigned byte 2 scaled by four through the four-entry
 *           handler table based at 0x801EFEEC, calling the selected entry with
 *           no arguments and without bounding the index, and then calls the
 *           shared main-executable helper at 0x800AF764. It re-reads the cell
 *           and, only when the record's byte 2 and its byte 0 are both non-zero,
 *           runs the record's active arm: the transform installer at
 *           0x801EF188, then three band draws through 0x801EF23C with the bases
 *           0x30, 0x20 and 0x18, the offsets 0x10, 0x50 and 0x80 and the masks
 *           3, 0xF and 0x1F, adding 4 to the record's byte 0xB before the second
 *           band, 2 before the band at 0x801EF9A4 and 4 before the third band,
 *           then subtracting 0xA so that byte ends where it started; then it
 *           adds 0x10 to the record's unsigned halfword 0x2E, draws through
 *           0x801EFC74, subtracts 0x20 from the same halfword, draws again and
 *           pops the matrix stack. Every store's opcode fills the delay slot of
 *           the call that follows it while its load and add issue before that
 *           call. The 0x18-byte frame only keeps $ra at 0x10; nothing is
 *           returned and no other state is read or written.
 *
 *           The word at 0x801EFEEC is the second word of the five-word in-image
 *           handler run at 0x801EFEE8, so the byte-2 table this function heads
 *           holds that run's four entries 1..4 and this function's own address is
 *           entry 0 of it. Nothing in the payload holds 0x801EEDB4 as a jal
 *           target or as a table word of the 0x801EFEEC run; it is dispatched as
 *           table index 0 of work byte 1 by
 *           dispatchStateByte1Tablefee8BmagicMagic14303_801EED70, so it has no
 *           direct caller.
 *
 *           Statement order, operand forms and declaration style follow the
 *           same-family source
 *           dispatchWorkByte2HandlerThenDrawWorkRecordAndPopWhenWorkBytes2And0SetBmagicMagic09703_801EEFC8,
 *           recorded exact with the identical call topology (byte-2 dispatch,
 *           the 0x800AF764 step call, the two-byte active guard, a transform
 *           installer, three band draws with ascending offsets and the masks
 *           7/0xF/0x1F, a byte-0xB tint walk, two fan draws around halfword 0x2E
 *           and PopMatrix); this body differs in its table base, its band bases
 *           and arguments, its byte-0xB steps and its two emitters. No
 *           byte-identical copy of the 0x15C-byte span (sha256
 *           af5c6c5b5f24727c6f9c10caf1dbac4edb97b8a5c969c5aceacfd09838d31fb2)
 *           exists in any of the 122 bmagic payloads, so the clone lever was
 *           declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawBandsAndFanAndPopWhenWorkBytes2And0SetBmagicMagic14303_801EEDB4(void) {
  D_801EFEEC[g_battle_work[2]]();
  func_800AF764();
  if (g_battle_work[2] != 0 && g_battle_work[0] != 0) {
    func_801EF188();
    func_801EF23C(0x30, 0x10, 3);
    g_battle_work[0xB] += 4;
    func_801EF23C(0x20, 0x50, 0xF);
    g_battle_work[0xB] += 2;
    drawRandomScaledTranslucentGouraudBandOverWorkByteACountFromWorkByteBPhaseBmagicMagic14303_801EF9A4();
    g_battle_work[0xB] += 4;
    func_801EF23C(0x18, 0x80, 0x1F);
    g_battle_work[0xB] -= 0xA;
    FIELD_REF(u16, g_battle_work, 0x2Eu) += 0x10;
    func_801EFC74();
    FIELD_REF(u16, g_battle_work, 0x2Eu) -= 0x20;
    func_801EFC74();
    PopMatrix();
  }
}
