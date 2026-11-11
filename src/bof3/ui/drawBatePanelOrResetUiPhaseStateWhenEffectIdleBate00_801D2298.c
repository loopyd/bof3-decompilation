#include "bof3/bof3.h"

/* Shared front-end effect-busy gate halfword of the main executable, read here
 * with a zero-extending halfword load before the frame is created. It lies
 * outside this EMI payload and keeps the original `lhu` width.
 * @source 0x80143C40 @kind unknown */
extern u16 D_80143C40;

/* Shared sprite-grid index byte of the main executable, handed straight to the
 * overlay sprite emitter func_801D627C. The original reads it with `lbu` into
 * the first argument register ahead of the frame setup.
 * @source 0x80144953 @kind unknown */
extern u8 D_80144953;

/* Shared UI byte of the main executable that this overlay arms with 1 on the
 * phase change and that its handlers never read back. Owned by the shared map,
 * so no target-map row is added.
 * @source 0x80148650 @kind unknown */
extern u8 D_80148650;

/* Shared UI phase byte of the main executable: the index this overlay's
 * three-entry phase handler table is dispatched with. Owned by the shared map.
 * @source 0x80148651 @kind unknown */
extern u8 D_80148651;

/* Shared UI sub-step byte of the main executable, zeroed here on the phase
 * change and advanced by the sub-step handlers. Owned by the shared map.
 * @source 0x80148652 @kind unknown */
extern u8 D_80148652;

/* Shared per-frame phase timer of the main executable, zeroed with the phase
 * change. The shared map does not own this address, so it keeps the raw
 * spelling at the original byte width.
 * @source 0x80148654 @kind unknown */
extern u8 D_80148654;

/* Sprite emitter of this overlay: takes the shared sprite-grid index as its
 * only argument and returns nothing this body consumes.
 * @source 0x801D627C @kind unknown */
extern void func_801D627C(u32 grid);

/* Panel emitter of this overlay: draws the panel anchored at the signed
 * coordinates the two arguments hold. Its own prologue narrows the pair with
 * `andi 0xFFFF`, so the parameters are declared wide here and the constant
 * arguments keep their bare `addiu` form.
 * @source 0x801D2304 @kind unknown */
extern void func_801D2304(s32 x, s32 y);

/* @source 0x801D2298
 * @behavior Third handler of the BATE overlay phase handler table D_801D8764,
 *           which dispatchUiPhaseTableBate00_801D2070 indexes with the shared UI
 *           phase byte, and the same drawing pair its sibling func_801D2190
 *           opens with. It reads the shared front-end effect-busy gate halfword
 *           D_80143C40 once at entry. While that gate is set the handler only
 *           redraws the shared screen: it passes the shared sprite-grid index
 *           byte D_80144953 to this overlay sprite emitter func_801D627C and
 *           then draws this overlay panel at the fixed coordinates 0x37, 0x37
 *           through func_801D2304. Once the gate is clear it resets the shared
 *           UI phase state instead of drawing: the per-frame phase timer
 *           D_80148654 goes to 0, the shared UI byte D_80148650 is armed with 1,
 *           and the shared UI phase byte D_80148651 and sub-step byte
 *           D_80148652 are cleared, which sends the next dispatch back to phase
 *           zero. Nothing is returned and the 0x18-byte frame only keeps the
 *           return address at 0x10 across the two calls of the busy arm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawBatePanelOrResetUiPhaseStateWhenEffectIdleBate00_801D2298(void) {
  if (D_80143C40 == 0) {
    D_80148654 = 0;
    D_80148650 = 1;
    D_80148651 = 0;
    D_80148652 = 0;
  } else {
    func_801D627C(D_80144953);
    func_801D2304(0x37, 0x37);
  }
}
