#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * with its own `lui 0x1F80` / folded `lw 0x44($vN)` pair for every access group
 * after the opening one, so each group below reads the cell symbol again. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Global work record published by the pointer cell at 0x80146884. This body
 * stores the submitted-effect byte at +0x93 and consumes two units of the
 * countdown halfword at +0x7E through one shared base. */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */

/* Shared EXE helper at 0x8019651C, outside this overlay payload. It takes the
 * work record plus three zero offsets and the mode word 1 and returns the
 * submitted effect byte this body stores. */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4); /* @source 0x8019651C */

/* @source 0x801F2C48
 * @behavior Entry 0 of this overlay two-entry work-byte-4 handler table at
 * 0x801F37F8, the entry the byte-4 dispatch step at 0x801F2C04 calls. It arms
 * the work record published through the scratchpad pointer cell 0x1F800044:
 * submits the record through the shared helper 0x8019651C with three zero
 * offsets and mode 1, stores the returned effect byte at +0x93 of the record
 * published by the pointer cell 0x80146884, raises flag bit 0x20 of record byte
 * 0, writes -0x40 into the signed level triplet at +0x5F, +0x5E and +0x5D in
 * that order through one load of the cell, writes 1 into marker byte +0x5C,
 * writes 2 into record byte +0x48, writes 1 into the handler byte at offset
 * +0x04 that the local dispatcher indexes its table with, and finally consumes
 * two units of the 0x80146884 countdown halfword at +0x7E. Takes no arguments
 * and returns nothing; the 0x20-byte frame keeps $ra at 0x18 and the fifth
 * 0x8019651C argument at 0x10.
 *
 * Name evidence: the local dispatcher passes record byte +4 to this table, so
 * writing 1 there selects entry 1 (0x801F2D0C), the handler that steps the same
 * level triplet down toward -0x80; the family role names of the -0x40 level
 * value and of the countdown consumption follow the exact siblings named below.
 * The roles of record byte +0x48 and of marker byte +0x5C are not established
 * for this overlay, so the name states the offset and the stored value instead
 * of inventing a role.
 *
 * Representation: one `u8*` local per level-triplet group and for the countdown
 * tail, because the original re-materialises the 0x1F800044 cell for every
 * access group; the byte fields keep their byte width and only the countdown
 * halfword at 0x7E is read and written as a halfword.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initWorkModeTint(
    void) {
  u8* work;
  u8* levels;

  D_80146884[0x93] = func_8019651C(D_1F800044, 0, 0, 0, 1);
  D_1F800044[0] |= 0x20;
  levels = D_1F800044;
  *(s8*)(levels + 0x5f) = -0x40;
  *(s8*)(levels + 0x5e) = -0x40;
  *(s8*)(levels + 0x5d) = -0x40;
  D_1F800044[0x5c] = 1;
  D_1F800044[0x48] = 2;
  D_1F800044[4] = 1;
  work = D_80146884;
  *(u16*)(work + 0x7e) -= 2;
}
