#include "bof3/bof3.h"

/* Scratchpad byte at 0x1F800000: the record arm publishes the shared slot
 * query result here and then indexes the shared record run with it, so the
 * reads that follow the record stores are real lbu.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* Shared scenario progress byte at 0x80146864: the sub-states gate on it, the
 * two release arms wait for 1 and 5 and the record arms store 5, 8 and 0x11.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared gate halfword at 0x80143C40, read with lhu: sub-state 13 waits for it
 * to read zero.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared effect bank byte at 0x8014832E: sub-states 1 and 13 arm it to 0x1F.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared area status byte at 0x80149333: the sub-state 14 arm publishes mode 2
 * twice.
 * @source 0x80149333 @kind unknown */
extern u8 D_80149333;

/* Shared mode byte at 0x80149332, read with lbu: the closing sub-state 34 arm
 * continues only while it reads zero.
 * @source 0x80149332 @kind unknown */
extern u8 D_80149332;

/* Shared fixed-point halfword at 0x8014932C, read with lhu: the sub-state 14
 * arm lowers it by 0x5C per frame and then clears it.
 * @source 0x8014932C @kind data */
extern u16 D_8014932C;

/* Shared frame counter byte at 0x80146867: the sub-state 14 arm counts frames
 * in it, and the closing arm clears it.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared scenario flag word at 0x8014686C, read with lw: the flag-bit helper
 * takes it as its base argument in sub-states 0, 5, 6, 15, 16 and 34.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared flag halfword at 0x80146258, read with lhu: sub-state 17 clears bit 6,
 * sub-states 31 and 32 set bit 3 and the closing arm flips bit 3.
 * @source 0x80146258 @kind data */
extern u16 D_80146258;

/* Shared primary state byte at 0x80146874: the release tail clears it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte at 0x80146875: this sequence switches on it, and every
 * arm that advances the sequence stores it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared mode-select byte at 0x80146865: sub-states 5 and 6 store 1 and 2 in
 * it, sub-state 16 branches on it, and sub-states 17 and 34 clear it.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared mode byte at 0x80146866: sub-states 11 and 16 store 1 and 0 in it and
 * the closing arm clears it.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared signed halfword at 0x801492DA: the record arm copies it, read with lh,
 * into the record word at +0x68.
 * @source 0x801492DA @kind data */
extern s16 D_801492DA;

/* Shared word at 0x80145ECC, read with lw: sub-states 5 and 6 continue only
 * while it reads above 0x7FFFFFF.
 * @source 0x80145ECC @kind data */
extern s32 D_80145ECC;

/* One 0x74-byte field record of the shared main-RAM run at 0x80143FC8. The
 * record arm stores the leading active byte, the arm byte at +5, the fade byte
 * at +9 and the three words at 0x64, 0x68 and 0x6C, and the original recomputes
 * the 0x74-byte stride from a fresh scratchpad read for each store.
 * @kind struct */
typedef struct {
  u8 unk_00;        /* 0x00 slot active byte, stored as 1 */
  u8 unk_01[4];     /* 0x01 */
  u8 unk_05;        /* 0x05 arm byte, stored as 0x13 */
  u8 unk_06[3];     /* 0x06 */
  u8 unk_09;        /* 0x09 fade byte, stored as 2 */
  u8 unk_0A[0x5A];  /* 0x0A */
  u32 unk_64;       /* 0x64 channel word, stored as -0x268 */
  u32 unk_68;       /* 0x68 word copied from D_801492DA */
  u32 unk_6C;       /* 0x6C size word, stored as 0x10E */
  u32 unk_70;       /* 0x70 */
} FieldRecord;

/* Base of the shared 0x74-byte field-record run at 0x80143FC8.
 * @source 0x80143FC8 @kind data */
extern FieldRecord D_80143FC8[];

/* Shared slot query at 0x8019601C: it answers the next free record index with
 * 0xFF meaning none, and the record arm publishes the answer in the scratchpad
 * byte before indexing with it.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared GAME.EMI helper at 0x801BE1B0: sub-states 10 and 30 open a window with
 * selector 9 and 0xD.
 * @source 0x801BE1B0 @kind function */
extern void func_801BE1B0(u32 arg0);

/* Shared overlay reset helper at 0x801C601C: sub-states 12 and 33 run it with
 * the window selector 3 and 0.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared scenario flag-word bit-set helper at 0x8015B580: the arms pass it the
 * shared flag word and bit 7, bits 0x27 and 0x26, bit 0x3E and bit 8.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared flag tester at 0x8015B5D4: sub-states 5 and 6 pass it the shared flag
 * word and bit 0x26 or 0x27 and continue on a non-zero answer.
 * @source 0x8015B5D4 @kind function */
extern s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Shared two-value request helper at 0x8015507C: sub-state 30 requests the pair
 * 0xB and 0x54.
 * @source 0x8015507C @kind function */
extern void func_8015507C(s32 arg0, s32 arg1);

