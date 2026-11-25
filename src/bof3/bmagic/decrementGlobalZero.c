#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's own
 * map as g_battle_work, the name this overlay's lifted siblings use. The
 * original materialises the address at each access
 * (lui $v1,0x1F80 / lw $v1,68($v1)), so the named cell is kept rather than a
 * constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt still binds the address for
 * the assembly spans that load it (0x801EEC7C, 0x801EF914), so no map row is
 * needed, and every access here is byte-addressed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF2D4
 * @behavior Entry 6 of the ten-pointer handler table based at 0x801EFA9C (the
 *           word at 0x801EFAB4, its only in-image reference); the same span is
 *           the fourth step of the work-byte-2 step table based at 0x801EFAA8.
 *           It reads the scratchpad work object through the pointer cell at
 *           0x1F800044 before the frame is created, decrements that object's
 *           unsigned byte 9 at byte width (lbu $v0,0x9($v1) / addiu $v0,$v0,-1
 *           / sb / andi $v0,$v0,0xFF) and tests the truncated register with
 *           bnez, so the stored byte rather than a reload supplies the compared
 *           value. The rest of the body runs only when that countdown reaches
 *           zero: it reads the shared battle work record pointer at 0x801EB4E0
 *           with one lui/lw direct cell load, subtracts one from that record's
 *           byte 0xB at byte width, and invokes the shared helper at 0x801E5988
 *           with the store filling the call delay slot. The 0x18-byte frame
 *           only keeps $ra at 0x10 across that call. Takes no arguments,
 *           returns nothing through the jr $ra at 0x801EF328 and reads or
 *           writes no other state. The roles of work-object byte 9 and record
 *           byte 0xB are not established, so both stay offset-anchored.
 *           Shape: the 92-byte span compares equal to the payload spans at
 *           magic041/03 0x801EF564, magic126/03 0x801EF0BC, magic110/03
 *           0x801EF93C and magic222/03 0x801EF770, all lifted exact as
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelper
 *           WhenZero on their targets, so this body is their transcription.
 *           Their statement order, operand forms and declaration style carry
 *           over unchanged; this target's own map binds the scratchpad cell as
 *           g_battle_work, the name its lifted siblings use, and the two
 *           out-of-payload addresses are already bound by this target's map and
 *           generated auto files.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalZero(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
