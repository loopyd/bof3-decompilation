#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. The original
 * re-materialises the cell (lui $vN,0x1F80 / lw $vN,68($vN)) for the two loads
 * this body makes, so it is named here: the cell is re-read after the indirect
 * call because the dispatched handler may republish it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Five-entry overlay handler table for work byte 2, inside this overlay's
 * trailing pointer run: the word at 0x801F0D94 holds 0x801F0070 and the table
 * starts at the next word, 0x801F0D98 = 0x801F0584, then 0x801F061C,
 * 0x801F066C, 0x801F06C8 and 0x801F0730, followed by a zero word at
 * 0x801F0DAC. The run's asm boundary func_801F0D94 defines the label, so this
 * extern binds the interior word the scaled index is added to. */
extern void (*D_801F0D98[])(void); /* @source 0x801F0D98 @kind data */

/* This overlay's 0x5AC-byte primitive-drawing routine at 0x801F07A8, still
 * unlifted in its own asm subsegment, which owns the symbol. It reads the work
 * object's bytes 0x09, 0x0B, 0x2E and 0x30, emits up to 0x20 semi-transparent
 * quad pairs from g_PrimCursor through SetPolyG4, SetSemiTrans,
 * func_801782FC and func_8014E5A0, and takes no arguments. */
void func_801F07A8(void);

/* @source 0x801F0504
 * @behavior Per-frame work-handler step, entry 3 of the sixteen-word code
 *           pointer run at 0x801F0D54 whose word at 0x801F0D60 holds this
 *           function's address, so the byte-1 dispatcher
 *           dispatchStateByte1Table0d54BmagicMagic22203_801EEEE0 selects it
 *           while the work object's byte 1 is 3 and no jal in this payload
 *           targets this address. It reads the scratchpad work object through
 *           the pointer cell at 0x1F800044 before it creates its frame, takes
 *           the object's unsigned byte 2, scales it by four and calls that
 *           entry of the five-entry handler table at 0x801F0D98 (0x801F0584,
 *           0x801F061C, 0x801F066C, 0x801F06C8, 0x801F0730) with no arguments.
 *           When that handler returns it re-reads the cell once for both gate
 *           bytes because the handler may have republished it, and when work
 *           byte 0 and work byte 2 are both non-zero it calls this overlay's
 *           drawing routine func_801F07A8 twice in a row, drawing the object's
 *           effect twice that frame. The 0x18-byte frame only keeps $ra at
 *           0x10, an indirect call and each of the three remaining calls leaves
 *           its delay slot a nop, and it takes no arguments, returns nothing
 *           and reads or writes no other state. The 128-byte span is
 *           instruction-for-instruction identical to the already-exact
 *           dispatchRecordByte2HandlerThenDrawWhenActiveBmagicMagic13103_801F1138
 *           of emi/bmagic/magic131/03, differing only in that body's seven
 *           immediate words: the two pointer-cell loads, the table-address
 *           displacement and the two jal targets, which is why this span has no
 *           byte-identical sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawTwiceWhenActiveBmagicMagic22203_801F0504(void) {
    u8 *step_record;
    u8 *gate_record;

    step_record = g_battle_work;
    D_801F0D98[step_record[2]]();
    gate_record = g_battle_work;
    if (gate_record[0] != 0 && gate_record[2] != 0) {
        func_801F07A8();
        func_801F07A8();
    }
}
