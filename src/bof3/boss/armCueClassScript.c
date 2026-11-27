#include "bof3/bof3.h"

extern u8 *boss028CurrentRecordState; /* @source 0x1F800044 @kind data */

/* Current enemy work record pointer cell in main RAM, outside this overlay's
 * image: the battle code publishes the record the overlays run against. This
 * target's generated undefined_syms_auto.txt already binds the address and
 * every access here is byte-addressed, so the declaration keeps the byte view
 * only and the record's other bytes stay offset-anchored. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */


/* Shared main-executable enemy script byte helper: it selects the script byte
 * named by its argument from the current enemy work record's +0xEC script
 * pointer and applies it through func_8014D8D4, as the exact battle03 lift of
 * that address measured. This overlay's use passes the script byte index 2, so
 * the prototype keeps the address-anchored spelling of the shared helper and
 * this target's generated undefined_funcs_auto.txt binds the address. */
extern void stateDeriveScriptFlag(u32 arg0); /* @source 0x801E2314 */

/* @source 0x800C18E4
 * @behavior Handler entry 0 of this overlay's byte-2 step run, the code pointer
 *           run at 0x800C2184 that the dispatcher
 *           callWorkTable2184Boss indexes with work
 *           byte +2 of the scratch work record at 0x1F800044. It loads the
 *           current enemy work record pointer from the
 *           main-RAM cell 0x801EB4E8, takes that record's unsigned byte +0xE0,
 *           scales it by the 0x88-byte class-row stride and stores the
 *           class-row byte at 0x800E40CE into scratch work byte +0x09. That
 *           byte is the countdown the run's next handler, entry 1 at
 *           0x800C1950, decrements and acts on when it reaches zero. The body
 *           then runs the shared enemy script byte helper stateDeriveScriptFlag for
 *           script byte 2 and, after re-reading the pointer cell, advances
 *           scratch work byte +0x02 by one, moving the dispatcher to that same
 *           entry 1. It writes no other state and leaves no value callers read;
 *           the 0x18-byte frame keeps $ra at 0x10 across the single call, the
 *           record pointer load precedes frame creation, and the class-row
 *           store fills the call's delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: no payload under out/binaries holds a byte-identical copy of
 * this span, so it has no clone seed; a search of every extracted payload for
 * the span bytes hit only this target at payload offset 0xE4. Two near
 * siblings cover it as shape: the leading 0x44 bytes, through the jal and its
 * delay-slot store, equal boss036/16's payload word for word from offset 0xFC,
 * where that body continues with further calls, and the tail from the post-call
 * cell reload to the return equals the last ten instructions of the exact lift
 * clearWorkByteBAndPublishClassRowByteThenRequestResource4AndAdvanceWorkByte2BossBoss05216_800C1DF8
 * word for word. Those two spans supply the statement
 * order and operand forms; the call argument and the class-row address are this
 * body's own. The class-row base stays an explicit fixed address rather than a
 * declared symbol: the original assembles it to `addu at,index,at` from a full
 * 32-bit constant, the form the exact battle15 lift func_800AFF98 records for
 * the same 0x800E40CE byte.
 */
void armCueClassScript(
    void) {
  u8 *work;
  u32 classIndex;

  work = D_801EB4E8;
  classIndex = work[0xE0];
  boss028CurrentRecordState[0x9] = PSX_REF(u8, 0x800E40CEu + classIndex * 0x88u);
  stateDeriveScriptFlag(2);
  boss028CurrentRecordState[0x2]++;
}
