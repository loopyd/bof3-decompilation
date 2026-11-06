#include "bof3/bof3.h"

/* Main-RAM ten-slot skill list at offset +0x7A of the 164-byte field records
 * (record base 0x80144968); emi/etc/shop/00 declares the same address as its
 * ten-slot list of skill ids.
 * @source 0x801449E2 @kind table */
extern u8 D_801449E2[];

/* Shared front-end ability helper of the EXE; emi/etc/sisyou/00 declares the same
 * address for the gate func_801D3A24 tests. Its byte result is unused here. */
void func_801651DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3); /* @source 0x801651DC */

/* @source 0x801FD6FC
 * @behavior Walks the ten slots of the skill list D_801449E2 that sits at offset
 * +0x7A of the 164-byte field record selected by the argument: for every
 * non-zero slot it passes the recorded skill id to the shared front-end ability
 * helper func_801651DC(skill, 0, 1, 0) and then clears that slot, so each
 * recorded skill is checked once and consumed. The argument is masked to a byte
 * before it is scaled by the record stride, and the u32 loop counter is compared
 * through its low byte, which is why the latch zero-extends it with `andi` before
 * the unsigned `sltiu` 10. Its 0x20-byte frame only keeps $s0 and $s1 with $ra,
 * and the two stores of those registers are scheduled around the scaling chain.
 * No image word names 0x801FD6FC: the overlay callback table at 0x801FE304 ends
 * at 0x801FE378 without it, so the shipped payload never reaches this function
 * through a jal or a pointer run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSkillListEntriesAfterAbilityCheckScenarioScena0200_801FD6FC(
    s32 record_index) {
  u8 *entry;
  u32 index;

  entry = D_801449E2 + (u8)record_index * 164;
  index = 0;
  do {
    if (*entry != 0) {
      func_801651DC(*entry, 0, 1, 0);
      *entry = 0;
    }
    index++;
    entry++;
  } while ((u8)index < 10);
}
