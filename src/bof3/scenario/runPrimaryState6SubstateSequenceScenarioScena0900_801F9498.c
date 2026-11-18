#include "bof3/bof3.h"

/* Shared sub-state byte at 0x80146875: this body switches on it and every arm
 * that advances the sequence stores it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared primary state byte at 0x80146874: the closing tail clears it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared scenario progress byte at 0x80146864: most arms gate on it, and the
 * arms that leave the sequence clear it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte at 0x80143BB0: the action arms arm it to 2 and
 * the follow-up arms leave the state once it reads 2.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared character bytes at 0x80146865 through 0x80146867; the third is also
 * the front-end record index the two claim arms publish.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared scenario flag word at 0x8014686C, read with lw: the arms that set a
 * scenario flag pass it to the bit-set helper with the bit number.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared effect-cue byte at 0x8014933B: the arms before a flag-set and cue
 * arm it to 1 through 4.
 * @source 0x8014933B @kind unknown */
extern u8 D_8014933B;

/* Shared effect-bank byte at 0x8014832E: the gate arms clear it, the cue arms
 * arm it to 0x1F.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared gate halfword at 0x80143C40, read with lhu: every arm that seeds a
 * front selector context waits for it to read zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Front-end record table base at 0x80143FC8 and the field bases of its
 * 0x74-byte rows: the original re-materialises `lui at,0x8014` for every store,
 * so each field is addressed as its own absolute symbol rather than through one
 * struct base.
 * @source 0x80143FC8 @kind unknown */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind unknown */
extern u8 D_80143FD1[];
/* @source 0x8014402C @kind unknown */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind unknown */
extern u8 D_80144030[];
/* @source 0x80144034 @kind unknown */
extern u8 D_80144034[];

/* Shared actor halfword at 0x801492DA, read with lh: the record claim arms copy
 * it into the claimed row's +0x68 word.
 * @source 0x801492DA @kind unknown */
extern u16 D_801492DA;

/* Scratchpad byte 0 of the overlay at 0x1F800000: both claim arms publish the
 * index the record allocator answers here and read it back for every store.
 * @source 0x1F800000 @kind unknown */
extern u8 D_1F800000;

/* Shared front-end action helper at 0x80150224: the sub-states 0, 2, 3, 5, 6,
 * 7, 0x32 and 0x64 request actions through it.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* Shared scenario flag-word bit-set helper at 0x8015B580: the arms at 0x0A,
 * 0x18, 0x23, 0x2D and 0x38 pass it the shared flag word and a bit number.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared overlay release helper at 0x8015C058: every arm that ends the sequence
 * runs it before the state bytes are cleared.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared window opener at 0x801BE1B0: sub-states 0x14, 0x1E, 0x28 and the
 * progress 0x0A branch of 0x33 run it.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared front-end request setter at 0x8019FA28: the arms that seed a front
 * selector context pass it a selection seed, two context words and a kind.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared record allocator at 0x8019601C: both claim arms store its answer in
 * scratchpad byte 0 before indexing the 0x74-byte record run with it.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared helper at 0x801C16B8: the progress 0x64 branch of sub-state 0x65 runs
 * it between the mode setter and the scripted-motion start.
 * @source 0x801C16B8 @kind function */
extern void func_801C16B8(void);

/* Shared front-end scripted-motion starter at 0x8015D554: the progress 0x64
 * branch of sub-state 0x65 runs it with (0, 0, 4, 0x1E).
 * @source 0x8015D554 @kind function */
