#include "bof3/bof3.h"

typedef void (*ScenarioScena1200WorkStateHandler)(void);

/* Code-pointer run based at 0x801FD178, selected by the work object's byte at
 * offset 0x01: entry 0 is the byte-2 dispatcher 0x801F71B8, entry 8 (0x801FD198)
 * is 0x801F7BEC, which advances that state byte. The target map owns the run's
 * base address under the pre-promotion spelling func_801FD178. */
extern ScenarioScena1200WorkStateHandler func_801FD178[];

/* @source 0x801F7174
 * @behavior Overlay per-frame state dispatcher: it reads the scratchpad work
 * object published by the pointer cell at 0x1F800044, takes that object's
 * unsigned state byte at offset 0x01, scales it by four and invokes the entry
 * of the in-image code-pointer run based at 0x801FD178 with no arguments. Entry
 * 0 (0x801FD178) is 0x801F71B8, the overlay's dispatcher for the object's byte
 * at offset 0x02; entry 8 (0x801FD198) is 0x801F7BEC, whose handler advances
 * exactly this state byte from 8 to 9 (clearFieldAdvanceStateAndQueueCue215ScenarioScena1200_801F7BEC),
 * and entry 11 (0x801FD1A4) is 0x801F7F28
 * (dispatchProgressHandlerScenarioScena1200_801F7F28), so the byte is a
 * per-frame state index. It reads no other state and writes none; the pointer
 * cell is loaded before the 0x18-byte frame is created and that frame only keeps
 * $ra across the indirect call. The table address is built as `lui $at,
 * %hi(table)` / `addu $at, $at, index * 4` / `lw $v0, %lo(table)($at)`. The
 * 68 bytes are instruction-for-instruction identical to the byte-1 dispatchers
 * at 0x801F7BA8 (this overlay, table 0x801FD198), 0x801F6F48
 * (dispatchWorkByte1HandlerScenarioScena0800_801F6F48), 0x801F73B8
 * (dispatchWorkByte1HandlerScenarioScena0100_801F73B8), 0x801F6F38 and
 * 0x801F77F8 (scena15/00) and 0x801F6DC0 (dispatchWorkByte1Handler), differing
 * only in that table address word. 0x801F7174 is the first code boundary of
 * this overlay's main segment (payload offset 0x574) and no in-image `jal` or
 * pointer word targets it, so its caller is outside the overlay. The name
 * records the proven role and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena1200_801F7174(void) {
  func_801FD178[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
