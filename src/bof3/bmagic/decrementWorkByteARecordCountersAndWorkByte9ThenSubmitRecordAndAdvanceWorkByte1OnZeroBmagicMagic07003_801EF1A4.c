#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the record it is handed.
 * include/bof3/battle/battle03_internal.h and
 * include/bof3/battle/battle15_internal.h declare
 * `void func_80196718(void* arg0)`, and the exact sibling
 * decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAndAdvanceWork
 * Byte1OnZeroBmagicMagic01000_801EEE20 calls the same address with the same
 * one-argument shape. */
void func_80196718(void* arg0);

/* Shared helper at 0x800AF814, invoked with the shared battle byte at
 * 0x80146384 that the same exact sibling reads. No include/ header declares it
 * and its role is not proven, so the name stays address-anchored and the
 * declaration keeps the single zero-extended byte argument the original
 * encodes (lbu $a0 before the jal). */
void func_800AF814(u8 arg0);

/*
 * Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The cell is read once per access group, so the original re-materializes its
 * address (lui $vN,0x1F80 / lw $vN,0x44($vN)) for each of the five groups. The
 * exact sibling lifted source of this target,
 * setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow5BmagicMagic07003_
 * 801EF15C, reaches the same cell through the same address-encoded name.
 */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/*
 * Counter table selected by the work object's byte 0xA: 12-byte records whose
 * leading bytes are the counters this handler completes. The original expresses
 * the three byte-width decrements as field accesses of the selected record, so
 * the record base carries the 12-byte scale and the +0/+1/+2 field offsets fold
 * into the relocation (the third counter's relocation therefore reads
 * 0x80145BD8). This is the same table and the same access form as the exact
 * sibling decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAndAdvanc
 * eWorkByte1OnZeroBmagicMagic01000_801EEE20, which decrements the same three
 * bytes of the same records under the byte 0xB selector.
 */
typedef struct BmagicMagic07003CounterRecord {
  u8 counter_00;
  u8 counter_01;
  u8 counter_02;
  u8 pad_03[9];
} BmagicMagic07003CounterRecord;

extern BmagicMagic07003CounterRecord D_80145BD6[]; /* @source 0x80145BD6 @kind table */

/*
 * Battle record published at 0x8014638C: the loaded word is handed to
 * func_80196718 unchanged. Its role is not proven, so the name stays
 * address-anchored.
 */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte that
 * func_800AF814 consumes. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EF1A4
 * @behavior Per-frame handler of this overlay's handler run: it reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once per access
 *           group and decrements, byte by byte, the counter bytes 0x0/0x1/0x2 of
 *           the 12-byte record that the work object's unsigned byte 0xA selects
 *           in the 0x80145BD6 table: every access re-reads the work object (lbu
 *           $v0,0xA($v0)), scales the byte by the 12-byte stride into a byte
 *           offset and performs a byte-width read-modify-write (lbu, addiu
 *           $v0,$v0,-1, sb). It then re-reads the cell and decrements the work
 *           object's countdown byte 9 the same way, testing the stored value
 *           against zero (andi $v0,$v0,0xFF, bnez). Only when the countdown
 *           reaches zero does it re-read the cell once more and: submit the
 *           battle record published at 0x8014638C through func_80196718, pass
 *           the shared battle byte at 0x80146384 to func_800AF814, and
 *           increment the work object's state byte 1, the entry stage index the
 *           dispatcher at 0x801EEC28 advances. Each group re-reads the cell
 *           because a store through the loaded work pointer may alias it, which
 *           is why the original materialises the lui/lw pair five times instead
 *           of holding one base. The 0x28-byte frame holds $ra at 0x20 and the
 *           outgoing argument area; there are no arguments and no return value,
 *           and no other state is read or written. The 0x138-byte span
 *           0x801EF1A4..0x801EF2DB is byte-identical to the unlifted 0x801EF1A4
 *           of emi/bmagic/magic071/03 and magic072/03 and to the unlifted
 *           0x801EF1A0 of emi/bmagic/magic162/03 and magic168/03, so no lifted
 *           source exists to clone; the nearest exact sibling is
 *           decrementWorkByteBRecordCountersAndWorkByte9ThenSubmitRecordAnd
 *           AdvanceWorkByte1OnZeroBmagicMagic01000_801EEE20, whose body differs
 *           only in that record selector byte (0xB there, 0xA here) and whose
 *           statement order, operand forms and declaration style this body
 *           follows.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteARecordCountersAndWorkByte9ThenSubmitRecordAndAdvanceWorkByte1OnZeroBmagicMagic07003_801EF1A4(void) {
  D_80145BD6[D_1F800044[0xA]].counter_00--;
  D_80145BD6[D_1F800044[0xA]].counter_01--;
  D_80145BD6[D_1F800044[0xA]].counter_02--;
  if (--D_1F800044[9] == 0) {
    func_80196718(D_8014638C);
    func_800AF814(D_80146384);
    D_1F800044[1]++;
  }
}
