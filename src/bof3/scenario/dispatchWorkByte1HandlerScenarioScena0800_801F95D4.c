#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler D_801FE908[];

/* @source 0x801F95D4
 * @behavior Dispatches this overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer through the scratchpad pointer slot at
 * 0x1F800044, takes its unsigned byte at offset 1, scales it by four and invokes
 * that entry of the overlay-local handler-pointer table D_801FE908 with no
 * arguments. D_801FE908 is entry 29 of the 51-entry per-frame handler-pointer run
 * at 0x801FE894 (entries 0 through 50, ending at 0x801FE960), and its entries are
 * 0x801F9618, 0x801F9668
 * (invokeHelperScenarioScena0800_801F9668) and
 * 0x801F98B0 (incrementWorkByte1SetCountdownScenarioScena0800_801F98B0) followed
 * by the rest of the run. It reads no other state and writes none: the pointer
 * slot is loaded before the 0x18-byte frame is created, and that frame only keeps
 * $ra across the indirect call while the callee's return value in $v0 is
 * discarded. The bytes are the same shape as the same overlay's work-byte-1
 * dispatchers at 0x801F6F48, 0x801F707C and 0x801F7928, which reach earlier
 * entries of the same run through the same work-object byte. No word anywhere in
 * this overlay's payload stores 0x801F95D4; its in-image reference lies outside
 * the overlay, in the concurrently loaded etc/game overlay
 * (BIN/ETC/GAME.EMI#0, load 0x80195800) at the shared handler-pointer word
 * 0x801C7DCC (payload offset 0x325CC), four words after that overlay's 0x801F7928
 * word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0800_801F95D4(void) {
  D_801FE908[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
