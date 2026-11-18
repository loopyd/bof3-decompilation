#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. The original
 * re-materialises the cell (lui $v0,0x1F80 / lw $v0,68($v0) before the table
 * call, lui $v1,0x1F80 / lw $v1,68($v1) after it), so it is named here: the cell
 * is re-read after the indirect call because the dispatched handler may
 * republish it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work-byte-2 handler table for this step, the interior base 0x801F0D8C of this
 * overlay's trailing pointer run: the eight words from 0x801F0D8C are
 * 0x801EFE74, 0x801EFF7C, 0x801F0070, 0x801F0584, 0x801F061C, 0x801F066C,
 * 0x801F06C8 and 0x801F0730. The byte-1 dispatcher's sixteen-entry run at
 * 0x801F0D54 covers the same bytes from its word 14, so this extern binds the
 * interior word the scaled index is added to and the generated
 * undefined_syms_auto.txt row already names it. */
extern void (*D_801F0D8C[])(void); /* @source 0x801F0D8C @kind data */

/* This overlay's 0xB0-byte matrix-setup routine at 0x801F0168, still unlifted in
 * its own asm subsegment, which owns the symbol. It pushes the GTE matrix stack,
 * builds a rotation/translation pair from work-object words 0x34, 0x38 and
 * signed halfword 0x3E, multiplies it with the shared matrix at 0x801492E8 and
 * installs the result through SetRotMatrix and SetTransMatrix. Leaf call: no
 * arguments, no result. */
void func_801F0168(void);

/* This overlay's 0x2EC-byte drawing routine at 0x801F0218, still unlifted in its
 * own asm subsegment, which owns the symbol. It branches on GetGraphType, sets
 * the draw mode and emits this step's primitives from the work-object fields
 * through the shared helper at 0x80155A08. No arguments, no result. */
void func_801F0218(void);

/* @source 0x801EFDEC
 * @behavior Per-frame work-handler step, entry 2 of the sixteen-word code
 *           pointer run at 0x801F0D54 whose word at 0x801F0D5C holds this
 *           function's address, so the byte-1 dispatcher
 *           dispatchStateByte1Table0d54BmagicMagic22203_801EEEE0 selects it
 *           while the work object's byte 1 is 2 and no jal in this payload
 *           targets this address. It reads the scratchpad work object through
 *           the pointer cell at 0x1F800044 before it creates its frame, takes
 *           the object's unsigned byte 2, scales it by four and calls that
 *           entry of the eight-entry handler table based at 0x801F0D8C
 *           (0x801EFE74, 0x801EFF7C, 0x801F0070, 0x801F0584, 0x801F061C,
 *           0x801F066C, 0x801F06C8, 0x801F0730) with no arguments. When that
 *           handler returns it re-reads the cell once for both gate bytes
 *           because the handler may have republished it, and when work byte 0
 *           and work byte 2 are both non-zero it runs the active arm: it calls
 *           this overlay's matrix-setup routine func_801F0168, which pushes the
 *           GTE matrix stack and installs the object's transform, then this
 *           overlay's drawing routine func_801F0218, then pops the matrix stack
 *           with PopMatrix. The 0x18-byte frame only keeps $ra at 0x10, the
 *           indirect call and each of the four remaining calls leaves its delay
 *           slot a nop, and it takes no arguments, returns nothing, makes no
 *           object stores and reads or writes no other state. The span is
 *           instruction-for-instruction the same shape as the already-exact
 *           dispatchWorkByte2HandlerThenDrawTwiceWhenActiveBmagicMagic22203_801F0504
 *           of this target, differing only in that body's table displacement,
 *           its two branch displacements and its two tail calls; no payload
 *           under out/binaries holds a byte-identical copy of the span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawTransformedAndPopWhenWorkBytes0And2SetBmagicMagic22203_801EFDEC(void) {
    u8 *step_record;
    u8 *gate_record;

    step_record = g_battle_work;
    D_801F0D8C[step_record[2]]();
    gate_record = g_battle_work;
    if (gate_record[0] != 0 && gate_record[2] != 0) {
        func_801F0168();
        func_801F0218();
        PopMatrix();
    }
}
