#include "bof3/bof3.h"

/* The original re-reads this cell on a path separated only by a store, so the
 * plain view is folded and loses the third `lui`/`lw` pair. Declared with the
 * same volatile pointer cell the byte-exact bmagic siblings use for 0x1F800044.
 */
extern u8* volatile scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);
void func_801F85F0(s32 arg0, s32 arg1, s16 arg2);

/* @source 0x801F7320
 * @behavior Call the overlay helper at 0x801F85F0 with the signed work
 * halfwords 0x2E, 0x30 and 0x32, reload the scratchpad work pointer at
 * 0x1F800044, then decrement work halfword 0x32 modulo 65536 only when the
 * low two bits of work halfword 0x5A are clear, reloading the work pointer
 * as the last statement of that arm. Decrement work halfword 0x5A modulo
 * 65536; when that countdown reaches zero the arm queues the front-end sound
 * cue 0x209, reloads the work pointer, re-arms 0x5A with 0x14 and advances
 * work byte 1 modulo 256. Takes no arguments and returns nothing; the frame
 * only keeps $ra across its two calls. The local helper's wider role and the
 * work fields' meanings remain unknown; the shape and helper identity follow
 * the byte-adjacent siblings 0x801F7164 and 0x801F73D4, which call the same
 * helper and drive the same 0x5A countdown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelper85F0DecrementWorkHalfword32AndTickCountdown5AQueueCue209ScenarioScena0800_801F7320(void) {
  u8* work;
  u8* next;
  u8* armed;
  u16 countdown;

  work = scenarioScena0800Work;
  func_801F85F0(*(s16*)(work + 0x2E), *(s16*)(work + 0x30),
                *(s16*)(work + 0x32));
  next = scenarioScena0800Work;
  if ((*(u16*)(next + 0x5A) & 3) == 0) {
    *(u16*)(next + 0x32) -= 1;
    next = scenarioScena0800Work;
  }
  countdown = *(u16*)(next + 0x5A) - 1;
  *(u16*)(next + 0x5A) = countdown;
  if (countdown == 0) {
    dispatchSoundCue(0x209);
    armed = scenarioScena0800Work;
    *(u16*)(armed + 0x5A) = 0x14;
    armed[1]++;
  }
}
