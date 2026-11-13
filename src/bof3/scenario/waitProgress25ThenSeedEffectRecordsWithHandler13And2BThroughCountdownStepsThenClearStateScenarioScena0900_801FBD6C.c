#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;
extern u16 D_80146876;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses every field through its own
 * absolute symbol, because the original re-materialises the row base and adds
 * the scaled row offset for each access, so each field base stays separate. */
extern u8 D_80143FC8[];
extern u8 D_80143FCD[];
extern u8 D_80143FD1[];
extern u8 D_80143FFC[];
extern u8 D_80144000[];
extern u8 D_80144004[];
extern u8 D_8014402C[];
extern u8 D_80144030[];
extern u8 D_80144034[];

/* Signed halfword position cells of the shared main executable that this body
 * copies into the seeded row. */
extern s16 D_801492D8;
extern s16 D_801492DA;
extern s16 D_801492DC;

/* Word cells of the shared main executable that this body copies into the
 * shared projection pair before it runs func_80153DE8. */
extern s32 D_80145EC4;
extern s32 D_80145EC8;
extern s32 D_80149308;
extern s32 D_8014930C;

/* Overlay-local byte holding the row index the shared front-end record
 * allocator answered last. */
extern u8 D_801FE554;

/* Front-end record allocator of the shared main-executable data segment: it
 * answers the index of the next free 0x74-byte row at 0x80143FC8, or 0xFF when
 * every row is occupied. */
u8 func_8019601C(void);
/* GAME.EMI entry 0 helper at 0x801BE1B0. */
void func_801BE1B0(u32 arg0);
/* Distance helper of the shared main executable: it takes a signed 16.16 word
 * pair and answers their distance as a halfword. */
s32 func_8015477C(s32 arg0, s32 arg1);
/* Shared scenario flag-word bit-set helper at 0x8015B580. */
void func_8015B580(u32 arg0, s32 arg1);
/* Shared overlay release helper at 0x8015C058. */
void func_8015C058(void);
/* Shared projection refresh helper at 0x80153DE8. */
void func_80153DE8(void);
/* Shared front-end teardown helper at 0x801A78F8. */
void func_801A78F8(void);

