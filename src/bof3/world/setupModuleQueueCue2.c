#include "bof3/bof3.h"

/* Signed scenario-state byte (the first byte of the etc/game scenario-state
 * record), read here with `lb`, so the declaration keeps the s8 width that
 * include/bof3/ui/shop00_internal.h and include/bof3/world/area01613_internal.h
 * give it. */
extern s8 D_80146870; /* @source 0x80146870 @kind data */

/* Front-side byte the front-selector update publishes once the player position
 * resolves into a pending world id's bounding box; read with `lbu` here, so it
 * stays u8. Bound by the shared map row. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Pending area-transition flag. This payload's own neighbour handler at
 * 0x801F33E4 stores 1 into it while the packed incoming coordinate pair falls
 * outside the 4-wide 0x41..0x44 / 0x14..0x17 window, and consumes it - clearing
 * the byte and queueing cue 0x20C - when a later pair falls inside that window.
 * No map, source or annotation names it, so the name stays address-anchored. */
extern u8 D_801455C7; /* @source 0x801455C7 @kind data */

/* Shared record/effect setup routine at 0x801EECA4: it copies the words at
 * 0x34/0x38/0x3C of the record published at 0x8014638C into the scratchpad
 * work object and allocates two rows through func_801E590C. Unlifted in its own
 * bmagic overlays, so no prototype is imported from there. */
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
 * Name evidence: no `jal` word in this payload targets 0x801F32D8 (its six
 *   call words name 0x801EECA4, 0x8015DF18 twice, 0x8015D71C, 0x8019FA28 and
 *   0x82002340), while the payload's last word 0x801F37C4 (payload offset
 *   0xBC4) holds 0x801F32D8 beside the in-overlay handler-table base 0x801F3708
 *   at 0x801F37B8, so the range is the module entry the world-map overlays
 *   reach from outside the payload. The two gated steps are the whole body: the
 *   progress-gated setup call and the front-cue request. The gate byte
 *   0x801455C7 carries no reviewed name, so the function name describes the
 *   entry role and keeps that byte address-anchored beyond its clear.
 *
 * Shape: the 80-byte span at payload 0x6D8..0x728 is byte-identical to the
 *   world04 area sibling spans 176, 177, 178, 180, 181, 182, 183, 184 and 185 at
 *   the same payload offset 0x6D8, and to area175's span at payload 0x18D8
 *   (`cmp -n 80` against the 182 and 184 payloads reports no differing byte;
 *   the same 80 bytes were located at those offsets in every listed payload).
 *   The span holds no address-specific immediate, so the already-exact
 *   area182/13 source for these bytes seeded this body: the clone dry-run
 *   reported the same 80 bytes and rewrote no identifier. Every literal,
 *   operand form and symbol binding below comes from this target's own original
 *   bytes: the `lb`/`slti 8` guard, the `lbu` test whose load the original keeps
 *   ahead of the unconditional byte clear, and the cue id materialised in the
 *   call's delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setupModuleQueueCue(void) {
  if (D_80146870 >= 8) {
    func_801EECA4();
  }
  D_801455C7 = 0;
  if (D_80143F03 != 0) {
    func_8015DF18(0x20C);
  }
}
