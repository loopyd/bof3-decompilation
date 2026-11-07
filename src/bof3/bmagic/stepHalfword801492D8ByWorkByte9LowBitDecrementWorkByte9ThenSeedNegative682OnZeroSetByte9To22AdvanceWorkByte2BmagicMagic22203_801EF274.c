#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is the named cell
 * symbol the exact siblings of this target use: the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), and the original re-materialises
 * it before every later store group because the stores through the previously
 * loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared halfword channel at 0x801492D8, in the main executable and outside
 * this overlay's payload, so the generated undefined_syms_auto.txt row of this
 * target already binds the address and no map row is needed. The two arms load
 * it with lhu, so the cell is read as the unsigned halfword the ui/ sources of
 * the same address also declare. */
extern u16 D_801492D8; /* @source 0x801492D8 @kind unknown */

/* @source 0x801EF274
 * @behavior Per-frame work-handler step of this overlay. It reads the
 *           scratchpad work object through the pointer cell at 0x1F800044,
 *           tests bit 0 of work byte 9 (lbu / andi 0x1 / beqz) and steps the
 *           shared halfword at 0x801492D8 by 20 in the selected direction, up
 *           (+0x14) when the bit is set and down (-0x14) when it is clear.
 *           Each arm takes its own pointer to the halfword, so both arms
 *           materialise the address with lui / addiu and read it at
 *           displacement 0, and the one store of the join writes through that
 *           same base. It then re-reads the cell and decrements work byte 9 in
 *           place (lbu / addiu -1 / sb / andi 0xFF / bnez), returning while
 *           that countdown is still nonzero. On zero it re-reads the cell,
 *           seeds the halfword at 0x801492D8 with -682, sets work byte 9 to 22,
 *           re-reads the cell and advances work byte 2 by one, which moves the
 *           dispatcher to the next work-byte-2 step. The signed store of -682
 *           reaches the bnez delay slot as one li and re-materialises $at for
 *           the store, the byte-9 seed 22 is materialised after that store, and
 *           each store group re-materialises the cell. Leaf with no frame;
 *           takes no arguments, returns nothing and reads or writes no other
 *           state. Bytes 0x801EF274..0x801EF323 are identical to
 *           0x801EF540..0x801EF5EF of emi/bmagic/magic226/03 except the byte-9
 *           seed at payload offset 132 (22 here, 60 there) and the
 *           displacement of the in-arm jump at offsets 44 and 45, so that
 *           sibling is a shape witness only: its own Splat row is still asm and
 *           carries no lifted copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepHalfword801492D8ByWorkByte9LowBitDecrementWorkByte9ThenSeedNegative682OnZeroSetByte9To22AdvanceWorkByte2BmagicMagic22203_801EF274(void) {
    u16 *half;
    u16 value;

    if (g_battle_work[9] & 1) {
        half = &D_801492D8;
        value = *half + 0x14;
    } else {
        half = &D_801492D8;
        value = *half - 0x14;
    }
    *half = value;
    if (--g_battle_work[9] == 0) {
        *(s16 *)&D_801492D8 = -0x2AA;
        g_battle_work[9] = 0x16;
        g_battle_work[2]++;
    }
}
