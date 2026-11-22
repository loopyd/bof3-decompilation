#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each access group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through the generated symbol row, so no target-map
 * row or weak-symbol binding is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared helper in the main executable; this overlay only calls it. */
/* @source 0x801E5988 @kind function */
extern void func_801E5988(void);

/* @source 0x801F0B8C
 * @behavior Entry index 31 of the 39-word handler code-pointer run that the
 *           leading tabledata segment stores at 0x801EEC04, the word at
 *           0x801EEC80. It adds 0x10 to the scratchpad work object colour
 *           bytes 0x5D, 0x5E and 0x5F, reading the work-object pointer cell
 *           0x1F800044 before each of the three updates. It then re-reads that
 *           cell and tests byte 0x5D through a signed view; when the value
 *           equals -0x40 (0xC0 unsigned) it decrements byte 0xB of the shared
 *           battle work record read through 0x801EB4E0 and invokes the shared
 *           no-argument helper 0x801E5988, with the record-byte store filling
 *           the call delay slot. The first cell load is hoisted above the frame
 *           adjustment, and the 0x18-byte frame only keeps the return address
 *           at 0x10 across the call. It takes no arguments and returns nothing
 *           and reads or writes no other state, so the roles of the colour
 *           bytes 0x5D, 0x5E and 0x5F and of record byte 0xB stay
 *           offset-anchored. No jal or j word in the payload targets
 *           0x801F0B8C and the only word naming the address is the handler-run
 *           entry at 0x801EEC80, so the caller that selects this entry lies
 *           outside the overlay. The 0xA4-byte span ends exactly at the next
 *           subsegment, 0x801F0C30, so the row claims no trailing bytes. The
 *           164 bytes compared equal to the still-raw span at 0x801F00A8 of the
 *           bmagic001/00 payload and to its span at 0x801EF688, which was
 *           lift-exact when this seed was written; that body supplied the
 *           statement order and this span supplied every literal and binding.
 *           The nearer same-payload sibling 0x801F1180 runs the same triplet
 *           add and channel test but ends by advancing work byte 2, so it gave
 *           shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkColorTripletBy16ThenDecrementGlobalWorkByteBAndInvokeHelperWhenChannelIsC0BmagicMagic01500_801F0B8C(void) {
    D_1F800044[0x5D] += 0x10;
    D_1F800044[0x5E] += 0x10;
    D_1F800044[0x5F] += 0x10;

    if (((s8 *)D_1F800044)[0x5D] == -0x40) {
        D_801EB4E0[0xB]--;
        func_801E5988();
    }
}
