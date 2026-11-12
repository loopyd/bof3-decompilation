#include "bof3/bof3.h"

/* Shared scenario progress byte of the main executable: the waiting arms of
 * this walker compare it against one value each and clear it on the way to the
 * next sub-state.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared mode halfword of the main executable: three arms flip bit 0x08, one
 * flips bit 0x20 and the closing arm sets bit 0x20, all with lhu/xori/ori/sh.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared effect-bank byte of the main executable: every request arm arms it
 * with 0xFF or 0x8F or 0x90, and the field-slot arms leave 0x1F in it.
 * @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;

/* Shared effect-bank byte of the main executable: two arms store 0x1F in it and
 * the gate arm clears it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared gate halfword of the main executable: the arm at sub-state 0x14 waits
 * for it to read 0.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared main-RAM byte of the main executable: the request arms store the
 * request kind pair index into it.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared main-RAM byte of the main executable: the request arms store the
 * next-window byte into it.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared main-RAM byte of the main executable: it is the record index the
 * field-slot arm publishes and the gate arm clears.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared scenario flag word of the main executable: the two arms that release
 * the overlay set or clear scenario bits through it.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared state byte of the main executable: the closing arm clears it, which
 * returns the overlay to the idle entry of its handler run.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* Shared sub-state byte of the main executable: every arm of this walker reads
 * it once through the jump table and most of them store the next value into it.
 * @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared main-RAM byte of the main executable, set to 1 by the first field-slot
 * arm before it queries the slot source.
 * @source 0x8014933B @kind unknown */
extern u8 D_8014933B;

/* Shared halfword of the main executable copied into the slot record's word at
 * offset 0x68 by the first field-slot arm.
 * @source 0x801492DA @kind unknown */
extern s16 D_801492DA;

/* Scratchpad byte at 0x1F800000: the two field-slot arms store the slot query
 * result here and then index the record run with this byte. Every read after a
 * record store is a real lbu, because that store may alias the byte.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* One 0x74-byte field record of the shared main-RAM run this walker fills in
 * from the slot query: the first field-slot arm stores the 0..0xFE record index
 * this way and the second one repeats the two leading stores for its own slot.
 * @kind struct */
typedef struct {
  u8 unk_00;        /* 0x00 slot active byte, stored as 1 */
  u8 unk_01[4];     /* 0x01 */
  u8 unk_05;        /* 0x05 arm byte, 0x13 then 0x7A */
  u8 unk_06[3];     /* 0x06 */
  u8 unk_09;        /* 0x09 sub-state byte, stored as 4 */
  u8 unk_0A[0x5A];  /* 0x0A */
  s32 unk_64;       /* 0x64 stored as -0x370 */
  s32 unk_68;       /* 0x68 copied from D_801492DA */
  s32 unk_6C;       /* 0x6C stored as 0x200 */
  u8 unk_70[4];     /* 0x70 */
} FieldRecord;

/* Base of the shared 0x74-byte field-record run of the main executable, indexed
 * by the slot byte the arms publish at 0x1F800000.
 * @source 0x80143FC8 @kind data */
extern FieldRecord D_80143FC8[];

/* Shared word-at-a-time slot query at 0x8019601C of the main executable: it
 * answers the next free 0..0xFE record index, with 0xFF meaning none, and this
 * walker publishes the answer in the scratchpad byte before indexing with it.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared overlay reset helper at 0x801C601C of the main executable: each
 * sub-state that steps from one window to the next runs it with the window
 * selector the original encodes as 0, 1, 2 or 3.
 * @source 0x801C601C @kind function */
extern void func_801C601C(u32 arg0);

/* Shared scenario flag helper at 0x8015B580: the arms that leave the walker
 * pass it the shared flag word and the bit to set.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared front-end request setter at 0x8019FA28: every request arm passes it a
 * 16-bit selection seed, the two 16.16 context words and the request kind.
 * @source 0x8019FA28 @kind function */
extern void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                          u8 context_kind);

/* Shared front-end cue queue at 0x8015DF18: the field-slot arm at sub-state 0x0C
 * queues cue 0x20C through it.
 * @source 0x8015DF18 @kind function */
extern void func_8015DF18(u16 cue);

/* Shared four-argument controller helper at 0x8015D554: the arm at sub-state
 * 0x13 runs it with the constants below.
 * @source 0x8015D554 @kind function */
extern void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared front-end mode setter at 0x8014ECAC: the arm at sub-state 0x13 sets
 * mode 0xD through it.
 * @source 0x8014ECAC @kind function */
extern void func_8014ECAC(u16 arg0);

