#include "bof3/bof3.h"

/* Shared gate-status halfword and gate byte. The area056/13 gate family at
 * 0x801F30A4-0x801F316C stores 9/10/11/0xD/0xE/0xFFFF through the same pair,
 * where 0xFFFF is the "no gate" value. */
extern volatile u16 D_801490A8;
extern volatile s8 D_801490C7;

/* Second byte of the shared flag bank based at 0x80144EA0; the sibling entry
 * 0x801F2C04 passes 0x80144EA0 itself to func_8015B580 as a flag bank. */
extern u8 D_80144EA1; /* @source 0x80144EA1 @kind unknown */

/* Leading three bytes of the shared front-end request record at 0x80145E90:
 * the gate byte, the type byte and a third selector byte. */
extern u8 D_80145E91; /* @source 0x80145E91 @kind unknown */
extern u8 D_80145E92; /* @source 0x80145E92 @kind unknown */
extern u8 D_80145E93; /* @source 0x80145E93 @kind unknown */

/* Shared scroll pair the world overlays read as row and column. */
extern s16 D_80145EC6[]; /* @source 0x80145EC6 @kind data */
extern s16 D_80145ECA;   /* @source 0x80145ECA @kind data */

/* Resident front-end cue routine and the two-argument cue routine. */
void func_8015DF18(u16 arg0);
void func_8015507C(s32 arg0, s32 arg1);

/* @source 0x801F3200
 * @behavior Stores the no-gate sentinel 0xFFFF into the shared gate-status
 *           halfword D_801490A8 and then returns while the shared gate byte
 *           D_801490C7 reads nonzero. When that byte reads zero it queues
 *           front-end cue 0x208 through func_8015DF18 and returns unless the
 *           low seven bits of the shared flag-bank byte D_80144EA1 are all
 *           set, that is unless `(D_80144EA1 & 0x7F) == 0x7F`; the bank's top
 *           bit is not part of the test. Only then does it read the shared
 *           scroll pair and publish a front-end request: gate byte
 *           D_80145E91 = 2, type byte D_80145E92 = 7 and selector byte
 *           D_80145E93 = 0, followed by func_8015507C over the pair.
 *
 *           The two pair reads precede the three record stores in the
 *           original, so the call arguments reach $a0/$a1 ahead of the store
 *           group and no copy is needed before the call. The 0x18-byte frame
 *           only keeps $ra at 0x10. Takes no arguments and returns nothing;
 *           the value left in $v0 is the last stored constant, which no caller
 *           reads.
 *
 *           The status store precedes the gate load in the original. Both
 *           shared cells use volatile accesses to preserve that ordering.
 * @status partial
 * @match 96.97
 * @residual Gate-load signedness at +0x10: the original uses lb, while this
 *           source emits lbu for the zero test. All other instructions and
 *           the 132-byte function size match under the owning profile.
 */
void clearGateStatusQueueCue208AndArmFrontEndRequestGate2Type7AtScrollPairWhenGateClearAndFlagByte7fWorld01Area05613_801F3200(
    void) {
  s32 cueArg;
  s32 cueId;

  D_801490A8 = 0xFFFF;
  if (D_801490C7 != 0) {
    return;
  }
  func_8015DF18(0x208);
  if ((D_80144EA1 & 0x7F) != 0x7F) {
    return;
  }
  cueArg = D_80145EC6[0];
  cueId = D_80145ECA;
  D_80145E91 = 2;
  D_80145E92 = 7;
  D_80145E93 = 0;
  func_8015507C(cueArg, cueId);
}
