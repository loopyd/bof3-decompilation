#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's handlers. The
 * original re-reads the cell after the helper call (lui $v1,0x1F80 /
 * lw $v1,68($v1)), so the pointer stays volatile and every access reloads it,
 * which also lets the assembler fold the constant address into the load. This
 * is the same declaration the sibling handler at 0x801EF158 of this target
 * uses. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer cell in main RAM, outside this overlay's
 * image. The original materializes the cell as lui $v1,0x801F / lw
 * $v1,0xB4E0, the canonical hi/lo split of 0x801EB4E0, so it binds that
 * address; this target's generated undefined_syms_auto.txt already carries the
 * row and no target-map entry is added. */
extern u8* D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable helper at 0x800AEF98. The overlay only passes it a
 * signed event selector and a value; its body and the meaning of 0x18 stay
 * unknown, and the generated undefined_funcs_auto.txt row binds the address. */
extern void func_800AEF98(s32 event, s32 value); /* @source 0x800AEF98 */

/* @source 0x801EEE9C
 * @behavior Entry 1 (index 1) of the three-entry code pointer run at
 *           0x801EEC04, whose words are 0x801EEC84, 0x801EEE9C and 0x801EEF08.
 *           The overlay entry dispatcher at 0x801EEC24 copies that run into its
 *           own 0x28-byte frame and invokes the entry that scratchpad
 *           work-object byte 1 selects, with no bounds test and with no jal
 *           anywhere in the payload naming this body. It reads the work-object
 *           pointer cell at 0x1F800044 before creating its 0x18-byte frame and
 *           tests work byte 0xB truncated to eight bits: while that byte is
 *           non-zero the handler does nothing. When it is zero the handler
 *           loads the shared battle work-record pointer cell at 0x801EB4E0,
 *           clears bit 0x40 of that record's byte 0 at byte width (lbu / andi
 *           0xBF / sb, the store filling the call delay slot), calls the shared
 *           main-executable helper at 0x800AEF98 with the arguments 0x18 and 0,
 *           then re-reads the work-object pointer cell and increments work byte
 *           1 at byte width, advancing dispatch to run entry 2 at 0x801EEF08.
 *           It takes no arguments and returns nothing; the frame only keeps the
 *           return address at 0x10 across the helper call, and it reads or
 *           writes no other state. The 0x6C-byte span differs from the span at
 *           0x801EF1FC of emi/bmagic/magic003/03 in exactly three words: that
 *           span's helper argument is 4 against 0x18 here, and its record
 *           pointer cell is 0x8014637C against 0x801EB4E0 here (payload byte
 *           comparison), so this body transcribes the statement order and
 *           operand forms of that already-exact sibling
 *           clearRecordFlag40AndInvokeHelperWhenWorkByteBZeroThenAdvanceWorkByte1BmagicMagic00303_801EF1FC
 *           and writes its own literals.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClearRecordFlag(void) {
  if (g_battle_work[0xB] == 0) {
    D_801EB4E0[0] &= 0xBF;
    func_800AEF98(0x18, 0);
    g_battle_work[1]++;
  }
}
