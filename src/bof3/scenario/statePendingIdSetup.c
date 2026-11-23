#include "bof3/scenario/scena06_state.h"

/* Pending world id halfword of the shared main image: every arm of this body
 * gates on it and none of them writes it. */
extern u16 D_80143F00;

/* Front-end phase byte of the shared main image: only its low seven bits are
 * read, as the selector of the phase switches in worlds 0x27 and 0x2E. */
extern u8 D_80145024;

/* Shared flag word this body probes through func_8015B5D4 and rewrites through
 * func_8015B580. */
extern u32 D_8014686C;

/* Shared effect bank byte the arms of worlds 0x35, 0x57, 0x5C and 0x5E arm with
 * 0x1F or clear. */
extern u8 D_8014832E;

/* Shared counter halfword world 0x5E state 2 arms with 0x480. */
extern u16 D_8014932C;

/* Shared effect id byte world 0x5E state 2 stores 2 into. */
extern u8 D_80149333;

/* Shared state byte the overlay dispatcher indexes with; this body advances it
 * to 2 on every path. */
extern s8 D_80146872;

/* Party record table of the shared main image, walked here at the 164-byte
 * stride of the records at 0x80144968: this body clears bit 0 of the first byte
 * of each row, the byte at record offset 7 that spimdisasm names 0x8014496F. */
extern u8 D_8014496F[][0xA4];

/* Shared party slot indexes the world 0x27 arms use to pick two record rows. */
extern u8 D_80181B12;
extern u8 D_80181B15;
extern u8 D_80181B16;

/* Shared flag tester of the shared main image: nonzero when the bit is set. */
s32 func_8015B5D4(u32 flags, u32 bit);

/* Shared flag writer of the shared main image: sets the bit. */
void func_8015B580(u32 flags, u32 bit);

/* Shared front-end startup helper of the shared main image. */
void func_8015C088(void);

/* Shared front-end release helper of the shared main image. */
void func_8015C058(void);

/* Shared scenario handler dispatchers: each runs one indexed handler of the
 * active scenario handler table, the first without and the second with the
 * overlay's own phase handover. */
void func_801C1E3C(u8 index);
void func_801C1DF0(u8 index);

/* Shared world and position helper of the shared main image. */
void func_801A78F8(void);

/* Shared effect helper of the shared main image taking three word arguments. */
void func_801BFE34(u32 arg0, u32 arg1, u32 arg2);

/* Overlay-local helper the arms of worlds 0x35 and 0x5E invoke. */
void func_801FCD38(void);

