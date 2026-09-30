#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler D_801FE938[];

/* @source 0x801F9DBC
 * @behavior Dispatches this overlay's per-frame handler from the scratchpad work
 * object: it reads the work-object pointer through the scratchpad pointer slot
 * at 0x1F800044, takes that object's unsigned byte at offset 1, scales it by
 * four and invokes that entry of the overlay-local handler-pointer table
 * D_801FE938 with no arguments. D_801FE938 is entry 41 of the 51-entry per-frame
 * handler-pointer run at 0x801FE894 (0x801FE894 + 41*4 = 0x801FE938), whose
 * words from there on are 0x801F9E00, 0x801F9E50, 0x801F9EC4, 0x801F9F2C
 * (incrementWorkByte1ScenarioScena0800_801F9F2C), 0x801F9F4C
 * (incrementWorkByte1ScenarioScena0800_801F9F4C), 0x801F9F6C
 * (invokeHelperScenarioScena0800_801F9F6C), 0x801FA19C, 0x801FE018
 * (dispatchRecordCallbackScenarioScena0800_801FE018), 0x801FE12C and
 * 0x801FE170. It reads no other state and writes none: the pointer slot is
 * loaded before the 0x18-byte frame is created, and that frame only keeps $ra
 * across the indirect call while the callee's return value in $v0 is discarded.
 * The 68 bytes are byte-identical to the same overlay's other work-byte-1
 * dispatchers apart from the loaded table word: 0x801F6F48 (0x801FE894, run
 * entry 0), 0x801F707C (0x801FE8A0, run entry 3), 0x801F7928 (0x801FE8D0, run
 * entry 15) and dispatchWorkByte1HandlerScenarioScena0800_801F95D4 (0x801FE908,
 * run entry 29), all of which reach the same run through the same work-object
 * byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0800_801F9DBC(void) {
  D_801FE938[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
