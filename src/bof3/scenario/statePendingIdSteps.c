#include "bof3/scenario/scena09_state.h"

/* Pending world id halfword of the shared EXE image, read with lhu: every arm
 * of this body gates on it and none of them writes it.
 * @source 0x80143F00 @kind data */
extern u16 D_80143F00;

/* Shared world sub-index byte at 0x80143F03, read with lbu: the world arms 0x27
 * and 0x29 require it to read zero, the 0x31 arm requires one, the 0x64 arm
 * requires the route byte, and the 0x77 arm requires two.
 * @source 0x80143F03 @kind unknown */
extern u8 D_80143F03;

/* Front-end phase byte at 0x80145024, read with lbu: only its low seven bits
 * are used, as the selector of the phase switches the 0x25, 0x37, 0x45 and 0x31
 * arms run to arm the front-end handlers.
 * @source 0x80145024 @kind unknown */
extern u8 D_80145024;

/* Shared scenario flag word at 0x8014686C, read with lw: it is the bank whose
 * bits the probes func_8015B5D4 test and the set and clear helpers
 * func_8015B580 and func_8015B5A8 arm.
 * @source 0x8014686C @kind data */
extern u32 D_8014686C;

/* Shared byte flag bank at 0x80144EB8: the 0x25 arm sets bit 0x3E of it, the
 * one call here whose bank is not D_8014686C.
 * @source 0x80144EB8 @kind data */
extern u8 D_80144EB8;

/* Shared route byte at 0x80146865, cleared by the arms that finish a route
 * step.
 * @source 0x80146865 @kind unknown */
extern u8 D_80146865;

/* Shared route byte at 0x80146866, read with lbu and cleared by the arms that
 * finish a route step; the 0x64 arm switches on it and the 0x29 and 0x31 arms
 * test it against the constants 1 and 2.
 * @source 0x80146866 @kind unknown */
extern u8 D_80146866;

/* Shared record index byte at 0x80146867, cleared with the two route bytes
 * above.
 * @source 0x80146867 @kind unknown */
extern u8 D_80146867;

/* Shared scenario state byte at 0x80146872: this body ends by advancing it to
 * 2 on every path.
 * @source 0x80146872 @kind data */
extern u8 D_80146872;

/* Shared scenario progress byte at 0x80146864: the arms that arm a primary
 * state clear it.
 * @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;

/* Shared effect bank byte at 0x8014832E: the arms that arm a primary state
 * clear it.
 * @source 0x8014832E @kind unknown */
extern u8 D_8014832E;

/* Shared area mode byte at 0x80149333: the 0x37 and 0x31 arms publish mode 2
 * into it.
 * @source 0x80149333 @kind unknown */
extern u8 D_80149333;

/* Shared fixed-point halfword at 0x8014932C, read with lhu: the 0x37 arm raises
 * it by 0xA80 and the 0x31 arm by 0xB80.
 * @source 0x8014932C @kind data */
extern u16 D_8014932C;

/* Shared mode halfword at 0x80146258, read with lhu: the arms that leave a
 * world set bit 8 of it.
 * @source 0x80146258 @kind data */
extern u16 D_80146258;

/* Shared signed word at 0x80145ECC, read with lw: the 0x76 arm only seeds its
 * field records while it reads above 0x2000000.
 * @source 0x80145ECC @kind data */
extern s32 D_80145ECC;

/* Scratchpad byte at 0x1F800000: the 0x76 arm publishes the slot query answer
 * there and then indexes the shared record run with a fresh read of it.
 * @source 0x1F800000 @kind data */
extern u8 D_1F800000[];

/* Base of the shared field-record run at 0x80143FC8, walked here at the
 * 0x74-byte stride the sibling lifts of the same table use: the 0x76 arm seeds
 * the leading active byte of the record the slot query answered.
 * @source 0x80143FC8 @kind data */
extern u8 D_80143FC8[];

/* Arm byte at +5 of that same record, which the 0x76 arm stores 0x96 and then
 * 0x95 into, referenced at its own address.
 * @source 0x80143FCD @kind data */