/* @source 0x801F7300
 * @behavior Word 1 of the three-word outer state table at 0x801FE3D8, which
 * callScenarioHandlerScenario (0x801F7298) indexes
 * with the signed shared main-RAM state byte D_80146872 (word 0 is the state-0
 * seeder stateSharedFieldsScenario at
 * 0x801F72D4 and word 2 is the primary state dispatcher
 * dispatchPrimaryStateTable at 0x801F7A00), so no
 * jal or j inside the payload reaches 0x801F7300. It reads the pending world id
 * halfword D_80143F00 (0x80143F00) once per arm and applies one arm per world
 * id, then stores 2 in D_80146872 so the next frame dispatches word 2. World
 * 0x27 probes flag 3 of the shared flag word D_8014686C (0x8014686C) and, only
 * when it reads clear, clears the progress byte g_ScenarioProgress
 * (0x80146864) and 0x80146865 and switches on the low seven bits of the
 * front-end phase byte D_80145024 (0x80145024): phase 0 clears bit 0 of the
 * record byte of party slot D_80181B15 (0x80181B15) and then of slot
 * D_80181B16 (0x80181B16), phase 1 clears slots D_80181B16 and D_80181B12
 * (0x80181B12) in that order, phase 2 clears slots D_80181B12 and D_80181B15,
 * and every phase then dispatches scenario handler 0, 1 or 2 through
 * func_801C1E3C, the record byte being bit 0 of the +7 byte of the 164-byte
 * records at 0x80144968. World 0x2E requires flag 0xB set and flag 0x3C clear,
 * then clears D_8014832E (0x8014832E), requests primary state 0x11 through the
 * progress helpers (func_8015C088, the four cleared state bytes and
 * D_80146874), sets flag 0x3C, and on the same phase byte dispatches handler 6
 * or 4 followed by handler 7 (phases 1 and 2), 8 (phases 3 and 4) or 9 (phase
 * 6) through func_801C1E3C and func_801C1DF0, phases 0 and 5 dispatching
 * nothing. World 0x35 dispatches handler 0 and clears g_ScenarioProgress with
 * 0x80146865..0x80146867 when flag 0xB reads clear, and when flag 0xE reads set
 * while flag 0x11 reads clear arms D_8014832E with 0x1F and runs func_801FCD38.
 * World 0x4C requires flag 0x2C set and flag 0x3F clear, then clears the same
 * four state bytes, sets flag 0x3F and runs func_801A78F8. World 0x57 takes the
 * address of the shared state byte D_80146866 (0x80146866) once and switches on
 * it: state 1 probes flag 0x1F, and state 2 probes flag 0x21 and sets it; on a
 * clear probe each runs func_8015C058, clears g_ScenarioProgress, 0x80146865,
 * that state byte itself, 0x80146867, D_80146875 and D_80146874 in that order,
 * state 1 arming D_8014832E with 0x1F first. World 0x5C switches on that state
 * byte: state 1 clears D_8014832E when flag 0x1A reads clear, state 2 arms it
 * with 0x1F, and state 4 runs func_801BFE34 with 0x1D0000, 0x3E0000 and 0x1B.
 * World 0x5E switches on that state byte again: state 1 arms D_8014832E with
 * 0x1F when flag 0xF reads clear, state 2 stores 0x480 in the shared counter
 * halfword D_8014932C (0x8014932C), clears 0x80146867 and D_8014832E, stores 2
 * in D_80149333 (0x80149333) and runs func_801FCD38 when flag 0x13 reads clear;
 * it then requires flag 0x11 set and flag 0x3B clear, sets flag 0x3B, clears
 * D_8014832E, runs func_801FCD38, requests secondary state 0x1E through
 * D_80146875 and primary state 6 through D_80146874. Worlds 0x2D, 0x41, 0x57
 * and 0x10 fall into the clearing tail, which clears g_ScenarioProgress,
 * 0x80146865, 0x80146866 and 0x80146867 in that order. Takes no arguments and
 * returns nothing; its 0x18-byte frame keeps $ra and $s0, which holds the
 * address of D_80146866 across the world 0x57 arm.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The record clears in world 0x27 fix the shape of this body: writing the
 * access as one flat expression, D_8014496F[slot * 0xA4] &= 0xFE, measured
 * 315/448 instructions because gcc materialized the symbol address into a
 * register with lui/addiu and folded the scaled index into it, and naming the
 * product in a u32 index local measured 416/448 with the address form right but
 * the index homed in $v1 instead of $v0. Writing the table as rows of 0xA4 and
 * indexing D_8014496F[slot][0] measured 446/448, the two remaining words being
 * the jump-table displacements of the held-out .rodata placement row.
 */
