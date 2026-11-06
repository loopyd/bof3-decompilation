#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C2744 through 0x800C2770 are
 * in-image code pointers, this payload's functions with the shared
 * main-executable handlers at 0x801E.... The whole run sits inside the
 * 0x800C2640 `textbin` row, which the reviewed Splat layout places, so it is
 * only referenced here. This dispatcher indexes it from 0x800C2744, so its base
 * is entry 0 of that table: 0x800C1EF4, then 0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x800C18A0, 0x801E4AE8, 0x801E4D8C,
 * 0x801E4F64, 0x801E52E8 and 0x801E5288. */
extern void (*D_800C2744[])(void); /* @source 0x800C2744 @kind data */

/* @source 0x800C1EB0
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at offset
 * 0x01 as the index and calls the handler pointer at that index of the local
 * handler table based at 0x800C2744. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after that load stay nops. The
 * pointer-cell load precedes frame creation; the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call. The payload contains no word or jal site
 * naming 0x800C1EB0, so its caller is outside this overlay. Takes no arguments
 * and forwards none. Same shape as the already-exact siblings
 * dispatchStateByte1Table2660BossBoss00816_800C1804,
 * dispatchStateByte1Table26ccBossBoss00816_800C1CD8 and
 * dispatchStateByte1Table2708BossBoss00816_800C1DC4, which read the same
 * dispatch byte through their own tables.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table2744BossBoss00816_800C1EB0, D_800C2744);
