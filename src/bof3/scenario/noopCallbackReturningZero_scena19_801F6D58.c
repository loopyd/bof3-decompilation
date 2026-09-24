#include "bof3/bof3.h"

/* @source 0x801F6D58
 * @behavior Zero-returning no-op callback for the scena19 overlay: it takes no
 * arguments, reads no state and writes no state, and its only effect is to hand
 * the constant 0 back to its caller. The entire 8-byte body is the plain `jr $ra`
 * epilogue with the zero result materialised in the delay slot (03E00008
 * 00001021, i.e. `addu $v0,$zero,$zero`), which is exactly `return 0;`.
 *
 * Identity/ownership: its address is stored as the fourth pointer word of this
 * overlay's five-word pointer run at 0x801F6DBC (0x801F6C04, 0x801F6D10,
 * 0x801F6D60, 0x801F6D58, 0x00000000); the word at 0x801F6DC4 holds the
 * byte-identical zero-returning twin stub at 0x801F6D60 and 0x801F6DCC is a null
 * word. No reader of that run exists inside this 492-byte overlay: the only
 * absolute table references in the overlay text are the seven-entry handler table
 * at 0x801F6DD0 (`lw $v0,0x6DD0($at)` in func_801F6C40) and 0x801F6DE0
 * (`lw $v0,0x6DE0($at)` in func_801F6D10), so this entry is only observed through
 * the stored pointer word. The preceding 8-byte entry at 0x801F6D50 is the
 * argument-less void no-op twin (`jr $ra` / nop).
 * @status unverified
 * @match 0.00
 * @residual none
 */
s32 noopCallbackReturningZero_scena19_801F6D58(void) {
  return 0;
}
