#include "bof3/bof3.h"

/* Shared sub-state byte of the main executable that this overlay's primary
 * state handlers switch on; this body walks it from 0 through 0xB. */
extern u8 D_80146875;
/* Shared scenario progress byte of the main executable: the claim arms wait for
 * it to read 3, 2 and 5, the context arms wait for it to read 0, and the walk
 * clears it twice. */
extern u8 g_ScenarioProgress;
/* Overlay-local byte that holds the claimed front-end record index for the rest
 * of the overlay, the last word of this payload at 0x801FBFEC. */
extern u8 D_801FBFEC;
/* Front-end record table of the main executable, declared once per record field
 * address the way the exact lifts of this same table spell it: each field the
 * body stores carries its own base, so every store re-forms its own address
 * from the scaled record index. The stride the index scales by is 0x74. */
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
/* Shared scenario flag word handed to the bit helper by value, the gate halfword
 * the two waiting arms poll, and the 16-bit countdown the timed arm walks. */
extern u32 D_8014686C;
extern u16 D_80143C40;
extern u16 D_80146876;
/* Shared mode-flag halfword: the last arm clears its bit 3. */
extern u16 D_80146258;
/* Shared front-end effect-bank byte, cleared on the way in and re-armed with
 * 0x1F on the way out. */
extern u8 D_8014832E;
/* Shared state byte the last arm clears, which idles the handler. */
extern u8 D_80146874;
/* Two shared front-end byte cells the context arms arm with 0xFF. */
extern u8 D_80143F1D;
extern u8 D_80143F1E;
extern u8 D_80143F1F;

