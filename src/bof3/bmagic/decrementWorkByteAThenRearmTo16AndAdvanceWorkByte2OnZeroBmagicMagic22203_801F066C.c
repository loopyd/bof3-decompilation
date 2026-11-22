#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address (lui $v1,0x1F80 / lw $v1,68($v1)) once for the byte-0xA countdown and
 * again for each of the two stores the zero arm makes, so every group reaches
 * the work object through this target-map symbol instead of a hoisted base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F066C
 * @behavior Per-frame work-handler step, entry 5 of the eight-word code pointer
 * run based at 0x801F0D8C whose word at 0x801F0DA0 holds this function address,
 * so the byte-1 dispatcher
 * dispatchWorkByte2HandlerThenDrawTransformedAndPopWhenWorkBytes0And2SetBmagicMagic22203_801EFDEC
 * selects it while work object byte 1 is 2 and work object byte 2 is 5, and no
 * jal in this payload targets this address. It reads the scratchpad work object
 * through the pointer cell at 0x1F800044, decrements its unsigned byte 0xA in
 * place with a byte-width add (lbu / addiu -1 / sb) and tests the stored byte
 * (andi 0xFF / bnez), so the countdown wraps modulo 256. When that byte reaches
 * zero it re-reads the cell to store 0x10 into byte 0xA - a constant the
 * original materialises in the branch delay slot - and re-reads the cell once
 * more to increment byte 0x2 with a byte-width add, which moves the byte-2
 * dispatcher to the next handler step. The function is a leaf with no frame; it
 * takes no arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAThenRearmTo16AndAdvanceWorkByte2OnZeroBmagicMagic22203_801F066C(void) {
    if (--g_battle_work[0xA] == 0) {
        g_battle_work[0xA] = 0x10;
        g_battle_work[2]++;
    }
}
