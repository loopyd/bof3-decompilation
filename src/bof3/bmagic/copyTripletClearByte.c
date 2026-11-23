#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay image.
 * This target's generated undefined_syms_auto.txt already binds the address
 * because the remaining assembly spans func_801EEC94, func_801EEEA8,
 * func_801EF070, func_801EF44C and func_801EF758 load it, so the name needs no
 * map row, and the exact in-target sibling decrementWorkByte9ThenSeedScratch
 * AngleFromWorkByteBStepPositionPairBySampledAngularPairTimesTwelveClearByte9
 * SetByteATo4AndAdvanceWorkByte2BmagicMagic03903_801EF584 declares the cell the
 * same way. Words 0x34, 0x38 and 0x3C read here are the position triplet that
 * exact sibling rewrites out of the same record: the record layout is shared
 * and the address is not this target's own. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map as g_battle_work, the name the exact lifts
 * workAdvanceAt80 and
 * add8ToWorkByte9AndDecrementWorkByteAThenDecrementGlobalWorkByteBAndInvoke
 * HelperWhenZeroBmagicMagic03903_801EF6E0 declare the same way, and the cell
 * this body re-materialises with its own lui 0x1F80 / lw 68($vN) pair at each
 * access group, because a store through the loaded pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF070
 * @behavior Per-frame step handler of this overlay's work byte 2 dispatcher
 *           group: its address is the word at 0x801EEC10, entry index 3 of the
 *           twelve-word code-pointer run 0x801EEC04..0x801EEC34 and entry 0 of
 *           the six-entry run 0x801EEC10 that
 *           dispatchWorkByte2HandlerThenInvokeDrawHelpersAndPopWhenActive
 *           BmagicMagic03903_801EEFA8 copies into its frame and indexes with
 *           work-object byte offset 0x2. Leaf with no frame. It loads the
 *           shared battle work record pointer cell 0x801EB4E0 into $a0 first,
 *           then the scratchpad work-object pointer cell 0x1F800044 into $v1,
 *           and copies the record words 0x34, 0x38 and 0x3C into the same
 *           work-object word offsets as three lw/nop/sw groups (the two nops
 *           are the load-delay slots). It clears work-object byte 9 with
 *           sb $zero, which the original places after the third load and before
 *           the store of the copied word 0x3C, materialises 1 with
 *           addiu $v0,$zero,0x1 and writes it to work-object byte 0xA through a
 *           fresh read of the pointer cell, then re-reads the cell once more and
 *           advances the dispatch state byte 0x2 with a byte-width
 *           read-modify-write (lbu / addiu 1 / sb), the sb filling the return
 *           delay slot. Takes no arguments, returns nothing and reads or writes
 *           no other state. The statement order, operand forms, s32 pointer
 *           locals and declaration style follow the exact sibling
 *           clearWorkFlag40AndCopyRecordTripletSetBytes9ATo1ThenAdvanceWorkByte2
 *           BmagicMagic09203_801EF8C0, whose body is this one preceded by a
 *           work-byte-0 0x40 flag clear; that sibling's own payload at
 *           0x801EF8C0 places its byte-9 store after lw 0x3C in the same slot,
 *           so the ordering is transcribed from measured bytes rather than
 *           guessed. The 100-byte span has no byte-identical twin anywhere under
 *           out/binaries. The 0x801EB4E0 cell is bound by this target's
 *           generated undefined_syms_auto.txt and the 0x1F800044 cell by its
 *           map, so this body needs no relocation of its own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyTripletClearByte(
    void) {
  s32 *src;
  s32 *dst;

  src = (s32 *)D_801EB4E0;
  dst = (s32 *)g_battle_work;
  dst[13] = src[13];
  dst[14] = src[14];
  dst[15] = src[15];
  ((u8 *)dst)[9] = 0;
  g_battle_work[0xA] = 1;
  g_battle_work[2]++;
}
