#include "bof3/bof3.h"

/* Signed scenario-state byte (the first byte of the etc/game scenario-state
 * record), read here with `lb`, so the declaration keeps the s8 width that
 * include/bof3/ui/shop00_internal.h and include/bof3/world/area01613_internal.h
 * give it. */
extern s8 D_80146870; /* @source 0x80146870 @kind data */

/* Front-side byte published by the front-selector updates (reviewed as the
 * "front-side byte D_80143F03" by the scenario annotations); read with `lbu`
 * here, so it stays u8. Bind by the shared map row. */
extern u8 D_80143F03;

/* Byte cleared unconditionally on every entry. This payload's own handler
 * func_801F33E4 (0x801F33E4, still an `asm` range) sets it to 1 when the
 * incoming coordinate pair leaves the 0x41..0x44 / 0x14..0x17 window and
 * consumes it when a later pair enters that window, so the byte is a pending
 * area-transition flag, but no target map, source or annotation names it; the
 * role stays unevidenced and the name stays address-anchored. */
extern u8 D_801455C7; /* @source 0x801455C7 @kind data */

/* The shared record/effect setup routine at 0x801EECA4 copies the words at
 * 0x34/0x38/0x3C of the record published at 0x8014638C into the scratchpad
 * work object and allocates two rows through func_801E590C. Unlifted in its own
 * bmagic overlays, so no prototype is imported from there. */
void func_801EECA4(void);

/* The shared frontend cue dispatcher at 0x8015DF18; the cue id arrives
 * zero-extended, as the same declaration in
 * include/bof3/scenario/sce10eff_internal.h records. */
void func_8015DF18(u16 arg0);

/* @source 0x801F32D8
 * @behavior World-overlay area-module entry. When the signed scenario-state
 *   byte 0x80146870 reads at least 8 it first runs the shared record/effect
 *   setup routine 0x801EECA4; it then clears the byte 0x801455C7 and, when the
 *   front-side byte 0x80143F03 reads nonzero, requests frontend cue 0x20C
 *   through the shared frontend cue dispatcher 0x8015DF18. Takes no arguments
 *   and returns nothing.
 *
 * Name evidence: nothing in this payload branches to 0x801F32D8, and its
 *   trailing descriptor run holds this function's own address in the payload's
 *   last word 0x801F37DC (payload offset 0xBDC), so the range is the module
 *   entry the world-map overlays call from outside the payload. The two gated
 *   steps are the whole body: the progress-gated setup call and the front-cue
 *   request. The gate byte 0x801455C7 is only ever set and consumed by the
 *   neighbouring handler func_801F33E4 and carries no reviewed name, so the
 *   function name describes the entry role and keeps that byte unnamed beyond
 *   its clear.
 *
 * Shape: the 80-byte span at payload 0x6D8..0x728 is byte-identical to the
 *   world04 area siblings 176, 177, 178, 179, 180, 181, 182, 183 and 185 at the
 *   same load address (one payload `cmp` per twin), and it holds no
 *   address-specific immediate, so the area182/13 exact source seeds the body.
 *   `lift clone --from emi/world04/area182/13@0x801F32D8` confirmed the same 80
 *   bytes and rewrote no identifier; the copy keeps this lane's evidenced name
 *   instead of the adapter's raw `func_` spelling. Every statement, operand
 *   form and declaration is the sibling's; the `lb`/`slti 8` guard, the `lbu`
 *   test whose load the original keeps ahead of the unconditional byte clear,
 *   and the cue id materialised in the call's branch delay slot all come from
 *   this target's own original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initAreaModuleAndQueueFrontCue20CWorld04Area18413_801F32D8(void) {
  if (D_80146870 >= 8) {
    func_801EECA4();
  }
  D_801455C7 = 0;
  if (D_80143F03 != 0) {
    func_8015DF18(0x20C);
  }
}
