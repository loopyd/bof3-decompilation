#include "bof3/bof3.h"

/* Shared sub-state byte of the main executable that this overlay's scenario
 * state handlers switch on; this body drives it from 0 through 5. */
extern u8 D_80146875;
/* Shared progress byte of the main executable: this body reads it and clears it
 * with a byte store. */
extern u32 g_ScenarioProgress;
/* Shared gate word of the main executable, waited out by two of the sub-states
 * below. */
extern u16 D_80143C40;
/* Shared front-end phase byte of the main executable: the row it is written with
 * here is 6, and two sub-states below wait for the main loop to clear it. */
extern u8 D_80143BB0;
/* Shared front-end effect-bank byte of the main executable, cleared on the way
 * into the front end and re-armed with 0x1F on the way out. */
extern u8 D_8014832E;
/* Two shared byte cells the window seed of sub-state 3 arms with 0xFF. */
extern u8 D_80143F1D;
extern u8 D_80143F1E;
/* Shared mode-flag halfword of the main executable: this body clears its bit 3. */
extern u16 D_80146258;
/* Shared state byte the completion arm clears, which idles the handler. */
extern u8 D_80146874;
/* Shared scenario flag word of the main executable, handed to the bit helper by
 * value. */
extern u32 D_8014686C;
/* Signed shared channel word the record seed copies into the row word +0x68. */
extern u16 D_801492DA;

/* Overlay-local front-end record table of the main executable: 20 records of
 * 0x74 bytes. The seed below writes one row, and the original materialises
 * `lui at,0x8014` per statement, so each field keeps its own extern base. */
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];
/* Overlay-local byte that holds the claimed record index for the rest of the
 * overlay, the last word of this payload at 0x801FBFEC. */
