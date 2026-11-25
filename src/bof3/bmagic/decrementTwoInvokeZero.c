#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's own
 * map as g_battle_work, the name its lifted siblings use. The original
 * re-materialises the address for the access (lui $v1,0x1F80 / lw
 * $v1,68($v1)), so the named cell is kept rather than a constant-address
 * accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined_syms_auto.txt already binds the address for
 * the assembly spans that load it, so no target-map row is needed, and every
 * access here is byte-addressed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF914
 * @behavior Third word of the three-word work-byte-2 handler run based at
 *           0x801EFAB8 (0x801EF870, 0x801EF8C4, 0x801EF914), the run that
 *           dispatcher 0x801EF7F8 selects with work-object byte 2; the same
 *           three words are the last three entries of the ten-word byte-1
 *           pointer run based at 0x801EFA9C. It reads the scratchpad work
 *           object through the pointer cell at 0x1F800044 before the frame is
 *           created, subtracts two from that object's byte 9 at byte width
 *           (lbu $v0,0x9($v1) / addiu $v0,$v0,-2 / sb $v0,0x9($v1)) and tests
 *           the stored value truncated to eight bits (andi $v0,$v0,0xFF /
 *           bnez), so the truncated register rather than a reload supplies the
 *           compared value. The rest of the body runs only when that countdown
 *           reaches zero: it reads the shared battle work record pointer at
 *           0x801EB4E0 with one lui/lw direct cell load, subtracts one from that
 *           record's byte 0xB at byte width, and invokes the shared helper at
 *           0x801E5988 with the store filling the call delay slot. The
 *           0x18-byte frame only keeps $ra at 0x10 across that call. Takes no
 *           arguments, returns nothing through the jr $ra at 0x801EF968 and
 *           reads or writes no other state. The roles of work-object byte 9 and
 *           record byte 0xB are not established, so both stay offset-anchored.
 *           Shape: the 92-byte span compares equal to the exact lift at
 *           emi/bmagic/magic172/03 0x801EF4BC (payload offsets 3348 and 2236
 *           over out/binaries/emi/bmagic/magic111/03.bin and
 *           out/binaries/emi/bmagic/magic172/03.bin); one `cmp` of the two
 *           payload slices reported no differing byte, so this body is that
 *           sibling's transcription with its statement order, operand forms and
 *           declaration style carried over unchanged. The only in-image word
 *           naming this address is 0x801EFAC0, the third word of that run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementTwoInvokeZero(void) {
  if ((g_battle_work[0x09] -= 2) == 0) {
    D_801EB4E0[0x0B] -= 1;
    func_801E5988();
  }
}
