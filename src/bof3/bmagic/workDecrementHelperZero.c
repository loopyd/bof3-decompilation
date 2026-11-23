#include "bof3/bof3.h"

/* Shared helper in main RAM, outside this overlay image. The payload only
 * calls it (jal 0x801E5988), so the generated undefined_funcs map already
 * binds the raw spelling and the address needs no map row; the prototype is
 * the one every other exact body in src/bof3/bmagic gives this helper. */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* Scratchpad work-object pointer cell, bound by this target own map row
 * g_battle_work = 0x1F800044 and spelled that way by this target other exact
 * lifts. This body reads the cell twice: the byte store through the loaded
 * pointer may alias it, so the named cell is evaluated once per access group
 * and the compiler re-materialises the lui/lw pair where the original does. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00D8
 * @behavior Entry 13 of the fourteen-word byte-1 handler table based at
 *           0x801F046C, which callStateTable046
 *           indexes with work record byte 1 and calls indirectly, and the sixth
 *           and last word of the six-word work-byte-2 run at 0x801F048C, where
 *           it is entry 5; the two-word dispatch base 0x801F049C that
 *           dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhenWorkBytes0And2SetBmagicMagic11003_801EFFB8
 *           indexes holds this address as its index 1. The word at 0x801F04A0
 *           is the only word in the payload that holds this address and the
 *           payload contains no jal to it, so the entry is reached only through
 *           that pointer run. Reads the scratchpad work object through the
 *           pointer cell at 0x1F800044 and adds 2 to its unsigned byte 9 at
 *           byte width (lbu / addiu 2 / sb); the sum is not tested, so the
 *           register is stored without a following andi. Re-reads the cell and
 *           subtracts one from unsigned byte 0x0A the same way, testing the
 *           stored byte truncated to eight bits (andi $v0,$v0,0xFF / bnez), so
 *           the body reaches the shared helper at 0x801E5988 only when that
 *           countdown hits zero, with a nop filling the call delay slot. The
 *           first cell load precedes frame creation because the increment needs
 *           no stack; the 0x18-byte frame only keeps $ra at 0x10 across the
 *           call. Takes no arguments, returns nothing through the jr $ra at
 *           0x801F0130 and reads or writes no other state. The roles of work
 *           object bytes 9 and 0xA are not established, so both stay
 *           offset-anchored. The 0x60-byte span 0x801F00D8..0x801F0137 is not
 *           byte-identical to any other payload span: a search of every file
 *           under out/binaries returns this span alone, and masking the jal
 *           word at 0x801F0120 returns none, so the body was written from the
 *           original bytes with its statement order, cell access grouping and
 *           operand forms taken from the reviewed lift
 *           add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic00603_801F0290,
 *           whose first 0x38 bytes equal this span apart from the one stored
 *           immediate (2 here, 8 there); this body drops the global work-byte
 *           0xB decrement that sibling carries inside its guard and keeps only
 *           the helper call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void workDecrementHelperZero(
    void) {
  g_battle_work[0x09] += 2;
  if (--g_battle_work[0x0A] == 0) {
    func_801E5988();
  }
}
