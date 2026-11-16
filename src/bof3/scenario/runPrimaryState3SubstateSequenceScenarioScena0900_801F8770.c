#include "bof3/bof3.h"

/* Scratchpad byte at 0x1F800000: the two record arms publish the shared slot
 * query result here and then index the shared record run with it, so the reads
 * that follow the record stores are real lbu.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* Shared scenario progress byte at 0x80146864: the sub-state 2 arm arms it to
 * 1, the sub-state 4 arm waits for 2, the sub-state 5 arm counts on 3 and the
 * closing tail clears it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared front-end phase byte at 0x80143BB0: sub-state 0 arms it to 2 and the
 * two arms that follow wait for it to leave 2, or in sub-state 12 to reach 0.
 * @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Shared gate halfword at 0x80143C40, read with lhu: sub-states 0, 2 and 9 wait
 * for it to read zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared effect bank byte at 0x8014832E: sub-state 1 arms it to 0x1F, sub-state
 * 9 clears it and the closing tail arms it to 0x1F again.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared area status byte at 0x80149333: the sub-state 5 arm publishes mode 2
 * twice.
 * @source 0x80149333 @kind unknown */
extern u8 D_80149333;

/* Shared fixed-point halfword at 0x8014932C, read with lhu: the sub-state 5
 * arm lowers it by 0x100 per frame and then arms it to -0x280.
 * @source 0x8014932C @kind data */
extern u16 D_8014932C;

/* Shared record index byte at 0x80146867: sub-state 4 publishes the index the
 * slot query answered and sub-state 5 counts frames in it.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared scenario flag word at 0x8014686C, read with lw: the sub-state 7 and 11
 * arms pass it to the shared bit-set helper.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared primary state byte at 0x80146874: the closing tail clears it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte at 0x80146875: this sequence switches on it, and every
 * arm that advances the sequence stores it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared signed halfword at 0x801492DA: the record arms copy it, read with lh,
 * into the record word at +0x68.
 * @source 0x801492DA @kind data */
extern s16 D_801492DA;

/* One 0x74-byte field record of the shared main-RAM run at 0x80143FC8. The
 * record arms store the leading active byte, the arm byte at +5, the fade byte
 * at +9 and the three words at 0x64, 0x68 and 0x6C, and the original
 * recomputes the 0x74-byte stride from a fresh scratchpad read for each store.
 * @kind struct */
typedef struct {
  u8 unk_00;        /* 0x00 slot active byte, stored as 1 */
  u8 unk_01[4];     /* 0x01 */
  u8 unk_05;        /* 0x05 arm byte, stored as 0x13 */
  u8 unk_06[3];     /* 0x06 */
  u8 unk_09;        /* 0x09 fade byte, stored as 0x14 or 0x8C */
  u8 unk_0A[0x5A];  /* 0x0A */
  u32 unk_64;       /* 0x64 channel word, stored as -0x252 or -0x35A */
  u32 unk_68;       /* 0x68 word copied from D_801492DA */
  u32 unk_6C;       /* 0x6C size word, stored as 0xA0 or 0x3FA */
  u32 unk_70;       /* 0x70 */
} FieldRecord;

/* Base of the shared 0x74-byte field-record run at 0x80143FC8.
 * @source 0x80143FC8 @kind data */
extern FieldRecord D_80143FC8[];

/* Shared slot query at 0x8019601C: it answers the next free record index with
 * 0xFF meaning none, and both record arms publish the answer in the scratchpad
 * byte before indexing with it.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared GAME.EMI helper at 0x801BE1B0: the sub-state 0 arm opens the window
 * with selector 0.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared overlay reset helper at 0x801C601C: the sub-state 0 arm runs it with
 * the window selector 0.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared scenario flag-word bit-set helper at 0x8015B580: the arms pass it the
 * shared flag word and bit 5 or 6.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared overlay release helper at 0x8015C058: the closing tail runs it before
 * clearing the progress, sub-state and primary state bytes.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared front-end request setter at 0x8019FA28: the sub-state 7 arm passes it
 * seed 0x37 with the context words 0x250000 and 0x130000 and kind 0x81, and the
 * sub-state 12 arm passes seed 0x37 with 0x160000 and 0x130000 and kind 0x82.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared front-end scripted-motion starter at 0x8015D554: the sub-state 9 arm
 * runs it with (0, 1, 4, 0x1E).
 * @source 0x8015D554 @kind function */
