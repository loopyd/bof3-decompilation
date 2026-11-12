#include "bof3/bof3.h"

/* Shared progress byte of the front-end state machine, cleared here.
 * @source 0x80146864 @kind unknown
 */
extern u8 g_ScenarioProgress;

/* Shared state byte beside the progress byte, cleared here; the byte at
 * 0x80146866 between it and D_80146867 is left untouched by this body.
 * @source 0x80146865 @kind unknown
 */
extern u8 D_80146865;

/* Shared state byte beside the progress byte, cleared here.
 * @source 0x80146867 @kind unknown
 */
extern u8 D_80146867;

/* Shared primary state byte of the front-end state machine, set to 7 here.
 * @source 0x80146874 @kind unknown
 */
extern u8 D_80146874;

/* Shared secondary sub-state byte of the front-end state machine, cleared
 * here.
 * @source 0x80146875 @kind unknown
 */
extern u8 D_80146875;

/* Shared scenario flag word the main-executable flag tester func_8015B5D4
 * indexes for flag numbers.
 * @source 0x8014686C @kind unknown
 */
extern u32 D_8014686C;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);
void func_8015507C(s16 x, s16 y);
void func_8015DF18(u16 arg0);

/* @source 0x801FCD58
 * @behavior Overlay callback reached only indirectly: the word at 0x801FD248,
 * index 3 of the nineteen-word code-pointer run based at 0x801FD23C, is this
 * address and is its only in-image reference, so the selector
 * dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54 (0x801FCC54) calls
 * it with jalr and answers its signed low byte when the fourth of the four
 * five-byte lookup records at 0x801FD228 is selected, and no jal names it.
 * It first asks the main-executable flag tester func_8015B5D4 (0x8015B5D4)
 * whether flag 0x2A is set in the shared scenario flag word D_8014686C
 * (0x8014686C) and answers -1 without touching anything else when it is. When
 * the flag is clear it runs the shared front-end startup helper func_8015C088
 * (0x8015C088), clears the shared progress byte g_ScenarioProgress (0x80146864)
 * and the shared state bytes 0x80146865 and 0x80146867 - the byte at 0x80146866
 * between them keeps its value - clears the secondary sub-state byte D_80146875,
 * requests primary scene state 7 in D_80146874, passes the signed halfwords at
 * +0x36 and +0x3A of the work record held by scratchpad pointer cell 0x1F800044
 * to the shared request helper func_8015507C (0x8015507C) as its two arguments,
 * queues front-end cue 0x103 through func_8015DF18 (0x8015DF18) and answers 1.
 * It takes no arguments, reads no state beyond the flag word and that pointer
 * cell, and writes only those five bytes; the 0x18-byte frame only keeps $ra
 * across the four calls, each byte store materialises the 0x8014 page in $at on
 * its own, the requested 7 sits in $v0 ahead of the four clearing stores, and
 * the answer 1 follows the last call. Its statement order is the source of the
 * emitted store order g_ScenarioProgress, 0x80146865, 0x80146867, D_80146875,
 * D_80146874. A bounded comparison of this span against every shipped EMI
 * payload under out/binaries/emi found no byte-identical copy, so the
 * gate-and-clear-and-request shape it shares with
 * clearProgressAndSharedStateBytesThenRequestPrimaryState5Substate5Or0IfPartyByte5ScenarioScena1200_801FCCBC
 * (0x801FCCBC of this overlay) is shape evidence and not clone proof.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the negative answer is the tail return, so the guard is
 * written as the `if (func_8015B5D4(D_8014686C, 0x2A) == 0)` body that falls
 * through into the shared epilogue with `return 1`, followed by `return -1`.
 * Seeding it as the leading `if (...) { return -1; }` with the body after it
 * measured 32/36 instructions and 144 bytes, first difference +0x18: that seed
 * emitted a separate out-of-line `li v0,-1` block after the body with `nop` in
 * the bnez delay slot, where the original fills that slot with `li v0,-1` and
 * jumps straight to the epilogue. The scratchpad cell is read through the
 * SPAD_PTR_SLOT(u8, 0x44) lvalue itself, one `lui`/`lw` pair into $v1;
 * dereferencing that slot as a pointer measured an extra `lbu` and `nop` and
 * moved the pointer into $v0.
 */
s32 requestPrimaryState7Substate0AndCue103UnlessFlag2ASetScenarioScena1200_801FCD58(
    void) {
  u8 *work;

  if (func_8015B5D4(D_8014686C, 0x2A) == 0) {
    func_8015C088();
    work = SPAD_PTR_SLOT(u8, 0x44);
    g_ScenarioProgress = 0;
    D_80146865 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 7;
    func_8015507C(*(s16 *)(work + 0x36), *(s16 *)(work + 0x3A));
    func_8015DF18(0x103);
    return 1;
  }
  return -1;
}
