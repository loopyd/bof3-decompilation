#include "bof3/bof3.h"
#include <rand.h>

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801F85F0(s32 arg0, s32 arg1, s16 arg2);
void func_80196070(void);

/* @source 0x801F75E4
 * @behavior Overlay work-state handler, entry 9 of the twelve-entry handler
 * sub-table at 0x801FE8A0 (entry 12 of the 51-entry run at 0x801FE894) that
 * the work-byte-1 dispatcher at 0x801F707C selects with unsigned byte 1 of
 * the scratchpad work object: it lays out the ring through helper 0x801F85F0
 * with the signed work halfwords 0x2E, 0x30 and 0x32, then adds the low byte
 * of rand() to work halfword 0x58 modulo 65536 and, when the low two bits of
 * work halfword 0x5A are clear, decrements work halfword 0x2C modulo 65536.
 * It then decrements work halfword 0x32 by 1 and the 0x5A countdown modulo
 * 65536, storing the countdown before the retreated halfword, and calls the
 * shared work-flag reset helper at 0x80196070 when that countdown reaches
 * zero. The scratchpad work pointer at 0x1F800044 is reloaded before the ring
 * call, after the rand() call and after the guarded arm; the 0x18-byte frame
 * only keeps $ra across those three calls. Takes no arguments and returns
 * nothing. The helper's wider role and the work fields' meanings remain
 * unknown; the shape and helper identity follow the byte-adjacent siblings
 * 0x801F74E0, which drives the same 0x58/0x5A/0x2C/0x32 fields, and
 * 0x801F774C, which calls the same helper and resets the work flags on
 * countdown expiry.
 *
 * Name evidence: entry 9 of the twelve-entry handler-pointer sub-table at
 * 0x801FE8A0 is this address, so the work-byte-1 dispatcher at 0x801F707C
 * reaches it through work byte 1; the operations below name the family's
 * fields the way the exact neighbour 0x801F74E0 does.
 *
 * Residual evidence: the compiler's own local-alloc dump for this source
 * places the reloaded work pointer in $v0 (5 references, first definition in
 * the tail block) and then gives the value halfword $a0 and the countdown
 * $v1. The original tail block instead holds the pointer in $a0 with the
 * countdown in $v0 and the value in $v1, which is the disposition this
 * compiler gives a work pointer that is globally allocated, as the
 * byte-adjacent 0x801F74E0 and 0x801F774C show. Every other instruction of
 * this function, its 0xA4 size and its 0x18-byte frame already match.
 * @status partial
 * @match 78.05
 * @residual The tail block's three quantities are cyclically rotated: the reloaded work pointer takes $v0 instead of $a0, so the value halfword takes $a0 and the countdown halfword $v1; every other instruction and the 0xA4 size match.
 */
void addRandomByteToWorkHalfword58AndTickCountdown5ADecrement2CAnd32ThenResetWorkFlagsScenarioScena0800_801F75E4(void) {
  s32 random;
  u8* work;
  u8* next;
  u8* after;
  u16 countdown;
  u16 value;

  work = scenarioScena0800Work;
  func_801F85F0(*(s16*)(work + 0x2E), *(s16*)(work + 0x30),
                *(s16*)(work + 0x32));
  random = rand();
  next = scenarioScena0800Work;
  *(u16*)(next + 0x58) += random & 0xFF;
  if ((*(u16*)(next + 0x5A) & 3) == 0) {
    *(u16*)(next + 0x2C) -= 1;
  }
  after = scenarioScena0800Work;
  value = *(u16*)(after + 0x32) - 1;
  countdown = *(u16*)(after + 0x5A) - 1;
  *(u16*)(after + 0x5A) = countdown;
  *(u16*)(after + 0x32) = value;
  if (countdown == 0) {
    func_80196070();
  }
}