/* Shared front-end release helper at 0x8015C058: the closing arm runs it before
 * clearing the sub-state and state bytes.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* @source 0x801F9F0C
 * @behavior Overlay secondary-state handler: it is the word stored at
 * 0x801FD1DC, entry 6 of the ten-word handler run at 0x801FD1C4 that
 * dispatchSecondaryStateScenarioScena1200_801F886C (0x801F886C) indexes with
 * the signed shared state byte D_80146874, so it runs while that byte reads 6;
 * that table word is the only image reference to this address, no jal or j in
 * the payload targets it, and the walker clears D_80146874 itself on its last
 * sub-state. It is the longest arm of that run: it switches on the shared
 * sub-state byte D_80146875 through the twenty-three-entry absolute jump table
 * at 0x801F6EAC, an overlay-local table of the leading tabledata blob, so
 * sub-states 0 through 0x16 are its table entries and every other value returns
 * untouched. Each arm either steps the window or waits for the shared progress
 * byte g_ScenarioProgress (0x80146864) to reach one value and clears it, and
 * then arms the shared effect-bank byte D_80143F1F, stores the next-window
 * bytes D_80146865 and D_80146866, flips or sets one bit of the shared mode
 * halfword D_80146258, and requests a front-end context through the shared
 * setter func_8019FA28 (0x8019FA28) with the 16-bit seeds 0xBC and 0x79, two
 * context words and a request kind from 0x80 through 0x8E, the arm at sub-state
 * 2 passing the bare kind 6. Sub-states 1, 7, 9 and 0x0D run the
 * shared overlay reset helper func_801C601C with selectors 0 through 3 and
 * advance one sub-state. Sub-states 3 and 0x12 take a slot index from the
 * shared query func_8019601C (0x8019601C), publish it in the scratchpad byte
 * 0x1F800000 and fill that entry of the shared 0x74-byte record run at
 * 0x80143FC8; the first one publishes the index in D_80146867, which the gate at
 * sub-state 4 reads before it clears it. Sub-state 0x0C queues cue 0x20C through
 * func_8015DF18 and sub-state 0x13 runs func_8015D554(0, 1, 4, 0x1E) and sets
 * front-end mode 0xD through func_8014ECAC. Sub-state 0x14 waits for the shared
 * gate halfword D_80143C40 to read 0, and sub-state 0x16 runs the shared
 * front-end release helper func_8015C058 (0x8015C058) and clears both the
 * sub-state and the state byte. Its 0x8D0 bytes have no copy in the tree: a
 * span search over every payload under out/binaries matched no second copy, so
 * the body was written from this span's own bytes with the statement order,
 * operand forms and declaration style of the same-target sibling
 * setProgress50AndAdvanceSubstate0BUnlessFrontEndMode2ThenReleaseFrontEndWhenSharedByte65Reads0AScenarioScena1200_801F88B0
 * (0x801F88B0), which measured exact when this seed was written. Every shared byte store outside
 * the progress arms re-materialises the 0x8014 page in $at, while an arm that
 * both tests the progress byte and clears it holds that cell's address in one
 * register for the arm, taken from $t0 and from $s0 for the three arms that
 * clear it after a call; the frame keeps $ra and $s0. Every record field store
 * hops a fresh index register because the record store may alias the scratchpad
 * byte, so the source reads that byte again for each field.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepSubstatesThroughFrontEndContextsAndFieldRecordSlotsThenReleaseStateScenarioScena1200_801F9F0C(void) {
  switch (D_80146875) {
  case 0:
    D_80146866 = 1;
    D_80146865 = 0;
    D_80146875 = 1;
    D_80146258 ^= 0x20;
    func_8019FA28(0xBC, 0x68000, 0x598000, 0x80);
    D_80143F1F = 0xFF;
    break;
  case 1:
    D_8014832E = 0x1F;
    func_801C601C(0);
    D_80146875 = 2;
    break;
  case 2: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 5) {
      break;
    }
    func_8015B580(D_8014686C, 0x28);
    *progress = 0;
    D_80146866 = 1;
    D_80146875 = 3;
    func_8015B580(D_8014686C, 0x15);
    func_8019FA28(0x79, 0x238000, 0x310000, 6);
    D_80143F1F = 0xFF;
    break;
  }
  case 3:
    D_8014933B = 1;
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) == 0xFF) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x13;
    D_80143FC8[D_1F800000[0]].unk_64 = -0x370;
    D_80143FC8[D_1F800000[0]].unk_68 = D_801492DA;
    D_80143FC8[D_1F800000[0]].unk_6C = 0x200;
    D_80143FC8[D_1F800000[0]].unk_09 = 4;
    D_80146875 = 4;
    D_80146867 = D_1F800000[0];
    break;
  case 4: {
    u8 *record_index = &D_80146867;
    if (D_80143FC8[*record_index].unk_00 != 0) {
      break;
    }
    *record_index = 0;
    D_8014832E = 0x1F;
    D_80146875 = 5;
    break;
  }
  case 5: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 5) {
      break;
    }
    *progress = 0;
    D_80146866 = 2;
    D_80146865 = 1;
    D_80146875 = 6;
    func_8019FA28(0xBC, 0x68000, 0x598000, 0x81);
    D_80143F1F = 0x8F;
    break;
  }
  case 6: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 0xA) {
      break;
    }
    *progress = 0;
    D_80146866 = 3;
    D_80146865 = 2;
    D_80146875 = 7;
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0x100000, 0x490000, 0x82);
    D_80143F1F = 0x90;
    break;
  }
  case 7:
    func_801C601C(1);
    D_80146875 = 8;
    break;
  case 8: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 2) {
      break;
    }
    *progress = 0;
    D_80146866 = 4;
    D_80146865 = 3;
    D_80146875 = 9;
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0x3C0000, 0x2F0000, 0x83);
    D_80143F1F = 0x90;
    break;
  }
  case 9:
    func_801C601C(2);
    D_80146875 = 0xA;
    break;
  case 10: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      break;
    }
    *progress = 0;
    D_80146866 = 4;
    D_80146865 = 4;
    D_80146875 = 0xB;
    D_80143F1F = 0xFF;
    D_80146258 ^= 8;
    func_8019FA28(0xBC, 0x68000, 0x88000, 0x84);
    D_80143F1F = 0x90;
    break;
  }
  case 11: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      break;
    }
    *progress = 0;
    D_80146865 = 5;
    D_80146875 = 0xC;
    D_80146866 = 0;
    D_80143F1F = 0xFF;
    D_80146258 ^= 8;
    func_8019FA28(0xBC, 0x78000, 0x1F0000, 0x85);
    D_80143F1F = 0x90;
    break;
  }
  case 12: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 2) {
      break;
    }
    func_8015DF18(0x20C);
    *progress = 0;
    D_80146866 = 4;
    D_80146865 = 6;
    D_80146875 = 0xD;
    D_80143F1F = 0xFF;
    D_80146258 |= 8;
    func_8019FA28(0xBC, 0x120000, 0x340000, 0x89);
    D_80143F1F = 0x90;
    break;
  }
  case 13:
    func_801C601C(3);
    D_80146875 = 0xE;
    break;
  case 14: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 3) {
      break;
    }
    *progress = 0;
    D_80146865 = 7;
    D_80146875 = 0xF;
    D_80146866 = 0;
    D_80143F1F = 0xFF;
    D_80146258 ^= 8;
    func_8019FA28(0xBC, 0x1D8000, 0x98000, 0x8A);
    D_80143F1F = 0x90;
    break;
  }
  case 15: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      break;
    }
    *progress = 0;
    D_80146865 = 8;
    D_80146875 = 0x10;
    D_80146866 = 0;
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0x40000, 0x3E0000, 0x8B);
    D_80143F1F = 0x90;
    break;
  }
  case 16: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      break;
    }
    *progress = 0;
    D_80146875 = 0x11;
    D_80146866 = 0;
    D_80146865 = 0;
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0xA0000, 0x5A0000, 0x8C);
    D_80143F1F = 0x90;
    break;
  }
  case 17: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 1) {
      break;
    }
    *progress = 0;
    D_80146866 = 5;
    D_80146865 = 9;
    D_80146875 = 0x12;
    D_80143F1F = 0xFF;
    func_8019FA28(0xBC, 0x120000, 0x400000, 0x8D);
    D_80143F1F = 0x90;
    break;
  }
  case 18: {
    u8 *progress = &g_ScenarioProgress;
    if (*progress != 3) {
      break;
    }
    D_1F800000[0] = func_8019601C();
    if ((D_1F800000[0] & 0xFF) == 0xFF) {
      break;
    }
    D_80143FC8[D_1F800000[0]].unk_00 = 1;
    D_80143FC8[D_1F800000[0]].unk_05 = 0x7A;
    *progress = 4;
    D_80146875 = 0x13;
    break;
  }
  case 19:
    if (g_ScenarioProgress != 4) {
      break;
    }
    func_8015D554(0, 1, 4, 0x1E);
    func_8014ECAC(0xD);
    D_80146875 = 0x14;
    break;
  case 20:
    if (D_80143C40 != 0) {
      break;
    }
    D_8014832E = 0;
    D_80146875 = 0x15;
    break;
  case 21:
    D_80146866 = 6;
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146875 = 0x16;
    func_8015B580(D_8014686C, 0x29);
    func_8019FA28(0xBC, 0x120000, 0x400000, 0x8E);
    D_80143F1F = 0xFF;
    break;
  case 22:
    if (g_ScenarioProgress != 4) {
      break;
    }
    D_80146865 = 0;
    D_80146866 = 7;
    D_80146867 = 0;
    D_80146258 |= 0x20;
    func_8015C058();
    D_80146875 = 0;
    D_80146874 = 0;
    break;
  }
}
