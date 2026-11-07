#include "bof3/bof3.h"

void func_800A9610(u8 arg0, s32 arg1);

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The original re-materialises its address at each later group (lui
 * $vN,0x1F80 / lw $vN,68($vN)) because a store through the loaded pointer may
 * alias the cell, so the later stores stay on the named cell while the seeding
 * pair below shares the one load the original makes before it creates the
 * frame. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The remaining asm of this target still loads the same cell, so Splat's
 * generated undefined_syms_auto.txt binds the address and no map row is
 * needed. This body reads the record's halfword pair 0x2E/0x30, the same pair
 * the exact sibling
 * decrementWorkCountdownThenSeedHalfwordsFromRecordSetWordCTo11AndByteATo20AdvanceWorkByte2BmagicMagic06903_801EF034
 * copies into the work object. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared battle target byte at 0x80146384: the target selector whose 0x80/0x40
 * bits pick the party or enemy group. It lives outside this overlay and the
 * remaining asm span func_801EEE80 of this target still reads it, so Splat's
 * generated undefined_syms_auto.txt binds it and no target-map row is needed. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801F0584
 * @behavior Per-frame work-handler step, entry 0 of the five-entry handler
 *           table at 0x801F0D98 that the byte-1 dispatcher
 *           dispatchWorkByte2HandlerThenDrawTwiceWhenActiveBmagicMagic22203_801F0504
 *           indexes with the scratchpad work object's byte 0x2, so no jal in
 *           this payload targets this address. It seeds the work object's
 *           halfword pair 0x2E/0x30 from the same halfwords of the shared
 *           battle record read through 0x801EB4E0, taking that record pointer
 *           and the work pointer through 0x1F800044 before it creates its
 *           0x18-byte frame and storing both halves through the one work base
 *           it loaded there (lhu/nop/sh twice). It then raises battler status
 *           flag 0x10 by passing the shared battle target byte at 0x80146384
 *           (unsigned byte load, zero-extended) and the flag word 0x10 to
 *           func_800A9610, which ORs that flag into the selected battlers'
 *           status word. After the call it re-reads the cell before each store
 *           group, because a store through the loaded pointer may alias the
 *           cell: it stores 8 into work byte 0xB, 0 into work byte 0x9 and 4
 *           into work byte 0xA, then re-reads the cell once more to advance
 *           work byte 0x2 with a byte-width read-modify-write (lbu/addiu 1/sb),
 *           which selects table entry 1 (0x801F061C) for the next frame. The
 *           $ra reload precedes that final byte-2 store. The function takes no
 *           arguments, returns nothing and reads or writes no other state.
 *           The 0x98-byte span has no byte-identical sibling in any extracted
 *           payload: a full-record scan of out/binaries finds it only at its
 *           own offset 0x1984 of this overlay. Statement order, operand forms
 *           and declaration style follow that exact sibling plus
 *           raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 *           for the target byte and helper call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHalfwordPairFromRecordRaiseFlag10RearmWorkBytesAndAdvanceWorkByte2BmagicMagic22203_801F0584(void) {
    u8 *record;
    u8 *work;
    u16 *source;
    u16 *halves;

    record = D_801EB4E0;
    source = (u16 *)record;
    work = g_battle_work;
    halves = (u16 *)work;
    halves[0x17] = source[0x17];
    halves[0x18] = source[0x18];
    func_800A9610(D_80146384, 0x10);
    g_battle_work[0xB] = 8;
    g_battle_work[9] = 0;
    g_battle_work[0xA] = 4;
    g_battle_work[2]++;
}
