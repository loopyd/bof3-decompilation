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

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_80150224(s32 action_id);
void func_8015C058(void);
void func_801BE1B0(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FA8EC
 * @behavior Word 11 (0-based) of the 18-word primary state table at
 *           0x801FE3E4, which dispatchPrimaryStateHandlerScenarioScena0600_801F7A00
 *           indexes with the shared primary scene state byte D_80146874. It
 *           steps the shared secondary scene state byte D_80146875 through a
 *           ten-entry case table, and every value of that byte above 9 falls to
 *           the epilogue, as do the unclaimed states 2, 3 and 4: state 0 arms
 *           secondary state 1 and then hands the shared scenario flag word
 *           D_8014686C to the flag tester func_8015B5D4 for flag 0x27, queues
 *           front-end action 0x0A through func_80150224 when that flag is set
 *           and action 8 when it is clear, and latches the shared front-end
 *           phase byte D_80143BB0 to 2; state 1, unless that phase byte already
 *           reads 2, runs the shared overlay release helper func_8015C058,
 *           clears the progress byte g_ScenarioProgress and the bytes
 *           0x80146865..0x80146867 and then the secondary and primary state
 *           bytes; state 5 tests flag 0x27 and then flag 0x25 the same way and,
 *           when either is set, queues action 0x0A or 8, latches the phase byte
 *           to 2 and arms secondary state 1, while with both flags clear it
 *           queues action 9, latches the phase byte to 2 and advances to
 *           secondary state 6; state 6, unless the phase byte reads 2, moves to
 *           secondary state 7 at progress 1 and otherwise releases the overlay,
 *           clears the progress byte and the shared bytes; state 7 runs the
 *           shared GAME.EMI helper func_801BE1B0 with mode 0 and advances to
 *           secondary state 8; state 8 advances to secondary state 9 at progress
 *           2; state 9, at progress 3, sets flag bit 0x25 of D_8014686C through
 *           func_8015B580, releases the overlay and clears the shared bytes. The
 *           0x18-byte frame keeps $ra and $s0 across the calls, the state-byte
 *           load and its bound test are hoisted above the frame setup, and the
 *           ten-word case table sits at 0x801F7044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondaryStateSequenceForPrimaryState11ScenarioScena0600_801FA8EC(
    void) {
  s32 mode;

  switch (D_80146875) {
    case 0:
      D_80146875 = 1u;
      if (func_8015B5D4(D_8014686C, 0x27) == 0) {
        mode = 8;
      } else {
        mode = 0xA;
      }
      func_80150224(mode);
      D_80143BB0 = 2u;
      break;

    case 1:
      if (D_80143BB0 == 2u) {
        break;
      }

      func_8015C058();
      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      D_80146866 = 0u;
      D_80146867 = 0u;
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;

    case 5:
      if (func_8015B5D4(D_8014686C, 0x27) != 0) {
        mode = 0xA;
      } else if (func_8015B5D4(D_8014686C, 0x25) != 0) {
        mode = 8;
      } else {
        func_80150224(9);
        D_80143BB0 = 2u;
        D_80146875 = 6u;
        break;
      }

      func_80150224(mode);
      D_80143BB0 = 2u;
      D_80146875 = 1u;
      break;

    case 6:
      if (D_80143BB0 != 2u) {
        u8 *progress = &g_ScenarioProgress;

        if (*progress == 1u) {
          D_80146875 = 7u;
          break;
        }

        func_8015C058();
        *progress = 0u;
        D_80146865 = 0u;
        D_80146866 = 0u;
        D_80146867 = 0u;
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;

    case 7:
      func_801BE1B0(0u);
      D_80146875 = 8u;
      break;

    case 8:
      if (g_ScenarioProgress == 2u) {
        D_80146875 = 9u;
      }
      break;

    case 9:
      if (g_ScenarioProgress == 3u) {
        func_8015B580(D_8014686C, 0x25);
        func_8015C058();
        D_80146865 = 0u;
        D_80146866 = 0u;
        D_80146867 = 0u;
        D_80146875 = 0u;
        D_80146874 = 0u;
      }
      break;
  }
}
