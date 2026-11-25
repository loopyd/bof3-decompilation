#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The entry reads the cell
 * once (lui $v1,0x1F80 / lw $v1,0x44($v1), load-delay nop) for the countdown,
 * and the zero branch re-reads it at four separate sites: once into $a0 for the
 * whole reset group, then three more times for the byte 0xB, byte 0x9 and byte
 * 0x1 stores, each read sitting behind a store issued through the pointer
 * loaded before it. Each site is written through the named cell so the
 * original reload points survive. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle record pointer at 0x801EB4E0, outside this overlay image and
 * already bound by the generated undefined_syms_auto.txt. The word stored there
 * holds the record whose 0x34, 0x38 and 0x3C words this body copies into the
 * work object; the cell is read once, before the work pointer, because the copy
 * group needs both bases. Other bmagic lifts bind the same main-executable cell
 * with this same address-anchored spelling. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EEFF4
 * @behavior Counts down the work object's unsigned byte 0x9 (lbu /
 *           addiu $v0,$v0,-1 / sb / andi 0xFF) and, only when the stored value
 *           is zero (bnez with an empty delay slot), runs the reset block.
 *           The reset block loads the shared battle record pointer at
 *           0x801EB4E0 into $v1, re-reads the work-object pointer cell into
 *           $a0, then copies the record's words 0x34, 0x38 and 0x3C into the
 *           work object's matching words (lw / sw pairs; the 0x3C word is held
 *           in a register while the byte stores run), stores 8 to work word
 *           0x20, 2 to work byte 0xA and 0 to work word 0x14. It then re-reads
 *           the cell three more times through separate bases and stores 0x10
 *           to work byte 0xB, 0x20 to work byte 0x9 and increments work byte
 *           0x1 by one (lbu / addiu $v0,$v0,1 / sb). Takes no arguments,
 *           returns nothing, keeps no frame and touches no other state.
 *
 *           The payload word at 0x801EFCD4 holds this address: it is the first
 *           entry of the three-pointer run at 0x801EFCD4 (0x801EEFF4,
 *           0x801EF0A4, 0x801EF108) that closes this payload. No jal in this
 *           payload reaches it, which is the dispatch shape rather than a
 *           missing caller, so the parameter roles stay structural and no
 *           prototype is owed. A scan of every payload under
 *           out/binaries/emi/bmagic found no byte-identical or near-identical
 *           copy of this 0xB0-byte span, so no clone is eligible; the entry and
 *           the shared-countdown shape follow this target's own exact sibling
 *           decrementWorkIncrementZero,
 *           and the record-triplet copy group follows the exact
 *           copyRecordTripletMinus4000Plus4000000SetBytes9BTo10AndAToB0ThenAdvanceWorkByte2BmagicMagic07903_801EEE64
 *           spelling (named word/byte bases with separate cell re-reads).
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCopyTripletReset(void) {
  u8 *source;
  u8 *work;
  u8 *next;
  u8 *step;
  u8 *tail;
  u32 value;

  if (--g_battle_work[9] == 0) {
    source = D_801EB4E0;
    work = g_battle_work;
    *(u32 *)(work + 0x34) = *(u32 *)(source + 0x34);
    *(u32 *)(work + 0x38) = *(u32 *)(source + 0x38);
    value = *(u32 *)(source + 0x3C);
    *(u32 *)(work + 0x20) = 8;
    work[0xA] = 2;
    *(u32 *)(work + 0x3C) = value;
    next = g_battle_work;
    *(u32 *)(work + 0x14) = 0;
    next[0xB] = 0x10;
    step = g_battle_work;
    step[9] = 0x20;
    tail = g_battle_work;
    tail[1]++;
  }
}
