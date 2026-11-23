#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-executable resource request helper; the sibling world, boss and
 * plchar lifts declared the same prototype and passed a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* @source 0x801F3A24
 * @behavior Entry 0 of this overlay's handler pointer run at 0x801F4A30, whose
 *           second to sixth words name 0x801F3A64, 0x801F3AEC, 0x801F3C1C,
 *           0x801F3CA0 and 0x801F3E54; no jal in this payload names it, so the
 *           run alone reaches it. It loads the scratchpad work-object pointer
 *           cell 0x1F800044 once, reads the object's unsigned byte at +0x08 and
 *           passes that byte's bit 0x04 as the selector of its single argument:
 *           the resource request helper func_8014D6B8 receives 0x40 when the bit
 *           reads set and 0x41 when it reads clear. The branch's delay slot
 *           holds the 0x41 candidate, so a clear bit keeps the value it already
 *           staged. It takes no arguments and returns nothing, and its 0x18-byte
 *           frame keeps only the return address at 0x10 across that one call.
 *           The role of work-object byte 0x08 is not established by this body,
 *           so the name describes the selection rather than the resource.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResource40PerWork(void) {
  func_8014D6B8((D_1F800044[8] & 4) ? 0x40 : 0x41);
}
