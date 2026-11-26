#include "bof3/emi/dispatchers.h"

extern void (*workByte1HandlerTable[])(void); /* @source 0x801F0878 @kind table */

/* Encoded addresses dispatched on work byte 2 by 0x801EEF28. */
extern u32 workByte2HandlerTable[]; /* @source 0x801F0880 @kind table */
/* Angle inputs used while constructing draw vertices in 0x801EF318. */
extern u32 drawVertexAngleTable[]; /* @source 0x801F089C @kind table */
/* Indexed handler addresses used by the dispatch at 0x801F0024.
 * Its selector and callback signatures remain unresolved. */
extern u32 indexedHandlerAddressTable[]; /* @source 0x801F08C4 @kind table */

/* @source 0x801EEEE4
 * @behavior Overlay per-frame handler dispatcher: it reads the scratchpad work
 * object published by the pointer cell at 0x1F800044, takes that object's
 * unsigned byte at offset 0x01, scales it by four and invokes the selected
 * entry of the in-image handler table based at 0x801F0878 with no arguments.
 * It reads no other state and writes none; the pointer cell is
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
EMI_DEFINE_DISPATCHER(callWorkByte1Handler5, callWorkByte1Handler5, workByte1HandlerTable);
