#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays
 * and bound by this target's own map row to 0x1F800044. This body reads the cell
 * once and both the byte load and the byte store go through that one loaded
 * pointer, so the access stays one plain cell load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay image.
 * The generated undefined_syms_auto.txt row binds the address and the remaining
 * asm of this target still references it, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0700
 * @behavior Per-call countdown step of this overlay: no jal and no table word
 *           in this payload names this span, so its caller is outside the image
 *           and the step is called from there. It reads the scratchpad work
 *           object through the pointer cell at 0x1F800044, subtracts four from
 *           its countdown byte 9 at byte width (lbu / addiu -4 / sb) and tests
 *           the stored byte truncated to eight bits (andi $v0,$v0,0xFF /
 *           bnez), so the truncated register, not a reload, supplies the
 *           compared value.
 *           Only when the countdown reaches zero does it read the shared battle
 *           work record pointer at 0x801EB4E0, decrement that record byte 0xB
 *           by one and invoke the shared helper at 0x801E5988, the store
 *           filling the call delay slot. The pointer-cell load precedes frame
 *           creation; the 0x18-byte frame only keeps $ra at 0x10 across the
 *           call. Takes no arguments, returns nothing and reads or writes no
 *           other state.
 *
 *           The 92-byte span differs from the exact sibling
 *           decrementWorkByte9ByTwoThenDecrementGlobalWorkByteBAndInvokeHelper
 *           WhenZeroBmagicMagic17203_801EF4BC at 0x801EF4BC in the magic172 #3
 *           payload in exactly one word, that sibling's subtracted constant,
 *           measured with one payload comparison over every word of both
 *           spans; the statement order, operand forms and declaration style
 *           are its own. A search of all payloads under out/binaries finds no
 *           second copy of this constant, so the clone lever was declined and
 *           this body was seeded from that sibling with this span's own literal
 *           written in.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9By4ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic17303_801F0700(void) {
    if ((g_battle_work[9] -= 4) == 0) {
        D_801EB4E0[0xB] -= 1;
        func_801E5988();
    }
}
