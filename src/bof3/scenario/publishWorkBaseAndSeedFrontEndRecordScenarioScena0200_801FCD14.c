#include "bof3/bof3.h"

/* The shared work base published as the scratchpad cursor by func_801F363C.
 * @source 0x80145E90 @kind unknown
 */
extern u8 D_80145E90[];
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
/* Shared selector byte this overlay reads as the 0x801FE26C table index.
 * @source 0x80144F5A @kind unknown
 */
extern u8 D_80144F5A;
/* Overlay-local byte table in the T_801FE26C textbin segment.
 * @source 0x801FE26C @kind table
 */
extern u8 T_801FE26C[];

u8 func_8019601C(void);

/* @source 0x801FCD14
 * @behavior Publishes the shared work base D_80145E90 as the scratchpad cursor
 * at 0x1F800044, stores the front-end record index returned by func_8019601C
 * into that base's byte 0x0B and, unless that byte reads 0xFF, seeds the
 * 0x74-byte front-end record it selects at 0x80143FC8: record byte 0x00 = 1,
 * byte 0x05 = 6, byte 0x06 = the caller's value plus 0x70 truncated to a byte,
 * word 0x0C = 0, word 0x10 = the byte T_801FE26C[D_80144F5A] and the halfwords
 * at 0x2E/0x30 copied from the work base at the same offsets. Nothing is
 * returned. The two callers pass 0x91
 * (runSubstateProgressFlagAndFrontEndRecordStepsThenReleaseScenarioScena0200_801FB6C4)
 * and 0x99
 * (func_801FB868) and ignore the result.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the record writes re-read work byte
 * 0x0B after every store, so the first three seed statements address the
 * scratchpad cursor cell directly. The last four seed statements share one
 * cursor load in $a1, which the source keeps as a single local pointer, and
 * only the byte 0x0B is re-read through it. The parameter is copied to $s0 in
 * the func_8019601C delay slot because it is not used until the third store.
 * The cursor cell needs its named symbol: the SPAD_PTR_SLOT constant address
 * is CSE'd into one saved register (frame 32 instead of 24, measured 91/110),
 * while the symbol form re-materialises `lui 0x1f80` per access as the
 * original does. The sibling func_801FCECC at 0x801FCECC is the same body with
 * 0x19/0x68 literals instead of 6/0x70.
 */
void publishWorkBaseAndSeedFrontEndRecordScenarioScena0200_801FCD14(u8 step) {
  u8* work;

  D_1F800044 = D_80145E90;
  D_1F800044[0x0B] = func_8019601C();

  if (D_1F800044[0x0B] != 0xFFu) {
    D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1u;
    D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 6u;
    D_80143FCE[(u32)D_1F800044[0x0B] * 0x74u] = (u8)(step + 0x70u);

    work = D_1F800044;
    *(u32*)(D_80143FD4 + (u32)work[0x0B] * 0x74u) = 0u;
    *(u32*)(D_80143FD8 + (u32)work[0x0B] * 0x74u) = T_801FE26C[D_80144F5A];
    *(u16*)(D_80143FF6 + (u32)work[0x0B] * 0x74u) = *(u16*)(work + 0x2E);
    *(u16*)(D_80143FF8 + (u32)work[0x0B] * 0x74u) = *(u16*)(work + 0x30);
  }
}
