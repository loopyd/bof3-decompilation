#include "bof3/bof3.h"

typedef void (*BmagicMagic06403Handler)(void);

/* Overlay-local handler table at 0x801F1394, the table this dispatcher
 * indexes: entry 0 = 0x801F0DA8 (clears the work byte at offset 0x9 and then
 * advances the work byte at offset 0x2), entry 1 = 0x801F0DD8, entry 2 =
 * 0x801F0E50 and entry 3 = 0x801F0E90, all four of them boundaries of this
 * overlay. The four words end at 0x801F13A4, which this overlay reads as a
 * halfword. The run sits inside the trailing data block based at 0x801F138C;
 * no map row owns its address, so the reference is bound by the target's
 * generated undefined-symbol table. */
extern BmagicMagic06403Handler D_801F1394[]; /* @source 0x801F1394 @kind data */

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
void dispatchWorkByte2HandlerBmagicMagic06403_801F0D64(void) {
  D_801F1394[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
