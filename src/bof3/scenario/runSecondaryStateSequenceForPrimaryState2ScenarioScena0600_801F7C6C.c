#include "bof3/bof3.h"

/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80145E98 @kind unknown */
extern u8 D_80145E98;
/* @source 0x1F800000 @kind unknown */
extern u8 D_1F800000;
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
/* @source 0x801492D8 @kind unknown */
extern s16 D_801492D8;
/* @source 0x801492DA @kind unknown */
extern s16 D_801492DA;

void func_801BE1B0(u32 arg0);
void func_80150224(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
s32 func_8019601C(void);

/* @source 0x801F7C6C
 * @behavior Primary state 2 of this overlay, reached as word 2 of the
 *           eighteen-word overlay handler table at 0x801FE3E4 that
 *           dispatchPrimaryStateHandlerScenarioScena0600_801F7A00 indexes with
 *           the shared primary scene state byte D_80146874. It steps the shared
 *           secondary scene state byte D_80146875 through eleven states, each
 *           gated on the shared front-end progress byte g_ScenarioProgress, and
 *           every other value of that byte falls to the epilogue.
 *           State 0, unless the shared front-end phase byte D_80143BB0 reads 2,
 *           raises the progress byte to 1, runs the GAME.EMI helper
 *           func_801BE1B0 with mode 3 and arms secondary state 1; state 1, at
 *           progress 4, hands the scenario flag word D_8014686C and mode 4 to
 *           func_8015B580, runs the shared release helper func_8015C058 and
 *           clears the progress byte, the secondary state byte and the primary
 *           state byte; state 5 opens the shared front-end transition for
 *           selector 0x42 through func_80150224, writes the phase byte
 *           D_80143BB0 as 2 and advances to state 6; state 6, unless that phase
 *           byte already reads 2, runs the release helper and clears the
 *           progress byte and both state bytes; state 10, unless the phase byte
 *           reads 2, raises the progress byte to 0x0F and lets the shared
 *           game-mode byte D_80145E98 select the mode index: game modes 2, 3 and
 *           4 run func_801BE1B0 with mode 4, 5 and 6, any other game mode leaves
 *           before advancing, and the taken modes advance to state 0x0B;
 *           state 11, at progress 0x14, hands mode 5 to func_8015B580 and runs
 *           the same release and clear path; state 15, unless the phase byte
 *           reads 2, raises the progress byte to 0x19, runs func_801BE1B0 with
 *           mode 8 and advances to state 0x10; state 0x10, at progress 0x1A,
 *           stores the object byte func_8019601C in scratchpad 0x1F800000 and,
 *           unless that byte is 0xFF, seeds the selected 0x74-byte front-end
 *           record at 0x80143FC8 (+0 = 1, +5 = 0x13, +9 = 0x3C, +0x64 = the
 *           signed halfword D_801492D8, +0x68 = the signed halfword D_801492DA
 *           and +0x6C = 0x3C), carries that scratch byte into D_80146865 and
 *           advances to state 0x11; state 0x11, once the +0 byte of the record
 *           selected by D_80146865 is clear, raises the progress byte to 0x1B,
 *           clears D_80146865 and advances to state 0x12; state 0x12, at
 *           progress 0x1C, seeds the same record with +9 = 0x3C and +0x6C =
 *           0x200 and advances to state 0x13; state 0x13, at progress 0x1E,
 *           hands mode 7 to func_8015B580 and runs the release and clear path
 *           shared with states 1 and 11. The 0x18-byte frame keeps $ra and $s0
 *           across the calls, the state-byte load and its bound test are hoisted
 *           above the frame setup, and the twenty-entry case table sits in the
 *           overlay rodata slot at 0x801F6C6C.
 *           Representation note: states 1, 11 and 0x13 reach the shared release
 *           block with the address of the progress byte still live across the
 *           two calls, and state 0x11 reads and clears the same byte of
 *           D_80146865 around one record lookup, so those four states take a
 *           local pointer to that byte; the plain global spelling of the same
 *           bodies measured 278 of 327 instructions at 1284 bytes, while this
 *           pointer spelling measured 327 of 327 at 1308 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState2ScenarioScena0600_801F7C6C(
    void) {
  switch (D_80146875) {
    case 0:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 1u;
      func_801BE1B0(3u);
      D_80146875 = 1u;
      break;

    case 1: {
      u8* progress = &g_ScenarioProgress;

      if (*progress == 4u) {
        func_8015B580(D_8014686C, 4);
        func_8015C058();
        *progress = 0u;
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;
    }

    case 5:
      func_80150224(0x42u);
      D_80143BB0 = 2u;
      D_80146875 = 6u;
      break;

    case 6:
      if (D_80143BB0 == 2u) {
        break;
      }

      func_8015C058();
      g_ScenarioProgress = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 10:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 0xFu;
      switch (D_80145E98) {
        case 2:
          func_801BE1B0(4u);
          break;

        case 3:
          func_801BE1B0(5u);
          break;

        case 4:
          func_801BE1B0(6u);
          break;

        default:
          return;
      }
      D_80146875 = 0xBu;
      break;

    case 11: {
      u8* progress = &g_ScenarioProgress;

      if (*progress == 0x14u) {
        func_8015B580(D_8014686C, 5);
        func_8015C058();
        *progress = 0u;
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;
    }

    case 15:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 0x19u;
      func_801BE1B0(8u);
      D_80146875 = 0x10u;
      break;

    case 16:
      if (g_ScenarioProgress == 0x1Au) {
        u8* scratch;
        u8 object_index;

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xFFu) {
          D_80143FC8[(u32)object_index * 0x74u] = 1u;
          D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
          *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = (s32)(s16)D_801492D8;
          *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
          *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x3C;
          D_80143FD1[(u32)*scratch * 0x74u] = 0x3C;
          D_80146875 = 0x11u;
          D_80146865 = *scratch;
        }
      }
      break;

    case 17: {
      u8* selected = &D_80146865;

      if (D_80143FC8[(u32)*selected * 0x74u] == 0u) {
        g_ScenarioProgress = 0x1Bu;
        *selected = 0u;
        D_80146875 = 0x12u;
      }
      break;
    }

    case 18:
      if (g_ScenarioProgress == 0x1Cu) {
        u8* scratch;
        u8 object_index;

        object_index = func_8019601C();
        scratch = &D_1F800000;
        *scratch = object_index;

        if (object_index != 0xFFu) {
          D_80143FC8[(u32)object_index * 0x74u] = 1u;
          D_80143FCD[(u32)*scratch * 0x74u] = 0x13u;
          *(s32*)(D_8014402C + (u32)*scratch * 0x74u) = (s32)(s16)D_801492D8;
          *(s32*)(D_80144030 + (u32)*scratch * 0x74u) = (s32)(s16)D_801492DA;
          *(s32*)(D_80144034 + (u32)*scratch * 0x74u) = 0x200;
          D_80143FD1[(u32)*scratch * 0x74u] = 0x3C;
          D_80146875 = 0x13u;
        }
      }
      break;

    case 19: {
      u8* progress = &g_ScenarioProgress;

      if (*progress == 0x1Eu) {
        func_8015B580(D_8014686C, 7);
        func_8015C058();
        *progress = 0u;
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;
    }
  }
}
