#include "bof3/bof3.h"

/* Shared secondary scene state byte: the sequence step this body advances.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;
/* Shared primary scene state byte: the table index that reaches this word.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;
/* Shared front-end progress byte gating every state below.
 * @source 0x80146864 @kind unknown
 */
extern u8 g_ScenarioProgress;
/* First of the shared bytes the reset tail clears.
 * @source 0x80146865 @kind unknown
 */
extern u8 D_80146865;
/* Shared scenario flag word the arms below set bits of.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;
/* Shared front-end phase byte: 2 means the front end already owns the frame.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;
/* Shared selection counter of the first name stream this body advances.
 * @source 0x80144DF2 @kind data
 */
extern u8 stateSelectionCounter;
/* Shared selection counter of the second name stream this body advances.
 * @source 0x80144DF5 @kind data
 */
extern u8 stateSelectionCounter2;
/* Twelve-byte name buffer filled from the first stream before its cue queues.
 * @source 0x801490D8 @kind data
 */
extern u8 D_801490D8[];
/* Terminator byte cleared after the first buffer above is filled.
 * @source 0x801490E4 @kind unknown
 */
extern u8 D_801490E4[];
/* Twelve-byte name buffer filled from the second stream before its cue queues.
 * @source 0x801490F8 @kind data
 */
extern u8 D_801490F8[];
/* Terminator byte cleared after the second buffer above is filled.
 * @source 0x80149104 @kind unknown
 */
extern u8 D_80149104[];

extern void strncpy(u8* dst, const u8* src, u32 count);
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3);
u8* func_80165D48(s32 arg0, u8 arg1);
void func_80150224(s32 action_id);
void func_80150284(u32 arg0);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C058(void);
void func_8015DF18(u32 cue_id);

/* @source 0x801FC498
 * @behavior Word 16 (0-based) of the 18-word primary state table at
 *           0x801FE3E4, which dispatchPrimaryStateTable
 *           indexes with the shared primary scene state byte D_80146874. It
 *           steps the shared secondary scene state byte D_80146875 through a
 *           22-entry case table in which only states 0, 1, 10, 12, 20 and 21
 *           have a body; every other value, and every value above 0x15, falls
 *           to the epilogue. State 0 arms progress 0x1E and secondary state 1
 *           unless the shared phase byte D_80143BB0 already reads 2. State 1,
 *           at progress 0x1F, advances the shared selection counter
 *           0x80144DF2 and gates the next entry through func_801650B4 mode 1:
 *           a zero answer raises the progress byte to 0x20 and joins the reset
 *           tail, while a nonzero answer copies the twelve-byte name
 *           func_80165D48 answers for mode 1 and that entry into the shared
 *           name buffer D_801490D8, clears its terminator D_801490E4, queues
 *           front-end action 0x94 through func_80150224, latches the phase byte
 *           to 2, sets flag bit 0x22 of the scenario flag word D_8014686C,
 *           queues cue 0x106 and arms secondary state 0x15. State 10 latches
 *           progress 0x23 and secondary state 0x0C unless the phase byte reads
 *           2. State 12 mirrors state 1 over the second selection counter
 *           0x80144DF5 at progress 0x24, with action 0x95, flag bit 0x23 and
 *           the name buffer D_801490F8 with its terminator D_80149104. State 20
 *           requests front-end mode 1 through func_80150284, latches the phase
 *           byte to 2 and arms secondary state 0x15. State 21, unless the phase
 *           byte reads 2, clears the progress byte and joins the reset tail,
 *           which clears D_80146865, runs the shared overlay release helper
 *           func_8015C058 and clears the secondary and primary state bytes. The
 *           body takes no arguments and returns nothing; its 0x20-byte frame
 *           keeps $ra over $s1 and $s0, the state-byte load is hoisted above
 *           the frame setup, and the 22-word case table sits in the payload
 *           leading table data at 0x801F71F4.
 *
 * Shape notes from the original bytes: the arms that read and write the
 * progress byte, and the arms that read their selection counter twice, reach
 * those cells through a local pointer, because the original holds those
 * addresses in $s1 and $s0 across the calls; the arms that only store the
 * progress byte once keep the direct symbol form. The counter pointer is
 * initialized after the progress guard, which is where the original loads
 * $s0, and the progress pointer before it, which is where it loads $s1. Writing
 * both pointers as initialized declarations at the top of the arm block instead
 * loaded the counter address ahead of the guard and measured 156/160
 * instructions (97.50%) with that pair of stores as the only difference. The
 * reset tail statement run appears in three arms and the 0x15 store into the
 * secondary state byte in three arms, so the compiler merges each run into one
 * shared block. This target's manifest places the compiler .rodata table onto
 * the payload leading table data at 0x801F71F4, so the emitted table words
 * compare equal slot for slot, including the sixteen default slots no case
 * owns. A scan of all 398 extracted payloads under out/binaries found no
 * byte-identical copy of this 0x280 span, so the clone lever was declined and
 * the body was seeded from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSecondarySequence16(void) {
  switch (D_80146875) {
    case 0:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 0x1Eu;
      D_80146875 = 1u;
      break;

    case 1: {
      u8* progress = &g_ScenarioProgress;
      u8* field;

      if (*progress != 0x1Fu) {
        break;
      }

      field = &stateSelectionCounter;
      if (func_801650B4(1, *field + 1, 1, 0) == 0) {
        *progress = 0x20u;
        D_80146865 = 0u;
        func_8015C058();
        D_80146875 = 0u;
        D_80146874 = 0u;
      } else {
        strncpy(D_801490D8, func_80165D48(1, *field + 1), 12);
        D_801490E4[0] = 0u;
        func_80150224(0x94);
        D_80143BB0 = 2u;
        func_8015B580(D_8014686C, 0x22);
        func_8015DF18(0x106);
        D_80146875 = 0x15u;
      }
      break;
    }

    case 10:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 0x23u;
      D_80146875 = 0xCu;
      break;

    case 12: {
      u8* progress = &g_ScenarioProgress;
      u8* field;

      if (*progress != 0x24u) {
        break;
      }

      field = &stateSelectionCounter2;
      if (func_801650B4(2, *field + 1, 1, 0) == 0) {
        *progress = 0x25u;
        D_80146865 = 0u;
        func_8015C058();
        D_80146875 = 0u;
        D_80146874 = 0u;
      } else {
        strncpy(D_801490F8, func_80165D48(2, *field + 1), 12);
        D_80149104[0] = 0u;
        func_80150224(0x95);
        D_80143BB0 = 2u;
        func_8015B580(D_8014686C, 0x23);
        func_8015DF18(0x106);
        D_80146875 = 0x15u;
      }
      break;
    }

    case 20:
      func_80150284(1);
      D_80143BB0 = 2u;
      D_80146875 = 0x15u;
      break;

    case 21:
      if (D_80143BB0 == 2u) {
        break;
      }

      g_ScenarioProgress = 0u;
      D_80146865 = 0u;
      func_8015C058();
      D_80146875 = 0u;
      D_80146874 = 0u;
      break;
  }
}
