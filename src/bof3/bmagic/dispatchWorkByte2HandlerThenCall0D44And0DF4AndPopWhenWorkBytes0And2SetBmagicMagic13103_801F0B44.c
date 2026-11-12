#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui/lw) ahead of the dispatch and again after the handler call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Three-entry overlay step-handler table for work-object byte 2. The run ends
 * at 0x801F1C30, which this target already names dispatchHandlerTable801F1C30. */
extern void (*dispatchHandlerTable801F1C24[])(void); /* @source 0x801F1C24 @kind data */

/* In-payload helpers called once work byte 0 and work byte 2 are both set.
 * 0x801F0DF4 draws the sixteen-quad translucent grey textured ring band of the
 * transform 0x801F0D44 installs. */
extern void func_801F0D44(void);
extern void
    drawSixteenGreyTexturedRingQuadsAtRadiusWorkWord14BetweenZ0AndZ400FromWorkByteBTexPageBmagicMagic13103_801F0DF4(
        void);

/* @source 0x801F0B44
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 ahead
 * of its 0x18-byte frame and calls the three-entry handler table entry at
 * 0x801F1C24 selected by unsigned work byte 2 scaled by four, without bounding
 * the index. Re-reads the same cell after the handler and tests work byte 0
 * and then work byte 2 from that second load; only when both are nonzero it
 * calls the in-payload helper at 0x801F0D44, then the ring-band draw helper at
 * 0x801F0DF4, then the SDK routine PopMatrix. Stores nothing, reads no other
 * state and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenCall0D44And0DF4AndPopWhenWorkBytes0And2SetBmagicMagic13103_801F0B44(void) {
    dispatchHandlerTable801F1C24[g_battle_work[2]]();
    if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
        func_801F0D44();
        drawSixteenGreyTexturedRingQuadsAtRadiusWorkWord14BetweenZ0AndZ400FromWorkByteBTexPageBmagicMagic13103_801F0DF4();
        PopMatrix();
    }
}
