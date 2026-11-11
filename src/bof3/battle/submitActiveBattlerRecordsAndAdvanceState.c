#include "bof3/battle/battle03_internal.h"

/* Shared battle hint byte whose low seven bits select the EMI stream slot that
 * func_8016728C starts; it lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80145024;

/* Panel/route selector outside this overlay: it draws one local work record
 * from the record byte +0x4B, the halfword +0x58 and the 0x800F0800 palette
 * row, the same call shape advanceWorkRoutesOrInitializeTint uses. */
extern void func_8014D664(u8 arg0, u32 arg1, u32 arg2, u32 arg3);

/* EMI stream slot starter keyed by the hint byte: family bases 0x1DB and 0x1EE
 * per startHintVariantEmiStreamAndAdvanceByte1. */
extern void func_8016728C(u32 slot_id, u32 mode);

/* @source 0x801D6F28
 * @behavior Returns while any of the localBattlerCount 0x140-byte local work
 * record flag words at D_80145FB4 still carries the queued-slot flag 0x2000.
 * When the D_801462E5 queue cursor differs from that count the battle state
 * byte BATTLE_GLOBAL_BYTE_62E2 steps back by one and the body returns;
 * otherwise it resets the layout bank through func_80161808, submits every
 * active record with the 0xBF5F-masked status halfword at D_80145F10 through
 * func_800A36F0 and func_80196718, publishes each record whose flag byte
 * carries bit 0 into the scratchpad work pointer and draws its panel through
 * func_8014D664 with the 0x800F0800 row and the 0x1800 flags, starts the EMI
 * stream slot selected by the low seven bits of D_80145024 through
 * func_8016728C, and then advances the battle state byte
 * BATTLE_GLOBAL_BYTE_62E2.
 * @status partial
 * @match 98.33
 * @residual 118/120 instructions at the original 480 bytes. The first live
 * difference is +0x00B4, the loop-2 entry guard: the build branches to the
 * loop-3 count reload at 0x801D7034 where the original threads the branch past
 * that load to the shared count test at 0x801D703C. The only other difference
 * is the record-pointer addu, scheduled between the lui and ori halves of the
 * 0x800F0800 constant instead of after both. Both are jump-threading and
 * scheduling placement: the guard, loop and address spellings below all
 * measured identical, so the residual carries no source-shape evidence.
 */
/* Matching note: the cursor test reads the battler count through the plain
 * `*(u8*)&` view, not the header's volatile cell. With the volatile spelling
 * the two compare loads materialise in the opposite order (the volatile load
 * first) and the first gate measured 109/120; the plain pair measured
 * 118/120. Each scan bound stays on the named volatile cell, so the loop
 * back-edge reloads the count once per iteration as the original does. */
void submitActiveBattlerRecordsAndAdvanceState(void) {
  volatile u8* state;
  u8  count;
  u8  index;
  u16 half;
  u8* record;

  /* the original caches this count byte once for the first scan; the header's
   * volatile view would force a per-iteration reload, so read it through a
   * plain u8 view. */
  count = *(u8*)&localBattlerCount;
  index = 0;
  if (count != 0) {
    do {
      if ((D_80145FB4[index].flags_00 & 0x2000u) != 0u) {
        return;
      }
      index += 1;
    } while (index < count);
  }
  if (D_801462E5 != *(u8*)&localBattlerCount) {
    BATTLE_GLOBAL_BYTE_62E2 -= 1;
  } else {
    func_80161808(0);
    for (index = 0; index < localBattlerCount; index++) {
      u32 offset = (u32)index * 0x140u;

      func_800A36F0(index, *(volatile u16*)(D_80145F10 + offset) & 0xBF5Fu);
      func_80196718(&D_80145E90[index]);
    }
    for (index = 0; index < localBattlerCount; index++) {
      u32 offset = (u32)index * 0x140u;

      if ((D_80145E90[index].flags_00 & 1u) != 0u) {
        record = (u8*)D_80145E90 + offset;
        half = *(u16*)(record + 0x58);
        battleWork = record;
        func_8014D664(record[0x4B], (u16)(half - 2), 0x800F0800, 0x1800);
      }
    }
    func_8016728C(D_80145024 & 0x7Fu, 0);
    state = &BATTLE_GLOBAL_BYTE_62E2;
    *state += 1;
  }
}
