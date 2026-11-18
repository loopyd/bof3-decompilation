#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materializes the cell address (lui $v0,0x1F80 / lw $v0,0x44)
 * before each decrement group and before the zero test, so the cell is read once
 * per access.
 * @source 0x1F800044 @kind data
 */
extern u8 *g_battle_work;

/* The 12-byte positional effect records based at 0x80145BD4 carry the countdown
 * triplet this handler decrements in their bytes +0x2/+0x3/+0x4, which Splat
 * reaches as the three separate words 0x80145BD6, 0x80145BD7 and 0x80145BD8.
 * The fields are signed: the decrements keep the truncated byte (lbu / addiu -2 /
 * sb) while the countdown test reads the same field sign-extended (lb).
 * @source 0x80145BD6 @kind table
 */
typedef struct BmagicMagic07503CounterRecord {
  s8 counter_00;
  s8 counter_01;
  s8 counter_02;
  u8 unk_03[9];
} BmagicMagic07503CounterRecord;

extern BmagicMagic07503CounterRecord D_80145BD6[];

/* Shared battle global halfword at 0x801462E8 and shared battle target byte at
 * 0x80146384. Both live outside this overlay, so they keep their
 * address-qualified names, and this target's generated undefined-symbol table
 * already binds both addresses.
 */
extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

extern void func_80196670(u8 kind); /* @source 0x80196670 */
extern void func_800A9820(u8 target); /* @source 0x800A9820 */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* @source 0x801EEEB0
 * @behavior Per-frame effect step reached as entry 2 of the three-word code
 *           pointer run at 0x801EEC04 (0x801EEC70, 0x801EEDE0, 0x801EEEB0), which
 *           the overlay entry dispatchWorkByte1HandlerBmagicMagic07503_801EEC10
 *           copies into its own frame and indexes with the scratchpad work
 *           object's unsigned byte 0x01. It subtracts two from the +0x2, +0x3
 *           and +0x4 countdown bytes of the 12-byte effect record selected by the
 *           work object's unsigned byte 0xA (the index is scaled by the record
 *           stride as idx*3*4), re-reading the scratchpad cell at 0x1F800044 and
 *           the index byte before every store. It then re-reads the cell and the
 *           index byte once and reads the record's first countdown byte signed; a
 *           non-zero value returns. On zero it ends the effect: submits that same
 *           work byte 0xA to the shared helper func_80196670, forwards the shared
 *           battle byte at 0x80146384 to func_800A9820, raises bit 0x4 of the
 *           shared battle halfword at 0x801462E8 and invokes func_801E5988 with
 *           no arguments. The 0x18-byte frame only keeps $ra at 0x10 across the
 *           three calls. Takes no arguments and returns nothing.
 *           Shape evidence: the same record triplet is written by the exact
 *           sibling storeDoubleWorkByteBIntoWorkByteARecordTripletAndAdvance
 *           WorkByte1WhenWorkByteBIs7BmagicMagic07503_801EEDE0 in this target and
 *           the same flag-4/forward/invoke tail belongs to the exact sibling
 *           raiseFlag4ThenForwardBattleByteAndInvokeHelperBmagicMagic05703
 *           _801EEFEC; no byte-identical twin of this 0x144-byte span exists in
 *           the 398 payloads under out/binaries, so it was seeded from the
 *           original bytes. The array-of-records spelling was the closing shape:
 *           a flat u8 array indexed by idx*12 measured 41 of 81 instructions in
 *           296 bytes because the decrement collapsed into a single pointer
 *           address, while the struct stride reproduced the two per-reference
 *           lui $at / addu $at address forms and matched every byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementEffectRecordCounterTripletBy2ThenSubmitEffectIndexForwardBattleByteRaiseFlag4AndInvokeHelperWhenFirstCounterIsZeroBmagicMagic07503_801EEEB0(void) {
  D_80145BD6[g_battle_work[0xA]].counter_00 -= 2;
  D_80145BD6[g_battle_work[0xA]].counter_01 -= 2;
  D_80145BD6[g_battle_work[0xA]].counter_02 -= 2;
  if (D_80145BD6[g_battle_work[0xA]].counter_00 == 0) {
    func_80196670(g_battle_work[0xA]);
    func_800A9820(D_80146384);
    D_801462E8 |= 4;
    func_801E5988();
  }
}
