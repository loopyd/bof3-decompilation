#include "bof3/bof3.h"

/* Pending world id halfword of the shared main executable: every arm of this
 * body gates on it with a fresh lhu and none of them writes it.
 * @source 0x80143F00 @kind data */
extern u16 D_80143F00;

/* Shared world sub-index byte of the main executable, read with lbu: the world
 * arm 0x85 requires it to read 2 before it requests the front-end context.
 * @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;

/* Front-end phase byte of the main executable, read with lbu: only its low
 * seven bits are used, as the selector of the phase switch the world arm 0x85
 * runs on state 3 to arm the shared front-end handlers.
 * @source 0x80145024 @kind unknown */
extern u8 D_80145024;

/* Shared effect bank byte of the main executable: the world arms 0x65, 0x79,
 * 0x82, 0x83, 0x84 and 0xBC arm it with 0x1F or clear it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared mode halfword of the main executable: the world arms 0x83 and 0x84
 * set bit 0x16, the arm 0x85 sets bit 0x08 and the arm 0xBC sets bits 0x0E.
 * @source 0x80146258 @kind unknown */
extern u16 D_80146258;

/* Shared effect id byte of the main executable: the cue arms store 2 into it.
 * @source 0x80149333 @kind unknown */
extern u8 D_80149333;

/* Shared scenario flag word of the main executable: the flag probes of the
 * world arms 0x85, 0x86 and 0xBC read it and the two arm helpers take it as
 * their first argument.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared scenario progress byte of the main executable, cleared by the world
 * arm 0x86 and by the closing tail.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared main-RAM state bytes of the main executable: the world arms 0x79,
 * 0x86 and the closing tail clear 0x80146865, 0x80146866 and 0x80146867, the
 * world arm 0x79 clears the state byte 0x80146875 and the world arm 0x85
 * requests sub-state 0x19 in it.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Secondary state byte of the main executable: the world arms 0x85 and 0x86
 * store 5 and 9 in it and the closing tail clears it.
 * @source 0x80146874 @kind unknown */
extern u8 D_80146874;

/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;

/* Shared scenario state byte of the main executable: it selects the word of
 * the overlay progress handler run at 0x801FD1B8 this body is word 1 of, and
 * every path of this body ends by storing 2 in it.
 * @source 0x80146872 @kind unknown */
extern u8 D_80146872;

/* Shared word of the main executable compared against 0x2FFFFF by the world
 * arm 0x86 before it clears the overlay state.
 * @source 0x80145EC4 @kind unknown */
extern s32 D_80145EC4;

/* Scratchpad byte at 0x1F800000: the world arm 0xBC stores the record index
 * the shared allocator answers here and then indexes the record run with this
 * byte. Every read after a record store is a real lbu, because that store may
 * alias the byte.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* One 0x74-byte front-end record of the shared main-RAM run the world arm 0xBC
 * fills in from the shared allocator: the arm stores an active byte, an arm
 * byte and one effect id in it.
 * @kind struct */
typedef struct {
  u8 unk_00;        /* 0x00 slot active byte, stored as 1 */
  u8 unk_01[4];     /* 0x01 arm byte, stored as 1 after unk_00 */
  u8 unk_05;        /* 0x05 effect id byte, 0x76, 0x79, 0x7B or 0x7C */
  u8 unk_06[0x6E];  /* 0x06 */
} FieldRecord;

/* Base of the shared 0x74-byte front-end record run of the main executable.
 * @source 0x80143FC8 @kind data */
extern FieldRecord D_80143FC8[];

/* Shared front-end flag tester at 0x8015B5D4 of the main executable: nonzero
 * when the bit is set in the flag word its first argument holds.
 * @source 0x8015B5D4 @kind function */
extern s32 func_8015B5D4(u32 flags, s32 id);

/* Shared front-end flag setter at 0x8015B580 of the main executable.
 * @source 0x8015B580 @kind function */
extern void func_8015B580(u32 flags, s32 bit);

/* Shared front-end startup helper at 0x8015C088 of the main executable.
 * @source 0x8015C088 @kind function */
extern void func_8015C088(void);

/* Shared front-end release helper at 0x8015C058 of the main executable.
 * @source 0x8015C058 @kind function */
extern void func_8015C058(void);

