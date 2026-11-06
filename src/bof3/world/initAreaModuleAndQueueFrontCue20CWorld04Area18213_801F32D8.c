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

/* Byte cleared unconditionally on every entry. No other target map, source or
 * annotation references this address, so the role stays unknown and the name
 * stays address-anchored. */
extern u8 D_801455C7; /* @source 0x801455C7 @kind data */

/* Shared record/effect setup routine at 0x801EECA4: it copies the words at
 * 0x34/0x38/0x3C of the record published at 0x8014638C into the scratchpad
 * work object and allocates two rows through func_801E590C. It is unlifted in
 * its own bmagic overlays, so no prototype is imported from there. */
void func_801EECA4(void);

/* Shared frontend cue dispatcher at 0x8015DF18; the cue id arrives
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
 * Name evidence: the overlay carries no in-image caller of this address, and
 *   its trailing descriptor words at 0x801F37B4 hold the in-overlay handler
 *   table base 0x801F3708 and, at 0x801F37C4, this function's own address, so
 *   the range is the module entry the world-map overlays call from outside the
 *   payload. The two gated steps are the whole body: the progress-gated setup
 *   call and the front-cue request. The gate byte 0x801455C7 has no other
 *   reference in any map, source or annotation, so the name describes the entry
 *   role and keeps that byte unnamed beyond its clear.
 *
 * Shape: the 80-byte span at payload 0x6D8..0x728 is byte-identical to the
 *   world04 area siblings 176, 177, 178, 179, 180, 181, 183 and 185 at the same
 *   load address (payload `cmp` of the span), and it holds no address-specific
 *   immediate. Every one of those sibling rows is still an unlifted `asm` span,
 *   so there is no exact source to clone and the clone lever is ineligible. The
 *   body is seeded from this target's original bytes with its own bindings:
 *   `lb`/`slti 8` guard, `lbu` test whose load the original keeps ahead of the
 *   unconditional byte clear, and the cue id materialised in the call's branch
 *   delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initAreaModuleAndQueueFrontCue20CWorld04Area18213_801F32D8(void) {
  if (D_80146870 >= 8) {
    func_801EECA4();
  }
  D_801455C7 = 0;
  if (D_80143F03 != 0) {
    func_8015DF18(0x20C);
  }
}
