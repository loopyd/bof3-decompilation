#include "bof3/bof3.h"

/* Shared main-executable enemy-ready helper at 0x801E30F8, outside this
 * overlay's image and bound through this target's generated undefined-funcs
 * map. It returns a byte that its callers in this tree mask to eight bits and
 * test for zero, which is how the exact boss lift
 * forwardWorkRecordIndexAndRollRandomActionThenRaiseFlagBit4AndResetScratchState
 * WhenEnemyReadyBossBoss02513_800C1F90 declares and calls it. */
extern u8 func_801E30F8(void); /* @source 0x801E30F8 @kind function */

/* Shared main-executable flag-and-slot helper at 0x801E5704, outside this
 * overlay's image and bound through this target's generated undefined-funcs
 * map. The exact main-executable lift raiseBit80AllocSlot, whose row sits at
 * this address in the battle overlay of the shared image, shows the body: when
 * the current enemy scratch flags allow it and rand() draws an odd value it
 * raises bit 0x80 of the battle flag halfword 0x801462E8 and allocates one
 * queued slot through 0x801E590C. That lift is another target's record, so this
 * body only calls the address and passes it no arguments. */
extern void func_801E5704(void); /* @source 0x801E5704 @kind function */

/* Shared main-executable scratch-state reset at 0x801E567C, outside this
 * overlay's image and bound through this target's generated undefined-funcs
 * map. The exact boss lifts at 0x800C1F90 of BIN/BOSS/BOSS025.EMI#13 and at
 * 0x800C227C of BIN/BOSS/BOSS055.EMI#16 call the same entry with no arguments
 * and declare it the same way. */
extern void func_801E567C(void); /* @source 0x801E567C @kind function */

/* Shared battle flag halfword at 0x801462E8, outside this overlay's image and
 * bound through this target's generated undefined-syms map, so no target-map
 * row is added here. The boss lifts named above raise single bits of this
 * halfword (0x80 and 0x10 in the boss025 entry, bit 0x4 here) and the boss055
 * entry tests bit 0x4 of it, but which phase flag each bit reports is not
 * established by this body, so the spelling stays address-anchored. */
extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x800C1AA0
 * @behavior Entry 1 of this overlay's work-byte-2 code pointer run based at
 *           0x800C2190: the word at 0x800C2194 holds this address, the exact
 *           sibling callWorkTable2190Boss indexes
 *           that base with the work record byte 2 scaled by four, and run word
 *           0 at 0x800C2190 (0x800C1A54) ends by advancing that byte, so the
 *           dispatcher reaches this body through the run word and no jal in the
 *           payload encodes its address. It first calls the shared
 *           main-executable enemy-ready helper 0x801E30F8 with no arguments and
 *           returns without further work while the low byte of the result is
 *           zero. When the helper reports, it raises bit 0x4 of the shared
 *           battle flag halfword 0x801462E8 at halfword width, calls the shared
 *           main-executable flag-and-slot helper 0x801E5704 with no arguments
 *           and then the shared scratch-state reset 0x801E567C; the raise is
 *           scheduled into the delay slot of that first call. The fixed address
 *           is reached through a pointer local assigned after the guard, which
 *           caches the base in $v1 for both the halfword load and the store;
 *           the direct-cell spelling measured before this gate instead
 *           materialised the address once per access and matched 13 of 22
 *           instructions at 0x58 bytes, against 20 of 20 at 0x50 here. The
 *           0x18-byte frame only keeps the return address at 0x10, the body
 *           takes no arguments, and the byte the helper left in $v0 is
 *           discarded because every path returns through the one epilogue. The
 *           0x50-byte span compares equal word for word with the span at
 *           0x800C1BF0 of BIN/BOSS/BOSS055.EMI#16; that twin lies inside a
 *           textbin data run of its own target, so it carries no source to
 *           clone and this seed comes from this payload's bytes with the
 *           declaration style of the exact siblings named above.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseAllocatorEnemyReady(
    void) {
  u16 *flag;

  if (func_801E30F8() == 0) {
    return;
  }
  flag = &D_801462E8;
  *flag |= 4;
  func_801E5704();
  func_801E567C();
}
