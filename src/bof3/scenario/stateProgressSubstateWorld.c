#include "bof3/scenario/scena06_state.h"

/* Shared world id halfword of the main-executable data segment: this body reads
 * it once per world gate, five gates in all.
 * @source 0x80143F00 @kind unknown
 */
extern u16 D_80143F00;
/* Shared scenario flag word of the same segment, handed to the flag tester
 * func_8015B5D4 and to the bit-set helper func_8015B580.
 * @source 0x8014686C @kind data
 */
extern u32 D_8014686C;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FD800
 * @behavior Code-pointer word 2 of the five-word run at 0x801FE3C4
 *           (0x801F7298, 0x801FCDD0, this address, 0x801FD294, 0x801FDD14); no
 *           in-image jal targets the address, so the two argument roles stay
 *           structural and the body answers as a signed word. It gates five
 *           separate world ids in the shared world id halfword D_80143F00, then
 *           tests the scenario flag word D_8014686C through func_8015B5D4 and
 *           the two packed position words, and arms one of six state tuples:
 *           for world 0x27 either progress 0x0A with sub-state 5 and primary
 *           state 2, or (behind flag 7 set, flag 8 clear) flag 8 set, progress
 *           cleared and primary state 3; for world 0x39 (flags 0x10 set, 0x11
 *           clear) a cleared state tuple with primary state 5; for world 0x43
 *           (flag 0 clear) the same tuple with primary state 0x0C; for world
 *           0x4D either (flag 0x28 clear, flag 0x27 set) primary state 0x0E, or
 *           (flag 0x2A clear, flag 0x27 set) progress 0x14 with sub-state 0x14
 *           and primary state 0x0E; for world 0x5E either (flag 0x0D set, flag
 *           0x0E clear) sub-state 0xA with primary state 4, or (flag 0x1E
 *           clear) sub-state 0x1E with primary state 4, or (flag 0x24 clear)
 *           progress 0x14 with sub-state 5 and primary state 0x0A. Every armed
 *           arm runs the shared front-end startup helper func_8015C088 first
 *           and answers 1; every other path answers 0.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation notes. The position gates are two-value equality runs on one
 * argument word and an unsigned window on the other, so each pair differs by
 * 0x8000 and the window is written as `(u32)(word - base) <= size`. The six
 * state tuples use the target's own two state macros where their store order
 * matches (progress, 0x65, 0x66, 0x67, sub-state, primary) and plain stores
 * where the arm sets progress itself. The first measurement missed one hunk
 * class: the state macros already run func_8015C088, so an extra call ahead of
 * them duplicated the startup helper in five arms (1300 -> 1340 bytes,
 * 281/325 instructions). The second measurement left four branches: the flag
 * 0x1E arm reaches the next gate when its flag test fails but answers 0 when
 * either position test fails, so that arm nests its position test and returns
 * 0 inside the flag block instead of joining the flat condition chain. The
 * clone lever is ineligible: a scan of the payload directory for this
 * 0x514-byte span found it only in this target's own SCENA06 slot 0 payload.
 */
s32 stateProgressSubstateWorld(
    s32 arg0, s32 arg1) {
  if (D_80143F00 == 0x27) {
    if ((arg0 == 0x310000 || arg0 == 0x318000) &&
        (u32)(arg1 - 0x270000) <= 0x28000) {
      func_8015C088();
      g_ScenarioProgress = 0x0A;
      D_80146875 = 5;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146874 = 2;
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 7) != 0 &&
        func_8015B5D4(D_8014686C, 8) == 0 &&
        (arg1 == 0x370000 || arg1 == 0x378000) &&
        (u32)(arg0 - 0x430000) <= 0x18000) {
      func_8015B580(D_8014686C, 8);
      SCENA06_RESET_PROGRESS_AND_REQUEST(3);
    }
  }
  if (D_80143F00 == 0x39 && func_8015B5D4(D_8014686C, 0x10) != 0 &&
      func_8015B5D4(D_8014686C, 0x11) == 0 &&
      (arg1 == 0x520000 || arg1 == 0x528000) &&
      (u32)(arg0 - 0x270000) <= 0xB8000) {
    SCENA06_RESET_PROGRESS_AND_REQUEST(5);
    return 1;
  }
  if (D_80143F00 == 0x43 && func_8015B5D4(D_8014686C, 0) == 0 &&
      (arg1 == 0x1F0000 || arg1 == 0x1F8000) &&
      (u32)(arg0 - 0x1C0000) <= 0x38000) {
    SCENA06_RESET_PROGRESS_AND_REQUEST(0x0C);
    return 1;
  }
  if (D_80143F00 == 0x4D) {
    if (func_8015B5D4(D_8014686C, 0x28) == 0 &&
        func_8015B5D4(D_8014686C, 0x27) != 0 &&
        (arg0 == 0x300000 || arg0 == 0x318000) &&
        (u32)(arg1 - 0x40000) <= 0x58000) {
      SCENA06_RESET_PROGRESS_AND_REQUEST(0x0E);
      return 1;
    }
    if (func_8015B5D4(D_8014686C, 0x2A) == 0 &&
        func_8015B5D4(D_8014686C, 0x27) != 0 &&
        (arg0 == 0x1B8000 || arg0 == 0x1C0000) &&
        (u32)(arg1 - 0x40000) <= 0x58000) {
      func_8015C088();
      g_ScenarioProgress = 0x14;
      D_80146875 = 0x14;
      D_80146865 = 0;
      D_80146866 = 0;
      D_80146867 = 0;
      D_80146874 = 0x0E;
      return 1;
    }
  }
  if (D_80143F00 != 0x5E) {
    return 0;
  }
  if (func_8015B5D4(D_8014686C, 0x0D) != 0 &&
      func_8015B5D4(D_8014686C, 0x0E) == 0 &&
      (arg0 == 0x430000 || arg0 == 0x438000) &&
      (u32)(arg1 - 0x320000) <= 0x28000) {
    SCENA06_SET_SUBSTATE_RESET_PROGRESS_AND_REQUEST(0x0A, 4);
    return 1;
  }
  if (func_8015B5D4(D_8014686C, 0x1E) == 0) {
    if ((arg0 == 0x1B8000 || arg0 == 0x1C0000) &&
        (u32)(arg1 - 0x3D0000) <= 0x28000) {
      SCENA06_SET_SUBSTATE_RESET_PROGRESS_AND_REQUEST(0x1E, 4);
      return 1;
    }
    return 0;
  }
  if (func_8015B5D4(D_8014686C, 0x24) == 0 &&
      (arg0 == 0x1B8000 || arg0 == 0x1C0000) &&
      (u32)(arg1 - 0x3D0000) <= 0x28000) {
    func_8015C088();
    g_ScenarioProgress = 0x14;
    D_80146875 = 5;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146874 = 0x0A;
    return 1;
  }
  return 0;
}
