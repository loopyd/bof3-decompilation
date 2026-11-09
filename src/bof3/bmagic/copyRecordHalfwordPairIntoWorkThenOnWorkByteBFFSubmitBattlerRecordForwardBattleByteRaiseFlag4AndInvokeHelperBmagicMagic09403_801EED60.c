#include "bof3/bof3.h"

void func_800AF814(u8 arg0);
void func_800A9820(u8 arg0);
void func_80196718(void *arg0);
void func_801E5988(void);

/* Scratchpad work-object pointer cell shared with the battle overlays. This
 * body reads the cell once and keeps the loaded pointer in a register across
 * the two halfword copies and the byte-0xB test, which is why the cell is
 * reached through a local pointer instead of a re-read access group. Bound at
 * 0x1F800044 by this target's map. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Battle record pointer at 0x8014638C in main RAM: this body copies its
 * halfword pair 0x2E/0x30 into the scratchpad work object and nothing else, so
 * its role stays address-anchored. The neighbouring span func_801EEC5C still
 * references the address, so Splat's generated undefined_syms_auto.txt binds it
 * and no target-map row is needed. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle target byte at 0x80146384: the battler index whose record this
 * body submits and the value it forwards to both battle helpers. It lives
 * outside this overlay, so it keeps its address-qualified name. The record
 * select reads the byte through the symbol, while the two later forwards hold
 * its address in a local pointer: the original materialises `lui $s0` plus
 * `addiu $s0,$s0,%lo(D_80146384)` once after the submit call and reloads the
 * byte with `lbu $a0,0($s0)` after each of those two calls, keeping the address
 * in a callee-saved register across the calls. The exact lift of
 * emi/boss/boss033/16@0x800C20CC reaches the same cell through the same local
 * pointer idiom. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared battle global halfword at 0x801462E8. The sibling battle lifts bind the
 * same word (BATTLE_GLOBAL_HALF_62E8); it lives outside this overlay, so it
 * keeps its address-qualified name. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Local battler work-record arrays selected by the battler index: the party
 * records hold 0x140-byte entries, the enemy records 0x118-byte entries. The
 * exact sibling
 * incrementWorkByteAThenIncrementWorkByte9AndSubmitSelectedBattlerRecordEffectWhenByte9Is10ClearByte9AndAdvanceWorkByte1BmagicMagic09403_801EEF7C
 * selects them with the same unsigned index test and the same scaled offsets. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */

/* @source 0x801EED60
 * @behavior Overlay work-step handler of the record published at the scratchpad
 *           pointer cell 0x1F800044. It loads the battle record pointer at
 *           0x8014638C and the work pointer once each, copies the record's
 *           unsigned halfword 0x2E into work halfword 0x2E, stores the record's
 *           halfword 0x30 minus 0x10 into work halfword 0x30, and returns while
 *           the work object's byte 0xB is not 0xFF. Only on that equality does
 *           it select the battler record of the shared target byte at
 *           0x80146384 - indices below three take the 0x140-byte-stride party
 *           entries at 0x80145E90, the rest the 0x118-byte-stride enemy entries
 *           at 0x801EB2E8 - submit it through func_80196718, forward the shared
 *           target byte, read again after that call through a local pointer to
 *           0x80146384, to func_800AF814 and func_800A9820, raise bit 0x4 of the
 *           shared battle global halfword at 0x801462E8 (unsigned halfword load,
 *           ori, halfword store) and invoke the shared helper func_801E5988. It
 *           takes no arguments, returns nothing, and the 0x18-byte frame keeps
 *           $ra and that $s0 pointer base across the four calls.
 *           Statement order, operand forms, offset-local spelling and
 *           declaration style follow the exact sibling
 *           incrementWorkByteAThenIncrementWorkByte9AndSubmitSelectedBattlerRecordEffectWhenByte9Is10ClearByte9AndAdvanceWorkByte1BmagicMagic09403_801EEF7C
 *           (the same unsigned index test, the same two scaled record bases and
 *           the same func_80196718 submit; its select block is instruction for
 *           instruction the select block here, minus the register that keeps the
 *           record alive past the call) and the exact siblings
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperBmagicMagic01000_801EEF58
 *           and
 *           forwardBattleByteThenRaiseFlag4AndInvokeHelperWhenWorkByteBZeroBmagicMagic10603_801EEE80
 *           (the func_800A9820 forward, the flag-0x4 raise and the func_801E5988
 *           invoke, and the byte-0xB work test that leaves the pointer cell in a
 *           register before the frame is created). This function's own
 *           halfword pair, its 0xFF test, its -0x10 adjustment and its
 *           func_800AF814 forward come from this target's original bytes. No
 *           extracted payload repeats this span: the discriminating immediate
 *           0xFF compared against the work byte 0xB, followed by the party/enemy
 *           select and the func_800AF814 forward, occurs only in this target's
 *           binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordHalfwordPairIntoWorkThenOnWorkByteBFFSubmitBattlerRecordForwardBattleByteRaiseFlag4AndInvokeHelperBmagicMagic09403_801EED60(void) {
  u8 *record;
  u8 *work;
  u8 *source;
  u8 *state;

  source = D_8014638C;
  work = g_battle_work;
  *(u16 *)(work + 0x2E) = *(u16 *)(source + 0x2E);
  *(u16 *)(work + 0x30) = *(u16 *)(source + 0x30) - 0x10;
  if (work[0xB] == 0xFF) {
    if (D_80146384 < 3u) {
      record = (u8 *)D_80145E90 + D_80146384 * 0x140u;
    } else {
      record = D_801EB2E8 + D_80146384 * 0x118u;
    }
    func_80196718(record);
    state = &D_80146384;
    func_800AF814(*state);
    func_800A9820(*state);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
