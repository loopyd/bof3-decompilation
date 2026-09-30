#include "bof3/bof3.h"

/* @source 0x801F8CDC
 * @behavior Overlay per-frame handler: it decrements the countdown byte at
 * offset 2 of the object pointer it receives in $a0 and re-stores it, advances
 * the object's byte at offset 3 by 6 and re-stores that, and only when the
 * decremented countdown reads zero does it re-arm the countdown from the
 * object's byte at offset 4 and increment the object's byte at offset 1. It
 * reads and writes no other memory, makes no call, keeps no register and has no
 * frame. The two writes are ordered by the emitted stream: byte 2 is loaded
 * first, byte 3 is loaded into the slot after it, byte 2's store precedes the
 * zero test of the masked countdown, and byte 3's store is scheduled into that
 * test's delay slot, so the byte-3 advance is unconditional and only the
 * re-arm and the byte-1 increment sit behind the branch. It is a handler, not
 * an ordinary helper: the payload has no jal to 0x801F8CDC, and the address is
 * pointer word 26 (0x801FE894 + 26*4 = 0x801FE8FC) of this overlay's 51-entry
 * per-frame handler run at 0x801FE894, i.e. entry 6 of the sub-run at
 * 0x801FE8E4 that func_801F7AD0 indexes with the work object's byte 2. The two
 * handlers beside it in that sub-run are its siblings in shape and vocabulary:
 * 0x801F8D18 (entry 8) also decrements byte 2 and increments byte 1 on expiry
 * but re-arms byte 2 with the constant 0x20, and 0x801F8D4C (entry 9) also
 * decrements byte 2 but integrates a velocity word into a position word and
 * clears byte 0 when the countdown expires. Byte 4 is evidenced as the re-arm
 * value for a record of this family by func_801F8B50, which seeds a record's
 * byte 4 with 0x40 while seeding its byte 2 with 8.
 * @status exact
 * @match 100
 * @residual none
 *
 * Matching note: the decisive clean-C lever is the *source statement order* of
 * the two independent byte chains, not their emitted order. Writing the byte-3
 * advance first (`object[3] = (u8)(object[3] + 6);`) ahead of the byte-2
 * countdown statement leaves the 15-instruction stream and its order untouched
 * but flips the allocator pair: the later-written byte-2 countdown chain (three
 * references) then takes $v1 and the byte-3 chain takes $v0, as in the
 * original. The pre-reload scheduler and the delay-slot pass still emit the
 * byte-2 load first and the byte-3 store in the `bnez` slot. A named local for
 * the byte-3 value instead forces `lbu $3,3($4)` to the head and loses the load
 * order (rejected).
 */
void func_801F8CDC(u8* object) {
  u8 countdown;

  object[3] = (u8)(object[3] + 6);
  countdown = (u8)(object[2] - 1);
  object[2] = countdown;
  if (countdown == 0) {
    object[2] = object[4];
    object[1]++;
  }
}
