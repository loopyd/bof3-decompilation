#include "bof3/bof3.h"

/* The shared work base this overlay publishes as the scratchpad cursor: the
 * front-end record allocator writes the claimed record index into its byte
 * 0x0B, and the record's halfwords 0x2E/0x30 are copied from it.
 * @source 0x80146110 @kind unknown
 */
extern u8 D_80146110[];
/* Scratchpad work-object pointer cell; the original re-materialises its
 * address for every access, so it keeps the symbol form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Front-end record table and the field bases of its 0x74-byte rows: the
 * original re-materialises `lui at,0x8014` for every store, so each field is
 * addressed as its own absolute symbol rather than through one struct base.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FCE @kind unknown */
extern u8 D_80143FCE[];
/* @source 0x80143FD4 @kind unknown */
extern u8 D_80143FD4[];
/* @source 0x80143FD8 @kind unknown */
extern u8 D_80143FD8[];
/* @source 0x80143FF6 @kind unknown */
extern u8 D_80143FF6[];
/* @source 0x80143FF8 @kind unknown */
extern u8 D_80143FF8[];
/* Shared selector byte: nonzero keeps this seeder out, and its value is also
 * the index this overlay reads its byte table with.
 * @source 0x80144F5C @kind unknown
 */
extern u8 D_80144F5C;
/* Overlay-local byte table of this payload's D_801F302C textbin segment; row
 * entry i supplies front-end record word 0x10. Bound by the target map. */
extern u8 D_801F302C[];

/* Front-end record allocator: returns the index of the next free 0x74-byte
 * record at 0x80143FC8, or 0xFF when the table is full. */
u8 func_8019601C(void);

/* @source 0x801F2E60
 * @behavior While the shared selector byte D_80144F5C reads zero, publishes the
 *           shared work base D_80146110 as the scratchpad work cursor at
 *           0x1F800044, stores the front-end record index returned by
 *           func_8019601C into that base's byte 0x0B and, unless that byte reads
 *           0xFF, seeds the 0x74-byte front-end record it selects at 0x80143FC8:
 *           record byte 0x00 = 1, byte 0x05 = 6, byte 0x06 = 3, word 0x0C = 0,
 *           word 0x10 = the byte D_801F302C[D_80144F5C] and the halfwords at
 *           0x2E/0x30 copied from the work base at the same offsets. A nonzero
 *           selector byte or a full record table leaves the table untouched.
 *           Takes no arguments and returns nothing. No in-image jal reaches this
 *           address and no other word of this payload holds it, so the overlay
 *           is entered from outside its own payload.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the record writes re-read work byte 0x0B
 * after every store, so the first three seed statements address the scratchpad
 * cursor cell directly. The last four seed statements share one cursor load in
 * $a1, which the source keeps as a single local pointer, and only byte 0x0B is
 * re-read through it. That split and the record-field spellings are taken from
 * the exact sibling publishWorkBaseAndSeedFrontEndRecordScenarioScena0200_801FCD14
 * at 0x801FCD14, whose body is the same instruction sequence. The selector byte
 * is read twice - once for the entry guard and once as the table index after the
 * allocator call - through one local `u8 *` that takes its address, so the
 * original materialises `lui $s0,%hi` plus `addiu $s0,$s0,%lo` once and reads it
 * back with `lbu 0($s0)` at both sites. Reading the named scalar extern directly
 * at both sites instead lets each access fold to its own `lui`/`lbu %lo` pair,
 * which is measured 103/115 instructions with 460->444 bytes and the first
 * difference at instruction 0; the address-taken local is what restores the
 * register form. This body has its own near-twin: the
 * unlifted func_801F2C94 at 0x801F2C94 in the same payload is instruction for
 * instruction the same stream, differing only in the two symbol displacements
 * (0x80144F5B/0x80145FD0 there, 0x80144F5C/0x80146110 here), so the shape is
 * fixed by that pair and only this function's own bindings are written.
 */
void publishWorkBaseAndSeedFrontEndRecordKind3FromModeTableWorld01Area03813_801F2E60(
    void) {
  u8* work;
  u8* selector;

  selector = &D_80144F5C;

  if (*selector == 0u) {
    D_1F800044 = D_80146110;
    D_1F800044[0x0B] = func_8019601C();

    if (D_1F800044[0x0B] != 0xFFu) {
      D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
      D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 6u;
      D_80143FCE[(u32)D_1F800044[0x0B] * 0x74u] = 3u;

      work = D_1F800044;
      *(u32*)(D_80143FD4 + (u32)work[0x0B] * 0x74u) = 0u;
      *(u32*)(D_80143FD8 + (u32)work[0x0B] * 0x74u) = D_801F302C[*selector];
      *(u16*)(D_80143FF6 + (u32)work[0x0B] * 0x74u) = *(u16*)(work + 0x2E);
      *(u16*)(D_80143FF8 + (u32)work[0x0B] * 0x74u) = *(u16*)(work + 0x30);
    }
  }
}
