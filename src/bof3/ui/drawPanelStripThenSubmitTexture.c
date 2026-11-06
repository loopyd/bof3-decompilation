#include "bof3/ui/shop00_internal.h"

/* Every panel task root access re-materialises its own target-local symbol
 * relocation, matching the sibling state dispatchers func_801E29C8 and
 * func_801E2AB0: reading the cell through the shared g_PanelTaskRoot view
 * folds its address into one hoisted base. */
#undef D_80148648

/* The texture id sits at task offset 0x10, past the shared 8-byte PanelTask
 * prefix, so it is reached through a byte pointer at the original offset; its
 * role is recovered only as far as selecting the func_801502D0 handle. */
/* @source 0x801E2808
 * @behavior panel handler at entry 0 of the handler table D_801E5D68, which
 *           func_801E27BC dispatches with panel task byte 2: calls the handler
 *           panelField6HandlerTable[D_80148648->state] selected by the panel
 *           task root state byte with no arguments (framed jalr), then draws
 *           the shop panel strip through func_801DAB90 with the same task
 *           root's x and field-6 halfwords, the constants 0x118 and 0x13 and
 *           the main-RAM CLUT-bank byte D_80144952. When the signed halfword at
 *           task offset 0x10 is nonzero it submits a texture through
 *           func_8014F800 at (x + 7, y + 3) with the constants 0 and 0xFF and
 *           the handle func_801502D0 of that id masked to 16 bits, and for the
 *           ids 0x36, 0x49, 0x4A and 0x52 it submits a second texture at the
 *           same position with the handle func_801502D0(0xF).
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The offset-0x10 accesses are load-bearing shapes. Testing the field on its
 * own expression and masking a fresh read for the handle argument reproduces
 * the original signed load, argument copy and delay-slot mask; binding the
 * guard's value to one local first measured 69/80 with an extra unsigned
 * reload, and a 32-bit local measured 67/80 at 312 bytes.
 */
void drawPanelStripThenSubmitTexture(void) {
  s16 textureId;

  panelField6HandlerTable[D_80148648->state]();
  func_801DAB90(D_80148648->x, D_80148648->field_06, 0x118, 0x13, D_80144952);
  if (*((s16*)((u8*)D_80148648 + 0x10)) != 0) {
    func_8014F800((s16)(D_80148648->x + 7), (s16)(D_80148648->field_06 + 3), 0,
                  0xFF,
                  func_801502D0((u16)*((s16*)((u8*)D_80148648 + 0x10))));
    textureId = *((s16*)((u8*)D_80148648 + 0x10));
    if (textureId == 0x36 || (u16)(textureId - 0x49) < 2 || textureId == 0x52) {
      func_8014F800((s16)(D_80148648->x + 7), (s16)(D_80148648->field_06 + 3),
                    0, 0xFF, func_801502D0(0xF));
    }
  }
}
