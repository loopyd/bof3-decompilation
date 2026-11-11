#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, already bound at that address
 * by this target's map as g_battle_work, the name this target's exact lifts
 * reserveFirstFreeRecordBmagicMagic21903_801EF934,
 * clearWorkBytes0Through4BmagicMagic21903_801EF998 and
 * decrementWorkByteAAndCountDownByte9ThenDecrementRecordByteBAndClearAtZero
 * BmagicMagic21903_801EF5DC use. This body reads the cell once per access
 * group, because the byte store in the first group and the word stores later
 * may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* No-argument update helper of the shared main executable, outside this
 * payload. Its exact lifts in the plchar overlays call it with no arguments and
 * discard its answer; this target's generated undefined_funcs_auto.txt already
 * binds the address. */
extern void func_8014D978(void); /* @source 0x8014D978 */

/* Shared atan2-style heading helper of the main executable: it takes the
 * position pair in the order (x, y) and answers the direction word. The exact
 * bmagic sibling
 * drawBattlerRecordPointSnapPositionWhenSlotUnavailableAndOnRangeHitRaiseFlag
 * 2000ForwardSlotSetHeadingTowardBattleRecordAndAdvanceWorkByte2BmagicMagic
 * 01200_801EF188 declares the same signature and stores its answer into the
 * work object's direction word, as this body does. */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* Shared point emitter of the main executable: four projected components and a
 * size word, the fifth argument passed on the stack. The exact same-target
 * sibling drawBattlerRecordPointSnapPositionBmagicMagic01200_801EF188 owes this
 * lift the same four-word local with an unassigned fourth component. */
extern void func_800AF3A0(s32 x, s32 y, s32 z, s32 depth, s32 size); /* @source 0x800AF3A0 */

/* Shared work-range test, the exact lift src/bof3/battle/
 * testWorkRangeAgainstPoint.c at the same address: the pair is compared against
 * the size word and both scratchpad range origins, and the answer is 1 only
 * when the size reaches both bounds. */
extern u32 func_800AF720(u32 x, u32 y, u32 size); /* @source 0x800AF720 */