extern void func_8015D554(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* Shared two-argument controller-step helper at 0x801636A0: sub-state 0x66 runs
 * it with (0, 1).
 * @source 0x801636A0 @kind function */
extern void func_801636A0(u32 arg0, u32 arg1);

/* Shared record-state query at 0x80163EA0: sub-state 0x67 advances only when
 * its low byte reads non-zero, so the read is masked with 0xFF.
 * @source 0x80163EA0 @kind function */
extern u32 func_80163EA0(void);

/* @source 0x801F9498
 * @behavior Word 6 (the word stored at 0x801FE404) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801F9498 and that one table word is
 * its only reference in this payload. It switches on the shared sub-state byte
 * D_80146875 through the hundred-and-six-entry absolute jump table at
 * 0x801F6D5C, an overlay-local table in this payload's leading tabledata blob,
 * reading that byte before it creates its 0x18-byte frame, which keeps only $ra
 * and $s0. Sub-states 0, 2, 3, 5 and 6 request front-end actions 0x0A, 0x0B,
 * 0x0D, 0x0C and 0x0F through the shared action helper func_80150224, arm the
 * shared phase byte D_80143BB0 (0x80143BB0) to 2 and advance the sub-state byte
 * to 1; sub-state 1, once that phase byte has left 2, runs the shared release
 * helper func_8015C058 and ends the state through the shared clearing tail that
 * zeroes the sub-state byte and the primary state byte D_80146874.
 * Sub-state 7 requests action 0x2B, arms the phase byte and advances to 8;
 * sub-state 8 waits on that phase byte and then advances to 9 once the shared
 * scenario progress byte g_ScenarioProgress (0x80146864) reads 0x0A, or to 0x0A
 * once it reads 0x14; 0x0A sets scenario flag bit 0x11 of the shared flag word
 * D_8014686C (0x8014686C, read with lw) through func_8015B580, runs the release
 * helper, clears the progress byte and the shared character bytes D_80146865,
 * D_80146866 and D_80146867 and ends the state.
 * The 0x14 to 0x19 band seeds front selector contexts: 0x14 opens window 3 and
 * advances, 0x15, 0x16 and 0x17 wait for progress 1, 2 and 0x0F before seeding
 * the contexts (0x75, 0x38000, 0x158000, 0x84), (0x75, 0x560000, 0x2E0000,
 * 0x85), (0x75, 0x38000, 0x150000, 0x86) and advancing to 0x16, 0x17 and
 * 0x18, 0x18 clears the progress byte, arms the effect-cue byte D_8014933B
 * (0x8014933B) to 1 and sets flag bit 0x0C, 0x19 waits for progress 1 and then
 * clears the progress byte and seeds (0x75, 0x560000, 0x2E0000, 0x87), and
 * 0x1A releases the overlay and clears the three character bytes once progress
 * reads 4. The 0x1E to 0x25 band claims front-end records: 0x1E runs
 * func_801BE1B0(8), 0x1F waits for progress 1 then runs func_8014ECAC(0) and
 * advances to 0x20, 0x20 waits for the gate halfword D_80143C40 (0x80143C40,
 * read with lhu) to read zero, clears the effect-bank byte D_8014832E and seeds
 * (0x76, 0x70000, 0x118000, 0x83), 0x21 stores the index the shared allocator
 * func_8019601C answers in scratchpad byte 0 of D_1F800000 (0x1F800000) and,
 * unless it reads 0xFF, seeds that index's 0x74-byte record row at 0x80143FC8
 * (+0 = 1, +5 = 0x13, +0x64 = -0x3B2, +0x68 = the s16 halfword D_801492DA
 * (0x801492DA), +0x6C = 0x38C, +9 = 4), then advances to 0x22 with the index
 * published in D_80146867; 0x22 clears that index and the progress byte and,
 * once the row's leading byte reads zero, sets progress 1, runs
 * func_8014ECAC(5), arms the effect bank to 0x1F and advances to 0x23; 0x23
 * waits on the gate halfword, arms the effect cue to 2 and sets flag bit 0x0D;
 * 0x24 waits for progress 2, clears it and seeds (0x75, 0x560000, 0x2B0000,
 * 0x89) after arming the effect bank; 0x25 releases the overlay and clears the
 * character bytes once progress reads 9.
 * The 0x28 to 0x2F band repeats that shape: 0x28 runs func_801BE1B0(0x0A),
 * 0x29 waits for progress 1 then runs func_8014ECAC(4), 0x2A waits on the gate
 * halfword, clears the effect bank and seeds (0x76, 0x88000, 0x78000, 0x83),
 * 0x2B claims a second record row through the same allocator, publishing
 * +0x64 = -0x2AA, +0x68 = the s16 halfword D_801492DA, +0x6C = 0x31E, +0 = 1,
 * +5 = 0x13 and +9 = 4 in the row it indexes from scratchpad byte 0 before
 * advancing to 0x2C, 0x2C clears that index and, once the row's leading byte
 * reads zero, sets progress 1, runs func_8014ECAC(5), arms the effect bank and
 * advances to 0x2D, 0x2D waits on the gate halfword, arms the effect cue to 3
 * and sets flag bit 0x0F, 0x2E waits for progress 2, clears it and seeds
 * (0x75, 0x530000, 0x300000, 0x8B), and 0x2F releases the overlay once progress
 * reads 8.
 * The 0x32 to 0x3A band queues the end of the front-end run: 0x32 requests
 * action 0x2B, arms the phase byte and advances to 0x33; 0x33 waits on the
 * phase byte and then, on progress 0x0A, runs func_801BE1B0(0x0C) and advances
 * to 0x34, or on progress 0x14 advances to 0x35; 0x34 releases the overlay once
 * progress reads 1; 0x35 runs func_8014ECAC(0) and advances to 0x36; 0x36 waits
 * on the gate halfword, stores progress 1, clears the effect bank and seeds
 * (0x76, 0x78000, 0x38000, 0x83); 0x37 runs func_8014ECAC(5), arms the effect
 * bank to 0x1F and advances to 0x38; 0x38 waits on the gate halfword, arms the
 * effect cue to 4 and sets flag bits 0x11 and 0x1D; 0x39 waits for progress 2,
 * clears it and seeds (0x75, 0x530000, 0x290000, 0x8D); 0x3A releases the
 * overlay once progress reads 7.
 * The 0x64 to 0x69 band ends the whole primary state: 0x64 waits on the phase
 * byte, requests action 0x1B, arms the phase byte to 2 and advances to 0x65;
 * 0x65, once progress reads 0x64, clears it, advances to 0x66, runs
 * func_8014ECAC(0x0D), func_801C16B8 and func_8015D554(0, 0, 4, 0x1E), or once
 * progress reads 0x65 clears it and releases the overlay; 0x66 runs
 * func_801636A0(0, 1) and advances to 0x67; 0x67 waits on the gate halfword,
 * clears the effect bank and advances to 0x68 while the low byte
 * func_80163EA0 answers is non-zero; 0x68 runs func_8014ECAC(0x0E), arms the
 * effect bank to 0x1F and advances to 0x69; 0x69 waits on the gate halfword,
 * runs func_80161C20(0x61, 0x64, 8) and then the release helper before the
 * clearing tail ends the state. Every other sub-state value up to 0x69 returns
 * without a store.
 *
 * Representation notes: the hundred-and-six-entry table spans the whole 0..0x69
 * range with the default entry in fifty-eight slots, so the bound is the
 * compiler's own `sltiu 0x6A` off the highest live case 0x69. The clearing tail
 * (`func_8015C058` plus the sub-state and primary state clears) is written out
 * in every arm that ends the sequence and cross-jumps into the two shared sites
 * at 0x801FA060 and 0x801FA068; the action tail is likewise written per arm and
 * cross-jumps into the single action site at 0x801F9510 with each arm's own
 * action constant left in the jump delay slot. Sub-state 0x64 keeps the address
 * of D_80143BB0 in $s0 because its arm tests and stores that one byte, which is
 * the only callee-saved register the body needs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState6SubstateSequenceScenarioScena0900_801F9498(void) {
  switch (D_80146875) {
  case 0:
    func_80150224(0x0A);
    D_80143BB0 = 2u;
    D_80146875 = 1u;
    break;

  case 1:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 2:
    func_80150224(0x0B);
    D_80143BB0 = 2u;
    D_80146875 = 1u;
    break;

  case 3:
    func_80150224(0x0D);
    D_80143BB0 = 2u;
    D_80146875 = 1u;
    break;

  case 5:
    func_80150224(0x0C);
    D_80143BB0 = 2u;
    D_80146875 = 1u;
    break;

  case 6:
    func_80150224(0x0F);
    D_80143BB0 = 2u;
    D_80146875 = 1u;
    break;

  case 7:
    func_80150224(0x2B);
    D_80143BB0 = 2u;
    D_80146875 = 8u;
    break;

  case 8:
    if (D_80143BB0 == 2u) {
      break;
    }
    if (g_ScenarioProgress == 0x0Au) {
      D_80146875 = 9u;
      break;
    }
    if (g_ScenarioProgress == 0x14u) {
      D_80146875 = 0x0Au;
    }
    break;

  case 9:
    g_ScenarioProgress = 0u;
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 0x0A:
    func_8015B580(D_8014686C, 0x11);
    func_8015C058();
    g_ScenarioProgress = 0u;
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 0x14:
    func_801BE1B0(3u);
    D_80146875 = 0x15u;
    break;

  case 0x15:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    D_80146875 = 0x16u;
    func_8019FA28(0x75, 0x38000, 0x158000, 0x84);
    break;

  case 0x16:
    if (g_ScenarioProgress != 2u) {
      break;
    }
    D_80146875 = 0x17u;
    func_8019FA28(0x75, 0x560000, 0x2E0000, 0x85);
    break;

  case 0x17:
    if (g_ScenarioProgress != 0x0Fu) {
      break;
    }
    D_80146875 = 0x18u;
    func_8019FA28(0x75, 0x38000, 0x150000, 0x86);
    break;

  case 0x18:
    D_8014933B = 1u;
    func_8015B580(D_8014686C, 0x0C);
    D_80146875 = 0x19u;
    break;

  case 0x19: {
    u8 *progress;

    progress = &g_ScenarioProgress;
    if (*progress != 1u) {
      break;
    }
    *progress = 0u;
    D_80146875 = 0x1Au;
    func_8019FA28(0x75, 0x560000, 0x2E0000, 0x87);
    break;
  }

  case 0x1A:
    if (g_ScenarioProgress != 4u) {
      break;
    }
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 0x1E:
    func_801BE1B0(8u);
    D_80146875 = 0x1Fu;
    break;

  case 0x1F:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    func_8014ECAC(0u);
    D_80146875 = 0x20u;
    break;

  case 0x20:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    D_80146875 = 0x21u;
    func_8019FA28(0x76, 0x70000, 0x118000, 0x83);
    break;

  case 0x21: {
    u8 *scratch;
    u8 index;

    index = func_8019601C();
    scratch = &D_1F800000;
    *scratch = index;
    if (index == 0xFFu) {
      break;
    }
    D_80143FC8[(u32)*scratch * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
    *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) = -0x3B2;
    *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
    *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = 0x38C;
    D_80143FD1[(u32)*scratch * 0x74u] = 4u;
    D_80146875 = 0x22u;
    D_80146867 = *scratch;
    break;
  }

  case 0x22: {
    u8 *index;

    index = &D_80146867;
    if (D_80143FC8[(u32)*index * 0x74u] != 0u) {
      break;
    }
    *index = 0u;
    g_ScenarioProgress = 1u;
    func_8014ECAC(5u);
    D_8014832E = 0x1Fu;
    D_80146875 = 0x23u;
    break;
  }

  case 0x23:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014933B = 2u;
    func_8015B580(D_8014686C, 0x0D);
    D_80146875 = 0x24u;
    break;

  case 0x24: {
    u8 *progress;

    progress = &g_ScenarioProgress;
    if (*progress != 2u) {
      break;
    }
    D_8014832E = 0x1Fu;
    *progress = 0u;
    D_80146875 = 0x25u;
    func_8019FA28(0x75, 0x560000, 0x2B0000, 0x89);
    break;
  }

  case 0x25:
    if (g_ScenarioProgress != 9u) {
      break;
    }
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 0x28:
    func_801BE1B0(0x0Au);
    D_80146875 = 0x29u;
    break;

  case 0x29:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    func_8014ECAC(4u);
    D_80146875 = 0x2Au;
    break;

  case 0x2A:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    D_80146875 = 0x2Bu;
    func_8019FA28(0x76, 0x88000, 0x78000, 0x83);
    break;

  case 0x2B: {
    u8 *scratch;
    u8 index;

    index = func_8019601C();
    scratch = &D_1F800000;
    *scratch = index;
    if (index == 0xFFu) {
      break;
    }
    D_80143FC8[(u32)*scratch * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
    *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) = -0x2AA;
    *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
    *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = 0x31E;
    D_80143FD1[(u32)*scratch * 0x74u] = 4u;
    D_80146875 = 0x2Cu;
    D_80146867 = *scratch;
    break;
  }

  case 0x2C: {
    u8 *index;

    index = &D_80146867;
    if (D_80143FC8[(u32)*index * 0x74u] != 0u) {
      break;
    }
    *index = 0u;
    g_ScenarioProgress = 1u;
    func_8014ECAC(5u);
    D_8014832E = 0x1Fu;
    D_80146875 = 0x2Du;
    break;
  }

  case 0x2D:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014933B = 3u;
    func_8015B580(D_8014686C, 0x0F);
    D_80146875 = 0x2Eu;
    break;

  case 0x2E: {
    u8 *progress;

    progress = &g_ScenarioProgress;
    if (*progress != 2u) {
      break;
    }
    *progress = 0u;
    D_80146875 = 0x2Fu;
    func_8019FA28(0x75, 0x530000, 0x300000, 0x8B);
    break;
  }

  case 0x2F:
    if (g_ScenarioProgress != 8u) {
      break;
    }
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 0x32:
    func_80150224(0x2B);
    D_80143BB0 = 2u;
    D_80146875 = 0x33u;
    break;

  case 0x33:
    if (D_80143BB0 == 2u) {
      break;
    }
    if (g_ScenarioProgress == 0x0Au) {
      func_801BE1B0(0x0C);
      D_80146875 = 0x34u;
      break;
    }
    if (g_ScenarioProgress == 0x14u) {
      D_80146875 = 0x35u;
    }
    break;

  case 0x34:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 0x35:
    func_8014ECAC(0u);
    D_80146875 = 0x36u;
    break;

  case 0x36:
    if (D_80143C40 != 0u) {
      break;
    }
    g_ScenarioProgress = 1u;
    D_8014832E = 0u;
    D_80146875 = 0x37u;
    func_8019FA28(0x76, 0x78000, 0x38000, 0x83);
    break;

  case 0x37:
    func_8014ECAC(5u);
    D_8014832E = 0x1Fu;
    D_80146875 = 0x38u;
    break;

  case 0x38:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014933B = 4u;
    func_8015B580(D_8014686C, 0x11);
    func_8015B580(D_8014686C, 0x1D);
    D_80146875 = 0x39u;
    break;

  case 0x39: {
    u8 *progress;

    progress = &g_ScenarioProgress;
    if (*progress != 2u) {
      break;
    }
    *progress = 0u;
    D_80146875 = 0x3Au;
    func_8019FA28(0x75, 0x530000, 0x290000, 0x8D);
    break;
  }

  case 0x3A:
    if (g_ScenarioProgress != 7u) {
      break;
    }
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;

  case 0x64: {
    u8 *phase;

    phase = &D_80143BB0;
    if (*phase == 2u) {
      break;
    }
    func_80150224(0x1B);
    *phase = 2u;
    D_80146875 = 0x65u;
    break;
  }

  case 0x65: {
    u8 *progress;

    progress = &g_ScenarioProgress;
    if (*progress == 0x64u) {
      *progress = 0u;
      D_80146875 = 0x66u;
      func_8014ECAC(0x0Du);
      func_801C16B8();
      func_8015D554(0u, 0u, 4u, 0x1E);
      break;
    }
    if (*progress == 0x65u) {
      *progress = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
    }
    break;
  }

  case 0x66:
    func_801636A0(0u, 1u);
    D_80146875 = 0x67u;
    break;

  case 0x67:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    if ((func_80163EA0() & 0xFFu) != 0u) {
      D_80146875 = 0x68u;
    }
    break;

  case 0x68:
    func_8014ECAC(0x0Eu);
    D_8014832E = 0x1Fu;
    D_80146875 = 0x69u;
    break;

  case 0x69:
    if (D_80143C40 != 0u) {
      break;
    }
    func_80161C20(0x61, 0x64, 8);
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
}
