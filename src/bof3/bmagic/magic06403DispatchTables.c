#include "bof3/emi/dispatchers.h"

extern void (*D_801F1394[])(void); /* @source 0x801F1394 @kind data */

/* @source 0x801F0D64
 * @behavior Overlay per-frame handler dispatcher: it reads the scratchpad work
 * object from the pointer cell at 0x1F800044, takes that object's unsigned byte
 * at offset 0x2, scales it by four and invokes the selected entry of the
 * four-word in-image handler run based at 0x801F1394 (0x801F0DA8, 0x801F0DD8,
 * 0x801F0E50, 0x801F0E90) with no arguments: no argument register is loaded
 * before the indirect call. It reads no other state and writes none; the
 * work-object pointer is loaded before the 0x18-byte frame is created and that
 * frame only keeps $ra across the indirect call. The table address is built as
 * `lui $at, %hi(table)` / `addu $at, $at, index * 4` / `lw $v0,
 * %lo(table)($at)`, and the index load is followed by a load-delay nop. The 68
 * bytes are instruction-for-instruction identical to the byte-2 dispatcher
 * dispatchByte2TableB478 (0x801E4F64, table 0x801EB478) of the battle module,
 * differing only in the %lo table-address immediate of the lw.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchWorkByte2HandlerBmagicMagic06403_801F0D64, D_801F1394);