/* Shared cue dispatcher at 0x80154FD8 of the main executable: the world arms
 * 0x83 and 0xBC queue cues 0x498, 0x200, 0x290 and 0x100 through it.
 * @source 0x80154FD8 @kind function */
extern void func_80154FD8(u32 cue);

/* Shared front-end work index allocator at 0x8019601C of the main executable:
 * it answers the next free front-end record index, with 0xFF meaning none.
 * @source 0x8019601C @kind function */
extern s32 func_8019601C(void);

/* Shared scenario handler dispatchers at 0x801C1E3C and 0x801C1DF0 of the main
 * executable: every phase arm runs one indexed handler of each through them.
 * @source 0x801C1E3C @kind function */
extern void func_801C1E3C(u8 index);

/* @source 0x801C1DF0 @kind function */
extern void func_801C1DF0(u8 index);

/* Shared front-end selection starter at 0x80161BBC of the main executable: the
 * world arm 0xBC starts selection 0x8F and selection 0x8B through it.
 * @source 0x80161BBC @kind function */
extern void func_80161BBC(u32 selection_id);

/* Shared front-end selection poll at 0x80162D00 of the main executable:
 * nonzero when the running selection has finished.
 * @source 0x80162D00 @kind function */
extern s32 func_80162D00(void);

/* Shared front-end script step at 0x8014B87C of the main executable: the two
 * selection waits of the world arm 0xBC step it with the count 1.
 * @source 0x8014B87C @kind function */
extern void yieldCallbackSlotScheduler(u16 countdown); /* @source 0x8014B87C @kind data */

/* Shared front-end cue helper at 0x80161C20 of the main executable: both
 * selection waits close with selection id, cue level 0x64 and shape 8.
 * @source 0x80161C20 @kind function */
extern void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);

