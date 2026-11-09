#include "bof3/bof3.h"

/* Row of the twelve-byte step triplet table the zero arm indexes with the
 * work object's byte 0xB: the original forms the offset with three scale steps
 * (sll $v0,$v1,1 / addu $v0,$v0,$v1 / sll $v0,$v0,2), so one row is twelve
 * bytes of three signed words, and each of the three words is loaded through
 * its own lui/addu/lw group at the row's three word addresses. The rows carry
 * a zero x word and signed fixed-point y and z steps. */
typedef struct BmagicMagic00903StepTriplet {
  s32 x; /* 0x0 */
  s32 y; /* 0x4 */
  s32 z; /* 0x8 */
} BmagicMagic00903StepTriplet;

/* The step triplet table is the tail of the code-pointer run this overlay
 * stores at 0x801EF89C, which Splat places as the single `textbin` row
 * D_801EF89C: the run's first five words are in-image code pointers
 * 0x801EF120, 0x801EF6B0, 0x801EF198, 0x801EF2D4 and 0x801EF340, and the
 * triplets begin at 0x801EF8B0, twenty bytes into that row. The indexed
 * address is declared here and the run stays the single D_801EF89C row in this
 * target's map, as the exact sibling
 * dispatchWorkByte2HandlerThenDrawWhenWorkBytes0And2SetBmagicMagic00903_801EF120
 * declares the run's 0x801EF8A4 word; this span's original bytes keep the
 * generated undefined_syms_auto.txt rows for 0x801EF8B0, 0x801EF8B4 and
 * 0x801EF8B8. */
extern BmagicMagic00903StepTriplet D_801EF8B0[]; /* @source 0x801EF8B0 @kind table */

/* Battle record pointer published at 0x8014638C, outside this payload
 * (0x801EEC00-0x801F0BE8) and shared with the other bmagic overlays: the
 * overlay only loads it, then reads its byte 0x8 and its words 0x34, 0x38 and
 * 0x3C. Its role is not proven, so the name stays address-anchored and Splat
 * binds the address through the generated undefined_syms_auto.txt row, as the
 * exact sibling
 * submitRecordStoreEffectByteBClearWorkByte9AndIncrementWorkByte2BmagicMagic06000_801F082C
 * reads the same cell. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared main-exe helper at 0x801DD350, called here with no arguments set up
 * by the caller (jal, no $a0-$a3 load in this span) and returning nothing, so
 * the prototype states the width this call site observes. The overlay only
 * calls it, and the target's generated undefined-funcs map already spells the
 * address with this raw name. */
extern void func_801DD350(void); /* @source 0x801DD350 */

/* Shared main-exe helper at 0x800AF764, called with no arguments by this span
 * and by the exact bmagic siblings, which declare it the same way. */
extern void func_800AF764(void); /* @source 0x800AF764 */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. The original re-materialises the cell with its own lui/lw pair for every
 * access group because the byte and word stores through the loaded pointer may
 * republish it: the byte-8 store and the two tail byte accesses read the named
 * cell inline, while the motion group and the position group each keep their
 * own base local, which is also the register lever the allocator needs (see the
 * behavior note). The record pointer stays in a saved register across the calls
 * because the calls cannot change it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF198
 * @behavior Handler of the work-byte-2 sequence: reads the scratchpad work
 *   object through the pointer cell at 0x1F800044 and decrements its unsigned
 *   countdown byte at offset 0x9 with a byte-width read-modify-write
 *   (lbu $v0,0x9 / addiu $v0,$v0,-1 / sb), testing the stored value against
 *   zero (andi $v0,$v0,0xFF / bnez, empty delay slot). Only when that countdown
 *   reaches zero does the arm run. It loads the battle record pointer published
 *   at 0x8014638C once into the saved register $s0, copies that record's byte
 *   0x8 into the work object's byte 0x8 through a fresh cell read, then takes
 *   one work base from a second cell read and stores the step triplet row the
 *   work object's byte 0xB selects: word 0xC gets the row's x word and word
 *   0x10 its y word, each with the index recomputed from the same work base
 *   because the preceding store may alias it (lbu $v1,0xB / three scale steps /
 *   lui $at,%hi(D_801EF8B0) or %hi(D_801EF8B4) / addu $at,$at,$v0 / lw / sw).
 *   The shared helper func_801DD350 is then called with no arguments and the
 *   word 0x10 store is scheduled into its delay slot. After the call the cell
 *   is re-read into a second base local, because the call may republish it, and
 *   the record's words 0x34 and 0x38 are added into the work object's own words
 *   0x34 and 0x38 (record word first, work word second), while word 0x3C is set
 *   to the record's word 0x3C plus the selected row's z word. The two base
 *   locals are the lever the allocator needs: the original loads the cell into
 *   $a0 for both groups, and one local reused across the call measured 63 of 79
 *   instructions with the whole body in place but the work base in $a1 and the
 *   post-call index byte in $a0, so each group keeps its own base local. The
 *   shared helper func_800AF764 is called next with the word 0x3C store in its
 *   delay slot, and the arm ends by clearing work byte 0xA through its own cell
 *   read and
 *   advancing work byte 0x2 with a byte-width read-modify-write through a last
 *   cell read. The 0x18-byte frame only keeps $ra at 0x14 and the record
 *   pointer at 0x10 across the two calls; the function takes no arguments,
 *   returns nothing and reads or writes no other state. The word at 0x801EF8A4,
 *   the third word of the code-pointer run at 0x801EF89C, holds this address,
 *   so the work-byte-2 dispatcher of this overlay reaches it as entry 0 of its
 *   handler table. The decrement and advance shape follows this target's exact
 *   sibling decrementWorkByte9ThenIncrementWorkByte1WhenZeroBmagicMagic00903_801EF01C
 *   and the record copy and position accumulation follow the exact sibling
 *   seedWorkMotionFromBattlerClassThenSetHeadingToward8014638CPublishHalfwordPairAndAdvanceWorkByte2BmagicMagic04403_801EEEFC,
 *   which reads the same record words with the same operand order. No overlay
 *   binary in the corpus repeats this 0x13C-byte body, so the span is lifted
 *   rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenCopyRecordByte8SeedWorkMotionPairAddRecordPositionClearByteAAndAdvanceWorkByte2BmagicMagic00903_801EF198(
    void) {
  u8 *source;
  u8 *work;
  u8 *position;

  if (--g_battle_work[9] == 0) {
    source = D_8014638C;
    g_battle_work[8] = source[8];
    work = g_battle_work;
    *(u32 *)(work + 0xC) = D_801EF8B0[work[0xB]].x;
    *(u32 *)(work + 0x10) = D_801EF8B0[work[0xB]].y;
    func_801DD350();
    position = g_battle_work;
    *(u32 *)(position + 0x34) =
        *(u32 *)(source + 0x34) + *(u32 *)(position + 0xC);
    *(u32 *)(position + 0x38) =
        *(u32 *)(source + 0x38) + *(u32 *)(position + 0x10);
    *(u32 *)(position + 0x3C) =
        *(u32 *)(source + 0x3C) + D_801EF8B0[position[0xB]].z;
    func_800AF764();
    g_battle_work[0xA] = 0;
    g_battle_work[2]++;
  }
}