void func_8015B580(u32 arg0, s32 bit);
void func_8015C058(void);
u8 func_8019601C(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801C1DF0(u32 arg0);
void func_801636A0(u32 arg0, u32 arg1);
u32 func_80163EA0(void);
void func_801C16B8(void);

/* @source 0x801F873C
 * @behavior Entry 2 of the nine-entry primary-state handler table at 0x801FBF54
 * that callPrimaryStateHandler3 (0x801F8474)
 * selects with the shared primary state byte D_80146874 and calls with no
 * arguments, and at the same time word 5 (the word stored at 0x801FBF5C) of the
 * twelve-entry scenario state-handler run at 0x801FBF48 that
 * callStateHandler3 (0x801F7D14) indexes
 * with the shared state byte D_80146872, so no in-image jal names it. It
 * switches on the shared sub-state byte D_80146875, read with lbu before its
 * 0x18-byte frame is created and bounded with sltiu against 0xC, through the
 * overlay-local twelve-word jump table at 0x801F6C1C in the payload leading
 * table data. Sub-state 0 requests front selector context seed 0xC2 with the
 * context words 0x130000 and 0x190000 and kind 1 through func_8019FA28 and
 * advances to 1. Sub-states 1, 3 and 5 wait for the progress byte to read 3, 2
 * and 5, claim a front-end record through func_8019601C, publish the returned
 * index in D_801FBFEC and, unless that index is 0xFF, seed the 0x74-byte row it
 * selects (+0 = 1, +5 = 0x75); sub-states 1 and 5 then queue the front-end cue
 * func_80161CD0(0x71, 0x64, 8) and advance to 2 and 6, while sub-state 3
 * advances to 4. Sub-states 2, 4 and 6 wait for the progress byte to read 0,
 * set bit 2, 3 and 4 of the scenario flag word D_8014686C through
 * func_8015B580, request front selector contexts 0xC2 with the words 0x3C8000,
 * 0xB0000 and kind 0x81, 0xC2 with 0x130000, 0x130000 and kind 3, and 0xC2 with
 * 0x3C8000, 0xB0000 and kind 0x83, arm D_80143F1E and D_80143F1F, D_80143F1F
 * alone, or no cell at all, with 0xFF, and advance to 3, 5 and 7. Sub-state 7
 * waits for the progress byte to read 1, sets the front-end local mode 0 through
 * func_8014ECAC, clears the progress byte and advances to 8. Sub-state 8 waits
 * for the gate halfword D_80143C40 to clear, arms the countdown D_80146876 with
 * 0x78, clears the effect-bank byte D_8014832E and advances to 9. Sub-state 9
 * decrements that countdown and, when it reaches zero, runs func_801C1DF0(3),
 * func_801636A0(0, 1), sets bit 5 of D_8014686C through func_8015B580 and
 * advances to 10. Sub-state 10, once the record-state query func_80163EA0
 * answers a nonzero low byte, runs func_801C16B8, requests front selector
 * context 0x97 with the words 0x330000 and 0x370000 and kind 1, arms D_80143F1D
 * and D_80143F1E with 0xFF and advances to 11. Sub-state 11 waits for the gate
 * halfword to clear, arms the effect bank with 0x1F, sets the local mode 0xB,
 * clears bit 3 of the mode-flag halfword D_80146258, releases the front end
 * through func_8015C058 and clears the progress byte and the sub-state byte,
 * which idles the handler. Any other sub-state value does nothing. It takes no
 * arguments, returns nothing and its frame keeps $ra and $s0.
 *
 * Name evidence: 0x801FBF54 + 2 * 4 = 0x801FBF5C holds this address as entry 2
 * of the nine-entry primary-state handler table, and the same table's entries 3,
 * 4, 5, 7 and 8 carry the canonical names
 * statePrimary3Substeps,
 * statePrimary4Substeps,
 * statePrimary5Substeps,
 * statePrimary7Substeps and
 * statePrimary8Substeps, so this entry
 * takes the same form at its own slot.
 *
 * Shape: statement order, operand forms, offset spellings and declaration style
 * come from the exact lift
 * claimSelectorContextBank
 * (0x801F84B8, this target, the neighbouring entry of the same table), whose
 * claim arm measured the same `sb` of the call result, the same `andi`-then-
 * compare against 0xFF and the same reload of D_801FBFEC for the second record
 * store; every literal above is written from this span's own bytes. The record
 * seed keeps the index in a local for its first store and reads D_801FBFEC again
 * before the +5 store, which is the store order this span shows. The progress
 * byte of sub-state 7 reaches the guard and the clear through one local pointer
 * because the original keeps that address in $s0 across func_8014ECAC, where
 * every other cell re-forms its own base with a fresh lui. The countdown of
 * sub-state 9 reaches its load and its store through one local pointer for the
 * same reason: the direct-symbol spelling of `if (--D_80146876 != 0u)` measured
 * 308 of 312 instructions at unchanged size, with `lui`/`lhu` through %lo plus a
 * fresh `lui at` for the store in place of the original `lui`/`addiu` base held
 * in $v1, and the pointer spelling measured 312 of 312.
 *
 * Clone declined: a masked scan of this 0x4E0-byte body over the 398 payloads
 * under out/binaries, with every `lui`, `j`/`jal` and `$at`-based access masked,
 * matched this span alone, so no byte-identical twin exists and the tables and
 * siblings above are naming and shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePrimary2Substeps(void) {
  u8 index;

  switch (D_80146875) {
  case 0:
    func_8019FA28(0xc2u, 0x130000u, 0x190000u, 1u);
    D_80146875 = 1u;
    break;

  case 1:
    if (g_ScenarioProgress != 3u) {
      break;
    }

    index = func_8019601C();
    D_801FBFEC = index;

    if (index != 0xffu) {
      D_80143FC8[(u32)index * 0x74u] = 1u;
      D_80143FCD[(u32)D_801FBFEC * 0x74u] = 0x75u;
    }

    func_80161CD0(0x71u, 0x64, 8);
    D_80146875 = 2u;
    break;

  case 2:
    if (g_ScenarioProgress != 0u) {
      break;
    }

    func_8015B580(D_8014686C, 2);
    func_8019FA28(0xc2u, 0x3c8000u, 0xb0000u, 0x81u);
    D_80143F1E = 0xffu;
    D_80143F1F = 0xffu;
    D_80146875 = 3u;
    break;

  case 3:
    if (g_ScenarioProgress != 2u) {
      break;
    }

    index = func_8019601C();
    D_801FBFEC = index;

    if (index != 0xffu) {
      D_80143FC8[(u32)index * 0x74u] = 1u;
      D_80143FCD[(u32)D_801FBFEC * 0x74u] = 0x75u;
    }

    D_80146875 = 4u;
    break;

  case 4:
    if (g_ScenarioProgress != 0u) {
      break;
    }

    func_8015B580(D_8014686C, 3);
    func_8019FA28(0xc2u, 0x130000u, 0x130000u, 3u);
    D_80146875 = 5u;
    break;

  case 5:
    if (g_ScenarioProgress != 5u) {
      break;
    }

    index = func_8019601C();
    D_801FBFEC = index;

    if (index != 0xffu) {
      D_80143FC8[(u32)index * 0x74u] = 1u;
      D_80143FCD[(u32)D_801FBFEC * 0x74u] = 0x75u;
    }

    func_80161CD0(0x71u, 0x64, 8);
    D_80146875 = 6u;
    break;

  case 6:
    if (g_ScenarioProgress != 0u) {
      break;
    }

    func_8015B580(D_8014686C, 4);
    func_8019FA28(0xc2u, 0x3c8000u, 0xb0000u, 0x83u);
    D_80143F1F = 0xffu;
    D_80146875 = 7u;
    break;

  case 7: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 1u) {
      break;
    }

    func_8014ECAC(0u);
    *progress = 0u;
    D_80146875 = 8u;
    break;
  }

  case 8:
    if (D_80143C40 != 0u) {
      break;
    }

    D_80146876 = 0x78u;
    D_8014832E = 0u;
    D_80146875 = 9u;
    break;

  case 9: {
    u16 *countdown = &D_80146876;

    if (--*countdown != 0u) {
      break;
    }

    func_801C1DF0(3u);
    func_801636A0(0u, 1u);
    func_8015B580(D_8014686C, 5);
    D_80146875 = 0xau;
    break;
  }

  case 10:
    if ((func_80163EA0() & 0xffu) == 0u) {
      break;
    }

    func_801C16B8();
    func_8019FA28(0x97u, 0x330000u, 0x370000u, 1u);
    D_80143F1D = 0xffu;
    D_80143F1E = 0xffu;
    D_80146875 = 0xbu;
    break;

  case 11:
    if (D_80143C40 != 0u) {
      break;
    }

    D_8014832E = 0x1fu;
    func_8014ECAC(0xbu);
    D_80146258 = (u16)(D_80146258 & 0xfff7u);
    func_8015C058();
    D_80146874 = 0u;
    D_80146875 = 0u;
    break;
  }
}
