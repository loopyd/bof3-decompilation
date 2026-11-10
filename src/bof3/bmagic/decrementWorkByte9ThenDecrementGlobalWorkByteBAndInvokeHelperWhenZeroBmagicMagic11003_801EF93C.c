#include "bof3/bof3.h"

/* Shared helper in main RAM, outside this overlay's image. The payload only
 * calls it (jal 0x801E5988), so the generated undefined_funcs map keeps the raw
 * spelling and the address needs no map row; the prototype matches the one every
 * other exact body in src/bof3/bmagic gives this helper. */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* Scratchpad work-object pointer cell, bound by this target's own map row
 * g_battle_work = 0x1F800044 and spelled that way by this target's four other
 * exact lifts. This body reads the cell once, so the single access stays one
 * plain pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the remaining assembly spans func_801EEC94, func_801EF6B4, func_801EF740 and
 * func_801EF8F8 load it, so the name needs no map row, and every access here is
 * byte-addressed, so the local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF93C
 * @behavior Entry 11 of the fourteen-word byte-1 handler table based at
 *           0x801F046C, which dispatchStateByte1Table046cBmagicMagic11003_801EF3F8
 *           indexes with work record byte 1 and calls indirectly; the same six
 *           handlers from 0x801EF844 on form the work-byte-2 run at 0x801F048C,
 *           where this entry is index 3. The word at 0x801F0498 is the only word
 *           in the payload that holds this address, so the entry has no jal
 *           caller. Reads the scratchpad work object through the pointer cell at
 *           0x1F800044 and decrements its unsigned byte 9 in place (lbu /
 *           addiu -1 / sb / andi 0xFF / bnez), testing the stored byte truncated
 *           to eight bits rather than a reload, so the rest of the body runs only
 *           when that countdown reaches zero. On zero it reads the shared battle
 *           work record pointer at 0x801EB4E0, decrements that record's byte 0xB
 *           by one at byte width and invokes the shared helper at 0x801E5988,
 *           the store filling the call delay slot. The pointer-cell load precedes
 *           frame creation because the countdown needs no stack; the 0x18-byte
 *           frame only keeps $ra at 0x10 across the call. Takes no arguments,
 *           returns nothing and reads or writes no other state. The 92-byte span
 *           0x801EF93C..0x801EF997 is byte-for-byte identical to sixteen payload
 *           spans of this family, four of them already exact siblings:
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZero
 *           BmagicMagic00603_801EF41C, the same-named magic06000_801F00D0,
 *           magic13103_801F02B0 and magic22203_801EF770 bodies, so this body was
 *           seeded by the clone lever from the magic22203 source and keeps its
 *           statement order, operand forms and declaration style; the scratchpad
 *           cell keeps this target's own mapped spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagicMagic11003_801EF93C(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B]--;
    func_801E5988();
  }
}
