#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds it at
 * 0x1F800044 as g_battle_work, the name its lifted sources
 * clearWorkByte9SetWorkByteATo88AndIncrementWorkByte2BmagicMagic13703_801EFCC0
 * and decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZero
 * BmagicMagic13703_801EFD9C already use. The original re-materialises the cell
 * before each access group because a store through the pointer it supplied may
 * alias the cell, so the body reads the named cell each time instead of holding
 * one copy. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Frame counter of the main executable, outside this overlay image; bit 0 is
 * the frame parity this body tests. Splat binds the address through this
 * target's generated undefined_syms_auto.txt, and the shared symbol map also
 * names it, so no target-map row is added. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Shared battle work record pointer in main RAM, outside this overlay image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added, and the access here
 * is byte-addressed, so the local declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared helper of the main executable, outside this overlay image; this site
 * leaves the work-object pointer it just stored through in the first argument
 * register, so the declaration takes that pointer. Splat binds the address
 * through this target's generated undefined_funcs_auto.txt. */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */

/* Shared helper of the main executable, outside this overlay image; the bmagic
 * call sites invoke it with no argument set and discard $v0, so it is declared
 * void with no arguments. */
extern void func_801E5988(void);

/* @source 0x801EF2C4
 * @behavior Steps the scratchpad work object one frame of a position advance.
 *           The pointer cell at 0x1F800044 is read first, before the frame is
 *           created, because the byte group below needs no stack. It reads work
 *           bytes 0xB and 9 through that one pointer, adds one plus work byte 9
 *           shifted right by six to byte 0xB and stores the sum at byte width.
 *           When bit 0 of the frame counter at 0x80143E6C is set it re-reads the
 *           cell and decrements work byte 0xA, again at byte width. It then
 *           re-reads the cell, stores 0x2000 into work word 0xC and zero into
 *           work word 0x10 and invokes the shared helper func_801DD350 with the
 *           work pointer, the zero store filling the call delay slot. After the
 *           call it re-reads the cell once and, through that single pointer,
 *           subtracts two from work byte 9 with a byte-width read-modify-write
 *           and adds the two step words into the position pair: word 0x34 gains
 *           word 0xC and word 0x38 gains word 0x10, each lw / addu / sw in
 *           place. The stored byte is compared truncated to eight bits (sb then
 *           andi $a1,$a1,0xFF / bne against 0x30), so the guarded tail runs only
 *           when that countdown lands on 0x30: it reads the shared battle work
 *           record pointer at 0x801EB4E0, decrements that record's byte 0xB and
 *           invokes the shared helper func_801E5988, the store filling the call
 *           delay slot. The 0x18-byte frame only keeps $ra at 0x10 across the
 *           two calls; the function takes no arguments, returns nothing and
 *           writes no other state.
 *
 *           The first 0x58 bytes match the body at emi/bmagic/magic067/03
 *           0x801EF4F4 byte for byte apart from this function's frame
 *           (addiu $sp,-0x18 / sw $ra,0x10($sp)), which that frameless sibling
 *           does not have: head, parity branch and byte-9 decrement are the same
 *           walk with this function's constant 0x30 written where the sibling
 *           carries 0x60. The sibling ends there by advancing work byte 2,
 *           while this body inserts the step-word reset, the func_801DD350 call
 *           and the position-pair accumulation and closes with the shared
 *           battle work tail, so the spans are shape evidence and not a
 *           byte-identical clone. A payload scan for the instruction triple
 *           addiu $v0,$zero,0x2000 / sw $v0,0xC($a0) / jal 0x801DD350 found only
 *           those two spans among the 396 extracted payloads. Statement order,
 *           the cell re-read points and the byte-9 test follow
 *           advanceWorkByteBDecrementByteAOnGlobalBit0AndByte9ByTwoThen
 *           AdvanceByte2At96BmagicMagic06703_801EF4F4 for the head and
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhen
 *           ZeroBmagicMagic13703_801EFD9C, an exact lift of this target, for the
 *           shared battle work tail. Each cell read takes its own pointer local,
 *           the spelling two exact siblings of the same run show under this
 *           compiler: the step-word group copies
 *           seedWorkWordCMinus2000ClearWord10ThenInvokeHelpersAndAdvanceWork
 *           Byte2BmagicMagic00303_801EF9AC, whose body stores the two words
 *           through that local and passes the same local to func_801DD350, so
 *           the call argument needs no move and the second store lands in the
 *           call delay slot; the countdown group copies the truncated byte idiom
 *           of advanceWorkByteAStepWorkPositionPairByAngleWord0CAddWord20To
 *           Word14AccumulateIntoHalfword3ECountdownWorkByte9AdvanceWorkByte2OnZ
 *           eroAndDrawSpriteMode1BmagicMagic01200_801EF3C4, where the byte count
 *           is cast to eight bits, stored and then compared, which is the
 *           addiu / sb / andi order this span shows. Roles of work bytes 0xA,
 *           0xB and 9, of the
 *           step and position words and of both shared helpers are not
 *           established here, so they stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByteBDecrementByteAOnGlobalBit0SetStepWordsThenStepPositionPairAndOnByte9At48DecrementGlobalWorkByteBAndInvokeHelperBmagicMagic13703_801EF2C4(void) {
  u8 *work;
  u8 *stepWork;
  u8 *tailWork;
  u32 next;
  u8 count;

  work = g_battle_work;
  next = work[0xB] + 1;
  work[0xB] = next + (work[9] >> 6);
  if (D_80143E6C & 1) {
    g_battle_work[0xA]--;
  }
  stepWork = g_battle_work;
  *(s32 *)(stepWork + 0xC) = 0x2000;
  *(s32 *)(stepWork + 0x10) = 0;
  func_801DD350(stepWork);
  tailWork = g_battle_work;
  count = (u8)(tailWork[9] - 2);
  tailWork[9] = count;
  *(s32 *)(tailWork + 0x34) += *(s32 *)(tailWork + 0xC);
  *(s32 *)(tailWork + 0x38) += *(s32 *)(tailWork + 0x10);
  if (count == 0x30) {
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
