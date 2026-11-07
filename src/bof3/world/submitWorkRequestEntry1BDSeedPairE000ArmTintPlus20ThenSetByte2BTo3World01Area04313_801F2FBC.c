#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, the record the whole world
 * family seeds. The original re-materialises this cell for every store group and
 * for the call argument, so each access below is written through the cell
 * symbol rather than through one hoisted pointer. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared work record published by the pointer cell at 0x80146884. This body
 * stores only its submitted-effect byte 0x93, through the pointer the cell
 * holds; the exposed ancestor of the same overlay family names that byte
 * "submitted effect byte" (func_801F3844, initializeWorkModeWithTintMinus64,
 * initializeWorkModeWithTintMinus128Consume2CountdownWorld02Area10814_801F4B58
 * all store a func_8019651C result there). */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */

void func_80196718(void* arg0); /* @source 0x80196718 */
void func_8014DD3C(s32 arg0);   /* @source 0x8014DD3C */
void func_8014D6B8(u32 flag);   /* @source 0x8014D6B8 */
extern s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4); /* @source 0x8019651C */

/* @source 0x801F2FBC
 * @behavior Boots the scratch work record published at the pointer cell
 * 0x1F800044 and leaves it in the state the next step reads: submits that
 * record through func_80196718, requests entry 0x1BD through func_8014DD3C and
 * entry 0 through func_8014D6B8, clears work byte 0x2A, stores 0xE000 in both
 * words of the 0x40/0x44 pair, submits the record's positional effect through
 * func_8019651C with the 0/0/0 offsets and mode 1 and stores the returned
 * submitted-effect byte at 0x93 of the work record published at 0x80146884,
 * writes 1 into work byte 0x5C, writes 0x14 into each of the signed channels
 * 0x5F, 0x5E and 0x5D in that order, and finally writes 3 into work byte 0x2B.
 * Takes no arguments and returns nothing. The 0x20-byte frame holds $ra at 0x18
 * and the fifth func_8019651C argument at 0x10.
 *
 * Shape evidence: the 0x5C/0x5D..0x5F group, the 0x93 submitted-effect store
 * and the 0x40/0x44 word pair each come from exact siblings of the same family.
 * func_801F3844 (world00/area032/13) stores the func_8019651C result at
 * 0x80146884+0x93, writes 0x5C = 1 and then the three channels in the same
 * 0x5F/0x5E/0x5D order through one record local; func_801F4B58
 * (world02/area108/14) and initializeWorkModeWithTintMinus64 write 0x5C = 1
 * before the same descending channel triplet; func_801D6414 (world00/area030/04)
 * writes the 0x40/0x44 pair as two word stores off one base. This function's own
 * literals differ from all of them (0xE000 in the pair, +0x14 in the channels,
 * 3 in byte 0x2B), so those are written from the original bytes.
 *
 * Every re-read of the 0x1F800044 cell is kept: the original materialises the
 * cell once for the func_80196718 argument, once for the 0x2A byte store, once
 * for the 0x40/0x44 pair (the same $a0 it then passes to func_8019651C), once
 * for the 0x5C byte, once for the channel triplet and once for byte 0x2B - six
 * `lui 0x1F80` / folded `lw 0x44($vN)` pairs, because the stores through the
 * loaded record may alias the cell.
 *
 * Measured in this lane: the channel triplet needs its own signed-channel
 * pointer local. Writing the three 0x5F/0x5E/0x5D stores off the unsigned work
 * pointer left the group's base in $a0 and measured 40/45 instructions at the
 * same byte size, the original keeping $v1 there; the signed local reproduces
 * the original's $v1 base and the group's one cell load. Shapes measured and
 * rejected: one local covering byte 0x5C and the triplet (37/45, 172 bytes - the
 * original re-reads the cell, so a shared local drops a load), the triplet
 * written straight through the cell expression (39/51, 204 bytes - the
 * compiler emits one cell load per store), one local covering the whole tail
 * through byte 0x2B (34/45, 164 bytes - no reload at all), the same with three
 * reassignments (34/45), a per-group local set for byte 0x5C (37/45) and three
 * distinct per-group locals (42/45).
 *
 * Name evidence: submission through func_80196718 ("submits the record it is
 * handed"), entry requests through func_8014DD3C ("requests resource N through
 * the shared func_8014DD3C helper") and func_8014D6B8 ("requests resource N
 * through func_8014D6B8") are the wording of the exact siblings named above and
 * of resetWorkRecordAndRequestEntryWorld00Area03004_801E0124. The roles of work
 * byte 0x2A, of the 0x40/0x44 pair (bmagic names the same pair the motion pair,
 * world00/area030/04 calls them the step words), of byte 0x5C and of byte 0x2B
 * are not established for this overlay, so the name states their offsets and
 * the values the original stores instead of inventing roles; the +0x14 channel
 * value is the signed level triple the family ramps and is kept as a literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitWorkRequestEntry1BDSeedPairE000ArmTintPlus20ThenSetByte2BTo3World01Area04313_801F2FBC(
    void) {
  u8* work;
  s8* levels;

  func_80196718(D_1F800044);
  func_8014DD3C(0x1BD);
  func_8014D6B8(0);
  D_1F800044[0x2A] = 0;

  work = D_1F800044;
  *(u32*)(work + 0x40) = 0xE000;
  *(u32*)(work + 0x44) = 0xE000;
  D_80146884[0x93] = func_8019651C(work, 0, 0, 0, 1);

  D_1F800044[0x5C] = 1;
  levels = (s8*)D_1F800044;
  *(levels + 0x5F) = 0x14;
  *(levels + 0x5E) = 0x14;
  *(levels + 0x5D) = 0x14;
  D_1F800044[0x2B] = 3;
}
