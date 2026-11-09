#include "bof3/bof3.h"

/* Scratchpad pointer cell holding this overlay's work record. The original
 * loads the cell before the frame is filled, once per probe column and once
 * more for the closing restore store, so each of those sites names the cell. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */

/* Two-column signed probe-offset table of this overlay's own data span at
 * 0x801F616C. The row is the record byte at +0x09 shifted right by three and
 * the column is the probe index, so the two columns of one row are the two
 * probe offsets this body spends. The bytes at 0x801F6180 and 0x801F6181 are
 * the two halves of the 32-entry two-byte direction run those offsets index.
 * All three are interior bytes of that reviewed run and D_<ADDRESS> names, so
 * the generated undefined-syms map binds them and no target-map row is needed. */
extern s8 D_801F616C[]; /* @source 0x801F616C @kind data */
extern s8 D_801F6180[]; /* @source 0x801F6180 @kind data */
extern s8 D_801F6181[]; /* @source 0x801F6181 @kind data */

/* Gate byte and octant byte of the 0x140-byte-stride work record array of the
 * main executable, both indexed here by the record number the position probe
 * answers. The gate byte is the byte func_801C0F9C requires to read 1 before a
 * record is accepted. */
extern u8 D_80145E91[]; /* @source 0x80145E91 @kind table */
extern u8 D_80145E98[]; /* @source 0x80145E98 @kind table */

/* World flag halfword and front-end phase byte of the main executable. Both
 * are shared symbols of the main image and both gates must be open before a
 * probed record is touched. */
extern u16 D_8014625A; /* @source 0x8014625A @kind data */
extern u8 D_80143BB0;  /* @source 0x80143BB0 @kind unknown */

/* Position probe of the main executable: it answers the record number that
 * occupies the supplied 16.16 point pair, or 0xFF when that point is free. */
extern s32 func_801BE0C0(s32 arg0, s32 arg1, u8 arg2);
/* Action request of the main executable, called with action 2 here. */
extern void func_8015C214(s32 arg0, s32 arg1);

/* @source 0x801F4614
 * @behavior Record table update of this overlay, re-run after the record
 *           builder at 0x801F4798 by both handlers of the state-byte-1
 *           dispatch run based at 0x801F6160. It keeps the record word at
 *           +0x3C in a register, copies the record word at +0x20 into it and
 *           writes the saved word back at the end. For each of the two columns
 *           of the signed offset table at 0x801F616C it takes the row that the
 *           record byte at +0x09 shifted right by three selects and forms a
 *           direction index from the record byte at +0x08 masked with 6 and
 *           scaled by four plus that column byte, scaled by two. It probes the
 *           point built from the record word at +0x34 plus the first byte of
 *           the 32-entry two-byte direction run at 0x801F6180 at that index
 *           shifted left by 16 and the record word at +0x38 plus the second
 *           byte shifted left by 16 with the shared probe at 0x801BE0C0. The
 *           column is skipped when the probe answers 0xFF, while the world
 *           flag halfword 0x8014625A holds either of the bits 0x1000 and
 *           0x0400, or while the front-end phase byte 0x80143BB0 is non-zero.
 *           Otherwise it reads the octant byte D_80145E98 of the answered
 *           0x140-byte work record, takes its low three bits as the value,
 *           replaces it with its successor modulo eight when the low bit is
 *           clear, and when the gate byte D_80145E91 of that record reads 1
 *           writes the value back and requests action 2 for that record
 *           through 0x8015C214. Takes no arguments, returns nothing and keeps
 *           a 0x28-byte frame with ra at +0x20 and the four saved registers s0
 *           to s3 at +0x10 to +0x1C.
 * @status partial
 * @match 87.63
 * @residual Live comparison is 85/97 instructions, 388 bytes on both sides.
 *           The first difference at +0x000C is the scratchpad pointer register:
 *           the original uses a0 for work and a1 for the probe table base,
 *           while this body uses a2 and a0. The split stride adds into a1
 *           before shifting it, where the original adds into v0 then shifts
 *           into a1. The remaining instructions match.
 */
void updateRecordTableFromProbeOffsetsAndStepFoundRecordOctantWorld03Area14813_801F4614(
    void) {
  u8* work;
  u8 i;
  s8* probe;
  s32 quadrant;
  s32 saved;
  s32 index;
  s32 off;
  u8 target;
  s32 mode;
  s16 octant;

  work = D_1F800044;
  saved = *(s32*)(work + 0x3C);
  quadrant = (work[0x08] & 6) * 4;
  probe = &D_801F616C[(work[0x09] >> 3) * 2];
  *(s32*)(work + 0x3C) = *(s32*)(work + 0x20);
  for (i = 0; i < 2; i++) {
    index = (quadrant + probe[i]) * 2;
    work = D_1F800044;
    target = (u8)func_801BE0C0(*(s32*)(work + 0x34) + (D_801F6180[index] << 16),
                               *(s32*)(work + 0x38) + (D_801F6181[index] << 16),
                               0);
    if (target == 0xFF) {
      continue;
    }
    if ((D_8014625A & 0x1400) != 0) {
      continue;
    }
    if (D_80143BB0 != 0) {
      continue;
    }
    off = (target << 2) + target;
    off <<= 6;
    mode = D_80145E98[off];
    octant = mode & 7;
    if ((mode & 1) == 0) {
      octant = (octant + 1) & 7;
    }
    if (D_80145E91[off] == 1) {
      D_80145E98[off] = octant;
      func_8015C214(target, 2);
    }
  }
  *(s32*)(D_1F800044 + 0x3C) = saved;
}