/* @source 0x801FBD6C
 * @behavior Word 11 (the word stored at 0x801FE418) of the seventeen-word
 * in-image primary state table at 0x801FE3EC that
 * dispatchPrimaryStateHandlerScenarioScena0900_801F83D8 (0x801F83D8) indexes
 * with the signed shared primary state byte D_80146874 and invokes with no
 * arguments, so no in-image jal targets this address and that table word is its
 * only image reference. It is the scripted state machine over the shared
 * sub-state byte D_80146875: it reads that byte once and dispatches through the
 * 36-entry in-image jump table at 0x801F72CC, whose entries 0 through 14, 30 and
 * 35 carry bodies while every other entry falls to the epilogue. Sub-state 0
 * runs the shared helper func_801BE1B0(2), sets the shared scenario progress
 * byte g_ScenarioProgress (0x80146864) to 0x20 and advances to 1. Sub-state 1,
 * once that progress byte reads 0x25, asks the shared front-end record allocator
 * func_8019601C for a row, stores the answer in the overlay-local byte
 * D_801FE554 and, unless that byte is 0xFF, seeds the 0x74-byte row at
 * 0x80143FC8: row byte 0 takes 1, row byte 5 takes the handler selector 0x13,
 * row word 0x64 takes -0x3C8, row word 0x6C takes 0xF8, row byte 9 takes 0x30
 * and row word 0x68 takes the signed position halfword D_801492DA, then it
 * advances to 2. Sub-state 2, once the progress byte reads 0x26, allocates and
 * seeds a row the same way with selector 0x2B, row word 0x34 = 0x238000, row
 * word 0x38 = 0x1F8000 and row word 0x3C = the answer of func_8015477C re-read
 * from row word 0x34 and 0x1F8000, shifted left 16, and then, whether or not a
 * row was free, arms the shared countdown halfword D_80146876 to 0x5F and
 * advances to 3. Sub-states 3, 9 and 11 decrement that countdown and, on it
 * reaching zero, advance to 4, 0x0A and 0x0C and increment the progress byte,
 * which the original emits as one shared tail. Sub-state 4, once the progress
 * byte reads 0x28, allocates and seeds a row with selector 0x13, row word 0x64 =
 * D_801492D8, row word 0x6C = -0x144, row byte 9 = 0x28 and row word 0x68 =
 * D_801492DA, then advances to 5; sub-state 5, once the byte reads 0x29, arms
 * the countdown to 0x78 and advances to 6; sub-state 6 decrements that countdown
 * and on zero allocates and seeds a row with selector 0x13, row word 0x64 =
 * D_801492D8, row word 0x6C = 0x200, row byte 9 = 0x20 and row word 0x68 =
 * D_801492DA, and only then, when the allocator answered a free row, advances to
 * 7, so a full table returns without touching the sub-state byte. Sub-state 7
 * proceeds only while the occupancy byte of the row the overlay-local byte
 * D_801FE554 names, read back from the table, has bit 0 clear: it allocates and
 * seeds a row with selector 0x13, row word 0x64 = -0x2AA, row word 0x68 =
 * D_801492DA, row byte 9 = 0x7C and row word 0x6C = D_801492DC, then advances to
 * 8 and increments the progress byte. Sub-state 8, once the byte reads 0x32,
 * arms the countdown to 0x5A and advances to 9; sub-state 10 does the same for
 * progress 0x35 with 0x1E and advances to 0x0B; sub-state 12 advances to 0x0D
 * once the byte reads 0x37; sub-state 13, once it reads 0x3B, copies the shared
 * word pair D_80145EC4 and D_80145EC8 into D_80149308 and D_8014930C, runs
 * func_80153DE8 and advances to 0x0E. Sub-state 0x0E, once the progress byte
 * clears to zero, runs the release helper func_8015C058 and sets scenario flag
 * bit 0x20 of the shared flag word D_8014686C through func_8015B580 before the
 * shared clearing tail; sub-state 30 runs func_8015C058, func_801BE1B0(3), sets
 * the progress byte to 0x20 and runs func_801A78F8 before that tail, and
 * sub-state 35 runs func_8015C058, func_801BE1B0(4) and the 0x20 progress store
 * before it. The clearing tail zeroes D_80146874 and D_80146875, which ends the
 * primary state. It takes no arguments, reads no state beyond the bytes named
 * here and writes none of the other fields, and its 0x18-byte frame only keeps
 * $ra: no saved register holds a base, so every record field base is
 * re-materialised with its own lui and the row offset is rebuilt by the
 * multiply-by-0x74 chain.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitProgress25ThenSeedEffectRecordsWithHandler13And2BThroughCountdownStepsThenClearStateScenarioScena0900_801FBD6C(
    void) {
  u8 index;

  switch (D_80146875) {
  case 0:
    func_801BE1B0(2u);
    g_ScenarioProgress = 0x20;
    D_80146875 = 1;
    break;
  case 1:
    if (g_ScenarioProgress == 0x25) {
      index = func_8019601C();
      D_801FE554 = index;
      if (index != 0xFFu) {
        u32 row;

        D_80143FC8[D_801FE554 * 0x74] = 1;
        D_80143FCD[D_801FE554 * 0x74] = 0x13;
        row = D_801FE554 * 0x74;
        *(s32*)(D_8014402C + row) = -0x3C8;
        *(s32*)(D_80144030 + row) = D_801492DA;
        *(s32*)(D_80144034 + row) = 0xF8;
        D_80143FD1[row] = 0x30;
        D_80146875 = 2;
      }
    }
    break;
  case 2:
    if (g_ScenarioProgress == 0x26) {
      index = func_8019601C();
      D_801FE554 = index;
      if (index != 0xFFu) {
        u32 row;

        D_80143FC8[D_801FE554 * 0x74] = 1;
        D_80143FCD[D_801FE554 * 0x74] = 0x2B;
        row = D_801FE554 * 0x74;
        *(s32*)(D_80143FFC + row) = 0x238000;
        *(s32*)(D_80144000 + row) = 0x1F8000;
        *(s32*)(D_80144004 + D_801FE554 * 0x74) =
            func_8015477C(*(s32*)(D_80143FFC + row), 0x1F8000) << 16;
      }
      D_80146876 = 0x5F;
      D_80146875 = 3;
    }
    break;
  case 3: {
    u16 *countdown = &D_80146876;

    if (--*countdown == 0) {
      u8 *progress = &g_ScenarioProgress;

      D_80146875 = 4;
      (*progress)++;
    }
    break;
  }
  case 4:
    if (g_ScenarioProgress == 0x28) {
      index = func_8019601C();
      D_801FE554 = index;
      if (index != 0xFFu) {
        u32 row;

        D_80143FC8[D_801FE554 * 0x74] = 1;
        D_80143FCD[D_801FE554 * 0x74] = 0x13;
        row = D_801FE554 * 0x74;
        *(s32*)(D_8014402C + row) = D_801492D8;
        *(s32*)(D_80144030 + row) = D_801492DA;
        *(s32*)(D_80144034 + row) = -0x144;
        D_80143FD1[row] = 0x28;
        D_80146875 = 5;
      }
    }
    break;
  case 5:
    if (g_ScenarioProgress == 0x29) {
      D_80146876 = 0x78;
      D_80146875 = 6;
    }
    break;
  case 6: {
    u16 *countdown = &D_80146876;

    if (--*countdown == 0) {
      index = func_8019601C();
      D_801FE554 = index;
      if (index != 0xFFu) {
        u32 row;

        D_80143FC8[D_801FE554 * 0x74] = 1;
        D_80143FCD[D_801FE554 * 0x74] = 0x13;
        row = D_801FE554 * 0x74;
        *(s32*)(D_8014402C + row) = D_801492D8;
        *(s32*)(D_80144030 + row) = D_801492DA;
        *(s32*)(D_80144034 + row) = 0x200;
        D_80143FD1[row] = 0x20;
        D_80146875 = 7;
      }
    }
    break;
  }
  case 7: {
    u32 row;

    row = D_801FE554 * 0x74;
    if ((D_80143FC8[row] & 1) == 0) {
      index = func_8019601C();
      D_801FE554 = index;
      if (index != 0xFFu) {
        row = D_801FE554 * 0x74;
        D_80143FC8[row] = 1;
        D_80143FCD[D_801FE554 * 0x74] = 0x13;
        row = D_801FE554 * 0x74;
        *(s32*)(D_8014402C + row) = -0x2AA;
        *(s32*)(D_80144030 + row) = D_801492DA;
        *(s32*)(D_80144034 + row) = D_801492DC;
        D_80143FD1[row] = 0x7C;
        D_80146875 = 8;
        {
          u8 *progress = &g_ScenarioProgress;

          *progress += 1;
        }
      }
    }
    break;
  }
  case 8:
    if (g_ScenarioProgress == 0x32) {
      D_80146876 = 0x5A;
      D_80146875 = 9;
    }
    break;
  case 9: {
    u16 *countdown = &D_80146876;

    if (--*countdown == 0) {
      u8 *progress = &g_ScenarioProgress;

      D_80146875 = 0x0A;
      (*progress)++;
    }
    break;
  }
  case 10:
    if (g_ScenarioProgress == 0x35) {
      D_80146876 = 0x1E;
      D_80146875 = 0x0B;
    }
    break;
  case 11: {
    u16 *countdown = &D_80146876;

    if (--*countdown == 0) {
      u8 *progress = &g_ScenarioProgress;

      D_80146875 = 0x0C;
      (*progress)++;
    }
    break;
  }
  case 12:
    if (g_ScenarioProgress == 0x37) {
      D_80146875 = 0x0D;
    }
    break;
  case 13:
    if (g_ScenarioProgress == 0x3B) {
      D_80149308 = D_80145EC4;
      D_8014930C = D_80145EC8;
      func_80153DE8();
      D_80146875 = 0x0E;
    }
    break;
  case 14:
    if (g_ScenarioProgress == 0) {
      func_8015C058();
      func_8015B580(D_8014686C, 0x20);
      D_80146874 = 0;
      D_80146875 = 0;
    }
    break;
  case 30:
    func_8015C058();
    func_801BE1B0(3u);
    g_ScenarioProgress = 0x20;
    func_801A78F8();
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  case 35:
    func_8015C058();
    func_801BE1B0(4u);
    g_ScenarioProgress = 0x20;
    D_80146874 = 0;
    D_80146875 = 0;
    break;
  }
}
