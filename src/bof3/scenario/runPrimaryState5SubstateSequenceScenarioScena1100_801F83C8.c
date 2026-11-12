#include "bof3/bof3.h"

/* PsyQ libc bounded copy of the shared EXE image, outside this payload: the
 * name-copy arm of sub-state 0x13 copies the twelve-byte name the local name
 * helper answers into the shared name buffer below and discards the answer. */
extern void strncpy(u8 *dst, const u8 *src, u32 count);

/* Shared front-end mode byte of the shared EXE image: the arms that start a
 * front-end sequence leave 2 behind and the arms that wait on it refuse to act
 * while it reads 2. */
extern u8 D_80143BB0;

/* Front-end record table of the shared EXE image, declared once per record
 * field address the way the sibling lifts of this same table spell it: the
 * address of each byte, halfword and word the body stores carries its own
 * symbol, so the table base never becomes a hoisted register and every store
 * re-forms its own address from the scaled work index. The generated
 * undefined_syms_auto.txt rows bind these names. */
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FCE[];
extern u8 D_80143FD4[];
extern u8 D_80143FD8[];
extern u8 D_80143FF6[];
extern u8 D_80143FF8[];

/* Shared flag/prompt byte of the shared EXE image: the two claim arms seed it
 * with 0x7C or 0x7D after they publish a front selection context. */
extern u8 D_80143F1F;

/* Front-end work-record run of the shared EXE image, published by the arms
 * that hand this overlay a work record. */
extern u8 D_80145E90[];

/* Selected-entry flag byte of the shared EXE image: bit 6 marks the front-end
 * sequence the claim arm of sub-state 6 starts and the release arm ends. */
extern u8 D_80145FA8;

/* Published work-record pointer cell of the shared EXE image. */
extern u8 *D_80146250;

/* Shared flag word of the shared EXE image, handed to the flag helper
 * func_8015B580 with the bit ids this body stores. */
extern u32 D_8014686C;

/* Shared scenario progress byte and the shared state bytes that follow it: the
 * progress byte is the completion counter this body waits on, the sub-state
 * byte below is its own switch word and the primary state byte selects the
 * handler run that reaches it. */
extern u8 g_ScenarioProgress;
extern u8 D_80146874;
extern u8 D_80146875;

/* Twelve-byte name buffer of the shared EXE image and the terminator byte the
 * name-copy arm clears directly after it. */
extern u8 D_801490D8[];
extern u8 D_801490E4;

/* Scratchpad byte 0 of the shared main executable: the two claim arms store the
 * index the allocator answers there and re-read it for every record store,
 * because a store through the record table may alias the slot itself. */
extern u8 D_1F800000[];

/* Scratchpad work-record pointer cell at 0x1F800044, which the arms of this
 * body read as the work record and which sub-state 9 publishes the work-record
 * run into.
 * @source 0x1F800044 @kind unknown
 */
extern u8 *D_1F800044;

/* Shared front-end record allocator of the shared EXE image: it answers the
 * work index of the record it just seeded. */
extern u8 func_8019601C(void);

/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared front-end action starter at 0x80150224. */
void func_80150224(u32 arg0);
/* Shared scenario flag-word helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end scripted-motion starter at 0x8015D554. */
void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Shared front-end sequence helper at 0x801636A0. */
void func_801636A0(u32 arg0, u32 arg1);
/* Shared front-end readiness poll of the shared EXE image. */
u32 func_80163EA0(void);
/* Shared entry test of the shared EXE image: it takes four unsigned bytes and
 * answers an unsigned byte. Sub-state 0x13 asks about entry 4 of stream 0x24
 * and keeps only whether the answer is nonzero. */
extern u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
/* Local name helper of the shared EXE image: it answers the pointer to the name
 * selected by the two arguments. */
