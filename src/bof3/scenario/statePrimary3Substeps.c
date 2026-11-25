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
extern u8 D_80143FD1[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];

/* Shared front-end selector halfword and the three words the later arms seed
 * around the shared distance helper: the two context words, the halfword it
 * answers into, and the two sub-state gate bytes the front selector seeds. */
extern u32 D_80143F7C;
extern u32 D_80143F80;
extern u16 D_80143F86;
extern u8 D_80143F1D;
extern u8 D_80143F1E;

/* Front-selection id the front-end cue helpers take as their first argument. */
extern u8 D_80145029;

/* Shared flag word and state bytes: the flag word the bit-set helper receives,
 * the primary state byte every clearing arm resets, the sub-state byte this
 * switch dispatches on, and the 16-bit countdown the timed arms walk. */
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

/* Shared effect-bank byte the front-end arms clear and re-arm, and the shared
 * gate halfword the two late arms wait on. */
extern u8 D_8014832E;
extern u16 D_80143C40;

/* Shared record angle halfword the claimed front-end record takes as its +0x68
 * word, and the record angle pair the late arm seeds. */
extern s16 D_801492DA;
extern s16 D_801492D8;
extern s16 D_801492DC;

/* Shared context words the mode arm seeds, then hands to the distance helper,
 * plus the halfword it strides by 0x800 and the byte flag it sets and clears. */
extern u32 D_80149308;
extern u32 D_8014930C;
extern u16 D_80149338;
extern u8 D_8014933B;

/* Overlay-local front-end record selector: the id the shared allocator answers
 * and the arms scale into the record table. */
extern u8 D_801FBFEC;

/* Shared scenario progress byte the early and late arms gate on. */
extern u8 g_ScenarioProgress;

/* Shared front-end record release helper at 0x8014ECAC. */
void func_8014ECAC(u16 arg0);
/* Shared front-end action queue helper at 0x80150224. */
void func_80150224(u32 action);
/* Shared main-executable helper at 0x80153DE8; role not established. */
void func_80153DE8(void);
/* Shared distance helper at 0x8015477C; it answers a signed word. */
s32 func_8015477C(s32 x, s32 y);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 flags, s32 bit);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared front-end startup helper at 0x8015C088. */
void func_8015C088(void);
/* Shared main-executable helper at 0x801661C4; role not established. */
void func_801661C4(s32 arg0);
/* Shared front-selector context seeder at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared free front-end record selector at 0x8019601C; 0xFF means none. */
s32 func_8019601C(void);
/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Shared main-executable helper at 0x801C601C; role not established. */
void func_801C601C(s32 arg0);

