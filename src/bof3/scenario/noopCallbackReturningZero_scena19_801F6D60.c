#include "bof3/bof3.h"

/* @source 0x801F6D60
 * @behavior Zero-returning no-op callback for the scena19 overlay: it takes no
 * arguments, reads no game state and writes no state, and its only effect is to
 * hand the constant 0 back to its caller. The whole 8-byte body is the epilogue
 * `jr $ra` with the constant-zero result materialised in the branch delay slot
 * (03E00008 00001021), i.e. plain `return 0;`. It is a placeholder handler that
 * lets a dispatch entry run to completion without acting.
 *
 * Identity/ownership: its address is word index 2 of the twelve-word pointer run
 * at 0x801F6DBC, which extends to the end of this 492-byte payload (0x801F6C04,
 * 0x801F6D10, 0x801F6D60, 0x801F6D58, 0x00000000, 0x801F6C40, 0x801F6C54,
 * 0x801F6C68, 0x801F6CA4, 0x801F6D50, 0x801F6D68, 0x801F6DB4). The byte-identical
 * twin stub at 0x801F6D58 is word index 3 of that same run. No `lui`/`addiu` pair
 * anywhere inside the overlay materialises the run base 0x801F6DBC, so this slot
 * is only reached through the pointer word, by a reader outside this payload.
 * @status unverified
 * @match 0.00
 * @residual none
 */
s32 noopCallbackReturningZero_scena19_801F6D60(void) {
  return 0;
}