extern u8 *func_80165D48(s32 arg0, u8 arg1);
/* Shared front-selector context seeder at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared sound-cue dispatcher of the main executable, bound by this target's
 * own map. Sub-states 8 and 0x13 dispatch cues 0x201 and 0x106. */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801F83C8
 * @behavior Word 5 (the word stored at 0x801FAF10) of the ten-entry
 * overlay-local second-level handler run at 0x801FAEFC that the exact
 * dispatcher dispatchSecondaryStateScenarioScena1100_801F7748 (0x801F7748)
 * indexes with the shared primary state byte D_80146874, so a primary state
 * reading 5 selects it; no jal or j of the payload names 0x801F83C8, and that
 * little-endian table word is its only reference. It switches on the shared
 * secondary sub-state byte D_80146875 (read with lbu) through the
 * forty-three-word absolute jump table jtbl_801F6E74_main at 0x801F6E74 in
 * this payload leading table data, bounded by `sltiu` against 0x2B; sub-states
 * 7, 0x0E, 0x15 and 0x17 through 0x27 are the table words that point at the
 * body exit.
 *
 * Sub-state 0, while the shared mode byte D_80143BB0 (0x80143BB0) is not 2,
 * stores 1 in the shared progress byte g_ScenarioProgress (0x80146864), runs
 * func_801BE1B0 (0x801BE1B0) with 1 and advances the sub-state byte to 1.
 * Sub-states 1, 0x13 and 0x29 wait on that progress byte and then seed a front
 * selection context: sub-state 1, once it reads 3, clears the progress byte and
 * then starts the scripted motion func_8015D554 (0x8015D554) with
 * (0, 0, 4, 0x1E), sets flag bit 7 of the shared flag word D_8014686C
 * (0x8014686C) through func_8015B580 (0x8015B580), advances to 2 and seeds the
 * context (0x44, 0x300000, 0x198000, 0x82) through func_8019FA28
 * (0x8019FA28) before storing 0x7D in the shared flag byte D_80143F1F
 * (0x80143F1F); sub-state 0x29, once it reads 1, does the same with the context
 * (0x44, 0x300000, 0x198000, 0x88) and advances to 0x2A. Sub-state 0x13, once
 * the progress byte reads 1, asks func_801650B4 (0x801650B4) about entry 4 of
 * stream 0x24: a zero answer copies the twelve-byte name func_80165D48
 * (0x80165D48) answers for stream 0x24 through strncpy into the shared name
 * buffer D_801490D8 (0x801490D8), clears the terminator byte D_801490E4
 * (0x801490E4) and queues front-end action 0x33 through func_80150224
 * (0x80150224), while a non-zero answer queues action 0x32 and then dispatches
 * cue 0x106 through the exact cue dispatcher dispatchSoundCue (0x8015DF18);
 * both halves store 2 in the mode byte and advance the sub-state byte to 0x14.
 *
 * Sub-states 2, 3, 4 and 0x2A only test the progress byte and advance: 2 on it
 * reading 5 to 3, 3 on it reading 9 to 4, 4 and 0x2A first on it reading 3 run
 * func_801BE1B0 with 3 and advance to 2, and otherwise on it reading 0x14
 * advance to 5. Sub-state 5, while the mode byte is not 2, advances to 6.
 * Sub-state 6 claims two front-end records through func_8019601C, publishing
 * each index in scratchpad byte 0 of D_1F800000 (0x1F800000) and skipping the
 * record seed while that index reads 0xFF; each seeded record gets byte 0 = 1
 * and the byte-5 field set to 0x8C for the first claim and 0x8B for the second,
 * every store re-reading the published index. It then publishes the work-record
 * run D_80145E90 (0x80145E90) into D_80146250 (0x80146250) and into the
 * scratchpad pointer cell 0x1F800044, advances to 7 and sets bit 6 of
 * D_80145FA8 (0x80145FA8). Sub-state 8 does the same publication, clears that
 * bit, dispatches cue 0x201, starts the scripted motion with (0, 1, 4, 0x1E)
 * and advances to 9. Sub-state 9 publishes the work-record run into the
 * scratchpad pointer cell only, stores the index func_8019601C answers in work
 * byte 0xB and, while that byte is not 0xFF, seeds the record it selects with
 * byte 0 = 1, byte 5 = 6, byte 6 = 3, word 0xC = 0, word 0x10 = 0x3A and the
 * two halfwords at 0x2E and 0x30 of the work record itself, and advances to
 * 0x0A; an index reading 0xFF leaves the sub-state byte alone. Sub-states 0x0A
 * and 0x10 run func_801636A0 with (6, 1) and (5, 1) and
 * advance to 0x0B and 0x11; 0x0B and 0x11 wait on the readiness poll
 * func_80163EA0 (0x80163EA0), then queue action 0x28, store 2 in the mode byte
 * and advance to 0x0C, or store 2 in the progress byte and advance to 0x12.
 *
 * Sub-states 0x0C and 0x12 wait on the mode byte and seed the context
 * (0x44, 0x2E8000, 0x178000, 0x84) and (0x44, 0x2E8000, 0x178000, 0x86) after
 * advancing to 0x0D and 0x13; sub-state 0x12 first clears the progress byte and
 * sets flag bit 8 of D_8014686C, and both store 0x7C in D_80143F1F. Sub-states
 * 0x0D, 0x16 and 0x28 end the sequence in one clearing tail: 0x0D once the
 * progress byte reads 1 and 0x16 once it reads 3 run the shared overlay release
 * helper func_8015C058 (0x8015C058) and clear the sub-state byte and the
 * primary state byte, while 0x28, after refusing to act while the mode byte
 * reads 2, does the same once the progress byte reads 0xFF and otherwise
 * clears the progress byte, runs func_801BE1B0 with 9 and advances to 0x29.
 *
 * It takes no arguments and returns nothing; its 0x20-byte frame keeps $ra,
 * $s1 and $s0. $s1 holds the constant 0xFF the two claim comparisons share
 * across the allocator calls, and $s0 holds the constant 1 of sub-state 0
 * across the func_801BE1B0 call, the scratchpad base of the claim arms and the
 * address of the progress byte in sub-state 0x28. Every shared byte, table and
 * helper above is addressed from the main executable, so the span has no
 * target-local copy: a scan of the extracted payloads under out/binaries for
 * this 0x7BC-byte span finds this payload only, at file offset 0x17C8, so the
 * clone lever is declined and every literal here was written from this span's
 * own bytes. The nearest shape donors are the exact neighbours at 0x801F7F08
 * and 0x801F8B84 of the same handler run, which supply the sub-state switch
 * over an absolute jump table, the front-end record claim with its scratchpad
 * index, the mode-byte guarded arms, the selection-context seed and the
 * clearing tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState5SubstateSequenceScenarioScena1100_801F83C8(void) {
  u8 index;

  switch (D_80146875) {
  case 0:
    if (D_80143BB0 != 2u) {
      g_ScenarioProgress = 1u;
      func_801BE1B0(1u);
      D_80146875 = 1u;
    }
    break;
  case 1: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 3u) {
      break;
    }
    *progress = 0u;
    func_8015D554(0, 0, 4, 0x1E);
    func_8015B580(D_8014686C, 7);
    D_80146875 = 2u;
    func_8019FA28(0x44u, 0x300000u, 0x198000u, 0x82u);
    D_80143F1F = 0x7Du;
    break;
  }
  case 2:
    if (g_ScenarioProgress != 5u) {
      break;
    }
    D_80146875 = 3u;
    break;
  case 3:
    if (g_ScenarioProgress != 9u) {
      break;
    }
    D_80146875 = 4u;
    break;
  case 4:
    if (g_ScenarioProgress == 3u) {
      func_801BE1B0(3u);
      D_80146875 = 2u;
    } else if (g_ScenarioProgress == 0x14u) {
      D_80146875 = 5u;
    }
    break;
  case 5:
    if (D_80143BB0 == 2u) {
      break;
    }
    D_80146875 = 6u;
    break;
  case 6: {
    u8 *slot;

    index = func_8019601C();
    slot = D_1F800000;
    slot[0] = index;
    if (index != 0xFF) {
      D_80143FC8[(u32)index * 0x74u] = 1;
      D_80143FCD[(u32)slot[0] * 0x74u] = 0x8C;
    }
    index = func_8019601C();
    slot[0] = index;
    if (index != 0xFF) {
      D_80143FC8[(u32)index * 0x74u] = 1;
      D_80143FCD[(u32)slot[0] * 0x74u] = 0x8B;
    }
    D_80146250 = D_80145E90;
    D_1F800044 = D_80145E90;
    D_80146875 = 7u;
    D_80145FA8 |= 0x40u;
    break;
  }
  case 8:
    D_80146250 = D_80145E90;
    D_1F800044 = D_80145E90;
    D_80145FA8 &= 0xBFu;
    dispatchSoundCue(0x201u);
    func_8015D554(0, 1, 4, 0x1E);
    D_80146875 = 9u;
    break;
  case 9: {
    u8 *work;

    D_1F800044 = D_80145E90;
    D_1F800044[0xB] = func_8019601C();
    if (D_1F800044[0xB] != 0xFFu) {
      D_80143FC8[(u32)D_1F800044[0xB] * 0x74u] = 1;
      D_80143FCD[(u32)D_1F800044[0xB] * 0x74u] = 6;
      D_80143FCE[(u32)D_1F800044[0xB] * 0x74u] = 3;
      work = D_1F800044;
      *(u32 *)(D_80143FD4 + (u32)work[0xB] * 0x74u) = 0;
      *(u32 *)(D_80143FD8 + (u32)work[0xB] * 0x74u) = 0x3A;
      *(u16 *)(D_80143FF6 + (u32)work[0xB] * 0x74u) =
          *(u16 *)(work + 0x2E);
      *(u16 *)(D_80143FF8 + (u32)work[0xB] * 0x74u) =
          *(u16 *)(work + 0x30);
      D_80146875 = 0xAu;
    }
    break;
  }
  case 10:
    func_801636A0(6u, 1u);
    D_80146875 = 0xBu;
    break;
  case 11:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    func_80150224(0x28u);
    D_80143BB0 = 2u;
    D_80146875 = 0xCu;
    break;
  case 12: {
    if (D_80143BB0 == 2u) {
      break;
    }
    D_80146875 = 0xDu;
    func_8019FA28(0x44u, 0x2E8000u, 0x178000u, 0x84u);
    D_80143F1F = 0x7Cu;
    break;
  }
  case 0xD:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 0xF:
    D_80146250 = D_80145E90;
    D_1F800044 = D_80145E90;
    D_80145FA8 &= 0xBFu;
    func_8015D554(0, 1, 4, 0x1E);
    func_801BE1B0(5u);
    D_80146875 = 0x10u;
    break;
  case 0x10:
    func_801636A0(5u, 1u);
    D_80146875 = 0x11u;
    break;
  case 0x11:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    g_ScenarioProgress = 2u;
    D_80146875 = 0x12u;
    break;
  case 0x12: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 5u) {
      break;
    }
    *progress = 0u;
    D_80146875 = 0x13u;
    func_8015B580(D_8014686C, 8);
    func_8019FA28(0x44u, 0x2E8000u, 0x178000u, 0x86u);
    D_80143F1F = 0x7Cu;
    break;
  }
  case 0x13:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    if ((func_801650B4(0, 0x24, 4, 0) & 0xFFu) == 0u) {
      strncpy(D_801490D8, func_80165D48(0, 0x24), 12);
      D_801490E4 = 0u;
      func_80150224(0x33u);
      D_80143BB0 = 2u;
      D_80146875 = 0x14u;
    } else {
      func_80150224(0x32u);
      D_80143BB0 = 2u;
      dispatchSoundCue(0x106u);
      D_80146875 = 0x14u;
    }
    break;
  case 0x14:
    if (D_80143BB0 == 2u) {
      break;
    }
    g_ScenarioProgress = 2u;
    D_80146875 = 0x16u;
    break;
  case 0x16:
    if (g_ScenarioProgress != 3u) {
      break;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 0x28: {
    u8 *progress;

    if (D_80143BB0 == 2u) {
      break;
    }
    progress = &g_ScenarioProgress;
    if (*progress == 0xFFu) {
      func_8015C058();
      *progress = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
    } else {
      *progress = 0u;
      func_801BE1B0(9u);
      D_80146875 = 0x29u;
    }
    break;
  }
  case 0x29: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 1u) {
      break;
    }
    *progress = 0u;
    D_80146875 = 0x2Au;
    func_8015D554(0, 0, 4, 0x1E);
    func_8019FA28(0x44u, 0x300000u, 0x198000u, 0x88u);
    D_80143F1F = 0x7Du;
    break;
  }
  case 0x2A:
    if (g_ScenarioProgress == 3u) {
      func_801BE1B0(3u);
      D_80146875 = 2u;
    } else if (g_ScenarioProgress == 0x14u) {
      D_80146875 = 5u;
    }
    break;
  }
}
