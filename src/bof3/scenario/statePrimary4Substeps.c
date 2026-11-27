#include "bof3/bof3.h"

/* Shared front-end mode byte of the shared EXE image: the arms that arm a
 * front-end sequence leave 2 behind, the arm that only waits for its release
 * takes 2 as the already-released value, and the arms that read it as a gate
 * refuse to act while it is still nonzero. */
extern u8 D_80143BB0;

/* Shared front-end phase bytes the late arms seed: the cue byte D_80143F1F and
 * the two sub-state gate bytes D_80143F1D and D_80143F1E the front selector
 * seeds. */
extern u8 D_80143F1D;
extern u8 D_80143F1E;
extern u8 D_80143F1F;

/* Front-end record table of the shared EXE image, declared as the address of
 * the byte the body reads: the flag byte D_80143FC8 carries bit 0 once the
 * overlay has filled the record it names. The record stride the index scales by
 * is 0x74. */
extern u8 D_80143FC8[];

/* Shared gate halfword every waiting arm tests against zero. */
extern u16 D_80143C40;

/* Shared flag halfword the last arm clears bit 0x8 of. */
extern u16 D_80146258;

/* Shared flag word and state bytes: the flag word the bit-set helper receives,
 * the primary state byte every clearing arm resets, the sub-state byte this
 * switch dispatches on. */
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

/* Shared effect-bank byte the later arms clear and arm with 0x1F. */
extern u8 D_8014832E;

/* Shared halfword one arm strides by -0x400 before storing it back. */
extern u16 D_80149338;

/* Shared front-end selection source bytes the cue arm reads: the selected
 * record byte D_80145F09 indexes the byte table D_80181B28, and the three
 * consecutive words D_80145EC4, D_80145EC8 and D_80145ECC are the context the
 * same call forwards. */
extern u8 D_80145F09;
extern u32 D_80145EC4;
extern u32 D_80145EC8;
extern u32 D_80145ECC;
extern u8 D_80181B28[];

/* Overlay-local front-end record selector: the id the cue arm stores and the
 * waiting arm scales into the record table. */
extern u8 D_801FBFEC;

/* Shared scenario progress byte the arms gate on. */
extern u8 g_ScenarioProgress;

/* Shared front-end action queue helper at 0x80150224. */
void func_80150224(u32 action);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 flags, s32 bit);
/* Shared front-selector context seeder at 0x8019FA28. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
/* Shared front-end cue helper at 0x8015CC00; it answers the record selector it
 * claimed as one byte. */
u8 func_8015CC00(u32 arg0, s32 arg1, u8 arg2, u32 arg3, u32 arg4, u32 arg5);
/* Shared front-end cue helper at 0x8015507C. */
void func_8015507C(s32 arg0, s32 arg1);
/* Shared front-end record release helper at 0x8014ECAC. */
void func_8014ECAC(u16 arg0);
/* Shared front-end cue helper at 0x8015D4F8. */
void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
/* Shared front-end record release helper at 0x8015D404. */
void func_8015D404(u32 arg0, s32 arg1);
/* Shared front-end helper at 0x8014B87C. */
void yieldCallbackSlotScheduler(u16 arg0);

