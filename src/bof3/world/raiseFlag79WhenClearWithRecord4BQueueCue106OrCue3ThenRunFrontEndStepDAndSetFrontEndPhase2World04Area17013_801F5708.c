#include "bof3/bof3.h"

/* Shared main-RAM flag bank of the main executable, outside this overlay's
 * payload. Only its address is used, as the base argument of the shared flag
 * tester and of the shared flag setter. Splat binds the address through this
 * target's generated undefined-syms map, so no target-map row is needed. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Front-end phase byte of the shared main executable; the sibling handler
 * 0x801F4574 of this overlay gates its sound cue on the same byte and the
 * handlers that own it treat 2 as busy. This body stores 2 once, after the
 * front-end step call below. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared flag tester of the main executable: reports whether the flag whose
 * index is the second argument is set in the bank the first argument points at.
 * The payload only calls it, so the address stays anchored. */
s32 func_8015B5D4(u32 bank, s32 flag); /* @source 0x8015B5D4 */

/* Shared flag setter of the same bank, the setter half of the family above. */
void func_8015B580(void* bank, u8 bit_index); /* @source 0x8015B580 */

/* Shared record gate of the main executable: the plchar and world lifts of this
 * repository call it with a record family byte and a record id to ask whether
 * that record is available, and report the answer as a byte. The byte width is
 * the width those lifts declare; the original tests the answer as a whole word,
 * which is the residual noted below. */
u8 func_801650B4(u8 arg0, u8 arg1, u8 arg2, u8 arg3); /* @source 0x801650B4 */

/* Shared main-executable resource request helper; the sibling lifts pass a
 * resource id and drop the result. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Shared front-end entry-action request; the exact sisyou lifts call it with an
 * action id taken from their master record. */
void func_80150224(s32 action_id); /* @source 0x80150224 */

/* Sound-cue dispatcher of the main executable. */
void func_8015DF18(u16 cue); /* @source 0x8015DF18 */

/* Main-executable cue dispatcher: the phase steps of this repository hand it a
 * single word cue id in the same id range as the sound cues above. */
void func_80150284(u32 cue); /* @source 0x80150284 */

/* Shared front-end helper that takes the step selector. */
void func_801BE1B0(u32 step); /* @source 0x801BE1B0 */

/* @source 0x801F5708
 * @behavior Front-end request of this overlay's tail group. It keeps the
 *           scratchpad work pointer cell 0x1F800044 in a callee-saved register
 *           for the whole body and writes it back at the end, so the sequence
 *           runs against the caller's work record. It first asks the shared
 *           flag tester func_8015B5D4 whether flag 0x79 of the main-RAM flag
 *           bank D_80144F28 is set and, only while that flag is clear, asks the
 *           shared record gate func_801650B4 whether record 0x4B of family 1 is
 *           reported. When flag 0x79 is clear and that record is reported it
 *           takes the long arm: sets flag 0x79 through func_8015B580, requests
 *           resource 1 through func_8014D6B8, requests front-end entry action
 *           0x22 through func_80150224 and queues sound cue 0x106 through
 *           func_8015DF18. Every other case falls to the short arm, which hands
 *           cue id 3 to the main-executable cue dispatcher func_80150284. Both
 *           arms then join: the shared front-end helper func_801BE1B0 runs the
 *           step selector 0xD, the front-end phase byte D_80143BB0 is set to 2,
 *           and the saved scratchpad work pointer is written back. Takes no
 *           arguments and returns nothing; the 0x20-byte frame keeps $s0, $s1
 *           and $ra.
 * @status partial
 * @match 95.56
 * @residual Two words of the 0xB4-byte span differ. The record gate above is
 *           declared with the byte width the other lifts of that call give it,
 *           so testing its answer costs one `andi v0,v0,0xff` the original does
 *           not carry. The original also materialises the short arm's argument
 *           constant in the `beqz` delay slot and leaves the `jal
 *           func_80150284` delay slot a nop, while this spelling fills that
 *           delay slot with the long arm's first argument and folds the
 *           constant into the call, which leaves the body one instruction short
 *           at the same 0xB4-byte size. Every other instruction matches, and the
 *           frame, the saved registers and the four arm calls are identical.
 */
void raiseFlag79WhenClearWithRecord4BQueueCue106OrCue3ThenRunFrontEndStepDAndSetFrontEndPhase2World04Area17013_801F5708(
    void) {
  u8* bank;
  u8* saved;

  bank = &D_80144F28;
  saved = SPAD_PTR_SLOT(u8, 0x44u);
  if (func_8015B5D4((u32)bank, 0x79) == 0 &&
      func_801650B4(1, 0x4B, 1, 0) != 0) {
    func_8015B580(bank, 0x79);
    func_8014D6B8(1);
    func_80150224(0x22);
    func_8015DF18(0x106);
  } else {
    func_80150284(3);
  }
  func_801BE1B0(0xD);
  D_80143BB0 = 2;
  SPAD_PTR_SLOT(u8, 0x44u) = saved;
}
