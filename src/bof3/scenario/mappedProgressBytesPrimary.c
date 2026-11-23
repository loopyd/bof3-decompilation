#include "bof3/scenario/scena06_state.h"

/* Shared command-code mapper at 0x8016628C: table, code, mode. */
extern s32 func_8016628C(s32 table, s32 code, s32 mode);

/* @source 0x801FDD7C
 * @behavior Entry 0 (the word at 0x801FE48C) of the 22-word in-image
 * code-pointer run 0x801FE48C..0x801FE4E4 that
 * dispatchCoordinateLookup (0x801FDD14) calls
 * with jalr after func_801A7AD8 answers the index of a matched lookup record,
 * so it takes no arguments. It first asks the shared command-code mapper
 * func_8016628C (0x8016628C) for the mapped code of table 1 at code 0x47 and
 * tests the whole answer against zero. A non-zero answer runs the shared
 * front-end startup helper func_8015C088 (0x8015C088), sets the shared
 * sub-state byte D_80146875 (0x80146875) to 5, clears the shared progress byte
 * g_ScenarioProgress (0x80146864) and the shared bytes D_80146865..D_80146867
 * (0x80146865..0x80146867) in that order and requests primary scene state 0x0B.
 * A zero answer runs the same startup helper, clears those four bytes in the
 * same order, clears D_80146875 and requests the same primary state 0x0B. Both
 * arms answer the constant 1, whose two identical request tails the compiler
 * merges into the single D_80146874 (0x80146874) store and epilogue after the
 * arm bodies. Its 0x18-byte frame only keeps $ra across the two calls, and each
 * byte store materialises the 0x8014 page in $at on its own. The following run
 * word waitFlag27RequestPrimary13
 * (0x801FDE34) keeps the same startup-and-clear sequence for a flag-0x27 wait,
 * so it is shape evidence and not clone proof, and no other payload in the tree
 * carries these 0xB0 bytes. Seen with the mirrored guard (`!= 0` and the
 * sub-state-5 arm as the if body), the same body measured 38 of 44
 * instructions (86.36%) at the original 0xB0 bytes, its first difference the
 * opposite branch polarity and the two arm bodies exchanged, so the guard is
 * written as an equality on the mapper's answer and the clear-only arm is the
 * if body.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 mappedProgressBytesPrimary(void) {
  if (func_8016628C(1, 0x47, 0) == 0) {
    SCENA06_RESET_PROGRESS_AND_REQUEST(0x0B);
    return 1;
  }
  SCENA06_SET_SUBSTATE_RESET_PROGRESS_AND_REQUEST(5, 0x0B);
  return 1;
}
