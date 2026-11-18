#include "bof3/bof3.h"

/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80143F1F @kind unknown */
extern u8 D_80143F1F;
/* @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* @source 0x8014932C @kind unknown */
extern u16 D_8014932C;
/* @source 0x8014932E @kind unknown */
extern u16 D_8014932E;
/* @source 0x80149332 @kind unknown */
extern u8 D_80149332;
/* @source 0x80149333 @kind unknown */
extern u8 D_80149333;
/* @source 0x80149308 @kind unknown */
extern u32 D_80149308;
/* @source 0x8014930C @kind unknown */
extern u32 D_8014930C;
/* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern u32 D_80145EC8;
/* @source 0x801492DA @kind unknown */
extern s16 D_801492DA;
/* @source 0x1F800000 @kind unknown */
extern u8 D_1F800000;
/* The front-end record table of the main image holds 0x74 bytes per record.
 * Each view below starts one cell of that record, so every index that walks
 * it carries the 0x74 stride. */
/* @source 0x80143FC8 @kind unknown */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FD1 @kind unknown */
extern u8 D_80143FD1[];
/* @source 0x8014402C @kind unknown */
extern u8 D_8014402C[];
/* @source 0x80144030 @kind unknown */
extern u8 D_80144030[];
/* @source 0x80144034 @kind unknown */
extern u8 D_80144034[];

