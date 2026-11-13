#include "bof3/bof3.h"

/* Shared front-end mode byte of the shared EXE image: the arms that start a
 * front-end sequence leave 2 behind and the arms that wait on it refuse to act
 * while it reads 2. */
extern u8 D_80143BB0;

/* Front-end record table of the shared EXE image, declared once per record
 * field address the way the sibling lifts of this same table spell it: the
 * address of each byte and word the body stores carries its own symbol, so the
 * table base never becomes a hoisted register and every store re-forms its own
 * address from the scaled record index. The generated undefined_syms_auto.txt
 * rows bind these names. The record stride the index scales by is 0x74. */
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FFC[];
extern u8 D_80144000[];
extern u8 D_80144004[];

/* Front-selection id the front-end cue helpers take as their first argument. */
extern u8 D_80145029;

/* Shared pad-state halfwords of the shared EXE image: the held-state word the
 * record angle arm tests for the two rotation bits 0x2000 and 0x8000, and the
 * pressed-state word the same arm tests for bit 0x40 before it advances. */
extern u16 D_80145AA4;
extern u16 D_80145AA8;

/* Shared flag word and state bytes: the flag word the bit-set helper receives,
 * the primary state byte every clearing arm resets, the sub-state byte this
 * switch dispatches on, and the 16-bit countdown the timed arms walk. */
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

/* Shared record angle word of the shared EXE image: the two rotation arms read
 * it, mask its low 12 bits and publish the rotated value back into it. */
extern u32 D_80147B5C;

/* Scratchpad halfword the two record arms publish the masked angle into. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared scenario progress byte the later arms gate on and stride. */
extern u8 g_ScenarioProgress;

/* Shared front-end sprite/effect id helper at 0x8015DF18. */
void func_8015DF18(u16 arg0);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared front-end action queue helper at 0x80150224. */
void func_80150224(u32 arg0);
/* Shared main-executable helper at 0x8015507C; role not established. */
void func_8015507C(u32 arg0, u32 arg1);
/* Shared front-end pair helper at 0x801636A0. */
void func_801636A0(u32 arg0, u32 arg1);
/* Shared main-executable helper at 0x80163EA0; the arms test its low byte. */
u32 func_80163EA0(void);
/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared front-end record index source at 0x8019601C; 0xFF means none. */
extern u8 func_8019601C(void);
/* Overlay-local rotation publisher of this same target. */
void setSharedIds11To13And10FromAngleOffsetsScenarioScena1300_801F9E70(
    s32 arg0);

