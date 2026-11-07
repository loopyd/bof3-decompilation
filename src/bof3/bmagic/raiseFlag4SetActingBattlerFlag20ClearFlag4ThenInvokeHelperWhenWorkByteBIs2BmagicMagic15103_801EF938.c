#include "bof3/bof3.h"

void func_801E5988(void);

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * the load (lui $v0,0x1F80 / lw $v0,68($v0)) and this function reads the cell
 * once. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle global halfword in main RAM, outside this overlay's image. The
 * exact sibling raiseFlag4AndInvokeHelperWhenWorkByteBZero
 * BmagicMagic13103_801EF61C raises the same bit 0x4 through a register-held
 * address. This target's generated undefined_syms_auto.txt carries the address
 * because the asm handlers func_801EF138 and func_801EF484 still reference it,
 * so no target-map row is added here. */
extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Acting battler byte in main RAM, outside this overlay's image. The original
 * reads it twice, once per flag-word access below, and the second read is
 * ordered after the first flag-word store instead of reusing the value the
 * first read left in a register; the volatile cell is what keeps those two
 * separate loads. The exact sibling
 * selectWorkRecordClearFlag40ThenAdvanceWorkByte1AndClearWorkBytes2And3
 * BmagicMagic15103_801EF2A0 indexes the parallel 0x140-byte local record array
 * with this same byte. */
extern volatile u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Flag word of the acting battler's 0x140-byte row in the local flag array at
 * 0x80145FB8, in main RAM outside this overlay's image. The row is 0x140 bytes,
 * so indexing this row type emits the original's sll 2 / addu / sll 6 scale and
 * its symbol-plus-offset load (lui $at,%hi / addu $at,$at,index / lw %lo($at)).
 * The exact battle lifts runSelectionRollSetRecordByte8OrFlag800ThenRefresh and
 * runSelectionPassSetSelectedFlag4000ClearByteThenCallA9BD8 describe the same
 * word at 0x80145FB8 + index*0x140. The word is volatile because the original
 * keeps the second battler read below the first store to it, which a plain word
 * view folds away together with the second row address. This target's generated
 * undefined_syms_auto.txt only carries the address while this span was asm, so
 * the target map binds it. */
typedef struct {
  volatile u32 flags; /* +0x00 */
  u8 unk_04[0x13C];
} Bmagic151ActingBattlerFlagRow;

extern Bmagic151ActingBattlerFlagRow D_80145FB8[]; /* @source 0x80145FB8 @kind table */

/* @source 0x801EF938
 * @behavior Work-byte-0xB step handler of the overlay work object. The
 *           scratchpad work-object pointer cell at 0x1F800044 is read once,
 *           before this function's 0x18-byte frame is created, and its byte
 *           0xB is compared against 2. Only on that match does the body raise
 *           bit 0x4 of the shared battle global halfword at 0x801462E8 through
 *           one register-held address, set bit 0x20 and then clear bit 0x4 of
 *           the acting battler's flag word at 0x80145FB8; each of the two flag
 *           accesses re-reads the acting battler byte D_80146374, so the row
 *           index is computed twice. The step then invokes the shared helper at
 *           0x801E5988. The 0x18-byte frame only keeps $ra across that call;
 *           nothing is returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4SetActingBattlerFlag20ClearFlag4ThenInvokeHelperWhenWorkByteBIs2BmagicMagic15103_801EF938(
    void) {
  u8 *work;
  u16 *flags;

  work = g_battle_work;
  if (work[0xB] == 2) {
    flags = (u16 *)&D_801462E8;
    *flags |= 4;
    D_80145FB8[D_80146374].flags |= 0x20;
    D_80145FB8[D_80146374].flags &= ~4;
    func_801E5988();
  }
}