/* @source 0x801F9570
 * @behavior Entry 4 of the nine-entry primary-state handler table at
 * 0x801FBF54 that the exact dispatcher
 * callPrimaryStateHandler3 (0x801F8474) selects
 * with the shared primary state byte D_80146874 and calls with no arguments, so
 * no in-image jal targets 0x801F9570 and the word at 0x801FBF64 (payload offset
 * 0x5364) is its only reference: the sub-state switch of the overlay's primary
 * state 4 handler. It reads the shared sub-state byte D_80146875 with lbu,
 * bounds it against 0x20 with sltiu and dispatches through the thirty-two-word
 * absolute table at 0x801F6CB4 in the overlay's leading table data, whose
 * entries for sub-states 2 to 9 and 0x0C to 0x13 point at the body exit, so
 * those sixteen values are the default and the body has cases 0, 1, 0x0A,
 * 0x0B and 0x14 to 0x1F.
 *
 * Sub-state 0 queues action 0x12, stores 2 in the front-end mode byte
 * D_80143BB0 and advances to 1. Sub-state 1 waits for the mode byte to leave 2,
 * then releases the overlay through func_8015C058 and clears both state bytes.
 * Sub-state 0x0A, once the progress byte reads 0x10, calls
 * func_8015CC00(4, 5, D_80181B28[D_80145F09], D_80145EC4, D_80145EC8,
 * D_80145ECC), publishes the answered selector in D_801FBFEC and advances to
 * 0x0B. Sub-state 0x0B waits while the flag byte D_80143FC8 of the record that
 * selector scales by 0x74 still carries bit 0, then releases the overlay, sets
 * bit 0x0D of D_8014686C and clears both state bytes.
 *
 * Sub-state 0x14 clears the progress byte and seeds the front selector context
 * func_8019FA28(0x56, 0xC0000, 0x98000, 3), stores 0xFF in D_80143F1F and
 * advances to 0x15. Sub-state 0x15, while the gate halfword D_80143C40 reads
 * zero, requests func_8015507C(0xC, 9), queues action 2, stores 2 in the mode
 * byte and advances to 0x16. Sub-state 0x16, once the progress byte reads 7,
 * clears bit 0 and 0x400 of the shared halfword D_80149338 and advances to
 * 0x17. Sub-state 0x17, once the progress byte reads 8, sets bit 0x0E of
 * D_8014686C, seeds func_8019FA28(0x58, 0x240000, 0x200000, 0x81), stores 0xFF
 * in D_80143F1F, 1 in D_80143F1D and advances to 0x18. Sub-state 0x18, while
 * D_80143C40 reads zero, queues action 0x0A, stores 2 in the mode byte and
 * advances to 0x19. Sub-state 0x19, once the progress byte reads 1, clears that
 * byte, seeds func_8019FA28(0x56, 0x100000, 0x98000, 0x81), stores 0x5C in
 * D_80143F1F, 0 in D_80143F1E and advances to 0x1A.
 *
 * Sub-state 0x1A, once the progress byte reads 0x18, releases a front-end
 * record with func_8014ECAC(0), cues func_8015D4F8(0, 1, 0x64, 0x20) and
 * advances to 0x1B. Sub-state 0x1B, while D_80143C40 reads zero, clears the
 * effect bank D_8014832E, queues action 0x14, stores 2 in the mode byte,
 * releases through func_8015D404(0, 1) and advances to 0x1C. Sub-state 0x1C
 * waits for the mode byte to leave 2, then arms it with 6 through
 * yieldCallbackSlotScheduler(1) and advances to 0x1D. Sub-state 0x1D, while the mode byte is
 * still nonzero, clears the progress byte, sets bit 0x0F of D_8014686C, seeds
 * func_8019FA28(0x56, 0x100000, 0x98000, 3), stores 0xFF in both D_80143F1D and
 * D_80143F1E and advances to 0x1E. Sub-state 0x1E, while the mode byte is still
 * nonzero, arms the effect bank with 0x1F, releases through func_8014ECAC(1)
 * and advances to 0x1F. Sub-state 0x1F, while D_80143C40 reads zero, releases
 * the overlay through func_8015C058, clears both state bytes and clears bit
 * 0x8 of the shared halfword D_80146258.
 *
 * The body takes no arguments and returns nothing; its 0x20-byte frame keeps
 * $ra at 0x1C and $s0 at 0x18, and the sub-state read is hoisted above that
 * frame allocation. The arms that must hold a byte address across a call take
 * it into a pointer, so their later store reaches the same cell through the
 * held register instead of re-forming the address, and the arm that stores the
 * same constant before and after a call keeps it in the saved register.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePrimary4Substeps(void) {
  switch (D_80146875) {
  case 0:
    func_80150224(0x12);
    D_80143BB0 = 2;
    D_80146875 = 1;
    break;
  case 1:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 10:
    if (g_ScenarioProgress != 0x10) {
      break;
    }
    D_801FBFEC = func_8015CC00(4, 5, D_80181B28[D_80145F09], D_80145EC4,
                               D_80145EC8, D_80145ECC);
    D_80146875 = 0xB;
    break;
  case 11:
    if ((D_80143FC8[D_801FBFEC * 116] & 1) != 0) {
      break;
    }
    func_8015C058();
    func_8015B580(D_8014686C, 0xD);
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 20:
    g_ScenarioProgress = 0;
    func_8019FA28(0x56, 0xC0000, 0x98000, 3);
    D_80143F1F = 0xFF;
    D_80146875 = 0x15;
    break;
  case 21:
    if (D_80143C40 != 0) {
      break;
    }
    func_8015507C(0xC, 9);
    func_80150224(2);
    D_80143BB0 = 2;
    D_80146875 = 0x16;
    break;
  case 22: {
    u16 delta;

    if (g_ScenarioProgress != 7) {
      break;
    }
    delta = D_80149338;
    D_80146875 = 0x17;
    D_80149338 = delta - 0x400;
    break;
  }
  case 23:
    if (g_ScenarioProgress != 8) {
      break;
    }
    func_8015B580(D_8014686C, 0xE);
    func_8019FA28(0x58, 0x240000, 0x200000, 0x81);
    D_80143F1F = 0xFF;
    D_80143F1D = 1;
    D_80146875 = 0x18;
    break;
  case 24:
    if (D_80143C40 != 0) {
      break;
    }
    func_80150224(0xA);
    D_80143BB0 = 2;
    D_80146875 = 0x19;
    break;
  case 25: {
    u8 *progress = &g_ScenarioProgress;

    if (*progress != 1) {
      break;
    }
    *progress = 0;
    func_8019FA28(0x56, 0x100000, 0x98000, 0x81);
    D_80143F1F = 0x5C;
    D_80143F1E = 0;
    D_80146875 = 0x1A;
    break;
  }
  case 26:
    if (g_ScenarioProgress != 0x18) {
      break;
    }
    func_8014ECAC(0);
    func_8015D4F8(0, 1, 0x64, 0x20);
    D_80146875 = 0x1B;
    break;
  case 27:
    if (D_80143C40 != 0) {
      break;
    }
    D_8014832E = 0;
    func_80150224(0x14);
    D_80143BB0 = 2;
    func_8015D404(0, 1);
    D_80146875 = 0x1C;
    break;
  case 28: {
    u8 *mode = &D_80143BB0;

    if (*mode == 2) {
      break;
    }
    yieldCallbackSlotScheduler(1);
    *mode = 6;
    D_80146875 = 0x1D;
    break;
  }
  case 29:
    if (D_80143BB0 != 0) {
      break;
    }
    g_ScenarioProgress = 0;
    func_8015B580(D_8014686C, 0xF);
    func_8019FA28(0x56, 0x100000, 0x98000, 3);
    D_80143F1D = 0xFF;
    D_80143F1E = 0xFF;
    D_80146875 = 0x1E;
    break;
  case 30: {
    u8 next = 0x1F;

    if (D_80143BB0 != 0) {
      break;
    }
    D_8014832E = next;
    func_8014ECAC(1);
    D_80146875 = next;
    break;
  }
  case 31:
    if (D_80143C40 != 0) {
      break;
    }
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    D_80146258 &= 0xFFF7;
    break;
  }
}