void statePendingIdSetup(void) {
  u8 *sub;

  if (D_80143F00 == 0x27) {
    if (func_8015B5D4(D_8014686C, 3) == 0) {
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      switch (D_80145024 & 0x7F) {
      case 0:
        D_8014496F[D_80181B15][0] &= 0xFE;
        D_8014496F[D_80181B16][0] &= 0xFE;
        func_801C1E3C(0);
        break;
      case 1:
        D_8014496F[D_80181B16][0] &= 0xFE;
        D_8014496F[D_80181B12][0] &= 0xFE;
        func_801C1E3C(1);
        break;
      case 2:
        D_8014496F[D_80181B12][0] &= 0xFE;
        D_8014496F[D_80181B15][0] &= 0xFE;
        func_801C1E3C(2);
        break;
      }
    }
  }
  if (D_80143F00 == 0x2E) {
    if (func_8015B5D4(D_8014686C, 0xB) != 0 &&
        func_8015B5D4(D_8014686C, 0x3C) == 0) {
      D_8014832E = 0;
      SCENA06_RESET_PROGRESS_AND_REQUEST(0x11);
      func_8015B580(D_8014686C, 0x3C);
      switch (D_80145024 & 0x7F) {
      case 0:
        break;
      case 1:
        func_801C1E3C(6);
        func_801C1DF0(7);
        break;
      case 2:
        func_801C1E3C(4);
        func_801C1DF0(7);
        break;
      case 3:
        func_801C1E3C(6);
        func_801C1DF0(8);
        break;
      case 4:
        func_801C1E3C(4);
        func_801C1DF0(8);
        break;
      case 5:
        break;
      case 6:
        func_801C1E3C(7);
        func_801C1DF0(9);
        break;
      }
    }
  }
  if (D_80143F00 == 0x35) {
    if (func_8015B5D4(D_8014686C, 0xB) == 0) {
      func_801C1DF0(0);
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
    }
    if (func_8015B5D4(D_8014686C, 0xE) != 0 &&
        func_8015B5D4(D_8014686C, 0x11) == 0) {
      D_8014832E = 0x1F;
      func_801FCD38();
    }
  }
  if (D_80143F00 == 0x4C) {
    if (func_8015B5D4(D_8014686C, 0x2C) != 0 &&
        func_8015B5D4(D_8014686C, 0x3F) == 0) {
      g_ScenarioProgress = 0;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      func_8015B580(D_8014686C, 0x3F);
      func_801A78F8();
    }
  }
  if (D_80143F00 == 0x57) {
    sub = &D_80146866;
    switch (*sub) {
    case 0:
      break;
    case 1:
      if (func_8015B5D4(D_8014686C, 0x1F) == 0) {
        D_8014832E = 0x1F;
        func_8015C058();
        g_ScenarioProgress = 0;
        D_80146865 = 0;
        *sub = 0;
        D_80146867 = 0;
        D_80146875 = 0;
        D_80146874 = 0;
      }
      break;
    case 2:
      if (func_8015B5D4(D_8014686C, 0x21) == 0) {
        func_8015B580(D_8014686C, 0x21);
        func_8015C058();
        g_ScenarioProgress = 0;
        D_80146865 = 0;
        *sub = 0;
        D_80146867 = 0;
        D_80146875 = 0;
        D_80146874 = 0;
      }
      break;
    }
  }
  if (D_80143F00 == 0x5C) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      if (func_8015B5D4(D_8014686C, 0x1A) == 0) {
        D_8014832E = 0;
      }
      break;
    case 2:
      D_8014832E = 0x1F;
      break;
    case 3:
      break;
    case 4:
      func_801BFE34(0x1D0000, 0x3E0000, 0x1B);
      break;
    }
  }
  if (D_80143F00 == 0x5E) {
    switch (D_80146866) {
    case 0:
      break;
    case 1:
      if (func_8015B5D4(D_8014686C, 0xF) == 0) {
        D_8014832E = 0x1F;
      }
      break;
    case 2:
      if (func_8015B5D4(D_8014686C, 0x13) == 0) {
        D_8014932C = 0x480;
        D_80146867 = 0;
        D_80149333 = 2;
        D_8014832E = 0;
        func_801FCD38();
      }
      break;
    }
    if (func_8015B5D4(D_8014686C, 0x11) != 0 &&
        func_8015B5D4(D_8014686C, 0x3B) == 0) {
      func_8015B580(D_8014686C, 0x3B);
      D_8014832E = 0;
      func_801FCD38();
      D_80146875 = 0x1E;
      D_80146874 = 6;
    }
  }
  if (D_80143F00 == 0x2D || D_80143F00 == 0x41 || D_80143F00 == 0x57 ||
      D_80143F00 == 0x10) {
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
  }
  D_80146872 = 2;
}