/* @source 0x801F9A44
 * @behavior Primary state 5 handler of this overlay: entry 5 of the nine-entry
 * primary-state handler run at 0x801FBF54 (word 0x801FBF68, payload offset
 * 0x5368), the only word or jal in the payload that names this address, which
 * the exact dispatcher dispatchPrimaryStateHandlerScenarioScena1300_801F8474
 * (0x801F8474) selects with the shared primary state byte D_80146874 and calls
 * with no arguments. It reads the shared sub-state byte D_80146875 with lbu,
 * bounds it against 0x11 with sltiu and dispatches through the seventeen-word
 * absolute table this overlay keeps at 0x801F6D34 in its leading table data,
 * whose entries for sub-states 4, 7, 8 and 9 point at the body exit, so those
 * four values are the default and the body has cases 0 to 3 and 5, 6, 0x0A to
 * 0x10.
 *
 * Sub-state 0 cues the front end with func_80161CD0(D_80145029, 0x64, 8), arms
 * sprite ids 0x208 and 0x207 through func_8015DF18 and advances to 1.
 * Sub-state 1 publishes the shared record angle D_80147B5C (0x80147B5C) masked
 * with 0xFFF through the overlay's own
 * setSharedIds11To13And10FromAngleOffsetsScenarioScena1300_801F9E70 and
 * advances to 2. Sub-state 2 takes that cell's address once, publishes its
 * masked low twelve bits as the scratchpad halfword 0x1F800000, and while the
 * shared held-pad word D_80145AA4 (0x80145AA4) carries bit 0x2000 adds 0x10 to
 * the whole 32-bit angle or while it carries bit 0x8000 subtracts 0x10, in
 * either case storing the rotated word back through the same cell address and
 * re-publishing its masked value through the rotation publisher; the two arms
 * end on one shared tail. It then advances to 3 while the pressed-pad word
 * D_80145AA8 (0x80145AA8) carries bit 0x40. Sub-state 3 publishes the same
 * masked angle and, while the masked value lies inside 0x4A0 to 0x4E0
 * (unsigned `value - 0x4A0 < 0x41`), advances to 0x0A and otherwise to 5.
 *
 * Sub-state 5 arms sprite ids 0x20A and 0x209, cues the front end with
 * func_80161C20(D_80145029, 0x6E, 0x20), seeds the countdown halfword
 * D_80146876 (0x80146876) with 0x30 and advances to 6. Sub-state 6 decrements
 * that countdown through its own address and, when the halfword reaches zero,
 * runs the release helper func_8015C058 and func_801BE1B0(1) and then clears
 * both D_80146874 and D_80146875. Sub-state 0x0A re-arms sprite ids 0x20A and
 * 0x209, seeds the countdown with 0x1E and advances to 0x0B. Sub-state 0x0B
 * decrements the countdown the same way and, when it reaches zero, runs
 * func_801BE1B0(2), queues action 0x0A with func_8015507C(0x0A, 8) and
 * advances to 0x0C. Sub-state 0x0C, once the shared progress byte
 * g_ScenarioProgress (0x80146864) reads 1, asks func_8019601C for a front-end
 * record index and, while that index is not 0xFF, claims the record it scales
 * by 0x74: flag byte D_80143FC8 set to 1, byte D_80143FCD set to 0x7F and the
 * three words D_80143FFC, D_80144000 and D_80144004 set to 0x98000, 0x68000
 * and 0xF000000; it then seeds the countdown with 0x28 and advances to 0x0D.
 * Sub-state 0x0D decrements the countdown and, at zero, stores 3 in
 * g_ScenarioProgress and advances to 0x0E. Sub-state 0x0E, once that progress
 * byte reads 4, runs func_801636A0(5, 1), queues action 5 through
 * func_80150224, stores 2 in the shared front-end mode byte D_80143BB0
 * (0x80143BB0) and advances to 0x0F. Sub-state 0x0F, while func_80163EA0
 * answers a non-zero low byte and the mode byte leaves 2, cues the front end
 * with func_80161C20(D_80145029, 0x6E, 0x20), advances to 0x10 through a
 * pointer on g_ScenarioProgress and increments that progress byte. Sub-state
 * 0x10, once the progress byte reads 0, runs func_8015C058, sets bit 0x10 of
 * D_8014686C through func_8015B580 and falls into the same clearing tail of
 * D_80146874 and D_80146875 that sub-state 6 ends on.
 *
 * The body takes no arguments and returns nothing; its 0x18-byte frame only
 * keeps $ra across the calls, and the sub-state read is hoisted above that
 * frame's allocation, which is why the original bytes open with the
 * `lui`/`lbu` pair on D_80146875 before the `addiu $sp`/`sw $ra` prologue. The
 * three countdown arms take the countdown's address into a local pointer so
 * one `lui`/`addiu` base reaches both the `lhu` and the `sh`, the way the exact
 * sibling runPrimaryState8SubstateSequenceScenarioScena1300_801FB1B4
 * (0x801FB1B4) spells its own progress-byte pair, and sub-state 2 likewise
 * keeps the record angle cell's address in a local pointer. Sub-states 2, 6
 * and 0x10 end on statement groups that a later arm repeats, which is why one
 * copy of each of those groups serves several table entries in the original
 * bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runPrimaryState5SubstateSequenceScenarioScena1300_801F9A44(void) {
  switch (D_80146875) {
  case 0:
    func_80161CD0(D_80145029, 0x64, 8);
    func_8015DF18(0x208);
    func_8015DF18(0x207);
    D_80146875 = 1;
    break;
  case 1:
    setSharedIds11To13And10FromAngleOffsetsScenarioScena1300_801F9E70(
        D_80147B5C & 0xFFF);
    D_80146875 = 2;
    break;
  case 2: {
    u32 *pAngle = &D_80147B5C;
    u32 angle = *pAngle;

    D_1F800000 = angle & 0xFFF;
    if (D_80145AA4 & 0x2000) {
      angle += 0x10;
      *pAngle = angle;
      setSharedIds11To13And10FromAngleOffsetsScenarioScena1300_801F9E70(
          angle & 0xFFF);
    } else if (D_80145AA4 & 0x8000) {
      angle -= 0x10;
      *pAngle = angle;
      setSharedIds11To13And10FromAngleOffsetsScenarioScena1300_801F9E70(
          angle & 0xFFF);
    }
    if (D_80145AA8 & 0x40) {
      D_80146875 = 3;
    }
    break;
  }
  case 3: {
    u32 angle = D_80147B5C & 0xFFF;

    D_1F800000 = angle;
    if (angle - 0x4A0 < 0x41) {
      D_80146875 = 0xA;
    } else {
      D_80146875 = 5;
    }
    break;
  }
  case 4:
    break;
  case 5:
    func_8015DF18(0x20A);
    func_8015DF18(0x209);
    func_80161C20(D_80145029, 0x6E, 0x20);
    D_80146876 = 0x30;
    D_80146875 = 6;
    break;
  case 6: {
    u16 *countdown = &D_80146876;

    *countdown = *countdown - 1;
    if (*countdown != 0) {
      break;
    }
    func_8015C058();
    func_801BE1B0(1);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  }
  case 7:
  case 8:
  case 9:
    break;
  case 10:
    func_8015DF18(0x20A);
    func_8015DF18(0x209);
    D_80146876 = 0x1E;
    D_80146875 = 0xB;
    break;
  case 11: {
    u16 *countdown = &D_80146876;

    *countdown = *countdown - 1;
    if (*countdown != 0) {
      break;
    }
    func_801BE1B0(2);
    func_8015507C(0xA, 8);
    D_80146875 = 0xC;
    break;
  }
  case 12: {
    u8 index;

    if (g_ScenarioProgress != 1) {
      break;
    }
    index = func_8019601C();
    if (index != 0xFF) {
      D_80143FC8[(u32)index * 0x74u] = 1;
      D_80143FCD[(u32)index * 0x74u] = 0x7F;
      *(u32 *)(D_80143FFC + (u32)index * 0x74u) = 0x98000;
      *(u32 *)(D_80144000 + (u32)index * 0x74u) = 0x68000;
      *(u32 *)(D_80144004 + (u32)index * 0x74u) = 0xF000000;
    }
    D_80146876 = 0x28;
    D_80146875 = 0xD;
    break;
  }
  case 13: {
    u16 *countdown = &D_80146876;

    *countdown = *countdown - 1;
    if (*countdown != 0) {
      break;
    }
    g_ScenarioProgress = 3;
    D_80146875 = 0xE;
    break;
  }
  case 14:
    if (g_ScenarioProgress != 4) {
      break;
    }
    func_801636A0(5, 1);
    func_80150224(5);
    D_80143BB0 = 2;
    D_80146875 = 0xF;
    break;
  case 15: {
    u8 *progress;

    if ((u8)func_80163EA0() == 0) {
      break;
    }
    if (D_80143BB0 == 2) {
      break;
    }
    func_80161C20(D_80145029, 0x6E, 0x20);
    progress = &g_ScenarioProgress;
    D_80146875 = 0x10;
    *progress = *progress + 1;
    break;
  }
  case 16:
    if (g_ScenarioProgress != 0) {
      break;
    }
    func_8015C058();
    func_8015B580(D_8014686C, 0x10);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  }
}
