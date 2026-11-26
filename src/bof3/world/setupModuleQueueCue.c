#include "bof3/bof3.h"

/* Signed scenario-state byte (the first byte of the etc/game scenario-state
 * record), read here with `lb`, so the declaration keeps the s8 width that
 * include/bof3/ui/shop00_internal.h and include/bof3/world/area01613_internal.h
 * give it. */
extern s8 D_80146870; /* @source 0x80146870 @kind data */

/* Front-side byte published by the front-selector updates; read with `lbu`
 * here, so it stays u8. Bind by the shared map row. */
extern u8 D_80143F03; /* @source 0x80143F03 @kind data */

/* Byte cleared unconditionally on every entry. This target's map, sources and
 * annotations reference it only through this clear, so the role stays unknown
 * and the name stays address-anchored. */
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
 * Name evidence: this payload carries no `jal` to 0x801F32D8, and its trailing
 *   descriptor words hold the in-overlay handler table base 0x801F3708 at
 *   0x801F37B8 and this function's own address at 0x801F37C4, so the range is
 *   the module entry the world-map overlays invoke from outside the payload.
 *   The two gated steps are the whole body: the progress-gated setup call and
 *   the front-cue request. The gate byte 0x801455C7 has no other reference in
 *   this target's map, sources or annotations, so the name describes the entry
 *   role and keeps that byte unnamed beyond its clear.
 *
 * Shape: the 80-byte span at payload 0x6D8..0x728 is byte-identical to the
 *   world04 area siblings 176, 177, 179, 180, 181, 182, 183, 184 and 185 at the
 *   same load address (one payload `cmp` of the span), and it holds no
 *   address-specific immediate. Siblings 182, 183 and 184 already had exact
 *   source when this file was written, so the duplicate lever applied:
 *   `bin/harness lift clone --from emi/world04/area182/13@0x801F32D8` reported
 *   80 byte-identical bytes and supplied the statement order, operand forms,
 *   offset-local spelling and declaration style. The `lb`/`slti 8` guard, the
 *   `lbu` test whose load the original keeps ahead of the unconditional byte
 *   clear, and the cue id materialised in the call's branch delay slot come
 *   from this span's own words, as do the `%hi`/`%lo` bindings above.
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
