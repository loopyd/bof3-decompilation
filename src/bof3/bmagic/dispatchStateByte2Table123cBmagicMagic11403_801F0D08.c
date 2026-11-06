#include "bof3/bof3.h"

/*
 * Code-pointer run at 0x801F123C, the table this dispatcher indexes: entry 0 =
 * 0x801F0D6C, entry 1 = 0x801F0E4C. The run sits at the end of the 0x50-byte
 * data span that starts at 0x801F11F4.
 */
extern void (*D_801F123C[])(void); /* @source 0x801F123C @kind data */

/* Global the battle step rewrites around a handler call; the two literals this
 * function publishes are read nowhere in this payload, so the cell keeps its
 * address-anchored name. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/* @source 0x801F0D08
 * @behavior Publishes 0x800C3800 into the global at 0x801459F0, then dispatches
 * one overlay work handler and publishes 0x800D3800 into the same global. The
 * handler is selected from the work record published at the scratchpad pointer
 * cell 0x1F800044: its unsigned byte at offset 2 is scaled by four and the
 * selected entry of the two-entry code-pointer run based at 0x801F123C
 * (0x801F0D6C, 0x801F0E4C) is invoked with no arguments. Both literals reach
 * the cell through their own lui/ori pair and a lui %hi/sw %lo store, and the
 * 0x18-byte frame only keeps $ra across the indirect call. The run at 0x801F121C
 * holds this function's own address as entry 2, so the overlay reaches it from
 * outside; no word of this payload jal-calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table123cBmagicMagic11403_801F0D08(void) {
  D_801459F0 = 0x800C3800;
  D_801F123C[SPAD_PTR_SLOT(u8, 0x44)[2]]();
  D_801459F0 = 0x800D3800;
}
