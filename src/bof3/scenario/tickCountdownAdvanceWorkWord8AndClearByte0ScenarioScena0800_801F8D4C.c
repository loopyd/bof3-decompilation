#include "bof3/bof3.h"

/* @source 0x801F8D4C
 * @behavior Overlay per-frame handler: it decrements the unsigned countdown
 * byte at offset 2 of the work object it receives in $a0 and re-stores it,
 * advances the object byte at offset 3 by 6, adds the fixed-point step 0x40000
 * to the object word at offset 8 and folds the resulting word at offset 8 into
 * the object word at offset 0x14, and only when the decremented countdown reads
 * zero does it clear the object byte at offset 0. It reads and writes no other
 * memory, makes no call and keeps no register, so its 0x4C bytes carry no frame
 * and no saved $ra.
 *
 * Name evidence: "tickCountdown" is the family wording this target's accepted
 * exact sibling tickCountdownAndAdvanceStateScenarioScena0800_801F8D18 uses for
 * the same `countdown = (u8)(object[2] - 1); object[2] = countdown;` chain
 * followed by a zero test of the masked byte; "AdvanceWorkWord8" names the
 * evidenced `+= 0x40000` step on the word at offset 8 without claiming a role
 * for that word, and "ClearByte0" the evidenced byte-0 clear that the countdown
 * gates. The word-8 value is also folded into the word at offset 0x14, which the
 * behavior note records; the name keeps the offset-anchored wording this repository
 * uses where a field role is not evidenced (WorkByte1, WorkTint, WorkFlags).
 *
 * Table evidence: the address is the pointer word at 0x801FE904, entry 28 of
 * this overlay's 51-entry per-frame handler run at 0x801FE894 and entry 8 of the
 * sub-run at 0x801FE8E4 that func_801F7AD0 indexes with the work object's byte
 * 2, so it is a handler of the same run as its two exact siblings above. No
 * in-image jal targets the address, so the table word is its only entry.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the decisive clean-C lever is statement order, not a compiler
 * control. `*(u32*)(object + 0x14) += *(u32*)(object + 8);` must directly follow
 * the word-8 store `*(u32*)(object + 8) += 0x40000;` so that the load of word
 * 8 is forwarded from the register the store left and never reloaded; with the
 * byte-3 statement between them the emitted stream gained a second `lw` of
 * 8($a2) and one instruction (20 vs 19, 4/19 matching, first= `lui v0,0x4` vs
 * `lui v1,0x4`). Moving only the byte-3 statement after the fold reached 19/19
 * with no other change.
 */
void tickCountdownAdvanceWorkWord8AndClearByte0ScenarioScena0800_801F8D4C(u8* object) {
  u8 countdown;

  countdown = (u8)(object[2] - 1);
  object[2] = countdown;
  *(u32*)(object + 8) += 0x40000;
  *(u32*)(object + 0x14) += *(u32*)(object + 8);
  object[3] = (u8)(object[3] + 6);
  if (countdown == 0) {
    object[0] = 0;
  }
}