/* Shared overlay release helper at 0x8015C058: the release tail and sub-state
 * 34 run it before clearing the progress, sub-state and primary state bytes.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared front-end request setter at 0x8019FA28: sub-states 0, 11, 16, 31 and 32
 * pass it this sequence's own seed, context words and kind.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* @source 0x801F8D30
 * @behavior Word 4 (the word stored at 0x801FE3FC) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets 0x801F8D30 and that table word is its
 * only reference in this payload. It switches on the shared sub-state byte
 * D_80146875 through the thirty-five-entry absolute jump table at 0x801F6CCC,
 * an overlay-local table in this payload's leading tabledata blob, reading that
 * byte before it creates its 0x18-byte frame, which keeps only $ra. Sub-state 0
 * advances the sub-state byte to 1, sets bit 7 of the shared flag word
 * D_8014686C (0x8014686C, read with lw) through func_8015B580 and requests
 * front selector context seed 0x31 with the two 16.16 context words 0x490000
 * and 0x238000 and kind 0x88 through func_8019FA28. Sub-state 1 advances to 2
 * and arms the shared effect bank byte D_8014832E (0x8014832E) to 0x1F.
 * Sub-state 2, once the shared scenario progress byte g_ScenarioProgress
 * (0x80146864) reads 1, runs the release tail. Sub-states 5 and 6 continue only
 * while the shared word D_80145ECC (0x80145ECC, read with lw) reads above
 * 0x7FFFFFF: sub-state 5 sets bit 0x27 of D_8014686C, tests bit 0x26 through
 * func_8015B5D4 and, on a non-zero answer, stores 5 in the progress byte, 1 in
 * the shared mode byte D_80146865 (0x80146865) and 0xA in the sub-state byte;
 * sub-state 6 runs the same arm with its two bits swapped and stores 2 in
 * D_80146865. Every other path of both sub-states runs the release tail.
 * Sub-state 10 runs the shared window opener func_801BE1B0(9) and advances to
 * 0xB. Sub-state 11, once the progress byte reads 7, stores 1 in the shared mode
 * byte D_80146866 (0x80146866), advances to 0xC and requests front selector
 * context seed 0x31 with the context words 0x248000 and 0x210000 and kind 0x8A.
 * Sub-state 12 runs the shared overlay reset helper func_801C601C(3), asks the
 * slot query func_8019601C for a record index and publishes it in the scratchpad
 * byte D_1F800000 (0x1F800000); unless that byte reads 0xFF it stores six fields
 * of the 0x74-byte record the index selects in the shared run at 0x80143FC8 -
 * the active byte 1 at +0, the arm byte 0x13 at +5, the word -0x268 at +0x64,
 * the shared signed halfword D_801492DA (0x801492DA, read with lh) at +0x68, the
 * word 0x10E at +0x6C and the fade byte 2 at +9 - and advances to 0xD.
 * Sub-state 13, while the shared gate halfword D_80143C40 (0x80143C40, read with
 * lhu) reads 0, arms D_8014832E to 0x1F and advances to 0xE. Sub-state 14
 * publishes mode 2 in the shared area status byte D_80149333 (0x80149333),
 * lowers the shared halfword D_8014932C (0x8014932C, read with lhu) by 0x5C and
 * increments the shared frame counter D_80146867 (0x80146867); while that
 * counter stays below 0x20 it returns, and on reaching 0x20 it clears
 * D_8014932C, publishes mode 2 again, clears D_80146867, stores 8 in the
 * progress byte and advances to 0xF. Sub-state 15, once the progress byte reads
 * 9, sets bit 0x3E of D_8014686C and advances to 0x10. Sub-state 16, once the
 * progress byte reads 0xA, clears D_80146866, advances to 0x11, sets bit 8 of
 * D_8014686C and then branches on D_80146865: 1 requests front selector context
 * seed 0x31 with the context words 0x80000 and 0x378000 and kind 0x8B, and 2
 * requests seed 0x31 with 0x1A0000 and 0x2F0000 and kind 0x8C. Sub-state 17,
 * once the progress byte reads 5, clears D_80146865 and bit 6 of the shared
 * flag halfword D_80146258 (0x80146258, read with lhu) and then runs the release
 * tail. Sub-state 30 requests the pair 0xB and 0x54 through func_8015507C, runs
 * func_801BE1B0(0xD) and advances to 0x1F. Sub-state 31, once the progress byte
 * reads 7, advances to 0x20, sets bit 3 of D_80146258 and requests front selector
 * context seed 0x75 with the context words 0x3F0000 and 0x58000 and kind 0x80;
 * sub-state 32 does the same on progress byte 5 with the sub-state 0x21, the
 * context words 0xE0000 and 0x148000 and kind 0x81. Sub-state 33 runs
 * func_801C601C(0) and advances to 0x22. Sub-state 34, once the progress byte
 * reads 0x11 and the shared mode byte D_80149332 (0x80149332, read with lbu)
 * reads zero, sets bit 0xB of D_8014686C, runs func_8015C058, clears D_80146865,
 * D_80146866, D_80146867, the sub-state byte and the primary state byte
 * D_80146874, and flips bit 3 of D_80146258. The release tail itself runs
 * func_8015C058 and clears the sub-state byte and D_80146874. Sub-states 3, 4,
 * 7, 8, 9, 18 through 29 and every value from 0x23 up return without touching
 * any state. It takes no arguments, reads no other state and writes no other
 * byte. Its 0x624 bytes have no copy in the tree: an exact-span search over
 * every payload under out/binaries finds no second copy of the body, and the
 * only in-payload absolute reference in it is the jump-table base 0x801F6CCC,
 * which every other overlay would carry at its own address. Statement order,
 * operand forms, offset spelling and declaration style come from this target's
 * primary-state-3 sequence at 0x801F8770; the literals, bit indices, literals
 * passed to the shared helpers and the record-arm store sequence are this
 * span's own, and its single release tail is the one the original shares from
 * sub-states 2, 5, 6 and 17.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState4SubstateSequenceScenarioScena0900_801F8D30(void) {
  switch (D_80146875) {
  case 0:
    D_80146875 = 1u;
    func_8015B580(D_8014686C, 7);
    func_8019FA28(0x31, 0x490000, 0x238000, 0x88);
    break;
  case 1:
    D_80146875 = 2u;
    D_8014832E = 0x1Fu;
    break;
  case 2:
    if (g_ScenarioProgress != 1u) {
      break;
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 5:
    if (D_80145ECC > 0x7FFFFFF) {
      func_8015B580(D_8014686C, 0x27);
      if (func_8015B5D4(D_8014686C, 0x26) != 0) {
        g_ScenarioProgress = 5u;
        D_80146865 = 1u;
        D_80146875 = 0xAu;
        break;
      }
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 6:
    if (D_80145ECC > 0x7FFFFFF) {
      func_8015B580(D_8014686C, 0x26);
      if (func_8015B5D4(D_8014686C, 0x27) != 0) {
        g_ScenarioProgress = 5u;
        D_80146865 = 2u;
        D_80146875 = 0xAu;
        break;
      }
    }
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 10:
    func_801BE1B0(9);
    D_80146875 = 0xBu;
    break;
  case 11:
    if (g_ScenarioProgress != 7u) {
      break;
    }
    D_80146866 = 1u;
    D_80146875 = 0xCu;
    func_8019FA28(0x31, 0x248000, 0x210000, 0x8A);
    break;
  case 12:
    func_801C601C(3);
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFFu) == 0xFFu) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = -0x268;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0x10E;
    D_80143FC8[D_1F800000[0]].unk_09 = 2;
    D_80146875 = 0xDu;
    break;
  case 13:
    if (D_80143C40 != 0u) {
      break;
    }
    D_8014832E = 0x1Fu;
    D_80146875 = 0xEu;
    break;
  case 14: {
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
    next_channel -= 0x5Cu;
    D_8014932C = next_channel;
    *count = next_count;
    if ((*count & 0xFFu) < 0x20u) {
      break;
    }
    g_ScenarioProgress = 8u;
    D_8014932C = 0u;
    D_80149333 = 2u;
    *count = 0u;
    D_80146875 = 0xFu;
    break;
  }
  case 15:
    if (g_ScenarioProgress != 9u) {
      break;
    }
    func_8015B580(D_8014686C, 0x3E);
    D_80146875 = 0x10u;
    break;
  case 16:
    if (g_ScenarioProgress != 0xAu) {
      break;
    }
    D_80146866 = 0u;
    D_80146875 = 0x11u;
    func_8015B580(D_8014686C, 8);
    switch (D_80146865) {
    case 1:
      func_8019FA28(0x31, 0x80000, 0x378000, 0x8B);
      break;
    case 2:
      func_8019FA28(0x31, 0x1A0000, 0x2F0000, 0x8C);
      break;
    }
    break;
  case 17:
    if (g_ScenarioProgress != 5u) {
      break;
    }
    D_80146865 = 0u;
    D_80146258 &= 0xFFBFu;
    func_8015C058();
    D_80146875 = 0u;
    D_80146874 = 0u;
    break;
  case 30:
    func_8015507C(0xB, 0x54);
    func_801BE1B0(0xD);
    D_80146875 = 0x1Fu;
    break;
  case 31:
    if (g_ScenarioProgress != 7u) {
      break;
    }
    D_80146875 = 0x20u;
    D_80146258 |= 8u;
    func_8019FA28(0x75, 0x3F0000, 0x58000, 0x80);
    break;
  case 32:
    if (g_ScenarioProgress != 5u) {
      break;
    }
    D_80146875 = 0x21u;
    D_80146258 |= 8u;
    func_8019FA28(0x75, 0xE0000, 0x148000, 0x81);
    break;
  case 33:
    func_801C601C(0);
    D_80146875 = 0x22u;
    break;
  case 34:
    if (g_ScenarioProgress != 0x11u) {
      break;
    }
    if (D_80149332 != 0u) {
      break;
    }
    func_8015B580(D_8014686C, 0xB);
    func_8015C058();
    D_80146865 = 0u;
    D_80146866 = 0u;
    D_80146867 = 0u;
    D_80146875 = 0u;
    D_80146874 = 0u;
    D_80146258 ^= 8u;
    break;
  }
}