/* @source 0x801F7F78
 * @behavior Overlay per-frame progress handler, word 1 of the progress handler
 * run at 0x801FD1B8: the dispatcher callStateChainStep
 * (0x801F7F28) indexes that run with the shared scenario state byte D_80146872,
 * and the little-endian word at 0x801FD1BC holds this address, so it runs
 * while that byte reads 1; no jal and no other pointer word names it. It reads
 * the pending world id halfword D_80143F00 with a fresh lhu once per arm and
 * applies the arm of the world it holds, and every path ends by storing 2 in
 * D_80146872, which selects word 2 (0x801F886C, the second-level state
 * dispatcher) of the same run. World 0x65 switches on the shared state byte
 * D_80146866: state 1 arms the shared effect bank byte D_8014832E and state 2
 * clears it. World 0x79 takes the address of that state byte once and switches
 * on it: state 1 clears D_8014832E, and state 2 arms it, runs the shared
 * front-end release helper func_8015C058 and clears g_ScenarioProgress,
 * 0x80146865, the state byte itself and 0x80146867. World 0x82 arms D_8014832E
 * on state 1. World 0x83 dispatches cue 0x498 through func_80154FD8 and stores
 * 2 in D_80149333 on state 1, arms D_8014832E and sets bit 0x16 of the shared
 * mode halfword D_80146258 on state 2, and arms D_8014832E on state 3. World
 * 0x84 clears D_8014832E and sets bit 0x16 of D_80146258 on state 1 and arms
 * D_8014832E on state 2. World 0x85 first probes the flag word D_8014686C for
 * flags 0x24 and 0x26 and, only when the first reads set, the second clear and
 * D_80143F03 reads 2, runs func_8015C088 and requests sub-state 0x19 in
 * D_80146875 and state 5 in D_80146874; it then switches on D_80146866 through
 * the eleven-word absolute jump table at 0x801F6C0C, where state 3 sets bits
 * 0x0E of D_80146258 and switches on the low seven bits of the front-end phase
 * byte D_80145024 through the twelve-word absolute jump table at 0x801F6C3C
 * (phases 8, 13, 16 and 17 dispatch handlers 0, 1, 2 and 4 through
 * func_801C1E3C followed by func_801C1DF0 with ids 3, 3, 3 and 9), and state
 * 0x0A allocates a front-end record index and seeds that record. World 0x86
 * probes flag 0x2F and requires D_80145EC4 to be at most 0x2FFFFF, then sets
 * that flag through func_8015B580, runs func_8015C088 and clears
 * g_ScenarioProgress, 0x80146865, 0x80146866, 0x80146867 and D_80146875 before
 * storing 9 in D_80146874. World 0x87 seeds one front-end record on state
 * 0x0A. World 0x88 switches on D_80146866 through the eleven-word absolute jump
 * table at 0x801F6C6C: state 1 dispatches scenario handler 3, state 2
 * dispatches cue 0x200 and stores 2 in D_80149333, and state 0x0A seeds one
 * front-end record. World 0xBC switches on D_80146866 through the ten-word
 * absolute jump table at 0x801F6C9C and, on states 1 to 9, probes flag 0x28 or
 * 0x2F-free paths and seeds the front-end records with effect ids 0x76, 0x79,
 * 0x7B and 0x7C, sets bit 0x08 of D_80146258, queues cues 0x290 and 0x100 with
 * D_80149333 = 2, and runs the front-end selections 0x8F and 0x8B: it starts
 * one through func_80161BBC, steps yieldCallbackSlotScheduler(1) until func_80162D00 reports
 * the selection finished, and closes with func_80161C20(selection, 0x64, 8).
 * Worlds 0x2D, 0x41, 0x57, 0x10 and 0x73 fall into the clearing tail, which
 * clears g_ScenarioProgress, 0x80146865, 0x80146866, 0x80146867, D_80146875 and
 * D_80146874 in that order. It takes no arguments, returns nothing and its
 * 0x18-byte frame only keeps $ra and $s0, which holds the address of
 * D_80146866 across the world 0x79 arm. The body transcribes the statement
 * order, operand forms and declaration style of the exact cross-target sibling
 * applyPendingWorldIdSetupThenAdvanceStateTo2ScenarioScena1100_801F72E4
 * (0x801F72E4 of emi/scenario/scena11/00), which holds the same 0x65 to 0x88
 * world chain and the same clearing tail but no 0x85, 0x86, 0x87 or 0xBC arm,
 * and its record-seeding arms keep the same shape as the exact same-target
 * sibling stepSubstatesThroughContexts
 * (0x801F9F0C). The 0x8F4 bytes of this span have no byte-identical copy in any
 * shipped payload under out/binaries, so no clone applied.
 *
 * The first seed declared the record index and the record pointer once at
 * function scope, and psyq gcc then answered the three record-seeding arms with
 * a different register set: it summed the record address into $a0 and put the
 * stored constants in $v1 and $v0, where the original sums into $v0 and keeps
 * the constants in $a0 and $v1. That seed measured 555/573 instructions with
 * the three arms as the only differing hunks. Declaring the index and the
 * pointer inside each case block made every arm its own scope, and all 573
 * instructions then matched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePendingWorldSetup(void) {
  u8 *sub;

  if (D_80143F00 == 0x65) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      D_8014832E = 0x1F;
      break;
    case 2:
      D_8014832E = 0;
      break;
    }
  }
  if (D_80143F00 == 0x79) {
    sub = &D_80146866;
    switch (*sub) {
    case 0:
      break;
    case 1:
      D_8014832E = 0;
      break;
    case 2:
      D_8014832E = 0x1F;
      func_8015C058();
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      *sub = 0;
      D_80146867 = 0;
      break;
    }
  }
  if (D_80143F00 == 0x82) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      D_8014832E = 0x1F;
      break;
    }
  }
  if (D_80143F00 == 0x83) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      func_80154FD8(0x498);
      D_80149333 = 2;
      break;
    case 2:
      D_8014832E = 0x1F;
      D_80146258 |= 0x16;
      break;
    case 3:
      D_8014832E = 0x1F;
      break;
    }
  }
  if (D_80143F00 == 0x84) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      D_8014832E = 0;
      D_80146258 |= 0x16;
      break;
    case 2:
      D_8014832E = 0x1F;
      break;
    }
  }
  if (D_80143F00 == 0x85) {
    if (func_8015B5D4(D_8014686C, 0x24) != 0) {
      if (func_8015B5D4(D_8014686C, 0x26) == 0) {
        if (D_80143F03 == 2) {
          func_8015C088();
          D_80146875 = 0x19;
          D_80146874 = 5;
        }
      }
    }
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      D_80146258 |= 0xE;
      switch (D_80145024 & 0x7F) {
      case 7:
        break;
      case 8:
        func_801C1E3C(1);
        func_801C1DF0(3);
        break;
      case 9:
        break;
      case 10:
        break;
      case 11:
        break;
      case 12:
        break;
      case 13:
        func_801C1E3C(0);
        func_801C1DF0(3);
        break;
      case 14:
        break;
      case 15:
        break;
      case 16:
        func_801C1E3C(2);
        func_801C1DF0(3);
        break;
      case 17:
        func_801C1E3C(4);
        func_801C1DF0(9);
        break;
      case 18:
        break;
      }
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    case 8:
      break;
    case 9:
      break;
    case 10: {
      u8 index;
      u8 *record;

      index = func_8019601C();
      if (index == 0xFF) {
        break;
      }
      record = (u8 *)D_80143FC8 + (u32)index * 0x74;
      record[0] = 1;
      record[5] = 0x77;
      record[1] = 1;
      break;
    }
    }
  }
  if (D_80143F00 == 0x86) {
    if (func_8015B5D4(D_8014686C, 0x2F) == 0 && D_80145EC4 <= 0x2FFFFF) {
      func_8015B580(D_8014686C, 0x2F);
      func_8015C088();
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146875 = 0;
      D_80146874 = 9;
    }
  }
  if (D_80143F00 == 0x87) {
    switch (D_80146866) {
    case 0:
      break;
    case 10: {
      u8 index;
      u8 *record;

      index = func_8019601C();
      if (index == 0xFF) {
        break;
      }
      record = (u8 *)D_80143FC8 + (u32)index * 0x74;
      record[0] = 1;
      record[5] = 0x77;
      record[1] = 1;
      break;
    }
    }
  }
  if (D_80143F00 == 0x88) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      func_801C1E3C(3);
      break;
    case 2:
      func_80154FD8(0x200);
      D_80149333 = 2;
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    case 8:
      break;
    case 9:
      break;
    case 10: {
      u8 index;
      u8 *record;

      index = func_8019601C();
      if (index == 0xFF) {
        break;
      }
      record = (u8 *)D_80143FC8 + (u32)index * 0x74;
      record[0] = 1;
      record[5] = 0x77;
      record[1] = 1;
      break;
    }
    }
  }
  if (D_80143F00 == 0xBC) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      if (func_8015B5D4(D_8014686C, 0x28) != 0) {
        break;
      }
      D_1F800000[0] = func_8019601C();
      if ((D_1F800000[0] & 0xFF) == 0xFF) {
        break;
      }
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x76;
      break;
    case 2:
      D_1F800000[0] = func_8019601C();
      if ((D_1F800000[0] & 0xFF) != 0xFF) {
        D_80143FC8[D_1F800000[0]].unk_00 = 1;
        D_80143FC8[D_1F800000[0]].unk_05 = 0x79;
      }
      func_80161BBC(0x8F);
      while (func_80162D00() == 0) {
        yieldCallbackSlotScheduler(1);
      }
      func_80161C20(0x8F, 0x64, 8);
      break;
    case 3:
      D_80146258 |= 8;
      func_80154FD8(0x290);
      D_80149333 = 2;
      break;
    case 4:
      func_80154FD8(0x100);
      D_80149333 = 2;
      break;
    case 5:
      func_801C1E3C(2);
      func_801C1E3C(3);
      func_801C1DF0(8);
      break;
    case 6:
      D_8014832E = 0x1F;
      D_1F800000[0] = func_8019601C();
      if ((D_1F800000[0] & 0xFF) == 0xFF) {
        break;
      }
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x7B;
      break;
    case 7:
      D_1F800000[0] = func_8019601C();
      if ((D_1F800000[0] & 0xFF) == 0xFF) {
        break;
      }
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x7B;
      break;
    case 8:
      D_1F800000[0] = func_8019601C();
      if ((D_1F800000[0] & 0xFF) == 0xFF) {
        break;
      }
      D_80143FC8[D_1F800000[0]].unk_00 = 1;
      D_80143FC8[D_1F800000[0]].unk_05 = 0x7C;
      break;
    case 9:
      func_80161BBC(0x8B);
      while (func_80162D00() == 0) {
        yieldCallbackSlotScheduler(1);
      }
      func_80161C20(0x8B, 0x64, 8);
      break;
    }
  }
  if (D_80143F00 == 0x2D || D_80143F00 == 0x41 || D_80143F00 == 0x57 ||
      D_80143F00 == 0x10 || D_80143F00 == 0x73) {
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 0;
  }
  D_80146872 = 2;
}