void func_8014ECAC(u16 arg0);
void func_80150224(s32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8016666C(s32 arg0, s32 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u32 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C601C(u32 arg0);
u8 func_8019601C(void);

/* @source 0x801F90C0
 * @behavior Primary state 6 of this overlay: word 6 (0-based) of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 indexes with
 *           the shared primary scene state byte D_80146874. It steps the shared
 *           secondary scene state byte D_80146875 through a thirty-three-entry
 *           case table, so every value of that byte at or above 0x21 falls to
 *           the epilogue, as do the unclaimed states 11 to 19 and 25 to 29.
 *           State 0 runs func_801BE1B0 with mode 3 and advances to state 1.
 *           State 1, at progress 9, arms secondary state 2, raises D_80146866
 *           to 2, hands the flag word D_8014686C and mode 0x12 to
 *           func_8015B580 and runs func_8019FA28 with 0x5E, 0x2E0000, 0x2E0000
 *           and 0x84. State 2 queues front-end action 0x38 through
 *           func_80150224, latches D_80143BB0 to 2 and advances to state 3.
 *           State 3, unless D_80143BB0 already reads 2, runs func_8014ECAC with
 *           mode 5, stores 0x1F in D_8014832E and advances to state 4. State 4,
 *           while the gate halfword D_80143C40 reads zero, raises progress to 1
 *           and advances to state 5. State 5, at progress 2, claims a front-end
 *           record through func_8019601C, publishes the claimed index in the
 *           scratchpad byte 0x1F800000 and, unless that index is 0xFF, seeds the
 *           0x74-byte record at D_80143FC8 with flag 1, byte 0x13, word -0x210,
 *           the D_801492DA halfword, word 0x2F2 and byte 0x14, then raises
 *           progress to 3 and advances to state 6. State 6 reseeds D_80149333
 *           with 2, decrements the halfword D_8014932C by 0x40 and counts
 *           D_80146867 up; once that count reaches 0x12 it clears D_8014932C and
 *           the count, reseeds D_80149333 with 2 and advances to state 7. State
 *           7, at progress 0x13, runs func_801C601C with mode 0, claims a
 *           front-end record through func_8019601C, publishes it in the
 *           scratchpad byte and, unless it is 0xFF, seeds that record with flag
 *           1, byte 0x13, word -0x2AA, the D_801492DA halfword, word 0x200 and
 *           byte 0x1E, then advances to state 8 and carries the claimed index
 *           into D_80146867. State 8, once the flag byte of the record selected
 *           by D_80146867 reads zero, raises progress to 0x14 and advances to
 *           state 9. State 9, at progress 0x16, arms secondary state 0x0A, hands
 *           the flag word and mode 0x13 to func_8015B580, runs func_8019FA28
 *           with 0x5E, 0x2C0000, 0x290000 and 0x85 and stores 0x42 in
 *           D_80143F1F. State 0x0A, at progress 0x22, hands the flag word and
 *           mode 0x10 to func_8015B580, runs the shared overlay release helper
 *           func_8015C058, clears D_80146865 to D_80146867 and enters the
 *           clearing tail. State 0x14 runs func_801BE1B0 with mode 6,
 *           func_801C601C with mode 1 and advances to state 0x15. State 0x15, at
 *           progress 4, stores 0x20 in D_8014932E, advances to state 0x16 and
 *           copies D_80145EC4 and D_80145EC8 into D_80149308 and D_8014930C.
 *           State 0x16, while D_80149332 reads zero, raises progress to 5 and
 *           advances to state 0x17. State 0x17, at progress 7, runs
 *           func_8016666C with (1000, 0), raises progress to 8 and advances to
 *           state 0x18. State 0x18, at
 *           progress 9, hands the flag word and mode 0x14 to func_8015B580,
 *           runs func_8015C058, clears D_80146865 to D_80146867 and enters the
 *           clearing tail. State 0x1E arms secondary state 0x1F and runs
 *           func_8019FA28 with 0x5E, 0x520000, 0x330000 and 0x8E. State 0x1F
 *           stores 0x1F in D_8014832E and arms secondary state 0x20. State 0x20,
 *           at progress 0x36, runs func_8015C058 and enters that same clearing
 *           tail, which stores zero in D_80146875 and D_80146874. Those three
 *           identical two-store tails share one clearing block. The 0x18-byte
 *           frame keeps $ra and $s0, which holds the progress byte address
 *           across the two record claims and the generic wait of state 0x17,
 *           and the thirty-three-word case table sits in the overlay rodata
 *           slot at 0x801F6D94.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState6ScenarioScena0600_801F90C0(void) {
  switch (D_80146875) {
    case 0:
      func_801BE1B0(3u);
      D_80146875 = 1u;
      return;

    case 1:
      if (g_ScenarioProgress != 9u) {
        return;
      }

      D_80146875 = 2u;
      D_80146866 = 2u;
      func_8015B580(D_8014686C, 0x12);
      func_8019FA28(0x5Eu, 0x2E0000u, 0x2E0000u, 0x84u);
      return;

    case 2:
      func_80150224(0x38);
      D_80143BB0 = 2u;
      D_80146875 = 3u;
      return;

    case 3:
      if (D_80143BB0 == 2u) {
        return;
      }

      func_8014ECAC(5u);
      D_8014832E = 0x1Fu;
      D_80146875 = 4u;
      return;

    case 4:
      if (D_80143C40 != 0u) {
        return;
      }

      g_ScenarioProgress = 1u;
      D_80146875 = 5u;
      return;

    case 5: {
      u8 *progress = &g_ScenarioProgress;
      u8 *scratch;
      u8 index;

      if (*progress != 2u) {
        return;
      }

      index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = index;

      if (index != 0xffu) {
        D_80143FC8[(u32)index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) = -0x210;
        *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = 0x2f2;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x14u;
        *progress = 3u;
        D_80146875 = 6u;
      }
      return;
    }

    case 6: {
      u8 *count = &D_80146867;

      D_80149333 = 2u;
      D_8014932C -= 0x40u;
      if (++*count >= 0x12u) {
        D_8014932C = 0u;
        D_80149333 = 2u;
        *count = 0u;
        D_80146875 = 7u;
      }
      return;
    }

    case 7: {
      u8 *scratch;
      u8 index;

      if (g_ScenarioProgress != 0x13u) {
        return;
      }

      func_801C601C(0u);
      index = func_8019601C();
      scratch = &D_1F800000;
      *scratch = index;

      if (index != 0xffu) {
        D_80143FC8[(u32)index * 0x74u] = 1u;
        D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
        *(s32 *)(D_8014402C + (u32)*scratch * 0x74u) = -0x2aa;
        *(s32 *)(D_80144030 + (u32)*scratch * 0x74u) = (s32)D_801492DA;
        *(s32 *)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
        D_80143FD1[(u32)*scratch * 0x74u] = 0x1eu;
        D_80146875 = 8u;
        D_80146867 = *scratch;
      }
      return;
    }

    case 8:
      if (D_80143FC8[(u32)D_80146867 * 0x74u] != 0u) {
        return;
      }

      g_ScenarioProgress = 0x14u;
      D_80146875 = 9u;
      return;

    case 9:
      if (g_ScenarioProgress != 0x16u) {
        return;
      }

      D_80146875 = 0xAu;
      func_8015B580(D_8014686C, 0x13);
      func_8019FA28(0x5Eu, 0x2C0000u, 0x290000u, 0x85u);
      D_80143F1F = 0x42u;
      return;

    case 10:
      if (g_ScenarioProgress != 0x22u) {
        return;
      }

      func_8015B580(D_8014686C, 0x10);
      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 20:
      func_801BE1B0(6u);
      func_801C601C(1u);
      D_80146875 = 0x15u;
      return;

    case 21:
      if (g_ScenarioProgress != 4u) {
        return;
      }

      D_8014932E = 0x20u;
      D_80146875 = 0x16u;
      D_80149308 = D_80145EC4;
      D_8014930C = D_80145EC8;
      return;

    case 22:
      if (D_80149332 != 0u) {
        return;
      }

      g_ScenarioProgress = 5u;
      D_80146875 = 0x17u;
      return;

    case 23: {
      u8 *progress = &g_ScenarioProgress;

      if (*progress != 7u) {
        return;
      }

      func_8016666C(1000, 0);
      *progress = 8u;
      D_80146875 = 0x18u;
      return;
    }

    case 24:
      if (g_ScenarioProgress != 9u) {
        return;
      }

      func_8015B580(D_8014686C, 0x14);
      func_8015C058();
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    case 30:
      D_80146875 = 0x1Fu;
      func_8019FA28(0x5Eu, 0x520000u, 0x330000u, 0x8Eu);
      return;

    case 31:
      D_8014832E = 0x1Fu;
      D_80146875 = 0x20u;
      return;

    case 32:
      if (g_ScenarioProgress != 0x36u) {
        return;
      }

      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      return;

    default:
      return;
  }
}