extern u8 D_801FBFEC;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
u8 func_8019601C(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F84B8
 * @behavior Front-end departure sequence of this overlay, word 4 of the
 * twelve-word scenario state-handler run at 0x801FBF48 that
 * dispatchScenarioStateHandlerScenarioScena1300_801F7D14 (0x801F7D14) indexes
 * with the shared scenario state byte D_80146872, and at the same time word 1
 * (0x801FBF58) of the nine-entry primary-state table D_801FBF54 that
 * dispatchPrimaryStateHandlerScenarioScena1300_801F8474 (0x801F8474) indexes
 * with the progress byte D_80146874, so no `jal` of the payload names it. It
 * switches on the shared sub-state byte D_80146875 (read with `lbu`, bounded by
 * `sltiu` against 6) through the overlay-local six-word jump table at
 * 0x801F6C04 in the payload leading table data.
 *
 * Sub-state 0 claims a front-end record through the shared claim func_8019601C,
 * publishes the returned index in D_801FBFEC and, unless that index is 0xFF,
 * seeds the 0x74-byte row it selects: +0 = 1, +5 = 0x13, the word +0x64
 * = -0x2AA, the word +0x6C = 0x200, +9 = 0x80 and the word +0x68 = the signed
 * shared channel word D_801492DA, then advances to 1. Sub-state 1 waits for the
 * progress byte to read 8, sets the shared front-end local mode 0 through
 * func_8014ECAC and advances to 2. Sub-state 2 waits for the gate word
 * D_80143C40 to clear, clears the effect-bank byte D_8014832E, yields to the
 * loader through func_8014B87C(1), arms the front-end phase byte D_80143BB0
 * with 6 and advances to 3. Sub-state 3 waits for that phase byte to clear,
 * clears the progress byte, sets bit 0 of the flag word D_8014686C through
 * func_8015B580, seeds the front selector context (0x7E, 0x4E0000, 0x2C8000, 1)
 * through func_8019FA28, arms D_80143F1D and D_80143F1E with 0xFF and advances
 * to 4. Sub-state 4 waits for the phase byte to clear, arms the effect bank
 * with 0x1F, sets the local mode 1 through func_8014ECAC and advances to 5.
 * Sub-state 5 waits for the gate word to clear, clears bit 3 of the mode-flag
 * halfword D_80146258, releases the front end through func_8015C058 and returns
 * the handler to its idle entry by clearing both the progress byte and the
 * sub-state byte. Any other sub-state value does nothing. It takes no
 * arguments, returns nothing and its 0x18-byte frame keeps only $ra.
 *
 * Name evidence: the claimed row is the 0x74-byte front-end record table
 * D_80143FC8 that the exact siblings func_801F9868 (0x801F9868,
 * emi/scenario/scena00/00) and
 * claimFrontEndRecordPublishWorkPointerAndResetStateScenarioScena1100_801F82D8
 * (0x801F82D8, emi/scenario/scena11/00) both claim through func_8019601C and
 * seed with the same fields; func_8019FA28 is the shared front-selector context
 * seeder of the exact lift func_8019FA28 (0x8019FA28, ui) that also owns the
 * D_80143BB0 phase-byte role; D_8014832E is the front-end effect bank the exact
 * lift bootScenarioStateScenarioScena1300_801F7D50 (0x801F7D50, this target)
 * arms with 0x1F; and func_8015C058 is the shared front-end release helper the
 * exact sibling
 * queueFrontEndMode6Or7ThenReleaseFrontEndOnSubstate33ScenarioScena1200_801F894C
 * (0x801F894C, emi/scenario/scena12/00) runs in its own release arm beside the
 * same two cleared state bytes.
 *
 * Shape: statement order, operand forms, offset spellings and declaration style
 * come from those three exact scenario row seeders and release arms, with this
 * span's own literals - the phase 6, the record fields 0x13, -0x2AA, 0x200,
 * 0x80 and the window 0x7E/0x4E0000/0x2C8000/1 - written from its bytes. The
 * per-statement extern base per record field is the spelling the exact sibling
 * func_801F7790 (0x801F7790, emi/scenario/scena16/00) records as the form that
 * keeps the original per-statement `lui at,0x8014`. The record seed keeps the
 * index in a local for its first store and reads D_801FBFEC again before the
 * 0x13 store and once more before the four stores that share one scaled index,
 * and its own statement order writes the channel-word store before the +0x6C
 * and +9 stores: the seed-order form measured 155 of 161 instructions at
 * unchanged size with the constant 1 and the channel word in each other's
 * registers, and this order measured 161 of 161.
 *
 * Clone declined: a scan of the 0x284-byte body over the 396 payloads under
 * out/binaries with every `lui`, `j`/`jal` and `$at`-based load, store and
 * immediate word masked matched this span alone, so no byte-identical or
 * relocation-different twin exists and the siblings above are shape evidence.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordWaitProgress8ArmFrontEndPhase6SeedSelectorContextArmEffectBankAndResetHandlerScenarioScena1300_801F84B8(void) {
  u8 index;
  u8 row;

  switch (D_80146875) {
  case 0:
    index = func_8019601C();
    D_801FBFEC = index;

    if (index == 0xffu) {
      break;
    }

    D_80143FC8[(u32)index * 0x74u] = 1u;
    D_80143FCD[(u32)D_801FBFEC * 0x74u] = 0x13u;
    row = D_801FBFEC;
    *(s32 *)(D_8014402C + (u32)row * 0x74u) = -0x2aa;
    *(s32 *)(D_80144030 + (u32)row * 0x74u) = (s32)(s16)D_801492DA;
    *(s32 *)(D_80144034 + (u32)row * 0x74u) = 0x200;
    D_80143FD1[(u32)row * 0x74u] = 0x80u;
    D_80146875 = 1u;
    break;

  case 1:
    if (*(u8 *)&g_ScenarioProgress != 8u) {
      break;
    }

    func_8014ECAC(0u);
    D_80146875 = 2u;
    break;

  case 2:
    if (D_80143C40 != 0u) {
      break;
    }

    D_8014832E = 0u;
    func_8014B87C(1u);
    D_80143BB0 = 6u;
    D_80146875 = 3u;
    break;

  case 3:
    if (D_80143BB0 != 0u) {
      break;
    }

    *(u8 *)&g_ScenarioProgress = 0;
    func_8015B580(D_8014686C, 0);
    func_8019FA28(0x7eu, 0x4e0000u, 0x2c8000u, 1u);
    D_80143F1D = 0xffu;
    D_80143F1E = 0xffu;
    D_80146875 = 4u;
    break;

  case 4:
    if (D_80143BB0 != 0u) {
      break;
    }

    D_8014832E = 0x1fu;
    func_8014ECAC(1u);
    D_80146875 = 5u;
    break;

  case 5:
    if (D_80143C40 != 0u) {
      break;
    }

    D_80146258 = (u16)(D_80146258 & 0xfff7u);
    func_8015C058();
    D_80146874 = 0u;
    D_80146875 = 0u;
    break;
  }
}