extern u8 D_80143FCD[];

/* Shared flag tester of the shared EXE image: nonzero when the bit is set. */
extern s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Shared flag setter of the shared EXE image. */
extern void func_8015B580(u32 arg0, s32 arg1);

/* Shared flag clearer of the shared EXE image. */
extern void func_8015B5A8(u32 arg0, s32 arg1);

/* Shared front-end handler runner of the shared EXE image, and the two shared
 * scenario handler dispatchers that run one handler of the active scenario's
 * own handler table. */
extern void func_801C1E3C(u8 index);
extern void func_801C1DF0(u8 index);
extern void func_801C601C(u32 arg0);

/* Shared front-end work index allocator of the shared EXE image: it answers the
 * next free record index with 0xFF meaning none. */
extern s32 func_8019601C(void);

/* @source 0x801F7920
 * @behavior Per-frame pending-world-id handler of this overlay: it gates one
 * arm per pending world id halfword D_80143F00 (0x80143F00, read with lhu, never
 * written here) and then advances the shared scenario state byte D_80146872
 * (0x80146872) to 2 on every path, so the shared state dispatcher
 * callStateHandler2 (0x801F78D0) leaves
 * state 2 for the primary state this body armed. The arms 0x25, 0x27, 0x29,
 * 0x35, 0x37, 0x45, 0x31, 0x64, 0x76 and 0x77 each probe the shared flag word
 * D_8014686C (0x8014686C) with func_8015B5D4 and, where the probed step is
 * ready, clear the effect bank byte D_8014832E (0x8014832E), arm the primary
 * state byte D_80146874 (0x80146874) and the sub-state byte D_80146875
 * (0x80146875), clear the scenario progress byte g_ScenarioProgress
 * (0x80146864), leave the world through bit 8 of the mode halfword D_80146258
 * (0x80146258) and run the front-end handlers for the low seven bits of the
 * phase byte D_80145024 (0x80145024) through func_801C1E3C and func_801C1DF0
 * (the 0x25 and 0x37 arms through an absolute jump table, the 0x45 and 0x31
 * arms through a compare tree, the 0x45 arm leaving phase 10 in place and the
 * 0x31 arm leaving phase 9 in place). The 0x25 arm is the only one that sets a
 * bit of the second bank D_80144EB8 (0x80144EB8), bit 0x3E, before it clears
 * bit 0x2F of D_8014686C through func_8015B580. The 0x76 arm is the only one
 * that seeds the shared field-record run D_80143FC8 (0x80143FC8): it publishes
 * the slot query answer of func_8019601C in the scratchpad byte D_1F800000
 * (0x1F800000), seeds the record at the 0x74-byte stride unless the answer
 * reads 0xFF, stores the arm byte 0x96 in its +5 byte, and repeats once with
 * arm byte 0x95 while the shared signed word D_80145ECC (0x80145ECC) still
 * reads above 0x2000000. The last arm groups the pending world ids 0x2D, 0x41,
 * 0x57, 0x10 and 0x73, which only clear the progress byte and the three shared
 * route bytes D_80146865, D_80146866 and D_80146867. Its 0x28-byte frame holds
 * $ra and the four saved registers the record seeding keeps live. No in-image
 * jal targets the address; the overlay reaches this body only as the word the
 * state dispatcher and the loader publish, and the two absolute jump tables the
 * 0x25 and 0x37 arms index sit in the payload's own leading data span at
 * 0x801F6C04 and 0x801F6C34.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePendingIdSteps(void) {
  s32 state;

  if (D_80143F00 == 0x25) {
    if (func_8015B5D4(D_8014686C, 0x2E) != 0 &&
        func_8015B5D4(D_8014686C, 0x2F) == 0) {
      func_8015C088();
      func_8015B580((u32)&D_80144EB8, 0x3E);
      D_80146258 |= 8;
      func_8015B580(D_8014686C, 0x2F);
      D_80146874 = 0xF;
      D_8014832E = 0;
      D_80146875 = 0xA;
      g_ScenarioProgress = 0;
      switch (D_80145024 & 0x7F) {
      case 7:
        func_801C1E3C(2);
        func_801C1DF0(0xA);
        break;
      case 8:
        func_801C1E3C(2);
        func_801C1E3C(7);
        func_801C1DF0(0xB);
        break;
      case 9:
        func_801C1E3C(2);
        func_801C1E3C(8);
        func_801C1DF0(0xB);
        break;
      case 10:
        func_801C1E3C(2);
        func_801C1DF0(0xC);
        break;
      case 13:
        func_801C1E3C(7);
        func_801C1DF0(0xA);
        break;
      case 14:
        func_801C1E3C(8);
        func_801C1DF0(0xA);
        break;
      case 16:
        func_801C1E3C(7);
        func_801C1E3C(8);
        func_801C1DF0(0xB);
        break;
      case 17:
        func_801C1E3C(7);
        func_801C1DF0(0xC);
        break;
      case 18:
        func_801C1E3C(8);
        func_801C1DF0(0xC);
        break;
      }
    }
  }
  if (D_80143F00 == 0x27) {
    if (func_8015B5D4(D_8014686C, 0x30) == 0) {
      func_801C601C(2);
    }
    if (func_8015B5D4(D_8014686C, 0x36) != 0 &&
        func_8015B5D4(D_8014686C, 0x31) != 0 && D_80143F03 == 0 &&
        func_8015B5D4(D_8014686C, 0x34) == 0) {
      func_8015C088();
      D_80146875 = 0x46;
      g_ScenarioProgress = 0;
      D_80146874 = 0xF;
      D_80146258 |= 8;
    }
  }
  if (D_80143F00 == 0x29) {
    if (func_8015B5D4(D_8014686C, 0x32) == 0) {
      func_8015B580(D_8014686C, 0x32);
      D_8014832E = 0;
      func_8015C088();
      D_80146875 = 0x3C;
      g_ScenarioProgress = 0;
      D_80146874 = 0xF;
    }
    if (D_80146866 == 1 && func_8015B5D4(D_8014686C, 0x38) != 0 &&
        D_80143F03 == 0) {
      D_80146258 |= 8;
      D_8014832E = 0;
      func_8015C088();
      D_80146875 = 0x6E;
      g_ScenarioProgress = 0;
      D_80146874 = 0xF;
    }
  }
  if (D_80143F00 == 0x35) {
    if (func_8015B5D4(D_8014686C, 0x2E) == 0) {
      func_801C601C(2);
    }
  }
  if (D_80143F00 == 0x37) {
    if (func_8015B5D4(D_8014686C, 4) != 0 &&
        func_8015B5D4(D_8014686C, 5) == 0) {
      func_8015C088();
      D_80149333 = 2;
      D_80146874 = 3;
      D_8014832E = 0;
      D_80146875 = 0;
      g_ScenarioProgress = 0;
      D_8014932C += 0xA80;
      switch (D_80145024 & 0x7F) {
      case 7:
        func_801C1E3C(1);
        func_801C1DF0(2);
        break;
      case 8:
        func_801C1E3C(2);
        func_801C1DF0(4);
        break;
      case 10:
        func_801C1E3C(2);
        func_801C1DF0(3);
        break;
      case 13:
        func_801C1E3C(0);
        func_801C1DF0(2);
        break;
      case 15:
        func_801C1E3C(0);
        func_801C1DF0(3);
        break;
      case 16:
        break;
      case 17:
        break;
      }
    }
  }
  if (D_80143F00 == 0x45) {
    if (func_8015B5D4(D_8014686C, 0) == 0) {
      D_8014832E = 0;
      D_80146874 = 1;
      D_80146875 = 0;
      g_ScenarioProgress = 0;
      switch (D_80145024 & 0x7F) {
      case 7:
        func_801C1E3C(0);
        func_801C1DF0(0);
        break;
      case 10:
        break;
      case 15:
        func_801C1E3C(0);
        func_801C1DF0(1);
        break;
      }
    }
  }
  if (D_80143F00 == 0x31) {
    if (func_8015B5D4(D_8014686C, 2) != 0 &&
        func_8015B5D4(D_8014686C, 3) == 0) {
      func_801C601C(2);
      switch (D_80145024 & 0x7F) {
      case 7:
        func_801C1E3C(1);
        func_801C1DF0(2);
        break;
      case 10:
        func_801C1E3C(2);
        func_801C1DF0(3);
        break;
      case 15:
        func_801C1E3C(0);
        func_801C1DF0(3);
        break;
      }
    }
    if (func_8015B5D4(D_8014686C, 6) != 0 &&
        func_8015B5D4(D_8014686C, 7) == 0) {
      D_8014832E = 0;
      D_80146874 = 4;
      D_80146875 = 0;
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
    }
    if (D_80146866 == 1) {
      D_8014832E = 0;
      D_80149333 = 2;
      D_8014932C += 0xB80;
    }
    if (D_80143F03 == 1 && func_8015B5D4(D_8014686C, 0x1E) == 0) {
      func_8015B5A8(D_8014686C, 0x11);
    }
  }
  if (D_80143F00 == 0x64) {
    state = D_80146866;
    switch (state) {
    case 0:
    case 1:
      break;
    case 2:
      if (func_8015B5D4(D_8014686C, 0x38) != 0 && D_80143F03 == state) {
        D_8014832E = 0;
        func_8015C088();
        D_80146875 = 0x78;
        g_ScenarioProgress = 0;
        D_80146874 = 0xF;
      }
      break;
    }
  }
  if (D_80143F00 == 0x76 && D_80143F03 == 4) {
    if (func_8015B5D4(D_8014686C, 0x13) == 0) {
      func_8015C088();
      D_80146874 = 7;
      D_80146875 = 0;
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_1F800000[0] = func_8019601C();
      if ((D_1F800000[0] & 0xFFu) != 0xFFu) {
        D_80143FC8[(u32)D_1F800000[0] * 0x74] = 1;
        D_80143FCD[(u32)D_1F800000[0] * 0x74] = 0x96;
      }
      if (D_80145ECC <= 0x2000000) {
        D_1F800000[0] = func_8019601C();
        if ((D_1F800000[0] & 0xFFu) != 0xFFu) {
          D_80143FC8[(u32)D_1F800000[0] * 0x74] = 1;
          D_80143FCD[(u32)D_1F800000[0] * 0x74] = 0x95;
        }
      }
    } else {
      s32 *timer = &D_80145ECC;

      if (*timer <= 0x2000000) {
        D_1F800000[0] = func_8019601C();
        if ((D_1F800000[0] & 0xFFu) != 0xFFu) {
          D_80143FC8[(u32)D_1F800000[0] * 0x74] = 1;
          D_80143FCD[(u32)D_1F800000[0] * 0x74] = 0x96;
        }
        if (*timer <= 0x2000000) {
          D_1F800000[0] = func_8019601C();
          if ((D_1F800000[0] & 0xFFu) != 0xFFu) {
            D_80143FC8[(u32)D_1F800000[0] * 0x74] = 1;
            D_80143FCD[(u32)D_1F800000[0] * 0x74] = 0x95;
          }
        }
      }
    }
  }
  if (D_80143F00 == 0x77) {
    if (D_80143F03 == 2 && func_8015B5D4(D_8014686C, 0x3D) != 0 &&
        func_8015B5D4(D_8014686C, 0x16) == 0) {
      D_80146258 |= 8;
      D_8014832E = 0;
      func_8015C088();
      D_80146874 = 8;
      D_80146875 = 0x1E;
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      func_8015B580(D_8014686C, 0x16);
    }
  }
  if (D_80143F00 == 0x2D || D_80143F00 == 0x41 || D_80143F00 == 0x57 ||
      D_80143F00 == 0x10 || D_80143F00 == 0x73) {
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
  }
  D_80146872 = 2;
}
