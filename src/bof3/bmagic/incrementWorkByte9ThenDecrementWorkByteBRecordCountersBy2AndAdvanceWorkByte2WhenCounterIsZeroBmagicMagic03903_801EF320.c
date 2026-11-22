#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The original re-materialises the cell address (lui $vN,0x1F80 / lw $vN,0x44)
 * before the work byte 9 update and again before each countdown group, because a
 * store through the loaded work pointer may alias the cell itself.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The 12-byte counter records selected by the work object's unsigned byte 0xB.
 * Their leading three bytes are the countdown triplet this handler subtracts 2
 * from, which Splat reaches as the three byte addresses 0x80145BD6, 0x80145BD7
 * and 0x80145BD8; the record stride is what folds those +1 and +2 offsets into
 * the lui %hi / lbu %lo pair. The fields are signed: the subtracts keep the
 * truncated byte (lbu / addiu -2 / sb) while the countdown test reads the same
 * field sign-extended (lb).
 */
typedef struct BmagicMagic03903CounterRecord {
  s8 counter_00;
  s8 counter_01;
  s8 counter_02;
  u8 unk_03[9];
} BmagicMagic03903CounterRecord;

extern BmagicMagic03903CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/* @source 0x801EF320
 * @behavior Per-frame step of this overlay's work byte 2 handler, entry 4 of the
 *           six-entry code pointer run at 0x801EEC10. Reads the scratchpad
 *           work-object pointer cell at 0x1F800044 and adds 1 to the work object
 *           byte at offset 0x9 with a byte-width read-modify-write (lbu /
 *           addiu 1 / sb); that stored value is not tested, so no truncation
 *           step follows it. It then subtracts 2 from the three countdown bytes
 *           of the 12-byte counter record selected by the work object's
 *           unsigned byte 0xB: each countdown group re-reads the cell and that
 *           work byte, scales the index by the record stride (idx*3*4) and
 *           decrements the selected record byte at offset 0x0/0x1/0x2 (lbu /
 *           addiu -2 / sb). Finally it re-reads the cell once more through $a0,
 *           reloads work byte 0xB and reads the record byte at offset 0x0
 *           signed; a non-zero value returns, and on zero it increments the
 *           work object byte at offset 0x2 through that same held pointer, which
 *           advances the work byte 2 dispatcher to its next handler step. Leaf
 *           with no frame, no saved registers and no calls; takes no arguments
 *           and returns nothing.
 *           Shape evidence: this body is this target's sibling
 *           incrementWorkByte9ThenAdd2ToWorkByteBRecordCountersAndAdvance
 *           WorkByte2WhenCounterIs10BmagicMagic03903_801EF1F4, measured exact
 *           over its own 300 bytes, with the +2 increments written as -2
 *           subtracts and the 0x10 compare written as a zero test; the signed
 *           record field that yields lb at the zero test is the spelling of the
 *           exact sibling decrementEffectRecordCounterTripletBy2ThenSubmitEffect
 *           IndexForwardBattleByteRaiseFlag4AndInvokeHelperWhenFirstCounterIsZero
 *           in magic075/03. No byte-identical twin of this 300-byte span exists
 *           among the payloads under out/binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenDecrementWorkByteBRecordCountersBy2AndAdvanceWorkByte2WhenCounterIsZeroBmagicMagic03903_801EF320(void) {
  g_battle_work[9]++;
  D_80145BD6[g_battle_work[0xB]].counter_00 -= 2;
  D_80145BD6[g_battle_work[0xB]].counter_01 -= 2;
  D_80145BD6[g_battle_work[0xB]].counter_02 -= 2;
  if (D_80145BD6[g_battle_work[0xB]].counter_00 == 0) {
    g_battle_work[2]++;
  }
}