extern void func_8015D554(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* Shared two-argument controller-step helper at 0x801636A0: the sub-state 10
 * arm runs it with (3, 1).
 * @source 0x801636A0 @kind function */
extern void func_801636A0(u32 arg0, u32 arg1);

/* Shared record-state query at 0x80163EA0, returning the live record byte
 * zero-extended into a word; the sub-state 11 arm continues only on a non-zero
 * low byte.
 * @source 0x80163EA0 @kind function */
extern u32 func_80163EA0(void);

/* Shared front-end effect helper at 0x801C1DF0: the sub-state 11 arm runs it
 * with selector 5.
 * @source 0x801C1DF0 @kind function */
extern void func_801C1DF0(u8 index);

/* Shared front-end action helper at 0x80150224: the sub-state 0 arm requests
 * action 0 and the sub-state 9 arm requests action 0x14.
 * @source 0x80150224 @kind function */
extern void func_80150224(s32 action_id);

/* @source 0x801F8770
 * @behavior Word 3 (the word stored at 0x801FE3F8) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801F8770 and that table word is its
 * only reference in this payload. It switches on the shared sub-state byte
 * D_80146875 through the fourteen-entry absolute jump table at 0x801F6C94, an
 * overlay-local table in this payload's leading tabledata blob, reading that
 * byte before it creates its 0x18-byte frame, which keeps only $ra. Sub-state
 * 0, while the shared gate halfword D_80143C40 (0x80143C40, read with lhu)
 * reads 0, requests front-end action 0 through func_80150224, arms the shared
 * phase byte D_80143BB0 (0x80143BB0) to 2, runs the shared overlay reset helper
 * func_801C601C(0) and the shared window opener func_801BE1B0(0), and advances
 * the sub-state byte to 1. Sub-state 1, once that phase byte leaves 2, runs the
 * shared front-end mode setter func_8014ECAC(5), arms the effect bank byte
 * D_8014832E (0x8014832E) to 0x1F and advances to 2. Sub-state 2, while the
 * gate halfword reads 0, stores 1 in the shared scenario progress byte
 * g_ScenarioProgress (0x80146864) and advances to 3. Sub-state 3 asks the
 * shared slot query func_8019601C for a record index and publishes it in the
 * scratchpad byte D_1F800000 (0x1F800000); unless that byte reads 0xFF it
 * stores six fields of the 0x74-byte record the index selects in the shared run
 * at 0x80143FC8 - the active byte 1 at +0, the arm byte 0x13 at +5, the word
 * -0x252 at +0x64, the shared signed halfword D_801492DA (0x801492DA, read with
 * lh) at +0x68, the word 0xA0 at +0x6C and the fade byte 0x14 at +9 - and then
 * advances to 4. Sub-state 4 runs the same arm on progress byte 2 with its own
 * literals (-0x35A, 0x3FA and 0x8C), advances to 5 and publishes the index it
 * used in the shared byte D_80146867 (0x80146867). Sub-state 5 publishes mode 2
 * in the shared area status byte D_80149333 (0x80149333), lowers the shared
 * fixed-point halfword D_8014932C (0x8014932C, read with lhu) by 0x100 and
 * increments D_80146867; while the incremented byte stays below 0xD it returns,
 * and on reaching 0xD it arms D_8014932C to -0x280, publishes mode 2 again,
 * clears D_80146867, stores 3 in the progress byte and advances to 7. Sub-state
 * 7, once the progress byte reads 4, advances to 8, sets flag bit 5 of the
 * shared flag word D_8014686C (0x8014686C, read with lw) through func_8015B580
 * and requests front selector context seed 0x37 with the two 16.16 context
 * words 0x250000 and 0x130000 and kind 0x81 through func_8019FA28. Sub-state 8,
 * once the progress byte reads 0x16, runs func_8014ECAC(0xD) and advances to 9.
 * Sub-state 9, while the gate halfword reads 0, clears the effect bank byte,
 * stores 0x17 in the progress byte, requests front-end action 0x14, arms the
 * phase byte to 2, advances to 0xA and runs the shared front-end
 * scripted-motion starter func_8015D554(0, 1, 4, 0x1E). Sub-state 0xA, once the
 * phase byte leaves 2, runs the shared controller-step helper func_801636A0(3,
 * 1) and advances to 0xB. Sub-state 0xB, unless the record-state query
 * func_80163EA0 returns a zero low byte, runs the shared effect helper
 * func_801C1DF0(5), advances to 0xC and sets flag bit 6 of D_8014686C through
 * func_8015B580. Sub-state 0xC, once the phase byte reads 0, advances to 0xD,
 * requests front selector context seed 0x37 with the context words 0x160000 and
 * 0x130000 and kind 0x82 through func_8019FA28 and queues the cue
 * func_80161C20(0x79, 0x64, 8). Sub-state 0xD arms the effect bank byte to 0x1F,
 * runs the shared release helper func_8015C058 and clears the progress byte,
 * the sub-state byte and the primary state byte D_80146874, which ends the
 * state. Sub-state 6 and every value from 0xE up return without touching any
 * state. It takes no arguments, reads no other state and writes no other byte.
 * Its 0x5C0 bytes have no copy in the tree: an exact-span search over every
 * payload under out/binaries finds no second copy of the 0x5C0-byte body, and
 * the only overlay-local absolute words in it are the fourteen switch-table
 * entries at 0x801F6C94, which every other overlay would carry at their own
 * addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState3SubstateSequenceScenarioScena0900_801F8770(void) {
  switch (D_80146875) {
  case 0:
    if (D_80143C40 != 0u) {
      break;
    }
    func_80150224(0);
    D_80143BB0 = 2u;
    func_801C601C(0);
    func_801BE1B0(0);
    D_80146875 = 1u;
    break;
  case 1:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_8014ECAC(5);
    D_8014832E = 0x1Fu;
    D_80146875 = 2u;
    break;
  case 2:
    if (D_80143C40 != 0u) {
      break;
    }
    g_ScenarioProgress = 1u;
    D_80146875 = 3u;
    break;
  case 3:
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFFu) == 0xFFu) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = -0x252;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0xA0;
    D_80143FC8[D_1F800000[0]].unk_09 = 0x14;
    D_80146875 = 4u;
    break;
  case 4:
    if (g_ScenarioProgress != 2u) {
      break;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFFu) == 0xFFu) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = -0x35A;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0x3FA;
    D_80143FC8[D_1F800000[0]].unk_09 = 0x8C;
    D_80146867 = D_1F800000[0];
    D_80146875 = 5u;
    break;
  case 5: {
    u8 *count;
    u8 next_count;
    u16 next_channel;

    do {
      count = &D_80146867;
      D_80149333 = 2u;
    } while (0);
    next_count = *count;
    next_channel = D_8014932C;
    next_count += 1u;
    next_channel -= 0x100u;
    D_8014932C = next_channel;
    *count = next_count;
    if ((*count & 0xFFu) < 0xDu) {
      break;
    }
    *(s16 *)&D_8014932C = -0x280;
    D_80149333 = 2u;
    *count = 0u;
    g_ScenarioProgress = 3u;
    D_80146875 = 7u;
    break;
  }
  case 6:
    break;
  case 7:
    if (g_ScenarioProgress != 4u) {
      break;
    }
    D_80146875 = 8u;
    func_8015B580(D_8014686C, 5);
    func_8019FA28(0x37, 0x250000, 0x130000, 0x81);
    break;
  case 8:
    if (g_ScenarioProgress != 0x16u) {
      break;
    }
    func_8014ECAC(0xD);
    D_80146875 = 9u;
    break;
  case 9:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0u;
    g_ScenarioProgress = 0x17u;
    func_80150224(0x14);
    D_80143BB0 = 2u;
    D_80146875 = 0xAu;
    func_8015D554(0, 1, 4, 0x1E);
    break;
  case 10:
    if (D_80143BB0 == 2u) {
      break;
    }
    func_801636A0(3, 1);
    D_80146875 = 0xBu;
    break;
  case 11:
    if ((func_80163EA0() & 0xFFu) == 0u) {
      break;
    }
    func_801C1DF0(5);
    D_80146875 = 0xCu;
    func_8015B580(D_8014686C, 6);
    break;
  case 12:
    if (D_80143BB0 != 0u) {
      break;
    }
    D_80146875 = 0xDu;
    func_8019FA28(0x37, 0x160000, 0x130000, 0x82);
    func_80161C20(0x79, 0x64, 8);
    break;
  case 13:
    D_8014832E = 0x1Fu;
    func_8015C058();
    g_ScenarioProgress = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  }
}
