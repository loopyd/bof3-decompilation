#include "bof3/bof3.h"

extern u8 D_80146867;
extern s8 D_80146874;
extern u8 D_80146875;
extern u32 D_8014686C;
extern u16 D_80146258;
extern u8 D_80143BB0;
extern u8 g_ScenarioProgress;
extern u8 D_80146888[];

void func_80150224(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8016666C(s32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);

/* @source 0x801FB8A0
 * @behavior Entry 4 of the overlay progress-handler table at D_801FE970 that
 * dispatchProgressHandlerScenarioScena0800_801FAB5C indexes with the signed
 * byte D_80146874: it drives the shared sub-state byte D_80146875 through six
 * steps. Sub-state 0 raises the shared progress byte g_ScenarioProgress to
 * 0xA, runs func_801BE1B0(1) and advances to 1. Sub-state 1 waits for that
 * progress byte to read 0, then releases the shared selector through
 * func_8015C058, sets scenario flag bit 5 through func_8015B580(D_8014686C, 5)
 * and advances to 2. Sub-state 2 indexes the 0x98-byte work records at
 * D_80146888 with the shared counter byte D_80146867 and, when that record's
 * flag byte holds bit 7, runs func_801BE1B0(2) and returns the overlay to
 * handler 0 by clearing D_80146874 and the sub-state byte. Sub-state 5 queues
 * front-end mode 0x24 through func_80150224, arms the shared primary mode byte
 * D_80143BB0 with 2 and advances to 6. Sub-state 6, unless that mode byte reads
 * 2, releases the shared selector through func_8015C058 and returns the
 * overlay to handler 0 the same way. Sub-state 0xA, at progress byte 0xA,
 * clears the progress byte, sets scenario flag bit 6 through
 * func_8015B580(D_8014686C, 6), runs func_8016666C(0xBB8, 0), returns the
 * overlay to handler 0, releases the shared selector through func_8015C058,
 * seeds the front selector context through func_8019FA28(1, 0x2C0000, 0xE8000,
 * 3) and clears flag bit 3 of the shared mode halfword D_80146258. Any other
 * sub-state value does nothing. It takes no arguments, returns nothing, and its
 * 0x18-byte frame only keeps $ra across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitForProgressOrRecordFlagThenSetFlagBit5Or6QueueMode24AndSeedFrontSelectorContextScenarioScena0800_801FB8A0(void) {
  u32 offset;

  switch (D_80146875) {
  case 0:
    g_ScenarioProgress = 0xa;
    func_801BE1B0(1);
    D_80146875 = 1;
    break;

  case 1:
    if (g_ScenarioProgress != 0) {
      break;
    }
    func_8015C058();
    func_8015B580(D_8014686C, 5);
    D_80146875 = 2;
    break;

  case 2:
    offset = (u32)D_80146867 * 0x98;
    if ((D_80146888[offset] & 0x80) == 0) {
      break;
    }
    func_801BE1B0(2);
    D_80146874 = 0;
    D_80146875 = 0;
    break;

  case 5:
    func_80150224(0x24);
    D_80143BB0 = 2;
    D_80146875 = 6;
    break;

  case 6:
    if (D_80143BB0 == 2) {
      break;
    }
    func_8015C058();
    D_80146874 = 0;
    D_80146875 = 0;
    break;

  case 0xa: {
    u8* progress = &g_ScenarioProgress;

    if (*progress != 0xa) {
      break;
    }
    *progress = 0;
    func_8015B580(D_8014686C, 6);
    func_8016666C(0xbb8, 0);
    D_80146874 = 0;
    D_80146875 = 0;
    func_8015C058();
    func_8019FA28(1, 0x2c0000, 0xe8000, 3);
    D_80146258 = (u16)(D_80146258 & 0xfff7);
    break;
  }
  }
}
