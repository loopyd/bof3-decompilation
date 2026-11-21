#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void clearWorkBytesBmagicMagic09303_801F0620(void); /* @source 0x801F0620 */

/* @source 0x801EF750
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 and
 *           decrements that object's unsigned byte 0x09 in place (lbu / addiu
 *           -1 / sb), then tests the stored byte truncated to eight bits
 *           (andi $v0,$v0,0xFF / bnez), so the truncated register rather than a
 *           reload supplies the compared value and the rest of the body runs
 *           only when the countdown reaches zero. On zero it reads the shared
 *           battle work record pointer at 0x801EB4E0, decrements that record's
 *           byte 0x0B by one at byte width and calls this overlay's work-byte
 *           clear routine at 0x801F0620, the store filling the call delay slot.
 *           The pointer-cell load precedes frame creation because the countdown
 *           needs no stack; the 0x18-byte frame only keeps $ra at 0x10 across
 *           the call. No jal word in this payload names this span: its address
 *           is the word at 0x801F08C0, the tenth word of the handler-pointer
 *           run based at 0x801F089C and the ninth entry of the work-byte-2
 *           dispatch table D_801F08A0, so it is reached only through that run.
 *           Takes no arguments, returns nothing through the jr $ra at
 *           0x801EF7A4 and reads or writes no other state. The roles of work
 *           byte 9, of record byte 0x0B and of the cleared work bytes are not
 *           established, so they stay offset-anchored. The 92-byte span differs
 *           from the exact sibling
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhen
 *           ZeroBmagicMagic00603_801EF41C in the call word alone, 0x801F0620
 *           here against the shared helper 0x801E5988 there, which was proven
 *           by comparing this span against every extracted payload and leaves
 *           no byte-identical twin; the statement order, operand forms and
 *           declaration style therefore come from that sibling and the callee
 *           binding from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytesWhenZeroBmagicMagic09303_801EF750(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B]--;
    clearWorkBytesBmagicMagic09303_801F0620();
  }
}
