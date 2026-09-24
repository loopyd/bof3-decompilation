#include "bof3/bof3.h"

/* @source 0x801F6CF4
 * @behavior Zero-returning no-op callback for the scena18 overlay: it takes no
 * arguments, reads no state and writes no state, and its only effect is to hand
 * the constant 0 back to its caller. The whole 8-byte body is the epilogue
 * `jr $ra` with the constant-zero result materialised in the delay slot
 * (03E00008 00001021), i.e. plain `return 0;`.
 *
 * Identity/ownership: its address is stored as the fourth pointer word of the
 * overlay's five-word pointer run at 0x801F6D58 (0x801F6C04, 0x801F6CAC,
 * 0x801F6CFC, 0x801F6CF4, 0x00000000 - the word at 0x801F6D64 is this function,
 * and 0x801F6D60 holds the byte-identical twin stub at 0x801F6CFC, while
 * 0x801F6D68 is a null word). No reader of that run exists inside this 392-byte
 * overlay: the three in-overlay dispatchers load from 0x801F6D6C (shared progress
 * byte D_80146872), 0x801F6D78 (byte at 0x80146874) and 0x801F6D7C (record byte
 * at offset 0x7A), so this slot is only observed through the pointer word.
 * @status unverified
 * @match 0.00
 * @residual none
 */
s32 noopCallbackReturningZero_scena18_801F6CF4(void) {
  return 0;
}
