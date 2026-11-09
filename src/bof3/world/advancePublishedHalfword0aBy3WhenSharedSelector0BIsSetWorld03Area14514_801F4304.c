#include "bof3/bof3.h"

/* Record pointer cell published in main RAM: the load reads the record address
 * from the cell, then the handler works on the record it points at. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* Shared main-exe byte-result query. Every reviewed call site masks the result
 * with `andi v0,v0,0xFF` before testing it, so the return is declared word-wide
 * and the caller applies the byte extension. */
u32 func_80166140(u32 arg0);

/* @source 0x801F4304
 * @behavior Overlay handler: queries the shared byte-result selector
 *           0x80166140 with the constant 0xB and, when the byte-extended
 *           result is nonzero, advances the halfword at offset 0x0A of the work
 *           record published through the main-RAM pointer cell 0x80148210 by 3.
 *           Takes no arguments, returns nothing and keeps the frame only to
 *           preserve $ra across the call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePublishedHalfword0aBy3WhenSharedSelector0BIsSetWorld03Area14514_801F4304(void) {
  if ((u8)func_80166140(0xB) != 0) {
    D_80148210[5] += 3;
  }
}