/* @source 0x801EF3CC
 * @behavior Step handler of this overlay's dispatch table, selected by call
 *           through a pointer word rather than by a jal: entry 2 of the
 *           five-word handler run based at 0x801EFA00 (payload word at
 *           0x801EFA08) holds this address, and the dispatcher at 0x801EF024
 *           indexes that run with work byte 0x01. The body first reads the
 *           scratchpad work object through the pointer cell at 0x1F800044 -
 *           the cell load is hoisted above its own frame setup - and, only
 *           while that record's unsigned byte 0x09 is nonzero, decrements the
 *           byte and calls the shared no-argument update helper at 0x8014D978
 *           with the store filling the call delay slot. It then reloads the
 *           cell once and projects the record's own position words into a
 *           four-word point on the frame: word 0x0C and word 0x10 shifted down
 *           nine bits and biased by -0x4000, and word 0x14 shifted down
 *           sixteen bits, biased by 0xC0 and shifted down one. The fourth
 *           component is never assigned, so its argument load reads the point
 *           slot past the three written words and passes dead stack content,
 *           exactly as the sibling handler at 0x801EF6A4 does. Through that
 *           same cell load it moves the record's heading word 0x18 into word
 *           0x1C, the previous-heading slot, and calls the shared atan2 helper
 *           at 0x8017A138 with the position pair 0x0C and 0x10 minus the
 *           record's own range origin words 0x34 and 0x38, which is the
 *           direction from that origin to the current position; it reloads the
 *           cell and stores the answer into word 0x18. It passes the point with
 *           size word 0x40 to the shared emitter at 0x800AF3A0.
 *           It then range-tests the raw position pair against the same work
 *           object through the shared helper at 0x800AF720 with the bound
 *           0x8000 and, on a hit, advances work byte 0x02, the selector of the
 *           next handler in this step group. When the pair is out of range it
 *           replaces word 0x1C with the magnitude of the heading change, the
 *           low twelve bits of the new word 0x18 subtracted from the low twelve
 *           bits of the previous word 0x1C, storing the difference before the
 *           sign test and negating it when negative, and advances work byte 0x02
 *           only when that magnitude reaches 0x601 and stays below 0xA00, an
 *           unsigned bound test on the stored difference. Both advances share
 *           one increment of work byte 0x02 and each path reads the cell again
 *           for its own pointer. Takes no arguments and returns nothing; the
 *           0x38-byte frame holds the four-word point at 0x18, the outgoing
 *           stack word at 0x10, $ra at 0x30 and the two saved registers that
 *           carry position words 0x0C and 0x10 across both calls, and the body
 *           returns through the jr $ra at 0x801EF528. The roles of work bytes
 *           0x01, 0x02 and 0x09, of the position and heading words, and of the
 *           two no-argument helpers remain unestablished beyond their sites, so
 *           they stay offset-anchored. No twin exists in any extracted payload
 *           under out/binaries: a byte scan of all 396 payloads found this
 *           0x164-byte span only at its own offset, and a repeat with every
 *           direct call and internal jump word masked found the same single
 *           self-match, so the body was transcribed from the original bytes
 *           rather than cloned.
 * @status partial
 * @match 84.27
 * @residual 75 of 89 instructions, 356 bytes at the original size, first
 *   difference at +0x44. The root cause of every remaining difference is the
 *   third point component: the original loads the record word 0x14 whole and
 *   shifts it (lw $v1,20($a2) / sra $v1,$v1,16 / addiu $v1,$v1,0xC0 /
 *   sra $v1,$v1,1), while the direct clean-C spelling here compiles to the
 *   narrow lh 22($a2) plus the same addiu and sra pair. That is one
 *   instruction short for the component and puts the word in $v0 instead of
 *   $v1, which cascades into two allocation-only differences: the post-call
 *   cell load and its store use $a2 where the original uses $v1, and the
 *   heading-magnitude group's pointer lands in $a2 where the original uses
 *   $a0. Everything else matches, including the countdown arm with its store
 *   in the call delay slot, the point[0]/point[1] projection, the heading
 *   call with its word 0x1C/0x18 save, the emitter call with the unassigned
 *   fourth component, the 0x8000 range test, the sign-fixed magnitude with its
 *   ~x + 1 negation, the shared 0x601..0xA00 bound gate and the single
 *   cross-jumped increment of work byte 0x02. Fourteen clean-C spellings of
 *   the third component were measured and all narrowed to lh or changed the
 *   instruction count: a direct pointer cast, an s32 local, an s32 pointer
 *   index, a u32 local with a signed cast, an explicit cast, an (s16) cast of
 *   the shifted word, a signed 16-bit bitfield at bit 16, a struct-typed
 *   member view, a union word view, a two-statement form, a (s64) shift, a
 *   signed division by 0x10000 (four instructions longer) and two forms that
 *   carried the word through the point slot between statements - so the RTL
 *   forwarding that removes that intermediate store runs before the narrowing
 *   decision. Blocking the narrow extraction needs
 *   compiler-source evidence for combine's narrow-memory path, and this
 *   toolchain ships binaries only (toolchains/gcc-2.7.2-psx has no combine.c),
 *   so that rung is unresolved rather than exhausted.
 */
void countDownWorkByte9DrawWorkPointShiftHeadingWordSetHeadingFromRangeOriginAndAdvanceWorkByte2OnRangeHitOrHeadingBandBmagicMagic21903_801EF3CC(
    void) {
  s32 point[4];
  u8 *work;
  s32 x;
  s32 y;
  s32 heading;
  u8 *next;

  if (g_battle_work[9] != 0) {
    g_battle_work[9]--;
    func_8014D978();
  }

  work = g_battle_work;
  x = *(s32 *)(work + 0xC);
  y = *(s32 *)(work + 0x10);
  point[0] = (x >> 9) - 0x4000;
  point[1] = (y >> 9) - 0x4000;
  point[2] = ((*(s32 *)(work + 0x14) >> 16) + 0xC0) >> 1;
  *(s32 *)(work + 0x1C) = *(s32 *)(work + 0x18);
  heading = func_8017A138(x - *(s32 *)(work + 0x34), y - *(s32 *)(work + 0x38));
  work = g_battle_work;
  *(s32 *)(work + 0x18) = heading;
  func_800AF3A0(point[0], point[1], point[2], point[3], 0x40);

  if (func_800AF720(x, y, 0x8000)) {
    g_battle_work[2]++;
  } else {
    work = g_battle_work;
    *(s32 *)(work + 0x1C) =
        (*(s32 *)(work + 0x1C) & 0xFFF) - (*(s32 *)(work + 0x18) & 0xFFF);
    if (*(s32 *)(work + 0x1C) < 0) {
      *(s32 *)(work + 0x1C) = ~*(s32 *)(work + 0x1C) + 1;
    }
    next = g_battle_work;
    if ((u32)(*(s32 *)(next + 0x1C) - 0x601) < 0x3FF) {
      next[2]++;
    }
  }
}
