#include "bof3/bof3.h"

/* Shared main-executable flag-and-slot helper at 0x801E5704, outside this
 * overlay's image and bound through this target's generated undefined-funcs
 * map. The exact main-executable lift raiseBit80AllocSlot, whose row sits at
 * this address in the battle image, shows the body: when the current enemy
 * scratch flags allow it and rand() draws an odd value it raises bit 0x80 of
 * the battle flag halfword 0x801462E8 and allocates one queued slot through
 * 0x801E590C. That lift is another target's record, so this body only calls
 * the address and passes it no arguments. */
extern void func_801E5704(void); /* @source 0x801E5704 @kind function */

/* Shared main-executable enemy-scratch reset at 0x801E567C, outside this
 * overlay's image and bound through this target's generated undefined-funcs
 * map. The exact main-executable lift resetEnemyScratchState shows the body:
 * it resets the current enemy scratch object's first two state bytes, clears
 * one pending bit and conditionally clears byte +0xF5 of that enemy. This
 * body calls it with no arguments after the helper above and discards its
 * result. */
extern void func_801E567C(void); /* @source 0x801E567C @kind function */

/* Shared battle flag halfword at 0x801462E8, outside this overlay's image and
 * bound through this target's generated undefined-syms map, so no target-map
 * row is added here. The boss lifts at 0x800C1AA0 of BIN/BOSS/BOSS028.EMI#16
 * and at 0x800C1F90 of BIN/BOSS/BOSS025.EMI#13 raise single bits of this
 * halfword (0x4 and 0x80) and the boss055 entry at 0x800C227C tests bit 0x4 of
 * it, but which phase flag each bit reports is not established by those
 * bodies, so the spelling stays address-anchored. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* @source 0x800C198C
 * @behavior Guarded shared-helper pair of this overlay. It reads the shared
 *           battle flag halfword 0x801462E8 at halfword width and tests bit
 *           0x4: while that bit is clear it returns without touching anything,
 *           and while it is set it calls the shared main-executable
 *           flag-and-slot helper 0x801E5704 with no arguments and then the
 *           shared enemy-scratch reset 0x801E567C, the same two entries in the
 *           same order that the boss028 lift at 0x800C1AA0 calls after raising
 *           that bit. This body writes no state itself and returns nothing;
 *           the result the last call leaves in $v0 is discarded by the one
 *           epilogue. The 0x18-byte frame only keeps $ra at 0x10, and the
 *           original schedules the frame reservation after the flag load, the
 *           test into the two slots ahead of the branch, the return-address
 *           store into the branch delay slot, and a nop into the delay slot of
 *           both calls. No jal in this payload names 0x800C198C: its only
 *           in-image reference is the word at 0x800C1DD8, record word 10 of
 *           the payload-final textbin row D_800C1DB0, and which consumer reads
 *           that word is not established by this body, so no caller-supplied
 *           argument role exists and the entry takes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callSlotAllocatorReset(
    void) {
  if (D_801462E8 & 4) {
    func_801E5704();
    func_801E567C();
  }
}
