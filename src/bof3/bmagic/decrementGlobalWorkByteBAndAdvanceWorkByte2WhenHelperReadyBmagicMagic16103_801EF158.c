#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's handlers. The
 * original re-reads the cell after the record store (lui $v1,0x1F80 /
 * lw $v1,68($v1)), so the pointer stays volatile and every access reloads it,
 * which also lets the assembler fold the constant address into the load. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer cell in main RAM, outside this overlay's
 * image. The original materializes the cell as lui $v1,0x801F / lw
 * $v1,0xB4E0, the canonical hi/lo split of 0x801EB4E0 (the halfword offset is
 * sign-extended), so it binds that address; the neighbouring bmagic lifts write
 * the same record's byte 0xB. */
extern u8* D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared readiness helper of the main executable at 0x8014DAEC. It reports a
 * readiness byte, which this handler tests truncated to eight bits, the same
 * way the exact sibling advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C
 * reads it. */
u8 func_8014DAEC(void);

/* @source 0x801EF158
 * @behavior Entry 3 (index 3) of the five-entry handler run at 0x801EEC10
 *           whose words are 0x801EF024, 0x801EF06C, 0x801EF0E8, 0x801EF158 and
 *           0x801EF1BC. The work byte 2 dispatcher at 0x801EEF80 copies the run
 *           into its own 0x30-byte frame and invokes the entry that scratchpad
 *           work-object byte 2 selects, with no bounds test and with no jal
 *           anywhere in the payload naming this body. Per frame it calls the
 *           shared readiness helper at 0x8014DAEC and tests the returned byte
 *           truncated to eight bits: while the helper reports not-ready the
 *           handler does nothing. Once it reports ready the handler loads the
 *           shared battle work-record pointer cell at 0x801EB4E0 and decrements
 *           that record's byte 0xB at byte width (lbu / addiu -1 / sb), then
 *           re-reads the work-object pointer cell at 0x1F800044 and increments
 *           work byte 2 with a byte-width add, advancing dispatch to run entry 4
 *           at 0x801EF1BC. It takes no arguments and returns nothing; the
 *           0x18-byte frame only keeps the return address at 0x10 across the
 *           helper call, and it reads or writes no other state. The 100-byte
 *           body is byte-identical to the span at 0x801EFA70 of
 *           emi/bmagic/magic004/03, which was still on its asm row when this
 *           span was lifted, so that twin supplied no source to clone. The span
 *           at 0x801EF648 of emi/bmagic/magic154/03 also still sat on its asm
 *           row at that time and differed from this body only in the record
 *           byte offset, 0x9 there against 0xB here (payload byte comparison),
 *           so the sibling supplied statement order and operand forms and this
 *           body writes its own offset.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndAdvanceWorkByte2WhenHelperReadyBmagicMagic16103_801EF158(void) {
  if (func_8014DAEC() != 0u) {
    D_801EB4E0[0xB]--;
    g_battle_work[2]++;
  }
}
