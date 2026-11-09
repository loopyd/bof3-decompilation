#include "bof3/bof3.h"

/* Shared record-submit helper at 0x80196718: submits the record it is handed.
 * The exact target sibling
 * submitRecordStoreEffectByteBClearWorkByte9AndIncrementWorkByte2BmagicMagic
 * 06000_801F082C declares the same one-argument shape. */
void func_80196718(void* arg0);

/* Shared helper at 0x800AF814, invoked with the shared battle byte at
 * 0x80146384. No include/ header declares it and its role is not proven, so the
 * name stays address-anchored and the declaration keeps the single
 * zero-extended byte argument the original encodes (lbu $a0 before the jal). */
void func_800AF814(u8 arg0);

/* Shared helper at 0x801E5988, invoked with no arguments after the global work
 * byte is decremented. The exact target sibling
 * decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhenZeroBmagic
 * Magic06000_801F00D0 declares and calls it the same way. */
void func_801E5988(void);

/* Shared frame counter in main RAM, outside this overlay image. The exact
 * target sibling
 * incrementWorkByteBRecordCountersOnFrameParityThenAdvanceWorkByte2WhenCounter
 * Is8BmagicMagic06000_801F08B8 reads the same word and treats its low bit as a
 * per-frame phase. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays,
 * bound at 0x1F800044 by this target map. The original re-materializes the cell
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) once per access, so each of the four
 * groups below reads it again through this named cell instead of holding one
 * address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Byte views of the three counter bytes this handler counts down. The counters
 * sit at record offsets 0x2, 0x3 and 0x4 of the 12-byte records based at
 * 0x80145BD4, and the original reaches the second and third byte through their
 * own addresses (0x80145BD7 and the reviewed symbol 0x80145BD8). The
 * decrements stay byte arrays with a computed byte offset, not record fields:
 * the record-typed form of the same three statements reserves a frame this
 * function does not carry for that reason alone, while the byte arrays emit the
 * original lui $at / addu $at,$at,$v1 pair around each counter load and store.
 * The exact target sibling
 * incrementWorkByteBRecordCountersOnFrameParityThenAdvanceWorkByte2WhenCounter
 * Is8BmagicMagic06000_801F08B8 reaches the same table in the same form. */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* Record view of the same table, needed for the signed tail compare: the
 * original reloads the first counter with lb, so the compare keeps the sign of
 * the stored byte rather than the zero-extended lbu the decrements use. Bytes
 * 0x2-0x4 of the 12-byte record are the counters and the payload gives no role
 * for the leading two bytes or the trailing seven. */
typedef struct BmagicMagic06000CounterRecord {
  u8 unk_00[2];
  s8 counter_02;
  s8 counter_03;
  s8 counter_04;
  u8 pad_05[7];
} BmagicMagic06000CounterRecord;

extern BmagicMagic06000CounterRecord D_80145BD4[]; /* @source 0x80145BD4 @kind table */

/* Shared battle work pointer in main RAM, outside this overlay image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt,
 * and every byte touched here is byte-addressed, so the local declaration keeps
 * the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Battle record published at 0x8014638C, outside this payload. The exact target
 * sibling submitRecordStoreEffectByteBClearWorkByte9AndIncrementWorkByte2
 * BmagicMagic06000_801F082C loads the same word and hands the record to
 * func_80196718, and this body passes the loaded pointer on the same way. Its
 * role is not proven, so the name stays address-anchored. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Shared battle byte at 0x80146384: the zero-extended battle target byte the
 * shared helpers consume. It lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801F09E0
 * @behavior Index 2 (the third pointer) of the three-entry handler run at
 *           0x801F0BDC that the work-byte-2 dispatcher at 0x801F07E8 resolves,
 *           so the work object byte 0x2 holds 2 while this body runs. When the
 *           low bit of the shared frame counter at 0x80143E6C is set it counts
 *           down the three counter bytes of the 12-byte counter record selected
 *           by the work object byte 0xB: each group re-reads the scratchpad
 *           pointer cell at 0x1F800044 and that work byte into a byte offset
 *           (idx*12 as idx*3*4), then decrements the selected byte through the
 *           address temporary the original spells as lui $at,%hi / addu
 *           $at,$at,$v1 around an lbu and an sb. The groups therefore count
 *           down in pairs of frames rather than every frame. It then re-reads
 *           the cell and the work byte 0xB and, when the selected record byte
 *           at offset 0x2 is zero, stores 1 into the work object byte 0x2 -
 *           the constant is materialised in the delay slot of the zero test -
 *           which returns the work-byte-2 dispatcher to its first handler;
 *           that compare loads with lb, so it reads the counter through the
 *           signed record view while the decrements above use the unsigned byte
 *           views. Finally, when bit 0x80 is set in the byte at offset 0xB of
 *           the shared battle work pointer 0x801EB4E0 (lbu, andi 0x80, no
 *           equality test), it loads the battle record published at 0x8014638C
 *           and submits it through func_80196718, passes the shared battle byte
 *           at 0x80146384 to func_800AF814, then re-reads the shared battle work
 *           pointer, decrements that record byte at offset 0xB by one and
 *           invokes the shared helper func_801E5988, the store filling the call
 *           delay slot. The 0x18-byte frame keeps only the return address at
 *           0x10 because of those calls. Takes no arguments, returns nothing
 *           and reads or writes no other state.
 *
 *           Statement order, operand forms, offset spelling and declaration
 *           style follow the exact target mirror sibling
 *           incrementWorkByteBRecordCountersOnFrameParityThenAdvanceWorkByte2
 *           WhenCounterIs8BmagicMagic06000_801F08B8 for the parity block and
 *           the signed record compare, with this span's own operators and
 *           constants written from its bytes (addiu -1, the zero test, the
 *           stored 1), and the call tail follows the exact target sibling
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndInvokeHelperWhen
 *           ZeroBmagicMagic06000_801F00D0. The sibling measured at 0x801F08B8
 *           is 296 bytes against this span's 388, and the 92 extra bytes here
 *           are the record submit, the battle-byte forward and the flag-0x80
 *           guard, so no byte-identical twin exists and the clone lever was
 *           declined.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBRecordCountersOnFrameParityThenSetWorkByte2ToOneWhenCounterIsZeroSubmitPublishedRecordForwardBattleByteAndInvokeHelperOnFlag80BmagicMagic06000_801F09E0(void) {
  u32 offset;

  if (D_80143E6C & 1u) {
    offset = g_battle_work[0xB] * 12;
    D_80145BD6[offset]--;
    offset = g_battle_work[0xB] * 12;
    D_80145BD7[offset]--;
    offset = g_battle_work[0xB] * 12;
    D_80145BD8[offset]--;
  }
  if (D_80145BD4[g_battle_work[0xB]].counter_02 == 0) {
    g_battle_work[2] = 1;
  }
  if (D_801EB4E0[0xB] & 0x80) {
    func_80196718(D_8014638C);
    func_800AF814(D_80146384);
    D_801EB4E0[0xB] -= 1;
    func_801E5988();
  }
}
