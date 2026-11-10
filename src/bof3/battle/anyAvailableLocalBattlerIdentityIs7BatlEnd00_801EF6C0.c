#include "bof3/bof3.h"

/* Volatile party-size byte at 0x801462F0. The original loads it once for the
 * entry test and again on every loop test (`lui`/`lbu` of the same global
 * immediately before the `sltu` bound), so the load is never hoisted. */
extern volatile u8 localBattlerCount;

/* Identity byte pair reached as 0x80145F16 plus slot*0x140, i.e. bytes +0x86
 * and +0x87 of the 0x140-byte local battler work records that run from
 * 0x80145E90. The record form keeps the original addressing: the base is built
 * as `lui $at,%hi(D_80145F16)` + `addu $at,$at,slot*0x140`, so the second byte
 * keeps its own `+0x87` displacement instead of becoming a `1($at)` offset. */
typedef struct BatlEndIdentityPair {
  volatile u8 identity_86;
  volatile u8 identity_87;
  u8          pad_88[0x13e];
} BatlEndIdentityPair;

extern BatlEndIdentityPair D_80145F16[]; /* @source 0x80145F16 @kind unknown */

/* The EXE-side per-slot availability test at 0x801DB524 (lifted as
 * isBattlerInactiveOrFlag4000 in the battle/03 target): it reports nonzero
 * when the slot is inactive or carries the 0x4000 rejection flag. Outside this
 * overlay, so it is declared here for its address. */
extern u8 func_801DB524(u8 arg0);

/* @source 0x801EF6C0
 * @behavior Reports whether any available local battler carries identity value
 *   seven: walks the slots below the volatile count byte at 0x801462F0, skips
 *   each slot whose availability test func_801DB524 reports nonzero, and
 *   returns 1 as soon as identity byte +0x86 or +0x87 of that slot's 0x140-byte
 *   work record reads 7. Returns 0 when the count byte is zero or when no
 *   walked slot matches. The count byte is re-read on every loop test, the
 *   constant 7 is materialised once in a preserved register, and the 0x20-byte
 *   frame keeps $ra and $s0-$s2 across the per-slot call. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 * The only caller, func_801EF348, walks the same records for its own per-slot
 * pass and multiplies the shared word at 0x80146330 by 1.5 (`v += v >> 1`)
 * after this body reports a match. The pair tested here is the +0x86/+0x87
 * half of the +0x85..+0x87 identity triple that battle/03's func_801E0E0C and
 * selectLocalCleanupSubstate compare against the identity values 0x18 and 0x43;
 * the value 7 has no further evidence here, so the name states the test only.
 */
u8 anyAvailableLocalBattlerIdentityIs7BatlEnd00_801EF6C0(void) {
  u8 index;

  index = 0;
  if (localBattlerCount != 0) {
    do {
      if (func_801DB524(index) == 0) {
        if (D_80145F16[index].identity_86 == 7) {
          return 1;
        }
        if (D_80145F16[index].identity_87 == 7) {
          return 1;
        }
      }
      index += 1;
    } while (index < localBattlerCount);
  }
  return 0;
}
