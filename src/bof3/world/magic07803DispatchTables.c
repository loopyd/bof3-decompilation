#include "bof3/bof3.h"

extern void (*D_801F0878[])(void);

/* @source 0x801EEEE4
 * @behavior Overlay per-frame handler dispatcher: it reads the scratchpad work
 * object published by the pointer cell at 0x1F800044, takes that object's
 * unsigned byte at offset 0x01, scales it by four and invokes the selected
 * entry of the nine-word in-image code pointer run based at 0x801F0878 with no
 * arguments. It reads no other state and writes none; the pointer cell is
 * loaded before the 0x18-byte frame is created and that frame only keeps $ra
 * across the indirect call. The table address is built as `lui $at,
 * %hi(table)` / `addu $at, $at, index * 4` / `lw $v0, %lo(table)($at)`. The 68
 * bytes are instruction-for-instruction identical to the byte-2 dispatchers
 * dispatchByte2TableB478 (0x801E4F64, table 0x801EB478) and its siblings in
 * `emi/battle/battle/03`, differing only in the index byte and table address,
 * and the role matches the exact sibling
 * dispatchWorkByte1HandlerBmagicMagic00800_801F0250 of this module.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchWorkByte1HandlerBmagicMagic07803_801EEEE4, D_801F0878);
