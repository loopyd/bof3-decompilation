#include "bof3/bof3.h"

/* Pending world id halfword of the shared EXE image: every arm of this body
 * gates on it and none of them writes it. */
extern u16 D_80143F00;

/* Front-end phase byte of the shared EXE image: only its low seven bits are
 * read, as the selector of the phase switch in world arm 0x65. */
extern u8 D_80145024;

/* Shared effect bank byte this body arms per world id. */
extern u8 D_8014832E;

/* Shared mode halfword whose bit 0x16 the world arms 0x83 and 0x84 set. */
extern u16 D_80146258;

/* Shared effect id byte the cue arms store 2 into. */
extern u8 D_80149333;

/* Front-end record table of the shared EXE image, walked here at the 0x74-byte
 * stride the sibling lifts of the same table use. */
extern u8 D_80143FC8[];

/* Shared flag word the two flag probes read. */
extern u32 D_8014686C;

/* Shared scenario progress byte and shared state bytes this body clears. */
extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

/* Shared scenario state byte the progress dispatcher indexes; this body only
 * advances it to 2. */
extern u8 D_80146872;

/* Shared scenario handler dispatchers of the shared EXE image: each runs one
 * indexed handler of the active scenario's own handler table. */
void func_801C1E3C(u8 index);
void func_801C1DF0(u8 index);

/* Shared flag tester of the shared EXE image: nonzero when the bit is set. */
s32 func_8015B5D4(u32 flags, u32 bit);

/* Shared front-end startup helper of the shared EXE image. */
void func_8015C088(void);

/* Shared cue dispatcher of the shared EXE image. */
void func_80154FD8(u32 cue);

/* Shared front-end work index allocator of the shared EXE image. */
u8 func_8019601C(void);

/* @source 0x801F72E4
 * @behavior Word 1 of the overlay progress handler run at 0x801FAEF0, the word
 * the dispatcher dispatchProgressHandlerScenarioScena1100_801F7288 (0x801F7288)
 * reads through the shared scenario state byte D_80146872 (word 0 is the
 * armEffectBankAndAdvanceStateScenarioScena1100_801F72C4 address that stores 1,
 * word 1 is this address, word 2 is dispatchSecondaryStateScenarioScena1100_801F7748),
 * so no jal or j inside the payload reaches 0x801F72E4. It reads the pending
 * world id halfword D_80143F00 (0x80143F00, `lhu`) once per arm and applies one
 * arm per world id. World 0x65 takes the address of the state byte D_80146866
 * once, and switches on it: state 1 arms the shared effect bank byte D_8014832E
 * (0x8014832E) with 0x1F and state 2 clears it, while state 0 re-probes the
 * shared flag word D_8014686C (0x8014686C, `lw`) through func_8015B5D4 with ids
 * 12 and 14 and, only when the first reads set and the second clear, runs the
 * shared front-end startup helper func_8015C088, clears g_ScenarioProgress,
 * 0x80146865, 0x80146866 and 0x80146867, clears D_8014832E and D_80146875,
 * stores 8 in D_80146874, and switches on the low seven bits of the front-end
 * phase byte D_80145024 (0x80145024) through the twelve-word absolute jump
 * table at 0x801F6C04 in this payload leading table data: phases 7, 8, 9, 10,
 * 13, 14, 15, 16 and 18 dispatch the active scenario's own handlers through
 * func_801C1E3C and func_801C1DF0 with ids 0, 1 and 2, phases 11, 12 and 17 are
 * the table words that point at the body exit, and phases 7, 9 and 14 run two
 * func_801C1E3C handlers before the func_801C1DF0 pair. World 0x79 clears D_8014832E when the
 * state byte reads 1, world 0x82 arms it with 0x1F on that same state, world
 * 0x83 dispatches cue 0x498 through func_80154FD8 and stores 2 in D_80149333 on
 * state 1, arms D_8014832E with 0x1F and sets bit 0x16 of the shared mode
 * halfword D_80146258 (0x80146258) on state 2, and arms D_8014832E with 0x1F on
 * state 3; world 0x84 sets bit 0x16 of D_80146258 and clears D_8014832E on
 * state 1 and arms D_8014832E with 0x1F on state 2; world 0x88 switches on the
 * state byte through the eleven-word absolute jump table at 0x801F6C34: state 1
 * dispatches scenario handler 3, state 2 dispatches cue 0x200 through
 * func_80154FD8 and stores 2 in D_80149333, and state 10 asks the shared work
 * index allocator func_8019601C for an index and, unless the masked index reads
 * 0xFF, seeds that front-end record in D_80143FC8 (0x80143FC8) at the 0x74-byte
 * stride with byte 0 = 1, byte 5 = 0x77 and byte 1 = 1. Worlds 0x2D, 0x41, 0x57,
 * 0x10 and 0x73 fall into the clearing tail, which clears g_ScenarioProgress,
 * 0x80146865, 0x80146866, 0x80146867, D_80146875 and D_80146874 in that order.
 * Every path ends by storing 2 in D_80146872, which selects word 2 of the same
 * progress run. Takes no arguments and returns nothing; its 0x18-byte frame
 * keeps $ra and $s0, and $s0 holds the address of D_80146866 across the whole
 * world 0x65 arm.
 *
 * The first seed listed only the state values that carry statements, and psyq
 * gcc answered the world 0x88 switch with a compare tree there: that spelling
 * measured 0x478 bytes against the original 0x464 and emitted one jump table.
 * Writing every state value as its own case made the eleven-word absolute table
 * at 0x801F6C34, and that tree spelling was the only measured miss before the
 * exact pass.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyPendingWorldIdSetupThenAdvanceStateTo2ScenarioScena1100_801F72E4(void) {
  u8 *sub;
  u8 index;
  u8 *record;

  if (D_80143F00 == 0x65) {
    sub = &D_80146866;
    switch (*sub) {
    case 0:
      if (func_8015B5D4(D_8014686C, 12) == 0) {
        break;
      }
      if (func_8015B5D4(D_8014686C, 14) != 0) {
        break;
      }
      func_8015C088();
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      *sub = 0;
      D_80146867 = 0;
      D_8014832E = 0;
      D_80146875 = 0;
      D_80146874 = 8;
      switch (D_80145024 & 0x7F) {
      case 7:
        func_801C1E3C(1);
        func_801C1E3C(0);
        func_801C1DF0(0);
        break;
      case 8:
        func_801C1E3C(1);
        func_801C1DF0(1);
        break;
      case 9:
        func_801C1E3C(1);
        func_801C1E3C(2);
        func_801C1DF0(0);
        break;
      case 10:
        func_801C1E3C(1);
        func_801C1DF0(2);
        break;
      case 13:
        func_801C1E3C(0);
        func_801C1DF0(1);
        break;
      case 14:
        func_801C1E3C(0);
        func_801C1E3C(2);
        func_801C1DF0(0);
        break;
      case 15:
        func_801C1E3C(0);
        func_801C1DF0(2);
        break;
      case 16:
        func_801C1E3C(2);
        func_801C1DF0(1);
        break;
      case 18:
        func_801C1E3C(2);
        func_801C1DF0(2);
        break;
      }
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
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      D_8014832E = 0;
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
    case 10:
      index = func_8019601C();
      if (index == 0xFF) {
        break;
      }
      record = D_80143FC8 + (u32)index * 0x74;
      record[0] = 1;
      record[5] = 0x77;
      record[1] = 1;
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
