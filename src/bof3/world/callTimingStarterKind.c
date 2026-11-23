#include "bof3/bof3.h"

/* Shared front-end timing starter of the main executable, outside this payload:
 * a mode word, its kind word and the pair of 100-step timing bounds. The other
 * bodies of this overlay's handler run reach it with kind 1 or 2; this one
 * passes kind 0.
 * @source 0x8015D300 @kind function
 */
extern void func_8015D300(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* Shared selection identifier byte of the main executable: the front-end
 * consumers index their 4-byte-stride record pair with it, pass it as the
 * published front-end cue argument, and treat 0xFF as no selection.
 * @source 0x80145029 @kind unknown
 */
extern u8 D_80145029;

/* @source 0x801F37B8
 * @behavior Entry 5 of the overlay handler pointer run at 0x801F4FD0: calls the
 * shared front-end timing starter at 0x8015D300 with arguments 0, 0, 100 and
 * 100, then stores the selection identifier 0x83 into the shared selection byte
 * at 0x80145029 and returns. The 0x18-byte frame keeps only $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callTimingStarterKind(void) {
  func_8015D300(0u, 0u, 100u, 100u);
  D_80145029 = 0x83;
}
