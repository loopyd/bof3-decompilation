#include "bof3/bof3.h"
#include "ui/panel_task_templates.h"

/* The primitive cursor 0x8014598C lives in the main executable, outside this
 * EMI payload, so the target's shared map binds that word and this source only
 * declares its width. */
extern u8 *g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Engine primitive-append helper of the main executable: queues the 0x10-byte
 * primitive at ordering-table index 1.
 * @source 0x8014E5A0 */
extern void func_8014E5A0(u32 ot_index, u32 primitive_size);

/* @source 0x801D4678
 * @behavior Emits one 8-byte sprite packet at g_PrimCursor, the BATE overlay
 *           copy of the panel icon wrapper: SetSprt8 initialises the packet,
 *           then the two signed coordinates fill the halfwords at 0x8 and 0xA,
 *           the two 32-bit arguments fill the bytes at 0xC and 0xD after a left
 *           shift of three, the fifth argument, read from the caller frame at
 *           0x40 as an unsigned halfword, fills 0xE and the sixth, read at 0x44
 *           as an unsigned byte, fills 0x04, 0x05 and 0x06. It marks the packet
 *           semi transparent with a zero argument and appends it through
 *           func_8014E5A0(1, 0x10) before returning; its 0x30-byte frame only
 *           keeps $s0 to $s6 and $ra across that work. The 0xB8 bytes equal
 *           under cmp emitPanelIconPrim at 0x801D2520 in emi/etc/sisyou/00 and
 *           emitPanelIconPrim at 0x801DA120 in emi/etc/shop/00, so the body is
 *           the shared PANEL_ICON_PRIM template with no target-specific
 *           literal, and the role name follows those two copies.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ICON_PRIM(emitPanelIconPrimBate00_801D4678)
