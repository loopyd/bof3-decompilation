#include "bof3/bof3.h"
#include "ui/panel_task_templates.h"

/* The primitive cursor 0x8014598C lives in the main executable, outside this
 * EMI payload, so the target's shared map binds that word and this source only
 * declares its width. */
extern u8 *g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Engine primitive-append helper of the main executable (lifted in
 * exe/slus_004_22): queues the 0x10-byte primitive at ordering-table index 1.
 * @source 0x8014E5A0 */
extern void func_8014E5A0(u32 ot_index, u32 primitive_size);

/* @source 0x801D5620
 * @behavior Emits one 8-byte sprite packet at g_PrimCursor, the SHISU copy of
 *           the panel icon wrapper: SetSprt8 initialises the packet, then the
 *           two signed coordinates fill the halfwords at 0x8 and 0xA, the two
 *           middle arguments fill the bytes at 0xC and 0xD after a left shift
 *           of three, the fifth argument, read from the caller frame as an
 *           unsigned halfword, fills 0xE and the sixth, read as an unsigned
 *           byte, fills 0x04, 0x05 and 0x06. It marks the packet semi
 *           transparent with a zero argument and appends it through
 *           func_8014E5A0(1, 0x10) before returning. Its three jal targets are
 *           the PsyQ SetSprt8 at 0x8017A9F4, SetSemiTrans at 0x8017A904 and the
 *           main-executable helper 0x8014E5A0, all outside this payload. The
 *           0xB8 bytes equal emitPanelIconPrim at 0x801D2520 in
 *           emi/etc/sisyou/00, and the body is the shared PANEL_ICON_PRIM
 *           template with no target-specific literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ICON_PRIM(emitPanelIconPrimShisu00_801D5620)