/* @source 0x801F8DFC
 * @behavior Entry 3 of the nine-entry primary-state handler table at
 * 0x801FBF54 that the exact dispatcher
 * callPrimaryStateHandler3 (0x801F8474) selects
 * with the shared primary state byte D_80146874 and calls with no arguments, so
 * no in-image jal targets 0x801F8DFC and the word at 0x801FBF60 (payload offset
 * 0x5360) is its only reference: the sub-state switch of the overlay primary
 * state 3 handler. It reads the shared sub-state byte D_80146875 with lbu,
 * bounds it against 0x19 with sltiu and dispatches through the twenty-five-word
 * absolute table this overlay keeps at 0x801F6C4C in its leading table data,
 * whose entries for sub-states 6 to 9 and 0x0E to 0x13 point at the body exit,
 * so those ten values are the default and the body has cases 0 to 5, 0x0A to
 * 0x0D and 0x14 to 0x18.
 *
 * Sub-states 0 and 2 claim a front-end record with func_8019601C once the
 * shared progress byte g_ScenarioProgress reads 2 and 3, publish the answered
 * id in D_801FBFEC and, while that id is not 0xFF, fill the 0x74-byte record it
 * scales: the flag byte D_80143FC8 set to 1, the byte D_80143FCD set to 0x13
 * and the words D_8014402C, D_80144034 and D_80144030 set to -0x370 or -0x2AA,
 * 0x299 or 0x200, and the shared angle halfword D_801492DA, with the byte
 * D_80143FD1 set to 0xFF or 0x40. Sub-state 0 then lowers the shared halfword
 * D_80149338 by 0x800, seeds the countdown D_80146876 with 0x80 and advances to
 * 1; sub-state 2 seeds that countdown with 0x20 and advances to 3. Sub-states 1
 * and 3 decrement the countdown through its own address and, at zero, store 1
 * or 0 into the shared byte D_8014933B and advance to 2 or 4. Sub-state 1
 * lowers D_80149338 by 0x800; sub-state 3 raises it by 0x800. Sub-state 4, once the progress byte reads zero, raises D_80149338 by
 * 0x800, sets bit 7 of D_8014686C through func_8015B580 and clears both state
 * bytes. Sub-state 5 runs the shared release helper func_8015C058 and
 * func_801BE1B0(7), then clears both state bytes.
 *
 * Sub-state 0x0A runs func_801661C4(0x0B) and func_801C601C(2) and advances to
 * 0x0B. Sub-state 0x0B, once the progress byte reads 0x12, clears the shared
 * effect bank D_8014832E, queues action 0x18, stores 2 in the front-end mode
 * byte D_80143BB0 and advances to 0x0C. Sub-state 0x0C, while that mode byte
 * leaves 2, seeds D_80149308 and D_8014930C with 0xA0000 and 0x620000 and the
 * same pair into D_80143F7C and D_80143F80, stores the distance helper answer
 * func_8015477C(D_80149308, D_8014930C) into the halfword D_80143F86, runs
 * func_80153DE8 and queues action 0x19, stores 2 in the mode byte, arms the
 * effect bank with 0x1F, seeds the shared angle pair D_801492D8 and D_801492DC
 * with -0x344 and 0x3A2 and then claims and fills one front-end record the same
 * way sub-state 2 does, with 0x64 in D_80143FD1. It advances to 0x0D only
 * when allocation succeeds. Sub-state
 * 0x0D, while the claimed record flag byte D_80143FC8 still carries no bit 0
 * and the mode byte leaves 2, sets bit 9 of D_8014686C, seeds the front
 * selector context func_8019FA28(0x91, 0x3F8000, 0x5E0000, 0x88), stores 0x5C
 * in D_80145029 and clears both state bytes.
 *
 * Sub-state 0x14, once the progress byte reads 1, runs the shared front-end
 * startup helper func_8015C088, seeds func_8019FA28(0x58, 0x130000, 0x250000,
 * 0x80), stores 1 in D_80143F1D and advances to 0x15. Sub-state 0x15, while the
 * shared gate halfword D_80143C40 reads zero, seeds the countdown with 0x3C and
 * advances to 0x16. Sub-state 0x16 walks that countdown and, at zero, claims one
 * front-end record and keeps the returned id local. When it is not 0xFF, it sets its
 * flag byte and its byte D_80143FCD to 1 and 0x80 before re-seeding the
 * countdown with 0x46 and advancing to 0x17. Sub-state 0x17 walks the countdown
 * and, at zero, seeds func_8019FA28(0x70, 0xC8000, 0x580000, 0x81), clears
 * D_80143F1E and advances to 0x18. Sub-state 0x18, while D_80143C40 still reads
 * zero, runs func_8015C058, sets bit 0x0B of D_8014686C and clears both state
 * bytes.
 *
 * The body takes no arguments and returns nothing; its 0x18-byte frame keeps
 * $ra at 0x14 and $s0 at 0x10, the sub-state read is hoisted above that frame
 * allocation, and the arms that must hold a halfword or byte address across a
 * call take it into the saved register, so their later store reaches the same
 * cell through it instead of re-forming the address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePrimary3Substeps(void) {
  switch (D_80146875) {
  case 0:
    if (g_ScenarioProgress != 2) {
      break;
    }
    D_801FBFEC = func_8019601C();
    if (D_801FBFEC != 0xFF) {
      u8 slot;
      s32 angle;
      u16 delta;

      D_80143FC8[D_801FBFEC * 116] = 1;
      D_80143FCD[D_801FBFEC * 116] = 0x13;
      slot = D_801FBFEC;
      *(s32 *)(D_8014402C + slot * 116) = -0x370;
      angle = D_801492DA;
      D_80143FD1[slot * 116] = 0xFF;
      delta = D_80149338;
      *(s32 *)(D_80144034 + slot * 116) = 0x299;
      *(s32 *)(D_80144030 + slot * 116) = angle;
      D_80149338 = delta - 0x800;
      D_80146876 = 0x80;
      D_80146875 = 1;
    }
    break;
  case 1: {
    u16 *countdown = &D_80146876;

    *countdown = *countdown - 1;
    if (*countdown != 0) {
      break;
    }
    D_8014933B = 1;
    D_80146875 = 2;
    D_80149338 = D_80149338 - 0x800;
    break;
  }
  case 2:
    if (g_ScenarioProgress != 3) {
      break;
    }
    D_801FBFEC = func_8019601C();
    if (D_801FBFEC != 0xFF) {
      u8 slot;
      s32 angle;

      D_80143FC8[D_801FBFEC * 116] = 1;
      D_80143FCD[D_801FBFEC * 116] = 0x13;
      slot = D_801FBFEC;
      *(s32 *)(D_8014402C + slot * 116) = -0x2AA;
      angle = D_801492DA;
      *(s32 *)(D_80144034 + slot * 116) = 0x200;
      D_80143FD1[slot * 116] = 0x40;
      *(s32 *)(D_80144030 + slot * 116) = angle;
      D_80146876 = 0x20;
      D_80146875 = 3;
    }
    break;
  case 3: {
    u16 *countdown = &D_80146876;

    *countdown = *countdown - 1;
    if (*countdown != 0) {
      break;
    }
    D_8014933B = 0;
    D_80146875 = 4;
    D_80149338 = D_80149338 + 0x800;
    break;
  }
  case 4:
    if (g_ScenarioProgress != 0) {
      break;
    }
    D_80149338 = D_80149338 + 0x800;
    func_8015B580(D_8014686C, 7);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 5:
    func_8015C058();
    func_801BE1B0(7);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 10:
    func_801661C4(0xB);
    func_801C601C(2);
    D_80146875 = 0xB;
    break;
  case 11:
    if (g_ScenarioProgress != 0x12) {
      break;
    }
    D_8014832E = 0;
    func_80150224(0x18);
    D_80143BB0 = 2;
    D_80146875 = 0xC;
    break;
  case 12: {
    u8 *mode = &D_80143BB0;
    u8 slot;
    s32 angle;

    if (*mode == 2) {
      break;
    }
    {
      u32 *context_x = &D_80149308;
      *context_x = 0xA0000;
    }
    {
      u32 *context_y = &D_8014930C;
      *context_y = 0x620000;
    }
    D_80143F7C = 0xA0000;
    D_80143F80 = 0x620000;
    D_80143F86 = func_8015477C(D_80149308, D_8014930C);
    func_80153DE8();
    func_80150224(0x19);
    *mode = 2;
    func_8014ECAC(1);
    D_8014832E = 0x1F;
    D_801492D8 = -0x344;
    D_801492DC = 0x3A2;
    D_801FBFEC = func_8019601C();
    if (D_801FBFEC != 0xFF) {
      D_80143FC8[D_801FBFEC * 116] = 1;
      D_80143FCD[D_801FBFEC * 116] = 0x13;
      slot = D_801FBFEC;
      *(s32 *)(D_8014402C + slot * 116) = -0x2AA;
      angle = D_801492DA;
      *(s32 *)(D_80144034 + slot * 116) = 0x200;
      D_80143FD1[slot * 116] = 0x64;
      *(s32 *)(D_80144030 + slot * 116) = angle;
      D_80146875 = 0xD;
    }
    break;
  }
  case 13:
    if ((D_80143FC8[D_801FBFEC * 116] & 1) != 0) {
      break;
    }
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015B580(D_8014686C, 9);
    func_8019FA28(0x91, 0x3F8000, 0x5E0000, 0x88);
    D_80145029 = 0x5C;
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 20:
    if (g_ScenarioProgress != 1) {
      break;
    }
    func_8015C088();
    func_8019FA28(0x58, 0x130000, 0x250000, 0x80);
    D_80143F1D = 1;
    D_80146875 = 0x15;
    break;
  case 21:
    if (D_80143C40 != 0) {
      break;
    }
    D_80146876 = 0x3C;
    D_80146875 = 0x16;
    break;
  case 22: {
    u16 *countdown = &D_80146876;
    u8 slot;

    if (*countdown != 0) {
      *countdown = *countdown - 1;
      break;
    }
    slot = func_8019601C();
    if (slot != 0xFF) {
      D_80143FC8[slot * 116] = 1;
      D_80143FCD[slot * 116] = 0x80;
    }
    *countdown = 0x46;
    D_80146875 = 0x17;
    break;
  }
  case 23: {
    u16 *countdown = &D_80146876;

    if (*countdown != 0) {
      *countdown = *countdown - 1;
      break;
    }
    func_8019FA28(0x70, 0xC8000, 0x580000, 0x81);
    D_80143F1E = 0;
    D_80146875 = 0x18;
    break;
  }
  case 24:
    if (D_80143C40 != 0) {
      break;
    }
    func_8015C058();
    func_8015B580(D_8014686C, 0xB);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  }
}
